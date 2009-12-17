#include <stdio.h>
#include <WORMUX_types.h>
#include <WORMUX_network.h>
#include <WORMUX_index_server.h>
#include <WORMUX_random.h>

void list_games(std::string version)
{
  connection_state_t conn = IndexServer::GetInstance()->Connect(version);
  if (conn != CONNECTED) {
    if (conn == CONN_WRONG_VERSION) {
      fprintf(stderr,"Sorry, version %s is not supported. Supported versions: %s\n",
	      version.c_str(), IndexServer::GetInstance()->GetSupportedVersions().c_str());
    } else {
      fprintf(stderr, "ERROR: Fail to connect to the index server\n");
    }
    return;
  }

  std::list<GameServerInfo> lst = IndexServer::GetInstance()->GetHostList();

  IndexServer::GetInstance()->Disconnect();

  if (lst.empty()) {
    fprintf(stderr, "Sorry, currently, no game servers for version %s\n", version.c_str());
    return;
  }

  for (std::list<GameServerInfo>::iterator it = lst.begin(); it != lst.end(); ++it) {
    printf("%s (%d) %s:%s - %s - %s\n",
	   version.c_str(), it->passworded, it->ip_address.c_str(), it->port.c_str(),
	   it->dns_address.c_str(), it->game_name.c_str());
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "usage: list_games <version> [<version> ...]\n");
    exit(EXIT_FAILURE);
  }

  RandomLocal().InitRandom();

  for (int i = 1; i < argc; i++)
    list_games(argv[i]);
}
