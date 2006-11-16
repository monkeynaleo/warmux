#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>
#include <string>
#include "../../src/network/top_server_msg.h"
#include "net_data.h"

class Client : public NetData
{
	bool handshake_done;
	bool is_hosting;

	bool SendSignature();
	bool SendList();
public:
	Client(int client_fd,unsigned int & ip);
	~Client();

	// Return false if the client closed the connection
	bool HandleMsg(const TopServerMsg & msg_id, const std::string & str);
};

#endif
