#ifndef _DATAFIELD_TYPES_H_
#define _DATAFIELD_TYPES_H_

/* Each datafield has an associated datatype */
enum field_type
{
    // Electrical values
    VOLTAGE,
    CURRENT,

    // Temperature values
    TEMPERATURE_C,
    TEMPERATURE_F,
    TEMPERATURE_K,
    
    // Solar values
    IRRADIANCE_WpM2,

    // Wind values
    CARDINAL_DIRECTION, //N, NE, E, etc.
    DEGREES_DIRECTION,

    INVALID_TYPE
};
typedef enum field_type FIELD_TYPE;

/* Each FIELD_TYPE has a data structure associated with it 
 * in order to perform conversions from raw values to units.
  In addition, the number of settings is #defined so that the
  settings module can figure out if everything is set */
struct voltagechannel
{
    float mvPerBit;
    float offset;
    float multiplier;
    float R1;
    float R2;
};
typedef struct voltagechannel VOLTAGECHANNEL;

struct currentchannel
{
    float mvPerBit;
    float offset;
    float mvPerAmp;
};
typedef struct currentchannel CURRENTCHANNEL;

struct thermistorchannel
{
    float R25;
    float B;
    float otherR;
    float maxADC;
    bool highside;
}; 
typedef struct thermistorchannel THERMISTORCHANNEL;

#endif
