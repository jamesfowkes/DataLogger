/*
 * C/C++ Includes
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*
 * Library Includes
 */

#include "DLUtility.h"

/*
 * Application Includes
 */

#include "app.serial-request-data.h"

/*
 * Public Functions
 */

bool APP_SerialRequestData_FormatArray(float * data, int n_fields, char * buffer, int buffer_length, TM * p_time)
{
	int i;
	char sprintf_buffer[12];

	time_t t;
	t = mktime(p_time);
	
	if (!data) { return false; }
	if (!buffer) { return false; }
	if (n_fields == 0) { return false; }

	FixedLengthAccumulator accumulator(buffer, buffer_length);
		
	accumulator.writeChar('s');

	sprintf(sprintf_buffer, "%ld", t);
	accumulator.writeString(sprintf_buffer);
	accumulator.writeChar(',');
	

	for (i = 0; i < n_fields; i++)
	{
		sprintf(sprintf_buffer, "%.1f", data[i]);
		accumulator.writeString(sprintf_buffer);
		accumulator.writeChar(',');
	}
	accumulator.remove(1);
	accumulator.writeChar('t');

	return true;
}
