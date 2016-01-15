#ifndef _NETWORK_IF_H_
#define _NETWORK_IF_H_

#define HTTP_PORT (80)

enum network_interface
{
    NETWORK_INTERFACE_LINKITONE_WIFI,
    NETWORK_INTERFACE_LINKITONE_GPRS
};
typedef enum network_interface NETWORK_INTERFACE;

typedef void (*NETWORK_HTTP_REQ_COMPLETE)(bool);

/*
 * NetworkInterface is a pure abstract class.
 * Each supported interface shall inherit from this base class
 */

class NetworkInterface
{
    public: 
        virtual bool tryConnection(uint8_t timeoutSeconds) = 0;
        virtual void sendHTTPRequest(const char * const url, const char * request, char * response, NETWORK_HTTP_REQ_COMPLETE on_complete_fn, bool useHTTPS=false) = 0;
        virtual bool isConnected(void) = 0;
        virtual void tick() = 0;
        virtual const char * last_error() = 0;
};

NetworkInterface * Network_GetNetwork(NETWORK_INTERFACE interface);
        
#endif

