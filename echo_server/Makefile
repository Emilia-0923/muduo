all: Server Client

.PHONY:
clean:
	rm -rf Server Client

Server: Server.cpp
	g++ -o $@ $^ -std=c++14 -lpthread -g

Client: Client.cpp
	g++ -o $@ $^ -std=c++14 -lpthread