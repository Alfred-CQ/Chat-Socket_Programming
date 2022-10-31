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

    SClients *client = new SClients(server_clients, client_FD, string(client_buffer));

    s_clients.push_back(client);

    cout << "New Client Registered [ " << string(client_buffer) << " ] with ClientFD [ " << client_FD << " ]\n";
    cout << "    📤 Sending response\n";

    if (send_Notification("\t✅ Successful registration", client_FD))
        cout << "    ✅ Response sent\n";

    server_clients++;

    return client;
}

bool Server::send_Notification(string message, int clientFD)
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
                bytes_send = send((*it)->scli_socketFD, "R", 1, 0);
                s_clients.erase(it);
                break;
            }
        }

        cout << "    🏃 Exit Broadcast close 🏃\n";
    }
    else
    {
        send_Notification("\t🛰  ✅ Broadcast completed", client->scli_socketFD);
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
    send_Notification(response, client->scli_socketFD);
    cout << response << endl;

    response.clear();

    response = "    📑 You have file " + filename;
    send_Notification(response, sd_friend);

    cout << "🔔 👤 Notified receiver " << endl;

    return 1;
}

bool Server::send_Game_Settings(string settings, SClients *client)
{
    int bytes_send{0};

    bytes_send = send(client->scli_socketFD, "G", 1, 0);
    bytes_send = send(client->scli_socketFD, &(settings.front()), 2, 0);

    return 1;
}

bool Server::send_Game_Tick(Tictactoe *game, string move)
{
    int bytes_send{0};

    game->current_turn = (game->current_turn + 1) % 2;

    string tick = move + std::to_string(game->current_turn);

    cout << "    🛰 Update Game 🛰\n";

    for (SClients *player : game->players)
    {
        bytes_send = send(player->scli_socketFD, "T", 1, 0);
        bytes_send = send(player->scli_socketFD, &(tick.front()), 3, 0);

        cout << "     👤 Send to [ " << player->scli_socketFD << " ]"
             << " Movement: " << tick << '\n';
    }

    cout << "    🛰 Update close 🛰\n";

    game->free_boxes++;

    return 1;
}

bool Server::send_Game_Winner(Tictactoe *game, int last_turn)
{
    string end_game_message, end_game_size_str;
    int bytes_send{0}, total_boxes = game->get_Size_Board() * game->get_Size_Board();

    if (game->free_boxes == total_boxes)
        end_game_message = "|  Draw game 🇽 🤝 ⭕    |";
    else
        end_game_message = "| Winner player  " + std::to_string((last_turn == 0 ? 1 : 2)) + " " + game->players[last_turn]->scli_nickname + " " + game->player_avatars[last_turn] + "🏅|";

    end_game_size_str = "0" + std::to_string(end_game_message.size());

    cout << "    🛰 Sending Winner Game 🛰\n";

    for (SClients *player : game->players)
    {
        bytes_send = send(player->scli_socketFD, "W", 1, 0);
        bytes_send = send(player->scli_socketFD, &(end_game_size_str.front()), 3, 0);
        bytes_send = send(player->scli_socketFD, &(end_game_message.front()), end_game_message.size(), 0);
        cout << "     👤 Send to [ " << player->scli_socketFD << " ]" << '\n';
        cout << "Message: " << end_game_message << endl;
        player->scli_available = true;
    }

    cout << "    🛰 Sending close 🛰\n";

    return 1;
}

