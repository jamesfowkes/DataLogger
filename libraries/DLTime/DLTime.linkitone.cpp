/*
 * DLTime.linkitone.cpp
 * 
 * Abstraction for Linkit ONE time resources
 *
 * Author: James Fowkes
 *
 * www.re-innovation.co.uk
 */

/*
 * Arduino/C++ Library Includes
 */

#include <Arduino.h>
#include <time.h>

/*
 * Linkit ONE Includes
 */

#include <LDateTime.h>

/*
 * Local Application Includes
 */

#include "DLUtility.Time.h"
#include "DLTime.h"
#include "DLGPS.h"

/*
 * Local object and variables
 */
/* 
 * Time_GetTime
 *  Get time from platform.
 *  Returns TRUE if time pointer was altered.
 *  TRUE return does not imply accuracy of the returned time!
 * Args:
 *  tm: The time pointer to be filled
 *  type: Determines which time will be returned;
 */

bool Time_GetTime(TM * tm, TIME_TYPE type)
{
    if (!tm) { return false; }
    
    bool success = false;
    
    switch(type)
    {
    case TIME_PLATFORM:
        // Get LinkItONE SDK datetime and copy to TM*
        datetimeInfo dt;
        LDateTime.getTime(&dt);
        tm->tm_sec = dt.sec;
        tm->tm_min = dt.min;
        tm->tm_hour = dt.hour;
        tm->tm_mday = dt.day;
        tm->tm_mon = dt.mon;
        tm->tm_year = GREGORIAN_TO_C_YEAR(dt.year);
        tm->tm_wday = -1; // Don't calculate day in week
        tm->tm_yday =  -1; // or day in year
        tm->tm_isdst = false; // Assume that datetime is never with DST
        break;
    case TIME_GPS:
        GPS_UpdateNow();
        success = GPS_InfoIsValid();
        if (success)
        {
            GPS_GetGPSTime(GPS_GetInfo(), tm);
        }
        break;
    case TIME_NETWORK:
        // TODO: Get time through network resources
        break;
    }
    
    return success;
}

void Time_SetPlatformTime(TM * tm)
{
    datetimeInfo dt;
    dt.sec = tm->tm_sec;
    dt.min = tm->tm_min;
    dt.hour = tm->tm_hour;
    dt.day = tm->tm_mday;
    dt.mon = tm->tm_mon;
    dt.year = C_TO_GREGORIAN_YEAR(tm->tm_year);
    LDateTime.setTime(&dt);
}