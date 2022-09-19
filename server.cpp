/* Server code in C */

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
#include <vector>
#include <map>
#include <thread>

#include "include/utils.h"

#define STR_LENGTH 256

using namespace std;

char type_message;

map<char, string> actions = {{'N', "Nickname"},
                             {'M', "Message"},
                             {'B', "Broadcast"},
                             {'L', "List of users"},
                             {'R', "Close session"},
                             {'F', "Send a file"},
                             {'T', "Temp action"},
                             {'A', "Actions"}};

struct IClients
{
    int s_id;
    string s_name;

    IClients(int id, string name) : s_id(id),
                                    s_name(name) {}
};

vector<IClients> m_clients;

void chilina(IClients client)
{

    char bufferRead[STR_LENGTH];
    int N, M, size_friend, size_message;

    int id_friend = -1;
    char b;
    /// POLLIN

    while (1)
    {
        N = recv(client.s_id, &b, 1, 0);

        if (b == 'M')
        {
            bzero(bufferRead, STR_LENGTH);

            N = recv(client.s_id, bufferRead, 5, 0);

            bufferRead[N] = '\0';

            string sz1(bufferRead, 0, 2);
            string sz2(bufferRead, 2, 3);

            size_friend = atoi(&sz1.front());
            size_message = atoi(&sz2.front());

            bzero(bufferRead, STR_LENGTH);
            N = recv(client.s_id, bufferRead, size_friend + size_message, 0);
            bufferRead[N] = '\0';
            string nick_friend(bufferRead, 0, size_friend), message(bufferRead, size_friend, size_message + 1);

            for (int i = 0; i < m_clients.size() && id_friend == -1; ++i)
            {
                if (nick_friend == m_clients[i].s_name)
                    id_friend = m_clients[i].s_id;
            }

            if (id_friend == -1)
            {
                cout << "Client not found" << endl;
                continue;
            }
            message.insert(0, to_string(client.s_name.size()));
            message.insert(1, client.s_name);
            N = send(id_friend, &message.front(), message.size(), 0);
            if (N <= 0)
                cout << "ERROR reading registration" << endl;
        }
    }

    // shutdown(client.s_id, SHUT_RDWR);
    // close(client.s_id);
}

void broadcast(IClients client)
{
    char bufferRead[STR_LENGTH];
    int N, M, tam;
    char b;

    while (b != 'R')
    {
        N = recv(client.s_id, &b, 1, 0);
        cout << "ALIVE" << b << endl;
        if (b == 'B' | b == 'R')
        {

            if (b == 'B')
            {
                bzero(bufferRead, STR_LENGTH);
                N = recv(client.s_id, bufferRead, 3, 0);
                bufferRead[N] = '\0';
                tam = atoi(bufferRead);

                N = recv(client.s_id, bufferRead, tam, 0);
                bufferRead[N] = '\0';
            }
            else
            {

                strcpy(bufferRead, " Dejo el chat ");
                strcat(bufferRead, &(client.s_name.front()));
                N = strlen(bufferRead);
                for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
                {
                    if ((*it).s_id == client.s_id)
                        cout << "Found" << endl;
                }
            }
            printf("Msg Clients: %s\n", bufferRead);
            for (IClients i : m_clients)
            {
                cout << "Msg to [ " << i.s_name << " ]" << '\n';
                M = send(i.s_id, bufferRead, N, 0);
            }
        }
    }
    cout << "Threead dead" << endl;
    shutdown(client.s_id, SHUT_RDWR);
    close(client.s_id);
}

void list(IClients client)
{
    char bufferRead[STR_LENGTH];
    int N, M, tam;
    char b;

    while (1)
    {
        N = recv(client.s_id, &b, 1, 0);
        string block;
        if (b == 'L')
        {

            cout << "Live XD" << endl;
            block = complete_digits(m_clients.size(), 1);

            N = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();

            for (IClients x : m_clients)
                block += complete_digits(x.s_name.size(), 1);

            N = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();

            for (IClients x : m_clients)
                block += x.s_name;

            N = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();
        }
    }

    shutdown(client.s_id, SHUT_RDWR);
    close(client.s_id);
}

int main(void)
{
    struct sockaddr_in stSockAddr;
    int SocketFD, n;

    struct sockaddr_in cli_addr;
    socklen_t client;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int)) == -1)
    {
        perror("Setsockopt");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr)) == -1)
    {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(SocketFD, 5) == -1)
    {
        perror("Listen");
        exit(1);
    }

    cout << "░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█░░░█▀█░█▀▀░█▀▀\n░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░█░░░█░█░█░█░▀▀█\n░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀▀▀\n";

    char server_buffer[256];
    char bye_message[] = "exit";
    char client_buffer[STR_LENGTH];
    int tam;
    string block;

    for (;;)
    {
        client = sizeof(struct sockaddr_in);

        int clientFD = accept(SocketFD, (struct sockaddr *)&cli_addr, &client);

        n = recv(clientFD, client_buffer, 1, 0);

        type_message = client_buffer[0];

    
        n = recv(clientFD, client_buffer, 2, 0);
        client_buffer[n] = '\0';
        tam = atoi(client_buffer);

        n = recv(clientFD, client_buffer, tam, 0);
        client_buffer[n] = '\0';

        IClients client(clientFD, string(client_buffer));

        m_clients.push_back(client);

        cout << "New Client Registered [ " << string(client_buffer) << " ] with ClientFD [ " << clientFD << " ]\n";
        cout << "    📤 Sending response\n";

        n = send(clientFD, "S", 1, 0);
        string response = "\t✅ Successful registration";
        string response_size_str = complete_digits(response.size(), 0);
        n = send(clientFD, &(response_size_str.front()), 3, 0);
        n = send(clientFD, &(response.front()), response.size(), 0);

        if (n > 0)
        {
            cout << "    ✅ Response sent\n";
        }

        //thread(chilina, client).detach();
        //thread(broadcast, client).detach();
        // thread(list, client).detach();
    }

    close(SocketFD);
    return 0;
}
