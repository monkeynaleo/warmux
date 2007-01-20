#ifndef CLOCK_H
#define CLOCK_H
#include <time.h>

class Clock
{
	time_t start_time;
	time_t last_refresh;

	char time_str[1024];
	char date_str[1024];

public:
	Clock();
	void HandleJobs();
	const char* TimeStr();
	const char* DateStr();
	void ShowUpTime();
};

extern Clock wx_clock;
#endif //CLOCK_H

