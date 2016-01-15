#ifndef _THINGSPEAK_H_
#define _THINGSPEAK_H_

/*
 * Typedefs
 */

enum connection_type
{
    LINKIT_ONE_CONN_TYPE_WIFI,
    LINKIT_ONE_CONN_TYPE_GPRS
};
typedef enum connection_type CONNECTION_TYPE;

enum network_state_enum
{
    NETWORK_STATE_IDLE,
    NETWORK_STATE_SENDING,
    NETWORK_STATE_RECEIVING
};
typedef enum network_state_enum NETWORK_STATE_ENUM;

/*
 * Forward class declarations
 */

class LGPRSClient;

class LinkItOneWiFi : public NetworkInterface
{
    public:
        LinkItOneWiFi();
        ~LinkItOneWiFi();
        bool tryConnection(uint8_t timeoutSeconds);
        void sendHTTPRequest(const char * const url, const char * request, char * response, NETWORK_HTTP_REQ_COMPLETE on_complete_fn, bool useHTTPS);
        bool isConnected(void);
        void tick();
        const char * last_error();
    private:
        bool m_connected;
};

class LinkItOneGPRS : public NetworkInterface
{
    public:
        LinkItOneGPRS(char * apn, char * username, char * password);
        ~LinkItOneGPRS();
        bool tryConnection(uint8_t timeoutSeconds);
        void sendHTTPRequest(char const * const url, const char * request, char * response, NETWORK_HTTP_REQ_COMPLETE on_complete_fn, bool useHTTPS=false);
        bool isConnected(void);
        void tick();
        const char * last_error();

    private:
        char * m_pAPN;
        char * m_pUser;
        char * m_pPwd;
        bool m_connected;
        LGPRSClient * m_client;
        NETWORK_HTTP_REQ_COMPLETE m_on_req_complete;
        bool m_last_result;
        void readResponse(char *);
        bool connect(char const * const url);

        bool send_next_bytes();
        bool recv_bytes();

        void log_error(const char * errors[]);

        NETWORK_STATE_ENUM m_state;
        const char * m_request_next_to_send;
        char * m_response;
        
        FixedLengthAccumulator m_error_accumulator;
        char m_error[256];

};

#endif

