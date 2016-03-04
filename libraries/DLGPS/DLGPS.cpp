/*
 * DLGPS.cpp
 * 
 * GPS sentence parsing library
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * C++ Library Includes
 */

#include <stdint.h>
#include <string.h>
#include <time.h>

/*
 * Local Application Includes
 */

#include "DLUtility.Location.h"
#include "DLUtility.Time.h"
#include "DLGPS.h"

/*
 * Defines and typedefs
 */

// Converts a character char '0' through '9' to given numeric type
#define C_TO_NUM(type, c) ((type)(c - '0'))

/*
 * Private Function Definitions*/

/*
 * parseSixCharsToThreeInts
 *
 * Generic function for processing HHMMSS and DDMMYY stringstream
 */
uint8_t parseSixCharsToThreeInts(const char * pSentence, uint8_t * puint1, uint8_t * puint2, uint8_t * puint3)
{
    *puint1 = C_TO_NUM(uint8_t, pSentence[0]) * 10;    // Tens of first uint8_t
    *puint1 += C_TO_NUM(uint8_t, pSentence[1]);        // Units
    *puint2 = C_TO_NUM(uint8_t, pSentence[2]) * 10;    // Tens of second uint8_t
    *puint2 += C_TO_NUM(uint8_t, pSentence[3]);        // Units
    *puint3 = C_TO_NUM(uint8_t, pSentence[4]) * 10;    // Tens of third uint8_t
    *puint3 += C_TO_NUM(uint8_t, pSentence[5]);        // Units
    
    return 6; // 6 chars were parsed by this function
}

uint8_t parseHHMMSSTime(const char * pSentence, uint8_t * h, uint8_t * m, uint8_t * s)
{
    return parseSixCharsToThreeInts(pSentence, h, m, s);
}

uint8_t parseDDMMYYDate(const char * pSentence,  uint8_t * d, uint8_t * m, uint8_t * y)
{
    return parseSixCharsToThreeInts(pSentence, d, m, y);
}

uint8_t parseLatitude(const char * pSentence, float * pResult)
{
    uint8_t charsParsed = 0;
    // First two chars are degrees
    *pResult = C_TO_NUM(float, pSentence[0]) * 10;
    *pResult += C_TO_NUM(float, pSentence[1]);

    // Next two chars are tens and units of minutes
    *pResult += C_TO_NUM(float, pSentence[2]) * 10.0 / 60.0;
    *pResult += C_TO_NUM(float, pSentence[3]) / 60.0;
    
    charsParsed = 5; // So far there have been 4 chars and a decimal point
    // Remaining chars are decimal portion of minutes
    
    float divisor = 600; // Divide by 600 to convert 0.1 minutes into decimal degrees
    while (pSentence[charsParsed] != ',')
    {
        *pResult += (C_TO_NUM(float, pSentence[charsParsed++]) / divisor);
        divisor *= 10; // then division by 6000, 60000, 600000...
    }
    
    charsParsed++; // Skip over comma
    
    // Char after comma is N or S (S is negative)
    if (pSentence[charsParsed++] == 'S') {*pResult = -*pResult; }
    
    return charsParsed;
}

uint8_t parseLongitude(const char * pSentence, float * pResult)
{
    uint8_t charsParsed = 0;
    // First three chars are degrees
    *pResult = C_TO_NUM(float, pSentence[0]) * 100;
    *pResult += C_TO_NUM(float, pSentence[1]) * 10;
    *pResult += C_TO_NUM(float, pSentence[2]);

    // Next two chars are tens and units of minutes
    *pResult += C_TO_NUM(float, pSentence[3]) * 10.0 / 60.0;
    *pResult += C_TO_NUM(float, pSentence[4]) / 60.0;
    
    charsParsed = 6; // So far there have been 5 chars and a decimal point
    // Remaining chars are decimal portion of minutes
    
    float divisor = 600; // Divide by 600 to convert 0.1 minutes into decimal degrees
    while (pSentence[charsParsed] != ',')
    {
        *pResult += (C_TO_NUM(float, pSentence[charsParsed++]) / divisor);
        divisor *= 10; // then division by 6000, 60000, 600000...
    }
    
    charsParsed++; // Skip over comma
    
    // Char after comma is W or E (W is negative)
    if (pSentence[charsParsed++] == 'W') {*pResult = -*pResult; }

    return charsParsed;
}

static uint8_t skipToNextComma(const char * pSentence)
{
    uint8_t count = 0;
    while (*pSentence++ != ',') { count++; }
    return count+1;
}
    
static bool rmcSentenceIsValid(const char * pSentenceStart)
{
    // RMC sentences have a A=active or V=Void char at the third field
	pSentenceStart += skipToNextComma(pSentenceStart);
	pSentenceStart += skipToNextComma(pSentenceStart);
    return *pSentenceStart == 'A';
}

/*
 * Public Function Definitions
 */

bool GPS_parseGPRMCSentence(const char * pSentence, GPS_DATA * pData)
{
    /* Refer to http://www.gpsinformation.org/dale/nmea.htm#RMS
     * Sample Data: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
     * Where:
     *  RMC          Recommended Minimum sentence C
     *  123519       Fix taken at 12:35:19 UTC
     *  A            Status A=active or V=Void.
     *  4807.038,N   Latitude 48 deg 07.038' N
     *  01131.000,E  Longitude 11 deg 31.000' E
     *  022.4        Speed over the ground in knots
     *  084.4        Track angle in degrees True
     *  230394       Date - 23rd of March 1994
     *  003.1,W      Magnetic Variation
     *  6A          The checksum data, always begins with *
     */
     
    if (!pSentence || !pData) { return false; } // NULL pointer check on arguments
    
    bool success = true;
    
    success &= strncmp(pSentence, "$GPRMC,", 7) == 0;
    success &= rmcSentenceIsValid(pSentence);
    
    if (success)
    {
        pSentence += 7; // Now pointing at first data, which is time
        pSentence += parseHHMMSSTime(pSentence, &pData->hour, &pData->min, &pData->sec);
		pSentence += skipToNextComma(pSentence); // Skip over any decimals in the time
        pSentence += skipToNextComma(pSentence); // Skip over valid char and comma
        pSentence += parseLatitude(pSentence, &pData->latitude) + 1; // +1 skips over comma
        pSentence += parseLongitude(pSentence, &pData->longitude) + 1; // +1 skips over comma
        pSentence += skipToNextComma(pSentence); // Skip over speed
        pSentence += skipToNextComma(pSentence); // Skip over trackAngle
        pSentence += parseHHMMSSTime(pSentence, &pData->dd, &pData->mm, &pData->yy);
    }
    
    return success;
}

void GPS_GetGPSTime(GPS_DATA const * const pData, TM * pTime)
{
    if (!pTime) { return; }
    if (!pData) { return; }
    
    pTime->tm_sec = pData->sec;
    pTime->tm_min = pData->min;
    pTime->tm_hour = pData->hour;
    pTime->tm_mday = pData->dd;
    pTime->tm_mon = pData->mm;
    pTime->tm_year = GREGORIAN_TO_C_YEAR(pData->yy + 2000);
}