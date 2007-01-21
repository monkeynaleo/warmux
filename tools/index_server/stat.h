#ifndef STAT_H
#define STAT_H
#include <stdio.h>
#include <string>

class ConnectionStats
{
	FILE* fd;
	std::string filename;

	void CloseFile();
public:
	unsigned long servers;
	unsigned long fake_servers;
	unsigned long clients;

	ConnectionStats(const std::string & fn);
	~ConnectionStats();
	void OpenFile();
	void Reset();
	void Write();
	void Rotate(); 
};

class Stats
{
public:
	// Keeps records of connections
	// done hourly. File is rotated
	// every day
	ConnectionStats hourly;

	// Keeps records of connections
	// done daily. File is never changed
	ConnectionStats daily;

	Stats();
	void Init();
	void NewServer();
	void NewFakeServer();
	void NewClient();
};

extern Stats stats;

#endif
