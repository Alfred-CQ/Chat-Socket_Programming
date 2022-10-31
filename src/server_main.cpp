/* Server code in C++ */
#include "include/server.hpp"

Server myServer(45000, "127.0.01");  

void manager(SClients* client, char action)
{   
    if (action == 'M' || action == 'I')
        myServer.message_clients(client);   
    else if (action == 'B')
        myServer.send_broadcast(client);
    else if (action == 'R')
        myServer.send_broadcast(client, true);
    else if (action == 'L')
        myServer.send_list_clients(client);
    else if (action == 'F')
        myServer.send_file(client);
    else if (action == 'U')
        myServer.recv_notification(client);
    else if (action == 'G')
    {
        SClients* scli_friend = myServer.message_clients(client);   
        myServer.make_game(client, scli_friend);
    }
    else if (action == 'T')
    {
        if (!myServer.update_Local_Games(client))
            return;
    }
}

int main(void)
{
    system("clear");
    cout << "░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█░░░█▀█░█▀▀░█▀▀\n░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░█░░░█░█░█░█░▀▀█\n░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀▀▀\n";

    fd_set main_fds, read_fds;

    int fdmax;

    FD_ZERO(&main_fds); 
    FD_ZERO(&read_fds);

    FD_SET(myServer.server_SocketFD, &main_fds);

    fdmax = myServer.server_SocketFD;

    /* Connect */
    socklen_t addrlen;
    struct sockaddr_storage remoteaddr;
    int newfd; 

    for (;;)
    {
        read_fds = main_fds;

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        for (int fd = 0; fd <= fdmax; ++fd)
        {
            if (FD_ISSET(fd, &read_fds))
            {
                if ( fd == myServer.server_SocketFD)
                {
                    addrlen = sizeof remoteaddr;
                    newfd = accept(myServer.server_SocketFD,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &main_fds); // add to master set
                        if (newfd > fdmax)
                        {
                            fdmax = newfd;
                        }

                        printf("selectserver: new connection on "
                               "socket %d\n", newfd);
                    }
                }
                else
                {
                    int bytes_received;
                    char action = '0';
                    bytes_received = recv(fd, &action, 1, 0);

                    if (bytes_received <= 0) {
                        
                        if (bytes_received == 0) 
                        {
                            printf("selectserver: socket %d close connection\n", fd);
                        } 
                        else 
                        {
                            perror("recv");
                        }
                        close(fd);
                        FD_CLR(fd, &main_fds);
                    }
                    else
                    {
                        cout << myServer.s_clients[fd] << std::endl;
                        if (action == 'N')
                            myServer.register_client(fd);
                        else
                            manager(myServer.s_clients[fd], action);  
                    }
                        
                }
            }
        }
    }

    close(myServer.server_SocketFD);
    
    return 0;
}