SClients *Server::message_clients(SClients *client)
{
    SClients *scli_friend = nullptr;

    int bytes_received{0}, bytes_send{0}, size_friend_nick, size_message;
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 5, 0);
    client_buffer[bytes_received] = '\0';

    string size_friend_nick_str(client_buffer, 0, 2),
        size_message_str(client_buffer, 2, 3);

    size_friend_nick = atoi(&size_friend_nick_str.front());
    size_message = atoi(&size_message_str.front());

    bytes_received = recv(client->scli_socketFD, client_buffer, size_friend_nick + size_message, 0);
    client_buffer[bytes_received] = '\0';

    string nickname_friend(client_buffer, 0, size_friend_nick),
        message(client_buffer, size_friend_nick, size_message);

    for (int index_clients = 0; index_clients < s_clients.size(); ++index_clients)
    {
        if (nickname_friend == s_clients[index_clients]->scli_nickname)
        {
            scli_friend = s_clients[index_clients];
            break;
        }
    }

    if (!scli_friend)
    {
        send_Notification("    ❌ friend's nickname not found", client->scli_socketFD);
        cout << "    ❌ friend's nickname not found" << endl;
        return 0;
    }
    else if (!scli_friend->scli_available)
    {
        send_Notification("    🎮 friend's in game ", client->scli_socketFD);
        cout << "     🎮 friend's in game " << endl;
        return 0;
    }

    bytes_send = send(scli_friend->scli_socketFD, "M", 1, 0);

    string block = complete_digits(client->scli_nickname.size(), 1) + complete_digits(message.size(), 0);

    bytes_send = send(scli_friend->scli_socketFD, &(block.front()), 5, 0);

    block.clear();

    block = client->scli_nickname + message;

    bytes_send = send(scli_friend->scli_socketFD, &(block.front()), block.size(), 0);

    cout << client->scli_nickname << " 📨 " << nickname_friend << endl;

    return scli_friend;
}

Tictactoe *Server::make_game(SClients *player_1, SClients *player_2)
{
    srand(time(NULL));
    int bytes_received{0}, size_friend_nick;
    char client_buffer[STR_LENGTH];

    bytes_received = recv(player_1->scli_socketFD, client_buffer, 1, 0);
    client_buffer[bytes_received] = '\0';

    string size_board_str(client_buffer, 0, 1);

    int size_board = atoi(&size_board_str.front());

    Tictactoe *game = new Tictactoe(player_1, player_2, size_board);

    player_1->scli_board = game;
    player_2->scli_board = game;

    player_1->scli_available = false;
    player_2->scli_available = false;

    s_games.push_back(game);

    int player = rand() % 2;

    send_Notification("\t✅ Successful regist with turn " +
                          std::to_string((player == 0 ? 1 : 2)) +
                          " " +
                          game->player_avatars[player],
                      player_1->scli_socketFD);

    string block_config = std::to_string(player) + size_board_str;

    send_Game_Settings(block_config, player_1);

    player = (player + 1) % 2;

    send_Notification("\t✅ Successful registration with turn " +
                          std::to_string((player == 0 ? 1 : 2)) +
                          " " +
                          game->player_avatars[player],
                      player_2->scli_socketFD);

    block_config = std::to_string(player) + size_board_str;

    send_Game_Settings(block_config, player_2);

    return game;
}

bool Server::update_Local_Games(SClients *client)
{
    int bytes_received{0}, bytes_send{0}, last_turn = client->scli_board->current_turn;
    char client_buffer[STR_LENGTH];

    bytes_received = recv(client->scli_socketFD, client_buffer, 2, 0);
    client_buffer[bytes_received] = '\0';
    string move(client_buffer, 0, 2);

    if (!client->scli_board->mark_Board(move, client->scli_board->current_turn))
    {
        bytes_send = send(client->scli_socketFD, "H", 1, 0);
        return 0;
    }

    send_Game_Tick(client->scli_board, move);

    int total_boxes = client->scli_board->get_Size_Board() * client->scli_board->get_Size_Board();

    if (client->scli_board->free_boxes == total_boxes || client->scli_board->check_Win(last_turn))
        send_Game_Winner(client->scli_board, last_turn);
    
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

    time_t time_message = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    cout << "\n🔔 Client notification -> " << client_buffer << " at " << std::ctime(&time_message) << '\n';

    return 1;
}