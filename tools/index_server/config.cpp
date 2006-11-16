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

void Config::Load()
{
	DPRINT(INFO, "Loading config file");
	FILE* cfg = fopen("wormux_server.conf", "r");

	if(cfg == NULL)
	{
		DPRINT(INFO, "Unable to open config file");
		return;
	}

	int line_nbr = 0;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, cfg)) != -1)
	{
		line_nbr++;
		char* ptr = line;
		if(*ptr == '#' || *ptr == '\n' || *ptr == '\0')
			continue;

		while(*ptr != '=' && *ptr != '\n' && *ptr != '\0' && (unsigned int)(ptr - line)<len)
			ptr++;

		if(*ptr != '=')
		{
			DPRINT(INFO, "Wrong format on line %i",line_nbr);
			continue;
		}
		*ptr = '\0';

		ptr++;
		if( (*ptr >= '0' && *ptr <= '9')
		||   *ptr == '-' )
		{
			int nbr = 0;
			if( sscanf(ptr, "%i\n", &nbr) <= 0)
				continue;
			int_value[ std::string(line) ] = nbr;
		}
		else
		{
			std::string val(ptr);
			if(val == "true\n")
				bool_value[ std::string(line) ] = true;
			else
			if(val == "false\n")
				bool_value[ std::string(line) ] = false;
			else
			{
				// We have a string. But first, we need to erase the '\n' at the end of the line
				unsigned int off = val.find('\n');
				if(off != val.size())
					val.erase(off);
				str_value[ std::string(line) ] = val;
			}
		}
	}
	if (line)
		free(line); 

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
