#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "client.h"
#include "debug.h"

Client::Client(int client_fd, struct sockaddr_in client_address)
{
	address = client_address;
	fd = client_fd;
}

Client::~Client()
{
	close(fd);
}

int & Client::GetFD()
{
	return fd;
}

bool Client::Receive()
{
	int received;
	if( ioctl(fd, FIONREAD, &received) == -1 )
		TELL_ERROR;

	if(received < 1)
		return false;

	char a;
	read(fd, &a, 1);
	DPRINT("Received: %c",a);
	return true;
}
