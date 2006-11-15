#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "debug.h"

Server::Server(int port)
{
	DPRINT(INFO, "Starting server... listening on port %i",port);
	// Init the listening socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if ( bind(fd, (struct sockaddr*) &address, sizeof(address)) == -1 )
		TELL_ERROR;

	// 128 is the size of the connection queue
	// -> TODO : use a sysctl to get the max size accepted by the kernel
	// ( see 'man listen' )
	if ( listen(fd, 128) == -1 )
		TELL_ERROR;

	// Init the socket set:
	FD_ZERO(&sock_set);
	FD_SET(fd, &sock_set);
}

Server::~Server()
{
	close(fd);
}

int & Server::GetFD()
{
	return fd;
}

fd_set & Server::GetSockSet()
{
	return sock_set;
}

Client* Server::NewConnection()
{
	int client_fd;
	struct sockaddr_in client_address;
	socklen_t addr_size = sizeof(client_address);

	if( (client_fd = accept(fd, (struct sockaddr*)&client_address, &addr_size)) == -1 )
	{
		PRINT_ERROR;
		DPRINT(INFO, "Client rejected..");
		return NULL;
	}

	FD_SET(client_fd, &sock_set);

//       struct hostent *info;
//	info = gethostbyaddr( &client_address.sin_addr, sizeof(client_address.sin_addr), AF_INET);
	unsigned int ip = client_address.sin_addr.s_addr;

	Client* client = new Client(client_fd, ip);
	DPRINT(CONN, "New connection opened by %i", ip);
	return client;
}

void Server::CloseConnection(int client_fd)
{
	FD_CLR(client_fd, &sock_set);
	DPRINT(CONN, "Connection closed.");
}

