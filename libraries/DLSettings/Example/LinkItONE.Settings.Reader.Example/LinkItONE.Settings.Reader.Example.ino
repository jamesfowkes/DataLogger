/*
 * LinkITtONE.Settings.Reader.Example.ino
 *
 * Settings file reader example
 *
 * See README.md for full details
 *
 * Basic summary:
 *
 * - Reads settings from Datalogger.Settings on SD card storage and echoes to command line
 */

/*
 * Standard Library Includes
 */
#include <stdint.h>

/*
 * Arduino Library Includes
 */

#include <Wire.h>

/*
 * LinkIt One Includes
 */

#include <LSD.h>
#include <LStorage.h>

/*
 * DataLogger Includes
 */

#include "DLSettings.h"
#include "DLSettings.Reader.h"
#include "DLUtility.h"
#include "DLLocalStorage.h"

static char const s_settingsFilename[] = "Datalogger.Settings";
static LocalStorageInterface * s_sdCard = LocalStorage_GetLocalStorageInterface(LINKITONE_SD_CARD);
static char lineBuffer[100];

void setup()
{
    // setup Serial port
    Serial.begin(115200);

    delay(10000);

    if (!s_sdCard->fileExists(s_settingsFilename))
    {
        Serial.print("Settings file '");
        Serial.print(s_settingsFilename);
        Serial.println(" not found.");
        return;
    }

    uint8_t hndl = s_sdCard->openFile(s_settingsFilename, false);
    while (!s_sdCard->endOfFile(hndl))
    {
        s_sdCard->readLine(hndl, lineBuffer, 50);
        Serial.print("Reading setting line '");
        Serial.print(lineBuffer);
        Serial.println("'");
        Settings_ReadFromString(lineBuffer);
    }
    s_sdCard->closeFile(hndl);

    // Echo out integer settings
    Serial.println("Integer Settings:");
    int i;
    for (i = 0; i < INT_SETTINGS_COUNT; i++)
    {
        Serial.print(Settings_getIntName((INTSETTING)i));
        Serial.print(": ");
        Serial.println(Settings_getInt((INTSETTING)i));
    }

    // Echo out string settings
    Serial.println("String Settings:");
    for (i = 0; i < STRING_SETTINGS_COUNT; i++)
    {
        Serial.print(Settings_getStringName((STRINGSETTING)i));
        Serial.print(": ");
        Serial.println(Settings_getString((STRINGSETTING)i));
    }
}

void loop()
{
    
}
