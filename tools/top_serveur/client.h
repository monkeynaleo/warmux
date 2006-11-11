#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>
#include <string>
#include "../../src/network/top_server_msg.h"

class Client
{
	int fd;
	int received;
	int ip_address;
	bool handshake_done;
	bool is_hosting;

	char* str;
	unsigned int str_size;

	enum TopServerMsg msg_id;

	int & GetIp();

	// Return false if the client closed the connection
	bool ReceiveStr(std::string & full_str);
	bool ReceiveInt(int & nbr);

	void SendInt(const int & nbr);
	void SendStr(const std::string & full_str);
	void SendSignature();
	void SendList();
public:
	Client(int client_fd,unsigned int & ip);
	~Client();

	int & GetFD();
	std::string GetHostName();

	// Return false if the client closed the connection
	bool Receive();
};

#endif
