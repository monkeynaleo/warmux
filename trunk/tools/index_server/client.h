#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>
#include <string>
#include "../../src/network/index_svr_msg.h"
#include "net_data.h"

class Client : public NetData
{
	std::string version;
	bool handshake_done;
	bool is_hosting;

	bool SendSignature();
	bool SendList();

	void SetVersion(const std::string & ver);
public:
	Client(int client_fd,unsigned int & ip);
	~Client();

	// Return false if the client closed the connection
	bool HandleMsg(const IndexServerMsg & msg_id, const std::string 
& str);
};

#endif
