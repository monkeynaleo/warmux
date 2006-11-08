#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>
#include <string>
#include "../../src/network/top_server_msg.h"

class Client
{
	int fd;
	int received;
	struct sockaddr_in address;
	bool handshake_done;

	char* str;
	unsigned int str_size;

	enum TopServerMsg msg_id;

	// Return false if the client closed the connection
	bool ReceiveStr(std::string & full_str);
	bool ReceiveInt(int & nbr);

	void SendInt(const int & nbr);
	void SendStr(const std::string & full_str);
	void SendSignature();
public:
	Client(int client_fd,struct sockaddr_in client_address);
	~Client();

	int & GetFD();

	// Return false if the client closed the connection
	bool Receive();
};

#endif
