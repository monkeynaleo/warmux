#include <stdio.h>
#include <string>
#include "stat.h"
#include "clock.h"
#include "debug.h"
#include "config.h"

Stats stats;

ConnectionStats::ConnectionStats(const std::string & fn)
{
	Reset();
	filename = fn;
}

ConnectionStats::~ConnectionStats()
{
	Write();
	CloseFile();
}

void ConnectionStats::OpenFile()
{
	std::string full_name;
	config.Get("my_hostname", full_name);

	struct tm* t;
	time_t now = time(NULL);
	t = localtime(&now);
	char time_str[1024];
	snprintf(time_str, 1024, "%4i-%02i-%02i_", 1900 + t->tm_year, 1 + t->tm_mon,t->tm_mday);

	full_name = std::string(time_str) + full_name + '_' + filename;

	DPRINT(INFO, "Opening logfile : %s",full_name.c_str());
	fd = fopen(full_name.c_str(), "a+");

	if(fd == NULL)
		TELL_ERROR;
}

void ConnectionStats::CloseFile()
{
	fclose(fd);
}

void ConnectionStats::Reset()
{
	servers = 0;
	fake_servers = 0;
	clients = 0;
}

void ConnectionStats::Rotate()
{
	Write();
	CloseFile();
	OpenFile();
}

void ConnectionStats::Write()
{
	struct tm* t;
	time_t now = time(NULL);
	t = localtime(&now);
	fprintf(fd, "%4i-%02i-%02i %i:%2i:%2i %lu %lu %lu\n", 1900 + t->tm_year,t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, fake_servers, servers, clients);

	fflush(fd);
	Reset();
}

Stats::Stats() : hourly("hourly"), daily("daily")
{
}

void Stats::Init()
{
	daily.OpenFile();
	hourly.OpenFile();
}

void Stats::NewServer()
{
	hourly.servers++;
	daily.servers++;
}

void Stats::NewFakeServer()
{
	hourly.fake_servers++;
	daily.fake_servers++;
}

void Stats::NewClient()
{
	hourly.clients++;
	daily.clients++;
}

