#include "../include/server.hpp"
#include "../include/utils.h"

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

SClients *Server::register_client()
{
    int bytes_received{0}, client_FD = accept(server_SocketFD, (struct sockaddr *)&cli_addr, &client);
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client_FD, client_buffer, 1, 0);
    bytes_received = recv(client_FD, client_buffer, 2, 0);
    client_buffer[bytes_received] = '\0';

    int size_nickaname = atoi(client_buffer);

    bytes_received = recv(client_FD, client_buffer, size_nickaname, 0);
    client_buffer[bytes_received] = '\0';

    SClients *client = new SClients(client_FD, string(client_buffer));

    s_clients.push_back(client);

    cout << "New Client Registered [ " << string(client_buffer) << " ] with ClientFD [ " << client_FD << " ]\n";
    cout << "    📤 Sending response\n";

    if (send_notification("\t✅ Successful registration", client_FD))
        cout << "    ✅ Response sent\n";

    return client;
}

bool Server::send_notification(string message, int clientFD)
{
    int bytes_send{0};
    bytes_send = send(clientFD, "S", 1, 0);

    string message_size_str = complete_digits(message.size(), 0);

    bytes_send = send(clientFD, &(message_size_str.front()), 3, 0);
    bytes_send = send(clientFD, &(message.front()), message.size(), 0);

    return 1;
}

bool Server::send_broadcast(SClients *client, bool exit)
{
    int bytes_received{0}, bytes_send{0}, size_message;
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 3, 0);
    client_buffer[bytes_received] = '\0';

    size_message = atoi(client_buffer);

    bytes_received = recv(client->scli_socketFD, client_buffer, size_message, 0);
    client_buffer[bytes_received] = '\0';

    string message_cstr(client_buffer, 0, size_message);

    if (exit)
        cout << "    🏃 Exit Broadcast launched 🏃\n";
    else
        cout << "    🛰 Broadcast launched 🛰\n";

    for (auto i : s_clients)
    {
        if (i->scli_socketFD != client->scli_socketFD && i->scli_available)
        {
            bytes_send = send(i->scli_socketFD, "M", 1, 0);

            string block = complete_digits(client->scli_nickname.size(), 1) + complete_digits(size_message, 0);
            bytes_send = send(i->scli_socketFD, &(block.front()), 5, 0);

            cout << "     👤 Send to [ " << i->scli_nickname << " ]" << '\n';

            block.clear();

            block = client->scli_nickname + message_cstr;

            bytes_send = send(i->scli_socketFD, &(block.front()), block.size(), 0);
        }
    }

    if (exit)
    {
        for (auto it = s_clients.begin(); it != s_clients.end(); ++it)
        {
            if ((*it)->scli_socketFD == client->scli_socketFD)
            {
                s_clients.erase(it);
                break;
            }
        }

        cout << "    🏃 Exit Broadcast close 🏃\n";
    }
    else
    {
        send_notification("\t🛰  ✅ Broadcast completed", client->scli_socketFD);
        cout << "    🛰 Broadcast close 🛰\n";
    }

    return true;
}

bool Server::send_list_clients(SClients *client)
{
    int bytes_send{0}, size_message;
    char client_buffer[STR_LENGTH];

    bytes_send = send(client->scli_socketFD, "L", 1, 0);

    string block = complete_digits(s_clients.size(), 1);

    bytes_send = send(client->scli_socketFD, &(block.front()), block.size(), 0);

    block.clear();

    for (auto client_p : s_clients)
        block += complete_digits(client_p->scli_nickname.size(), 1);

    bytes_send = send(client->scli_socketFD, &(block.front()), block.size(), 0);

    block.clear();

    for (auto client_p : s_clients)
        block += client_p->scli_nickname;

    bytes_send = send(client->scli_socketFD, &(block.front()), block.size(), 0);

    cout << " 📝 Client list printed" << endl;

    return 1;
}

