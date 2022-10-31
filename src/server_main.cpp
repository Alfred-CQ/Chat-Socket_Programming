/* Server code in C++ */
#include "include/server.hpp"

Server myServer(45000, "127.0.01");  

void manager(SClients* client)
{
    int n;
    char action = '1';

    while (action != 'R')
    {
        n = recv(client->scli_socketFD, &action, 1, 0);

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
                continue;
        }
    }

    shutdown(client->scli_socketFD, SHUT_RDWR);
    close(client->scli_socketFD);
}

int main(void)
{
    system("clear");
    cout << "░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█░░░█▀█░█▀▀░█▀▀\n░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░█░░░█░█░█░█░▀▀█\n░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀▀▀\n";

    for (;;)
        thread(manager, myServer.register_client()).detach();

    close(myServer.server_SocketFD);
    
    return 0;
}
