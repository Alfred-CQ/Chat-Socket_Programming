/* Server code in C */
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <fstream>

#include "include/server.hpp"
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
                             {'A', "Actions"},
                             {'U', "User notification"}};

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
    while (action != 'R')
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
        else if (action == 'B' || action == 'R')
        {
            n = recv(client.s_id, client_buffer, 3, 0);
            client_buffer[n] = '\0';

            size_message = atoi(client_buffer);

            n = recv(client.s_id, client_buffer, size_message, 0);
            client_buffer[n] = '\0';

            string message_cstr(client_buffer, 0, size_message);

            if (action == 'B')
                cout << "    🛰 Broadcast launched 🛰\n";
            else
                cout << "    🏃 Exit Broadcast launched 🏃\n";

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

            if (action == 'B')
            {
                n = send(client.s_id, "S", 1, 0);
                string response = "\t🛰  ✅ Broadcast completed";
                string response_size_str = complete_digits(response.size(), 0);
                n = send(client.s_id, &(response_size_str.front()), 3, 0);
                n = send(client.s_id, &(response.front()), response.size(), 0);
                cout << "    🛰 Broadcast close 🛰\n";
            }
            else
            {
                for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
                {
                    if ((*it).s_id == client.s_id)
                        m_clients.erase(it);
                }

                cout << "    🏃 Exit Broadcast close 🏃\n";
            }

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
        else if (action == 'F')
        {
            int size_filename, size_file, bytes_per_packet = 100;

            n = recv(client.s_id, client_buffer, 7, 0);
            client_buffer[n] = '\0';

            string size_friend_nick_str(client_buffer, 0, 2),
                size_filename_str(client_buffer, 2, 2),
                size_file_str(client_buffer, 4, 3);

            size_friend_nick = atoi(&size_friend_nick_str.front());
            size_filename = atoi(&size_filename_str.front());
            size_file = atoi(&size_file_str.front());

            n = recv(client.s_id, client_buffer, size_friend_nick + size_filename, 0);
            client_buffer[n] = '\0';

            string nickname_friend(client_buffer, 0, size_friend_nick),
                filename(client_buffer, size_friend_nick, size_filename);

            // cout << "NN: " << nickname_friend << " " << filename << endl;

            int i;
            for (i = 0; i < m_clients.size() && sd_friend == -1; ++i)
            {
                if (nickname_friend == m_clients[i].s_name)
                    sd_friend = m_clients[i].s_id;
            }

            n = send(sd_friend, &action, 1, 0);

            block = size_filename_str + size_file_str;
            n = send(sd_friend, &(block.front()), block.size(), 0);
            n = send(sd_friend, &(filename.front()), filename.size(), 0);

            while (size_file > 0)
            {
                n = recv(client.s_id, client_buffer, bytes_per_packet, 0);
                client_buffer[n] = '\0';

                n = send(sd_friend, client_buffer, bytes_per_packet, 0);

                size_file -= bytes_per_packet;

                if (size_file < bytes_per_packet)
                    bytes_per_packet = size_file;
            }

            block.clear();

            n = send(client.s_id, "S", 1, 0);
            string response = "    ✅ File send successfully";
            string response_size_str = complete_digits(response.size(), 0);

            n = send(client.s_id, &(response_size_str.front()), 3, 0);
            n = send(client.s_id, &(response.front()), response.size(), 0);

            cout << response << endl;

            n = send(sd_friend, "S", 1, 0);
            response = "    📑 You have file " + filename;
            response_size_str = complete_digits(response.size(), 0);

            n = send(sd_friend, &(response_size_str.front()), 3, 0);
            n = send(sd_friend, &(response.front()), response.size(), 0);

            cout << "🔔 👤 Notified receiver " << endl;
        }
        else if (action == 'U')
        {
            n = recv(client.s_id, client_buffer, 3, 0);
            client_buffer[n] = '\0';
            size_message = atoi(client_buffer);

            n = recv(client.s_id, client_buffer, size_message, 0);
            client_buffer[n] = '\0';
            time_t time_message = chrono::system_clock::to_time_t(chrono::system_clock::now());
            cout << "\n🔔 Client notification -> " << client_buffer << " at " << std::ctime(&time_message) << '\n';
        }
    }

    shutdown(client.s_id, SHUT_RDWR);
    close(client.s_id);
}

int main(void)
{
    Server myServer(45000, "127.0.01");  
    
    int n;
    struct sockaddr_in cli_addr;
    socklen_t client;

    cout << "░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄░░░█░░░█▀█░█▀▀░█▀▀\n░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄░░░█░░░█░█░█░█░▀▀█\n░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀▀▀\n";

    char server_buffer[256];
    char bye_message[] = "exit";
    char client_buffer[STR_LENGTH];
    int tam;
    string block;

    for (;;)
    {
        client = sizeof(struct sockaddr_in);

        int clientFD = accept(myServer.server_SocketFD, (struct sockaddr *)&cli_addr, &client);

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
    }

    close(myServer.server_SocketFD);
    return 0;
}
