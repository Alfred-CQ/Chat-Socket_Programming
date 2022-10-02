#ifndef _SERVER_HPP_
    #define _SERVER_HPP_

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>

    #include <iostream>

    typedef unsigned int uint;

    class Server
    {
        public:

            uint server_port;
            std::string server_ip;

            struct sockaddr_in server_SockAddr;
            int server_SocketFD;

            Server(uint port, std::string ip);
            ~Server();

        private:
            /* data */  
    };

    Server::Server(uint port, std::string ip)
    {
        server_port = port;
        server_ip = ip;

        if ((server_SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Socket");
            exit(1);
        }

        if (setsockopt(server_SocketFD, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int)) == -1)
        {
            perror("Setsockopt");
            exit(1);
        }

        memset(&server_SockAddr, 0, sizeof(struct sockaddr_in));

        server_SockAddr.sin_family = AF_INET;
        server_SockAddr.sin_port = htons(this->server_port);
        server_SockAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_SocketFD, (struct sockaddr *)&server_SockAddr, sizeof(struct sockaddr)) == -1)
        {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(server_SocketFD, 5) == -1)
        {
            perror("Listen");
            exit(1);
        }
    }

    Server::~Server()
    {
    }

#endif