bool Server::send_file(SClients *client)
{
    int bytes_received{0}, bytes_send{0}, size_friend_nick, size_filename, size_file, idx_client, sd_friend{-1}, packet{bytes_per_packet};
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 7, 0);
    client_buffer[bytes_received] = '\0';

    string size_friend_nick_str(client_buffer, 0, 2),
        size_filename_str(client_buffer, 2, 2),
        size_file_str(client_buffer, 4, 3);

    size_friend_nick = atoi(&size_friend_nick_str.front());
    size_filename = atoi(&size_filename_str.front());
    size_file = atoi(&size_file_str.front());

    bytes_received = recv(client->scli_socketFD, client_buffer, size_friend_nick + size_filename, 0);
    client_buffer[bytes_received] = '\0';

    string nickname_friend(client_buffer, 0, size_friend_nick),
        filename(client_buffer, size_friend_nick, size_filename);

    for (idx_client = 0; idx_client < s_clients.size(); ++idx_client)
    {
        if (nickname_friend == s_clients[idx_client]->scli_nickname)
        {
            sd_friend = s_clients[idx_client]->scli_socketFD;
            break;
        }
    }

    bytes_send = send(sd_friend, "F", 1, 0);

    string block = size_filename_str + size_file_str;
    bytes_send = send(sd_friend, &(block.front()), block.size(), 0);
    bytes_send = send(sd_friend, &(filename.front()), filename.size(), 0);

    while (size_file > 0)
    {
        bytes_received = recv(client->scli_socketFD, client_buffer, packet, 0);
        client_buffer[bytes_received] = '\0';

        bytes_send = send(sd_friend, client_buffer, packet, 0);

        size_file -= packet;

        if (size_file < packet)
            packet = size_file;
    }

    string response = "    ✅ File send successfully";
    send_notification(response, client->scli_socketFD);
    cout << response << endl;

    response.clear();

    response = "    📑 You have file " + filename;
    send_notification(response, sd_friend);

    cout << "🔔 👤 Notified receiver " << endl;

    return 1;
}

bool Server::message_clients(SClients *client)
{
    int bytes_received{0}, bytes_send{0}, size_friend_nick, size_message, index_sd_friend{0}, sd_friend{-1};
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 5, 0);
    client_buffer[bytes_received] = '\0';

    string size_friend_nick_str(client_buffer, 0, 2),
        size_message_str(client_buffer, 2, 3);

    size_friend_nick = atoi(&size_friend_nick_str.front());
    size_message = atoi(&size_message_str.front());

    bytes_received = recv(client->scli_socketFD, client_buffer, size_friend_nick + size_message, 0);
    client_buffer[bytes_received] = '\0';
    cout << " [ SERVER ] " << size_friend_nick << " - " << size_message << "\n";

    string nickname_friend(client_buffer, 0, size_friend_nick),
        message(client_buffer, size_friend_nick, size_message);

    for (index_sd_friend = 0; index_sd_friend < s_clients.size(); ++index_sd_friend)
    {
        if (nickname_friend == s_clients[index_sd_friend]->scli_nickname)
        {
            sd_friend = s_clients[index_sd_friend]->scli_socketFD;
            break;
        }
    }

    if (sd_friend == -1)
    {
        send_notification("    ❌ friend's nickname not found", client->scli_socketFD);
        cout << "    ❌ friend's nickname not found" << endl;
        return 0;
    }
    else if (!s_clients[index_sd_friend]->scli_available)
    {
        cout << "{ sd_friend } " << sd_friend << endl;
        send_notification("    🎮 friend's in game ", client->scli_socketFD);
        cout << "     🎮 friend's in game " << endl;
        return 0;
    }

    bytes_send = send(sd_friend, "M", 1, 0);

    string block = complete_digits(client->scli_nickname.size(), 1) + complete_digits(message.size(), 0);

    bytes_send = send(sd_friend, &(block.front()), 5, 0);

    block.clear();

    block = client->scli_nickname + message;

    bytes_send = send(sd_friend, &(block.front()), block.size(), 0);

    cout << client->scli_nickname << " 📨 " << nickname_friend << endl;

    return 1;
}

bool Server::recv_notification(SClients *client)
{
    int bytes_received{0}, size_message;
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 3, 0);
    client_buffer[bytes_received] = '\0';
    size_message = atoi(client_buffer);

    bytes_received = recv(client->scli_socketFD, client_buffer, size_message, 0);
    client_buffer[bytes_received] = '\0';

    time_t time_message = chrono::system_clock::to_time_t(chrono::system_clock::now());
    cout << "\n🔔 Client notification -> " << client_buffer << " at " << std::ctime(&time_message) << '\n';

    return 1;
}