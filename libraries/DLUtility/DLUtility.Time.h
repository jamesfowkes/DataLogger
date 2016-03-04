#ifndef _DL_UTILITY_TIME_H_
#define _DL_UTILITY_TIME_H_

/*
 * Defines and Typedefs
 */

enum days_of_week_enum
{
	SUN,
	MON,
	TUE,
	WED,
	THU,
	FRI,
	SAT,
};

enum month_enum
{
	JANUARY,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
};

// Convert between C years and gregorian years
#define GREGORIAN_TO_C_YEAR(year) (year - FIRST_C_YEAR_GR) 
#define C_TO_GREGORIAN_YEAR(year) (year + FIRST_C_YEAR_GR) 

// Time and date epoch defines
#define FIRST_UNIX_YEAR_GR (1970) // The unix epoch starts at 1970 (gregorian)
#define FIRST_UNIX_YEAR_C GREGORIAN_TO_C_YEAR(FIRST_UNIX_YEAR_GR)

#define FIRST_C_YEAR_GR (1900) // Dates in the TM struct format start at 1900 (gregorian)
#define FIRST_C_YEAR_C GREGORIAN_TO_C_YEAR(FIRST_C_YEAR_GR)

#define FIRST_DAY_OF_WEEK_1_JAN_1970 (THU) // The unix epoch starts on a Thursday

// Needed for leap years: number of days up to Feb 28th
#define FEB28TH_DAYS_INTO_YEAR (31 + 28)

// Convert a gregorian year to its YY representation
#define TWO_DIGIT_YEAR(year) (year % 100)

// Hours to other times
#define HOURS_PER_DAY (24)

// Minutes to other times
#define MINS_PER_HOUR (60)
#define MINS_PER_DAY ((uint16_t)MINS_PER_HOUR * (uint16_t)HOURS_PER_DAY)

// Seconds to other times
#define S_PER_MIN (60)
#define S_PER_HOUR ((uint16_t)S_PER_MIN * (uint16_t)MINS_PER_HOUR)
#define S_PER_DAY ((uint16_t)S_PER_HOUR * (uint16_t)HOURS_PER_DAY)

// Milliseconds to other times
#define MS_PER_S (1000UL)
#define MS_PER_MIN (MS_PER_S * S_PER_MIN)
#define MS_PER_HOUR (MS_PER_S * S_PER_HOUR)

// Aggregate macros (days, hours, minutes, seconds to seconds)
#define MINS_SECS_TO_SECS(m, s) ((m*S_PER_MIN) + s)
#define HRS_MINS_SECS_TO_SECS(h, m, s) (h * S_PER_HOUR) + MINS_SECS_TO_SECS(m, s)
#define DAYS_HRS_MINS_SECS_TO_SECS(d, h, m, s) (d * S_PER_DAY) + HRS_MINS_SECS_TO_SECS(h, m, s)

/* This struct taken from C library time.h */
/*struct _tm
{
	int tm_sec;    // seconds after the minute (0 to 61)
	int tm_min;    // minutes after the hour (0 to 59)
	int tm_hour;   // hours since midnight (0 to 23)
	int tm_mday;   // day of the month (1 to 31)
	int tm_mon;    // months since January (0 to 11)
	int tm_year;   // years since 1900
	int tm_wday;   // days since Sunday (0 to 6 Sunday=0)
	int tm_yday;   // days since January 1 (0 to 365)
	int tm_isdst;  // Daylight Savings Time
};*/

typedef struct tm TM;

typedef int C_STRUCT_YEAR;
typedef int GREGORIAN_YEAR;

/*
 * Public Function Prototypes
 */

bool is_leap_year(GREGORIAN_YEAR year);
uint16_t days_in_month(uint8_t month, bool is_leap_year);
uint16_t calculate_days_into_year(const TM * tm);

void unix_seconds_to_time(time_t sec, TM * tm);
time_t time_to_unix_seconds(TM const * const tm);

void time_increment_seconds(TM * tm);

#endif
