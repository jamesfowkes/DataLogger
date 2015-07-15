/*
 * LinkItONE.Crest
 *
 * Application for Crest PV on LinkItONE hardware
 *
 * See README.md for full details
 *
 * Basic summary:
 *
 * - Reads data from ADS1115 ADCs (addresses 0x48, 0x49, 0x4A) 10x per second
 * - Averages data
 * - Stores data to SD card
 * - Sends data to Thingspeak
 * - configuration of these functions through files on SD card
 *
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
#include <LGSM.h>
#include <LGPS.h>
#include <LGPRS.h>
#include <LDateTime.h>
#include <LBattery.h>

/*
 * DataLogger Includes
 */

#include "DLError.h"
#include "DLUtility.Time.h"
#include "DLTime.h"
#include "DLCSV.h"
#include "DLGPS.h"
#include "DLFilename.h"
#include "DLLocalStorage.h"
#include "DLUtility.h"
#include "DLLocation.h"

#include "DLDataField.Types.h"
#include "DLDataField.h"
#include "DLDataField.Manager.h"

#include "DLSettings.h"
#include "DLSettings.Global.h"
#include "DLSettings.Reader.Errors.h"
#include "DLSettings.Global.Reader.h"
#include "DLSettings.Reader.h"
#include "DLSettings.DataChannels.h"

#include "DLSensor.ADS1x1x.h"
#include "DLSensor.LinkItONE.h"
#include "DLSensor.Thermistor.h"

#include "DLHTTP.h"
#include "DLNetwork.h"
#include "DLService.h"
#include "DLService.thingspeak.h"

#include "DLSMS.h"
#include "DLSMS.messaging.h"

#include "DLPlatform.h"

#include "DLBattery.h"

#include "TaskAction.h"

/*
 * Application Includes
 */

#include "app.h"
#include "app.upload_manager.h"
#include "app.sd_storage.h"
#include "app.data.h"
#include "app.error.h"

// Pointers to fuctionality objects

static ServiceInterface * s_thingSpeakService;
static NetworkInterface * s_gprsConnection;

static char * s_requestBuffer;
static uint32_t s_uploadBufferSize;
static float * s_uploadData;

static bool s_debugUpload = false;
static bool s_debugGPS = false;

static ADS1115 s_ADCs[] = {
    ADS1115(0x48),
    ADS1115(0x49),
    ADS1115(0x4A)
};

static LinkItONEADC s_internalADCs[] = {
    LinkItONEADC(A0),
    LinkItONEADC(A1),
    LinkItONEADC(A2)
};

static uint32_t s_uploadInterval = 0;

static TM s_gpsTime;
static TM s_rtcTime;

#define ADC_READS_PER_SECOND (5)
#define MS_PER_ADC_READ (1000 / ADC_READS_PER_SECOND)

/*
 * APP_SetDebugModules
 *
 * For each module in the comma separated list of modules, turn serial debugging on
 */
void APP_SetDebugModules(char const * const modules)
{
    if (strstr(modules, "LocalStorage"))
    {
        Serial.println("Turning debugging on for local storage functionality.");
        APP_SD_EnableDebugging();
    }

    if (strstr(modules, "Upload"))
    {
        Serial.println("Turning debugging on for remote upload functionality.");
        s_debugUpload = true;
    }
    
    if (strstr(modules, "GPS"))
    {
        Serial.println("Turning debugging on for GPS functionality.");
        s_debugGPS = true;
    }

    if (strstr(modules, "Batt"))
    {
        Battery_Set_Debug(true);
    }
}

static void tryConnection(void)
{
    // Try to connect to GPRS network
    if (s_debugUpload)
    {
        Serial.print("Attempting to attach to ");
        Serial.print(Settings_getString(GPRS_APN));
        Serial.print(" with username/pwd ");
        Serial.print(Settings_getString(GPRS_USERNAME));
        Serial.print("/");
        Serial.println(Settings_getString(GPRS_PASSWORD));
    }
    s_gprsConnection->tryConnection(10);
}

static void delayStart(uint8_t seconds)
{
    Serial.println("Starting application in... ");

    while (seconds--)
    {
        Serial.print(seconds);
        Serial.print("...");
        delay(1000);
    }
    Serial.println();
}

/*
 * Tasks
 */
static void heartbeatTaskFn(void);
TaskAction heartbeatTask(heartbeatTaskFn, 1000, INFINITE_TICKS);
static void heartbeatTaskFn(void)
{
    static bool ledState = false;
    digitalWrite(HEARTBEAT_LED_PIN, ledState ? HIGH : LOW);
    // 1s on, 4s off by setting task interval
    heartbeatTask.SetInterval(ledState ? 1000 : 4000);
    ledState = !ledState;
}

