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

#include <LTask.h>
#include <LGPRS.h>
#include <LGPRSClient.h>

#include "DLNetwork.h"
#include "DLNetwork.linkitone.h"

/*
 * Private Variables
 */

static int s_failed_connection_count = 0;

/*
 * Private Functions
 */

boolean CallReset(void * userdata) 
{ 
    vm_reboot_normal_start(); 
    return true; 
}

 /*
 * Public Functions 
 */

LinkItOneGPRS::LinkItOneGPRS(char * apn, char * username, char * password)
{
    m_pAPN = apn;
    m_pUser = username;
    m_pPwd = password;
    m_connected = false;
    m_client = NULL;
}

LinkItOneGPRS::~LinkItOneGPRS() {}

bool LinkItOneGPRS::tryConnection(uint8_t timeoutSeconds)
{
    unsigned long timeout = millis() + (timeoutSeconds*1000);
    
    m_connected = false;
    while (!m_connected && (millis() < timeout))
    {
        m_connected = LGPRS.attachGPRS(m_pAPN, m_pUser, m_pPwd);
        if(!m_client) { m_client = new LGPRSClient(); }
    }
    return m_connected;
}

bool LinkItOneGPRS::connect(char const * const url)
{
    bool success = m_connected && (m_client != NULL);

    if (success)
    {
        Serial.print("LinkItOneGPRS::connect: Have GPRS. Trying to connect to ");
        Serial.print(url);
        Serial.print("...");
        success &= m_client->connect(url, HTTP_PORT);
        Serial.println(success ? " connected." : " failed.");
    }
    else
    {
        if (!m_connected) { Serial.println("LinkItOneGPRS::connect: No GRPS connection!"); }
        if (!m_client)  { Serial.println("LinkItOneGPRS::connect: No LGPRSClient!"); }

        s_failed_connection_count++;
        if (s_failed_connection_count == 6)
        {
            Serial.println("LinkItOneGPRS::connect: Restarting due to failed connection limit!");
            LTask.remoteCall(CallReset, NULL); 
        }
    }
    
    return success;
}

bool LinkItOneGPRS::sendHTTPRequest(const char * const url, const char * request, char * response, bool useHTTPS)
{
    (void)useHTTPS; // Not currently supported with LinkItOne Arduino SDK

    bool success = connect(url);

    if (success)
    {
        Serial.println("LinkItOneGPRS::sendHTTPRequest: sending");
        m_client->print(request);
        Serial.println("LinkItOneGPRS::sendHTTPRequest: reading response");
        readResponse(response);
    }
    else
    {
        Serial.print("LinkItOneGPRS::sendHTTPRequest: Failed to connect to ");
        Serial.println(url);
    }
    return success;
}

void LinkItOneGPRS::readResponse(char * response)
{
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

            Serial.println("LinkItOneGPRS::readResponse: Disconnecting from client");
            m_client->stop();
        }
        
    }
}

bool LinkItOneGPRS::isConnected(void) { return m_connected; }
