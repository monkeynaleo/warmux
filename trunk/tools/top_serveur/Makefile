CXX=g++
CXXFLAGS=-g -DDEBUG -Wall -Werror

wormux_server: main.o server.o client.o
	$(CXX) -o wormux_server *.o

main.o: main.cpp server.h client.h debug.h
server.o: server.cpp server.h client.h debug.h
client.o: client.cpp client.h debug.h

clean:
	@rm -f *.o *~ wormux_server 2>&1 > /dev/null
