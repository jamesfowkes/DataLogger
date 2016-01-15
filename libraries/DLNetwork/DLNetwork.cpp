/*
 * DLNetwork.cpp
 * 
 * Connect and communicate with network resources
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
 * Local Application Includes
 */

#include "DLUtility.h"

#include "DLSettings.Global.h"
#include "DLNetwork.h"
#include "DLNetwork.linkitone.h"

/*
 * Public Functions 
 */

NetworkInterface * Network_GetNetwork(NETWORK_INTERFACE interface)
{
    NetworkInterface * pInterface = NULL;
    switch(interface)
    {
    case NETWORK_INTERFACE_LINKITONE_WIFI:
        pInterface = new LinkItOneWiFi();
        break;
    case NETWORK_INTERFACE_LINKITONE_GPRS:
        char * pAPN = Settings_getString(GPRS_APN);
        char * pUser = Settings_getString(GPRS_USERNAME);
        char * pPwd = Settings_getString(GPRS_PASSWORD);
        pInterface = new LinkItOneGPRS(pAPN, pUser, pPwd);
        break;
    }
    return pInterface;
}
