/*
 * DLCSV.cpp
 *
 * James Fowkes
 * 03 April 2015
 *
 * www.re-innovation.co.uk
 *
 * Utility routines for CSV file data
 */

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#endif

#include <time.h>

#include "DLUtility.h"

void CSV_writeTimestampToBuffer(TM * time, char * buffer)
{
    if (!buffer) { return; };
    
    // Format is YYYY-MM-DD HH:MM:SS +0000
    sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d +0000",
        C_TO_GREGORIAN_YEAR(time->tm_year), time->tm_mon, time->tm_mday,
        time->tm_hour, time->tm_min, time->tm_sec);

}
