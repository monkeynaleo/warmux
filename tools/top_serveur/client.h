#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>

class Client
{
	int fd;
	struct sockaddr_in address;
public:
	Client(int client_fd,struct sockaddr_in client_address);
	~Client();

	int & GetFD();

	// Return false if the client closed the connection
	bool Receive();
};

#endif
