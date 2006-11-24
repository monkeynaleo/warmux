#include <string>
#include <fstream>
#include "config.h"
#include "debug.h"

Config config;

Config::Config()
{
	Load();
	SetDefault("port", 9997);
	SetDefault("chroot", true);
	SetDefault("chroot_gid", 500);
	SetDefault("chroot_uid", 500);
	SetDefault("connexion_max", -2);
	Display();
}

static ssize_t getline(std::string& line, std::ifstream& file)
{
        line.clear();
        std::getline(file, line);
        return line.size();
}

void Config::Load()
{
	DPRINT(INFO, "Loading config file");

	int line_nbr = 0;

	// Parse the file
	std::ifstream fin;
	fin.open("wormux_server.conf", std::ios::in);
	if(!fin)
	{
		DPRINT(INFO, "Unable to open config file");
		return;
	}

	ssize_t read;
	std::string line;

	while ((read = getline(line, fin)) > 0)
	{
		line_nbr++;
		if(line.at(0) == '#' || line.at(0) == '\n' || line.at(0) == '\0')
			continue;

		std::string::size_type equ_pos = line.find('=',0);
		if(equ_pos == std::string::npos)
		{
			DPRINT(INFO, "Wrong format on line %i",line_nbr);
			continue;
		}

		std::string opt = line.substr(0, equ_pos);
		std::string val = line.substr(equ_pos+1);

		if( (val.at(0) >= '0' && val.at(0) <= '9')
		||   val.at(0) == '-' )
		{
			int nbr = atoi(val.c_str());
			int_value[ opt ] = nbr;
		}
		else
		{
			if(val == "true\n")
				bool_value[ opt ] = true;
			else
			if(val == "false\n")
				bool_value[ opt ] = false;
			else
			{
				// We have a string. But first, we need to erase the '\n' at the end of the line
				unsigned int off = val.find('\n');
				if(off != val.size())
					val.erase(off);
				str_value[ opt ] = val;
			}
		}
	}

	DPRINT(INFO, "Config loaded");
}

void Config::Display()
{
	DPRINT(INFO, "Current config:");
	for(std::map<std::string, bool>::iterator cfg = bool_value.begin();
		cfg != bool_value.end();
		++cfg)
	{
		DPRINT(INFO, "(bool) %s = %s", cfg->first.c_str(), cfg->second?"true":"false");
	}

	for(std::map<std::string, int>::iterator cfg = int_value.begin();
		cfg != int_value.end();
		++cfg)
	{
		DPRINT(INFO, "(int) %s = %i", cfg->first.c_str(), cfg->second);
	}

	for(std::map<std::string, std::string>::iterator cfg = str_value.begin();
		cfg != str_value.end();
		++cfg)
	{
		DPRINT(INFO, "(str) %s = %s", cfg->first.c_str(), cfg->second.c_str());
	}
}

bool Config::Get(const std::string & name, bool & value)
{
	if( bool_value.find(name) == bool_value.end() )
	{
		DPRINT(INFO, "Configuration option not found: %s", name.c_str());
		return false;
	}

	value = bool_value[ name ];
	return true;
}

bool Config::Get(const std::string & name, int & value)
{
	if( int_value.find(name) == int_value.end() )
	{
		DPRINT(INFO, "Unknown config option: %s", name.c_str());
		return false;
	}

	value = int_value[ name ];
	return true;
}

bool Config::Get(const std::string & name, std::string & value)
{
	if( str_value.find(name) == str_value.end() )
	{
		DPRINT(INFO, "Unknown config option: %s", name.c_str());
		return false;
	}

	value = str_value[ name ];
	return true;
}

void Config::SetDefault(const std::string & name, const bool & value)
{
	bool val;
	if( ! Get(name, val) )
	{
		DPRINT(INFO, "Setting to default value : %s", name.c_str());
		bool_value[ name ] = value;
	}
}

void Config::SetDefault(const std::string & name, const int & value)
{
	int val;
	if( ! Get(name, val) )
	{
		DPRINT(INFO, "Setting to default value : %s", name.c_str());
		int_value[ name ] = value;
	}
}

void Config::SetDefault(const std::string & name, const std::string & value)
{
	bool val;
	if( ! Get(name, val) )
	{
		DPRINT(INFO, "Setting to default value : %s", name.c_str());
		str_value[ name ] = value;
	}
}
