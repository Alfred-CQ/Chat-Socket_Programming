CXXFLAGS = -c

all: server client

server: server_main.o server.o
	$(CXX) -std=c++17 -pthread server_main.o server.o -o server

server_main.o: src/server_main.cpp
	$(CXX) -std=c++17 -c src/server_main.cpp

server.o: src/protocols/server.cpp
	$(CXX) -std=c++17 -c src/protocols/server.cpp

client: client_main.o client.o
	$(CXX) -std=c++17 -pthread client_main.o client.o -o client

client_main.o: src/client_main.cpp
	$(CXX) -std=c++17 -c src/client_main.cpp

client.o: src/protocols/client.cpp
	$(CXX) -std=c++17 -c src/protocols/client.cpp

clean:
	rm -f *.o