static void readFromADCsTaskFn(void)
{
    uint8_t adc = 0;
    uint8_t ch = 0;
    uint8_t field = 0;
    int32_t allData[15];

    // Read the ADC1x1x ICs for fields 1 - 12
    for (adc = 0; adc < 3; adc++)
    {
        for (ch = 0; ch < 4; ch++)
        {
            field = (adc*4)+ch;
            if (Settings_ChannelSettingIsValid(field+1))
            {
                allData[field] = s_ADCs[adc].readADC_SingleEnded(ch);
            }
            else
            {
                allData[field] = 0;
            }
        }
    }

    // Read the internal ADCs for fields 13-15
    for (adc = 0; adc < 3; adc++)
    {
        field = adc + 12;
        if (Settings_ChannelSettingIsValid(field+1))
        {
            allData[field] = s_internalADCs[adc].read();
        }
        else
        {
            allData[field] = 0;
        }
    }
    
    APP_Data_NewDataArray(allData);
}
TaskAction readFromADCsTask(readFromADCsTaskFn, MS_PER_ADC_READ, INFINITE_TICKS, "ADC Read Task");

static void remoteUploadTaskFn(void);
TaskAction remoteUploadTask(remoteUploadTaskFn, 1000, INFINITE_TICKS, "Upload Task");
static void remoteUploadTaskFn(void)
{

    bool success = false;

    if (APP_Data_UploadIsPending() && APP_Data_UploadDataRemaining())
    {
        /* The data upload flag is set and there is data to upload.
        Reset the flag and task interval and perform the upload */
        APP_Data_SetUploadPending(false);
        remoteUploadTask.SetInterval(s_uploadInterval * 1000);

        uint16_t nFields = APP_Data_GetNumberOfFields();
        
        if (!s_gprsConnection->isConnected())
        {
            if (s_debugUpload) { Serial.println("GPRS not connected. Attempting new connection."); }
            tryConnection();
        }

        // Try to upload to ThingSpeak
        char response_buffer[200] = "";
        while(APP_Data_UploadDataRemaining())
        {

            if (s_debugUpload)
            {
                Serial.print("Remaining records to upload: ");
                Serial.println(APP_Data_UploadDataRemaining());

                Serial.print("Attempting upload to ");
                Serial.println(s_thingSpeakService->getURL());
            }
            
            APP_Data_GetUploadData(s_uploadData);
            
            char created_at[30];
            TM createTime;
            Time_GetTime(&createTime, TIME_PLATFORM);
            CSV_writeTimestampToBuffer(&createTime, created_at);
            s_thingSpeakService->createPostAPICall(
                s_requestBuffer, s_uploadData, APP_Data_GetChannelNumbers(), nFields, s_uploadBufferSize, NULL);

            if (s_debugUpload)
            {
                Serial.print("Request: '");
                Serial.print(s_requestBuffer);
                Serial.println("'");
            }

            success = s_gprsConnection->sendHTTPRequest(s_thingSpeakService->getURL(), s_requestBuffer, response_buffer);

            Error_Running(ERR_RUNNING_DATA_UPLOAD_FAILED, !success);

            if (success)
            {
                if (s_debugUpload) { Serial.println("Upload complete!"); }
            }
            else
            {
                if (s_debugUpload)
                {
                    Serial.print("Could not connect to ");
                    Serial.print(s_thingSpeakService->getURL());
                    Serial.println("!");
                }
            }
        }        
    }
    else
    {
        /* Set the upload pending flag and start calling this task every second to
        check for data */
        APP_Data_SetUploadPending(true);
        remoteUploadTask.SetInterval(1000);
    }
}

void gpsTaskFn(void)
{
    Location_UpdateNow();
    bool success = GPS_InfoIsValid();

    Error_Running(ERR_RUNNING_NO_GPS, !success);

    if (success)
    {
        Time_GetTime(&s_gpsTime, TIME_GPS);
        if (s_debugGPS)
        {
            Serial.print("Updating RTC time from GPS (");
            Time_PrintTime(&s_gpsTime);
            Serial.print(" ");
            Time_PrintDate(&s_gpsTime);
            Serial.println(")");
        }
        Time_SetPlatformTime(&s_gpsTime);
    }
    else
    {
        if (s_debugGPS) { Serial.println("No valid GPS info."); }
    }
}
TaskAction gpsTask(gpsTaskFn, 30 * 1000, INFINITE_TICKS, "GPS Task");

