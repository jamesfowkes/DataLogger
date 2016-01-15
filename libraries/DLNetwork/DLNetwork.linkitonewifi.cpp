/*
 * linkitonewifi.cpp
 * 
 * Connect and communicate over Wifi
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
 * Private Variables
 */

/*
 * Public Functions 
 */

LinkItOneWiFi::LinkItOneWiFi() {}

LinkItOneWiFi::~LinkItOneWiFi() {}

bool LinkItOneWiFi::tryConnection(uint8_t timeoutSeconds)
{
	// WIFI FUNCTIONALITY NOT YET IMPLEMENTED
    (void)timeoutSeconds;
    return false;
}

void LinkItOneWiFi::sendHTTPRequest(const char * const url, const char * request, char * response, NETWORK_HTTP_REQ_COMPLETE on_complete_fn, bool useHTTPS)
{
	// WIFI FUNCTIONALITY NOT YET IMPLEMENTED
	(void)url;
	(void)request;
	(void)response;
	(void)useHTTPS;
	(void)on_complete_fn;
}

bool LinkItOneWiFi::isConnected(void) { return false; }
void LinkItOneWiFi::tick() {}
const char * LinkItOneWiFi::last_error() { return ""; }