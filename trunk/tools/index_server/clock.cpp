#include <time.h>
#include <stdio.h>
#include "clock.h"
#include "sync_slave.h"
#include "debug.h"
#include "stat.h"

Clock wx_clock;

Clock::Clock()
{
	start_time = time(NULL);
	last_refresh = time(NULL);
}

void Clock::HandleJobs()
{
	if(time(NULL) == last_refresh)
		return;


	// Every day
	if(time(NULL) % (60 * 60 * 24) == 0)
	{
		ShowUpTime();
		DPRINT(INFO, "Day changed to : %s", DateStr());
		stats.hourly.Rotate();
		stats.daily.Write();
	}
	else
	// Every hour
	if(time(NULL) % (60 * 60) == 0)
	{
		stats.hourly.Write();
	}
	else
	// Refresh connections to the servers every minutes
	if(time(NULL) % 60 == 0)
	{
		sync_slave.Start();
	}
	
	last_refresh = time(NULL);
}

const char* Clock::TimeStr()
{
	struct tm* t;
	time_t now = time(NULL);
	t = localtime(&now);
	snprintf(time_str, 1024, "%2i:%02i", t->tm_hour,
							t->tm_min);
	return time_str;
}

const char* Clock::DateStr()
{
	time_t now = time(NULL);
	char* d = ctime(&now);
	strncpy(date_str, d, 1024);
	return date_str;
}

void Clock::ShowUpTime()
{
	double d = difftime(time(NULL), start_time);

	unsigned long t = (unsigned long) d;
	unsigned short sec, min, hr, day;
	sec = (unsigned short) t % 60;
	t /= 60;
	min = (unsigned short) t % 60;
	t /= 60;
	hr = (unsigned short) t % 24;
	t /= 24;
	day = (unsigned short) t % 31;
	t /= 31;
	DPRINT(INFO, "Up since %i months, %i days and %i:%02i:%02i hours (%i days)",(int)t,
									(int)day,
									(int)hr,
									(int)min,
									(int)sec,
									(int)t*31+day);
}
