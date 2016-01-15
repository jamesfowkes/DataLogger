/*
 * linkitonegprs.cpp
 * 
 * Connect and communicate over GPRS
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * Arduino/C++ Library Includes
 */

#include <Arduino.h>

/*
 * LinkIt One Includes
 */

#include <LGPRS.h>
#include <LGPRSClient.h>

#include "DLUtility.h"

#include "DLNetwork.h"
#include "DLNetwork.linkitone.h"


/*
 * Defines and Typedefs
 */

#define BYTES_PER_SEND 16
#define BYTES_PER_RECV 16

#define LOG_ERROR(...) {const char * __error_parts__[] = {__VA_ARGS__, NULL}; log_error(__error_parts__); }

/*
 * Private Variables
 */

/*
 * Public Functions 
 */

LinkItOneGPRS::LinkItOneGPRS(char * apn, char * username, char * password) : 
    m_error_accumulator(m_error, 256)
{
    m_pAPN = apn;
    m_pUser = username;
    m_pPwd = password;
    m_connected = false;
    m_client = new LGPRSClient();
    m_state = NETWORK_STATE_IDLE;
    m_request_next_to_send = NULL;
    m_response = NULL;
    m_on_req_complete = NULL;
}

LinkItOneGPRS::~LinkItOneGPRS() {}

const char * LinkItOneGPRS::last_error()
{
    return m_error;
}

bool LinkItOneGPRS::tryConnection(uint8_t timeoutSeconds)
{
    unsigned long timeout = millis() + (timeoutSeconds*1000);
    
    m_connected = false;
    while (!m_connected && (millis() < timeout))
    {
        m_connected = LGPRS.attachGPRS(m_pAPN, m_pUser, m_pPwd);
    }
    return m_connected;
}

bool LinkItOneGPRS::connect(char const * const url)
{
    bool success = m_connected;

    if (success)
    {
        success &= m_client->connect(url, HTTP_PORT);
    }
    
    return success;
}

void LinkItOneGPRS::log_error(const char * errors[])
{
    m_error_accumulator.reset();
    while(*errors)
    {
        m_error_accumulator.writeString(*errors++);
    }
}

void LinkItOneGPRS::sendHTTPRequest(const char * const url, const char * request, char * response, NETWORK_HTTP_REQ_COMPLETE on_complete_fn, bool useHTTPS)
{
    (void)useHTTPS; // Not currently supported with LinkItOne Arduino SDK

    m_last_result = false;

    if (m_state != NETWORK_STATE_IDLE)
    {
        LOG_ERROR("sendHTTPRequest: ERROR: Already connect to a client.");
        on_complete_fn(m_last_result);
    }

    if (connect(url))
    {
        m_on_req_complete = on_complete_fn;
        m_state = NETWORK_STATE_SENDING;
        m_request_next_to_send = request;
        m_response = response;
        m_last_result = true;
    }
    else
    {
        LOG_ERROR("sendHTTPRequest: Failed to connect to ", url);
        on_complete_fn(m_last_result);
    }
}

void LinkItOneGPRS::tick()
{
    switch(m_state)
    {
    case NETWORK_STATE_IDLE:
        break;
    case NETWORK_STATE_SENDING:
        if (send_next_bytes())
        {
            Serial.println("Network tick: Send complete. Getting response.");
            m_state = NETWORK_STATE_RECEIVING;
        }
        break;
    case NETWORK_STATE_RECEIVING:
        if (recv_bytes())
        {
            Serial.println("Network tick: Recv complete. Going idle.");
            LOG_ERROR("");
            m_state = NETWORK_STATE_IDLE;
            m_on_req_complete(m_last_result);
        }
        break;
    }
}

bool LinkItOneGPRS::recv_bytes()
{
    uint16_t count = 0;
    char next = 0;
            
    if (!m_response)
    {
        return true;
    }

    // if the server's disconnected, stop the m_client:
    if (m_client->connected())
    {
        if(m_client->available())
        {
            do
            {
                next = m_client->read();
                *m_response++ = next > -1 ?  next : '\0';
                count++;
            } while ((next > -1) && (count < BYTES_PER_RECV));
        }
        else
        {
            return true;
        }
    }
    else
    {
        LOG_ERROR("recv_bytes: Client disconnected.")
        m_client->stop();
        m_last_result = false;
        return true;
    }
    return next == -1;
}

void LinkItOneGPRS::readResponse(char * response){}
/*{
    // if there are incoming bytes available
    // from the server, read them and print them:
    uint16_t i = 0;
    
    if (m_connected && response)
    {

        // if the server's disconnected, stop the m_client:
        if (m_client->connected())
        {
            if(m_client->available())
            {
                char next;
                do
                {
                    next = m_client->read();
                    response[i++] = next > -1 ?  next : '\0';
                    
                } while (next > -1);
            }
            else
            {
                //Serial.println("Client not available");
            }
        }
        else
        {
            Serial.println("Disconnecting from client");
            m_client->stop();
        }
    }
}*/

bool LinkItOneGPRS::send_next_bytes()
{
    bool send_complete = false;

    char to_send[BYTES_PER_SEND+1];
    strncpy_safe(to_send, m_request_next_to_send, sizeof(to_send));
    m_client->print(to_send);
    
    if (strlen(m_request_next_to_send) == BYTES_PER_SEND)
    {
        m_request_next_to_send += BYTES_PER_SEND;
    }
    else
    {
        send_complete = true;
    }
    
    return send_complete;
}

bool LinkItOneGPRS::isConnected(void) { return m_connected; }
