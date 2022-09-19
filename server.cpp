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
    int n, m, size_friend_nick, size_message;
    char action, client_buffer[STR_LENGTH];
    string block;
    /// POLL IN
    while (1)
    {
        int sd_friend = -1;

        n = recv(client.s_id, &action, 1, 0);

        if (action == 'M')
        {
            n = recv(client.s_id, client_buffer, 5, 0);
            client_buffer[n] = '\0';

            string size_friend_nick_str(client_buffer, 0, 2),
                size_message_str(client_buffer, 2, 3);

            size_friend_nick = atoi(&size_friend_nick_str.front());
            size_message = atoi(&size_message_str.front());

            n = recv(client.s_id, client_buffer, size_friend_nick + size_message, 0);
            client_buffer[n] = '\0';

            string nickname_friend(client_buffer, 0, size_friend_nick),
                message(client_buffer, size_friend_nick, size_message + 1);

            int i;
            for (i = 0; i < m_clients.size() && sd_friend == -1; ++i)
            {
                if (nickname_friend == m_clients[i].s_name)
                    sd_friend = m_clients[i].s_id;
            }

            if (sd_friend == -1)
            {
                n = send(client.s_id, "S", 1, 0);
                string response = "    ❌ friend's nickname not found";
                string response_size_str = complete_digits(response.size(), 0);

                n = send(client.s_id, &(response_size_str.front()), 3, 0);
                n = send(client.s_id, &(response.front()), response.size(), 0);

                cout << response << endl;
                continue;
            }

            n = send(sd_friend, &action, 1, 0);

            block = complete_digits(client.s_name.size(), 1) + complete_digits(message.size(), 0);

            n = send(sd_friend, &(block.front()), 5, 0);

            block.clear();

            block = client.s_name + message;

            n = send(sd_friend, &(block.front()), block.size(), 0);

            cout << client.s_name << " 📨 " << nickname_friend << endl;

            block.clear();
        }
        else if (action == 'B')
        {
            n = recv(client.s_id, client_buffer, 3, 0);
            client_buffer[n] = '\0';

            size_message = atoi(client_buffer);

            n = recv(client.s_id, client_buffer, size_message, 0);
            client_buffer[n] = '\0';

            string message_cstr(client_buffer, 0, size_message);

            cout << "    🛰 Broadcast launched 🛰\n";
            for (IClients i : m_clients)
            {
                if (i.s_id != client.s_id)
                {
                    n = send(i.s_id, "M", 1, 0);

                    block = complete_digits(client.s_name.size(), 1) + complete_digits(size_message, 0);
                    n = send(i.s_id, &(block.front()), 5, 0);

                    cout << "     👤 Send to [ " << i.s_name << " ]" << '\n';

                    block.clear();

                    block = client.s_name + message_cstr;

                    n = send(i.s_id, &(block.front()), block.size(), 0);
                }
            }

            n = send(client.s_id, "S", 1, 0);
            string response = "\t🛰  ✅ Broadcast completed";
            string response_size_str = complete_digits(response.size(), 0);
            n = send(client.s_id, &(response_size_str.front()), 3, 0);
            n = send(client.s_id, &(response.front()), response.size(), 0);
            cout << "    🛰 Broadcast close 🛰\n";

            block.clear();
        }
        else if (action == 'L')
        {
            n = send(client.s_id, "L", 1, 0);

            block = complete_digits(m_clients.size(), 1);

            n = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();

            for (IClients x : m_clients)
                block += complete_digits(x.s_name.size(), 1);

            n = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();

            for (IClients x : m_clients)
                block += x.s_name;

            n = send(client.s_id, &(block.front()), block.size(), 0);

            block.clear();

            cout << " 📝 Client list printed" << endl;
        }
    }

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

        thread(chilina, client).detach();
        //  thread(list, client).detach();
    }

    close(SocketFD);
    return 0;
}
