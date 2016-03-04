/*
 * DLSettings.DataChannels.Helper.cpp
 * 
 * Provides private functionality for data channel setting parsing
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * Standard Library Includes
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef TEST
#include <iostream>
#endif

/*
 * Local Includes
 */

#include "DLDataField.Types.h"
#include "DLSettings.h"
#include "DLUtility.h"

/*
 * Private Variables
 */

static const char * s_channelTypes[] = {
    "voltage",
    "current",
    "temperature_c"
};

/*
 * Private Functions
 */

static int8_t getChannel(char const * const pSetting)
{
    long channel = -1;

    char * pStartOfConv = (char *)pSetting;
    char * pEndOfConv = pStartOfConv;
    channel = strtol(pStartOfConv, &pEndOfConv, 10);
    if (pStartOfConv == pEndOfConv) { return -1; }
    
    return (int8_t)channel;
}

/*
 * Public Functions
 */

int8_t Settings_getChannelFromSetting(char const * const setting)
{
    char lcaseSetting[31];
    int8_t channel = -1;

    if (!setting) { return -1; }

    uint32_t length = strlen(setting);
    if (length > 30) { return -1; }

    strncpy_safe(lcaseSetting, setting, 31);
    toLowerStr(lcaseSetting);

    if (0 == strncmp(lcaseSetting, "channel", 7))
    {
        channel = getChannel(lcaseSetting+7);
    }
    else if (0 == strncmp(lcaseSetting, "ch", 2))
    {
        channel = getChannel(lcaseSetting+2);      
    }

    if (channel < 1) { return -1; }

    // Channels in file are CH1, CH2, CH3..., which correspond to indexes 0, 1, 2..
    // i.e. file is one-indexed, code is zero-indexed.
    return channel - 1; 
}

bool Setting_getChannelSettingStr(char * buffer, char const * const setting)
{
    if (!setting) { return false; }
    if (!buffer) { return false; }

    // Find the separator dot and copy up to the first whitespace or end of string
    char const * pDot = strchr((char*)setting, '.');
    if (!pDot) { return false; }

    char const * pToCopy = pDot+1;
    while (*pToCopy && !isspace(*pToCopy))
    {
        *buffer++ = tolower(*pToCopy++);
    }

    *buffer = '\0';

    return true;
}

FIELD_TYPE Setting_parseSettingAsType(char const * const setting)
{
    uint8_t i;
    char lcaseSetting[31];
    strncpy_safe(lcaseSetting, setting, 31);
    toLowerStr(lcaseSetting);

    for (i = 0; i < N_ELE(s_channelTypes); ++i)
    {
        if (0 == strncmp(lcaseSetting, s_channelTypes[i], strlen(s_channelTypes[i])))
        {
            return (FIELD_TYPE)i;
        }
    }

    return INVALID_TYPE;
}

bool Setting_parseSettingAsInt(int32_t * pResult, char const * const setting)
{

    char * pStartOfConv = (char *)setting;
    char * pEndOfConv = pStartOfConv;
    *pResult = strtol(pStartOfConv, &pEndOfConv, 10);
    return (pStartOfConv != pEndOfConv);
}

bool Setting_parseSettingAsFloat(float * pResult, char const * const setting)
{
    char * pStartOfConv = (char *)setting;
    char * pEndOfConv = pStartOfConv;
    *pResult = strtod(pStartOfConv, &pEndOfConv);
    return (pStartOfConv != pEndOfConv);
}
