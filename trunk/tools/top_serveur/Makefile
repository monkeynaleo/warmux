CXX=g++
CXXFLAGS=-g -DDEBUG -Wall -Werror
#LDFLAGS=-static -static-libgcc

wormux_server: main.o server.o client.o config.o
	$(CXX) $(LDFLAGS) -o wormux_server *.o

main.o: main.cpp server.h client.h debug.h config.h
server.o: server.cpp server.h client.h debug.h
client.o: client.cpp client.h debug.h
config.o: config.cpp config.h

clean:
	@rm -f *.o *~ wormux_server 2>&1 > /dev/null
