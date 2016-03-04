#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "DLUtility.Time.h"
#include "DLUtility.HelperMacros.h"

// For non-leap years ONLY!
static const uint16_t s_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// For non-leap years ONLY!
static const uint16_t s_days_into_year[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

bool is_leap_year(GREGORIAN_YEAR year)
{
	return ((year & 3) == 0 && ((year % 25) != 0 || (year & 15) == 0));
}

uint16_t days_in_month(uint8_t month, bool is_leap_year)
{
	if ((month == FEBRUARY) && is_leap_year)
	{
		return 29;
	}
	else if (month > DECEMBER)
	{
		return 0;
	}
	else
	{
		return s_days_in_month[month];
	}
}

uint16_t calculate_days_into_year(const TM * tm)
{
	uint16_t days = s_days_into_year[tm->tm_mon] + tm->tm_mday;

	if (is_leap_year(tm->tm_year + 1900))
	{
		if (days > FEB28TH_DAYS_INTO_YEAR)
		{
			days++; //There's an extra day (Feb 29th) not accounted for months days array
		}
	}

	days--; // Subtract 1 because tm->tm_mday is 1-31, and first day of year is 0th day
	return days;
}

void unix_seconds_to_time(time_t sec, TM * tm)
{
	uint16_t day;
	GREGORIAN_YEAR year;
	uint16_t days_in_year;
	uint8_t month;
	
	bool bFoundYear = false;
	bool bIsLeapYear = false;
	
	tm->tm_sec = sec % 60;
	sec /= 60;
	tm->tm_min = sec % 60;
	sec /= 60;
	tm->tm_hour = sec % 24;
	day = sec / 24;

	tm->tm_wday = (day + FIRST_DAY_OF_WEEK_1_JAN_1970) % 7; // weekday

	year = FIRST_UNIX_YEAR_GR;
	
	while (!bFoundYear)
	{
		bIsLeapYear = is_leap_year(year);
		days_in_year = bIsLeapYear ? 366 : 365;
		
		bFoundYear = day < days_in_year;
		
		if (!bFoundYear)
		{
			day -= days_in_year;
			year++;
		}
	}
	
	tm->tm_yday = day;
	
	/* Convert from year to C year (offset to 1900) */
	tm->tm_year = year - 1900;
	
	for(month = 0; day >= days_in_month(month, bIsLeapYear); month++ )
	{
		day -= days_in_month(month, bIsLeapYear);
	}
	
	tm->tm_mon = month; // 0..11
	tm->tm_mday = day + 1; // 1..31
}

time_t time_to_unix_seconds(TM const * const tm)
{
	time_t secs = 0;
	
	if (tm->tm_year < FIRST_UNIX_YEAR_C) { return 0; }
	
	uint32_t days = tm->tm_yday;
	
	// Convert number of years from the UNIX epoch to specified year
	C_STRUCT_YEAR year = tm->tm_year;

	while (year-- > FIRST_UNIX_YEAR_C)
	{
		days += is_leap_year(C_TO_GREGORIAN_YEAR(year)) ? 366 : 365;
	}
	
	secs += (days * 86400UL);

	secs += (tm->tm_hour * 3600UL);
	secs += (tm->tm_min * 60UL);
	secs += tm->tm_sec;
	
	return secs;
}

void time_increment_seconds(TM * tm)
{
	if (!tm) { return; }
	
	incrementwithrollover(tm->tm_sec, 59);
	
	if (tm->tm_sec == 0)
	{
		incrementwithrollover(tm->tm_min, 59);
	}
	else
	{
		return;
	}
	
	if (tm->tm_min == 0)
	{
		incrementwithrollover(tm->tm_hour, 23);
	}
	else
	{
		return;
	}
	
	bool bIsLeapYear = is_leap_year(tm->tm_year);
	
	if (tm->tm_hour == 0)
	{
		incrementwithrollover(tm->tm_wday, (int)SAT);
		incrementwithrollover(tm->tm_mday, (int)days_in_month(tm->tm_mon, bIsLeapYear));
		incrementwithrollover(tm->tm_yday, bIsLeapYear ? 365 : 364);
	}
	else
	{
		return;
	}
	
	if (tm->tm_mday == 0)
	{
		tm->tm_mday = 1; // Month starts at 1, not 0
		incrementwithrollover(tm->tm_mon, 11);
	}
	else
	{
		return;
	}	
}