void setupUploadVars(void)
{
    uint16_t nFields = APP_Data_GetNumberOfFields();

    /* Get pointers to the GPRS data and online storage services */
    s_thingSpeakService = Service_GetService(SERVICE_THINGSPEAK);
    s_gprsConnection = Network_GetNetwork(NETWORK_INTERFACE_LINKITONE_GPRS);

    // Allocate space for CSV data and HTTP request building.
    // Allow 10 chars per field, plus 20 for timestamp.
    // Then allocate twice as much as that estimate (minimum 512)

    s_uploadBufferSize = APP_Data_GetUploadBufferSize();
    Serial.print("Upload buffer size: ");
    Serial.println(s_uploadBufferSize);
    
    s_requestBuffer = new char [s_uploadBufferSize];

    // Allocate space for floats to pass to upload module
    s_uploadData = new float[nFields];
}

void setupADCs(void)
{
    uint8_t i = 0;
    for (i = 0; i < 3; i++)
    {
        s_ADCs[i].begin();
        s_ADCs[i].setGain(GAIN_ONE);

        if (Settings_stringIsSet(FAKE_ADC_READS))
        {
            s_ADCs[i].fake(0, 0, 1023);
            s_ADCs[i].fake(1, 0, 1023);
            s_ADCs[i].fake(2, 0, 1023);
            s_ADCs[i].fake(3, 0, 1023);
        }
    }
}

void setupTime(void)
{
    Time_GetTime(&s_rtcTime, TIME_PLATFORM);
    Serial.print("RTC datetime: ");
    Time_PrintTime(&s_rtcTime);
    Serial.print(" ");
    Time_PrintDate(&s_rtcTime, true);
}

void setup()
{   
    // setup Serial port
    Serial.begin(115200);

    delayStart(10);

    pinMode(HEARTBEAT_LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);

    Location_Setup(0);
 
    APP_SD_Init(); // This just initialises the SD card module so settings can be read
    
    /* Tell the settings modules which settings are 
    required. If these settings are missing, the application
    will not start (Error_Fatal will be called) */

    Settings_Init();
    Settings_requireInt(UPLOAD_AVERAGING_INTERVAL_SECS);
    Settings_requireInt(STORAGE_AVERAGING_INTERVAL_SECS);
    Settings_requireInt(DATA_STORAGE_INTERVAL_SECS);
    Settings_requireInt(DATA_UPLOAD_INTERVAL_SECS);
    
    Settings_requireString(GPRS_APN);
    Settings_requireString(GPRS_USERNAME);

    Settings_requireString(THINGSPEAK_URL);
    Settings_requireString(THINGSPEAK_API_KEY);
    
    APP_SD_ReadGlobalSettings("Datalogger.settings.conf");
    
    /* If exectution proceeds this far, all required settings were read.
    Start configuring the application */
    int storage_averaging_interval = Settings_getInt(STORAGE_AVERAGING_INTERVAL_SECS);
    int upload_averaging_interval = Settings_getInt(UPLOAD_AVERAGING_INTERVAL_SECS);
    int storage_interval = Settings_getInt(DATA_STORAGE_INTERVAL_SECS);
    s_uploadInterval = Settings_getInt(DATA_UPLOAD_INTERVAL_SECS);

    // The SD card module needs to know how often writes occur
    APP_SD_Setup(storage_interval * 1000);

    APP_Data_Setup(
        storage_averaging_interval, // Seconds to average storage readings over
        upload_averaging_interval,  // Seconds to average upload readings over
        ADC_READS_PER_SECOND, // Number of reads per second per field
        storage_interval, // Number of seconds between SD card writes
        s_uploadInterval, // Number of seconds between uploads to thingspeak
        "Datalogger.channels.conf" // Filename to read channel settings from
        );

    /* If execution got this far, the application data storage functionality
    was set up successfully and there are valid fields.
    Configure the remaining application functionality. */

    APP_Error_Setup();
    
    SMS_Setup(SMS_INTERFACE_LINKITONE);

    Battery_Setup();

    setupTime();

    setupUploadVars();
    
    setupADCs();
    
    readFromADCsTask.ResetTime();

    remoteUploadTask.SetInterval(s_uploadInterval * 1000);
    remoteUploadTask.ResetTime();
    
    gpsTask.ResetTime();
}

void loop()
{
    readFromADCsTask.tick();
    APP_Data_Tick();
    APP_SD_Tick();
    remoteUploadTask.tick();
    heartbeatTask.tick();
    gpsTask.tick();
    Battery_Tick();
    APP_Error_Tick();
}

