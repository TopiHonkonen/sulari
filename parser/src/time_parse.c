#include <string.h>
#include <stdlib.h>
#include "time_parse.h"

int time_parse(char *time)
{
	int seconds;

	if (*time == NULL) 
		return TIME_ARRAY_ERROR;

	int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours

	//Boundary checks
	if (values[0] > 23 || values[0] < 0)
		return TIME_VALUE_ERROR;
	if (values[1] > 59 || values[1] < 0)
		return TIME_VALUE_ERROR;
	if (values[2] > 59 || values[2] < 0)
		return TIME_VALUE_ERROR;

	seconds = values[0] * 60 * 60 +
			values[1] * 60 +
			values[2];

	if (seconds < 0)
		seconds = TIME_LEN_ERROR;

	return seconds;
}
