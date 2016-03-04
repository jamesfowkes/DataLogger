/*
 * DLTime.cpp
 * 
 * Generic time functions
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
 * Local Application Includes
 */

#include "DLUtility.Time.h"
#include "DLTime.h"

void Time_PrintTime(TM * pTime, bool endwithCRLF)
{
    if (pTime->tm_hour < 10) { Serial.print("0"); }
    Serial.print(pTime->tm_hour);
    Serial.print(":");

    if (pTime->tm_min < 10) { Serial.print("0"); }
    Serial.print(pTime->tm_min);
    Serial.print(":");

    if (pTime->tm_sec < 10) { Serial.print("0"); }
    Serial.print(pTime->tm_sec);

    if (endwithCRLF) { Serial.println(); }
}

void Time_PrintDate(TM * pTime, bool endwithCRLF)
{
    if (pTime->tm_mday < 10) { Serial.print("0"); }
    Serial.print(pTime->tm_mday);
    Serial.print("-");

    if (pTime->tm_mon < 10) { Serial.print("0"); }
    Serial.print(pTime->tm_mon);
    Serial.print("-");

    uint8_t year = TWO_DIGIT_YEAR(pTime->tm_year);
    if (year < 10) { Serial.print("0"); }
    Serial.print(year);

    if (endwithCRLF) { Serial.println(); }
}
