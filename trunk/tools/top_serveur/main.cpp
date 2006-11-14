#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <list>

#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"

// map < socket_fd, client >
std::list<Client*> clients;

int main(int argc, void** argv)
{
/*	if(chroot("./") == -1)
		TELL_ERROR;
	if(chdir("/") == -1)
		TELL_ERROR;
	if(setgid(500) == -1)
		TELL_ERROR;
	if(setuid(500) == -1)
		TELL_ERROR;

	char* path = getenv("PWD");
	printf("Working folder: %s\n", path);

	if (fopen("/stats","w") == NULL)
		exit(0);
*/
	int port = 0;
	config.Get("port", port);

	Server listen_sock(port);

	// Set of socket where an activity have been detected
	fd_set acting_sock_set;

	while(1)
	{
		DPRINT("Waiting for incoming connections...");

		acting_sock_set = listen_sock.GetSockSet();
		// Lock the process until activity is detected
		if ( select(FD_SETSIZE, &acting_sock_set, NULL, NULL, NULL) < 1 )
			TELL_ERROR;

		// Find the socket where activity have been detected

		// First check for already established connections
		for(std::list<Client*>::iterator client = clients.begin();
			client != clients.end();
			++client)
		{
			if( FD_ISSET( (*client)->GetFD(), &acting_sock_set) )
			{
				if( !(*client)->Receive() )
				{
					// Connection closed
					listen_sock.CloseConnection( (*client)->GetFD() );
					delete *client;
					clients.erase(client);
					DPRINT("%i connections up!", clients.size());
					break;
				}
			}
		}
		// Then check if there is any new incoming connection
		if( FD_ISSET(listen_sock.GetFD(), &acting_sock_set) )
		{
			clients.push_back( listen_sock.NewConnection() );
			DPRINT("%i connections up!", clients.size());
		}
	}
}
