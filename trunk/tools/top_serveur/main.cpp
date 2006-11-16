#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <list>

#include "server.h"
#include "client.h"
#include "config.h"
#include "debug.h"

// map < socket_fd, client >
std::list<Client*> clients;

#ifdef STATIC
void SetChroot()
{
	if(chroot("./") == -1)
		TELL_ERROR;
	if(chdir("/") == -1)
		TELL_ERROR;

	int uid, gid;
	config.Get("chroot_uid", uid);
	config.Get("chroot_gid", gid);

	if(setgid(gid) == -1)
		TELL_ERROR;
	if(setuid(uid) == -1)
		TELL_ERROR;
}
#endif

int main(int argc, void** argv)
{
	bool chroot_opt;
	config.Get("chroot", chroot_opt);
#ifdef STATIC
	if (chroot_opt)
		SetChroot();
#else
	if (chroot_opt)
		DPRINT(INFO, "Wasn't statically linked... chroot option unavailable");
#endif
	if(getuid() == 0)
	{
		DPRINT(INFO, "Don't start me as root user!!");
		exit(EXIT_FAILURE);
	}

	// Ignore broken pipe signal (elsewise we would break,
	// as soon as we are trying to write on client that closed)
	if( signal(SIGPIPE, SIG_IGN) == SIG_ERR )
		TELL_ERROR;

	int port = 0;
	config.Get("port", port);

	Server listen_sock(port);

	// Set of socket where an activity have been detected
	fd_set acting_sock_set;

	while(1)
	{
		DPRINT(CONN, "Waiting for incoming connections...");

		acting_sock_set = listen_sock.GetSockSet();
		// Lock the process until activity is detected
		if ( select(FD_SETSIZE, &acting_sock_set, NULL, NULL, NULL) < 1 )
			TELL_ERROR;

		// Find the socket where activity have been detected:
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
					DPRINT(CONN, "%i connections up!", clients.size());
					break;
				}
			}
		}
		// First check if there is any new incoming connection
		if( FD_ISSET(listen_sock.GetFD(), &acting_sock_set) )
		{
			Client* client = listen_sock.NewConnection();
			if(client != NULL)
				clients.push_back( client );

			DPRINT(CONN, "%i connections up!", clients.size());
		}
	}
}
