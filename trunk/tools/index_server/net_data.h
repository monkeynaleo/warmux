#ifndef NET_DATA_H
#define NET_DATA_H
#include <netinet/in.h>
#include <string>
#include "../../src/network/index_svr_msg.h"


class NetData
{
	char* str;
	unsigned int str_size;
	enum IndexServerMsg msg_id;

	int fd;
	int ip_address;

	bool connected;
protected:
	int received;

	// Return false if the client closed the connection
	bool ReceiveStr(std::string & full_str);
	bool ReceiveInt(int & nbr);

	bool SendInt(const int & nbr);
	bool SendStr(const std::string & full_str);
public:
	NetData();
	virtual ~NetData();

	const int & GetFD() { return fd; };
	const int & GetIP() { return ip_address; };

	void Host( const int & client_fd, const unsigned int & ip );
	void ConnectTo( const std::string & address, const int & port);
	bool Receive();
	virtual bool HandleMsg(const IndexServerMsg & msg_id, const std::string & full_str) = 0;
};

#endif
