/* Server code in C++ */

#include "include/server.hpp"

Server myServer(45000, "127.0.01");  

void manager(SClients* client)
{
    int n;
    char action = '1';
    int size_board;

    Tictactoe* game = nullptr;

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
            myServer.message_clients(client);   

            srand(time(NULL));
            int bytes_received{0}, size_friend_nick, index_sd_friend{0}, sd_friend{-1};
            char client_buffer[STR_LENGTH];

            bytes_received = recv(client->scli_socketFD, client_buffer, 3, 0);
            client_buffer[bytes_received] = '\0';

            string size_friend_nick_str(client_buffer, 0, 2),
                    size_board_str(client_buffer, 2, 1);
            size_friend_nick = atoi(&size_friend_nick_str.front());
            size_board = atoi(&size_board_str.front());
            
            bytes_received = recv(client->scli_socketFD, client_buffer, size_friend_nick, 0);
            client_buffer[bytes_received] = '\0';

            string nickname_friend(client_buffer, 0, size_friend_nick);

            for (index_sd_friend = 0; index_sd_friend < myServer.s_clients.size(); ++index_sd_friend)
            {
                if (nickname_friend == myServer.s_clients[index_sd_friend]->scli_nickname)
                {
                    sd_friend = myServer.s_clients[index_sd_friend]->scli_socketFD;
                    break;
                }
            }

            game = new Tictactoe(client->scli_socketFD, sd_friend, size_board);
            client->scli_board = game;
            myServer.s_clients[index_sd_friend]->scli_board = game;

            client->scli_available = false;
            myServer.s_clients[index_sd_friend]->scli_available = false;

            myServer.s_games.push_back(game);
            int player = rand() % 2;

            myServer.send_notification("\t✅ Successful regist with turn " + std::to_string((player == 0 ? 1 : 2)) + " " + game->player_avatars[player], client->scli_socketFD);
            
            string block_config = std::to_string(player) + size_board_str;
            
            n = send(client->scli_socketFD, "G", 1, 0);
            n = send(client->scli_socketFD, &(block_config.front()), 2, 0);

            player = (player + 1) % 2;
            myServer.send_notification("\t✅ Successful registration with turn " + std::to_string((player == 0 ? 1 : 2)) + " " + game->player_avatars[player], sd_friend);
            
            block_config.clear();
            block_config = std::to_string(player) + size_board_str;

            n = send(sd_friend, "G", 1, 0);
            n = send(sd_friend, &(block_config.front()), 2, 0);
        }
        else if (action == 'P')
        {
            bool notification;
            int temp = client->scli_board->current_turn;
            char client_buffer[STR_LENGTH];

            n = recv(client->scli_socketFD, client_buffer, 2, 0);
            client_buffer[n] = '\0';
            string move(client_buffer, 0, 2);

            if (!client->scli_board->mark_Board(move, client->scli_board->current_turn))
            {
                notification = true;
                n = send(client->scli_socketFD, "O", 1, 0);
                continue;
            }
            client_buffer[n + 1] = '\0';
            client->scli_board->current_turn = (client->scli_board->current_turn + 1) % 2;
            client_buffer[n] = char(client->scli_board->current_turn + 48);

            cout << "    🛰 Broadcast launched 🛰\n";

            n = send(client->scli_board->players_sd.first, "K", 1, 0);
            n = send(client->scli_board->players_sd.first, client_buffer, 3, 0);


            cout << "     👤 Send to [ " << client->scli_board->players_sd.first << " ]" << " Movement: " << client_buffer << '\n';

            n = send(client->scli_board->players_sd.second, "K", 1, 0);
            n = send(client->scli_board->players_sd.second, client_buffer, 3, 0);

            cout << "     👤 Send to [ " << client->scli_board->players_sd.second << " ]" << " Movement: " << client_buffer << '\n';
            cout << "    🛰 Broadcast close 🛰\n";

            client->scli_board->free_boxes++;


            string end_game_message;

            if ( client->scli_board->free_boxes == size_board*size_board && client->scli_board->check_Win(temp))
                end_game_message =  "|  Draw game 🇽 🤝 ⭕    |";
            else
                end_game_message = "| Winner player  " + std::to_string((temp == 0 ? 2 : 1)) + " " + client->scli_nickname + " " + client->scli_board->player_avatars[temp] + "🏅|";
    
            string end_game_size_str = "0" + std::to_string(end_game_message.size());//complete_digits(end_game_message.size(), 0);
   
            if ( client->scli_board->free_boxes ==  size_board * size_board|| client->scli_board->check_Win(temp))
            {
                
                    n = send(client->scli_board->players_sd.first, "W", 1, 0);
                    n = send(client->scli_board->players_sd.first, &(end_game_size_str.front()), 3, 0);
                    n = send(client->scli_board->players_sd.first, &(end_game_message.front()), end_game_message.size(), 0);
                    cout << "     👤 Send to [ " << client->scli_board->players_sd.first << " ]" << '\n';
                    cout << "Message: " << client_buffer << endl;      

                    n = send(client->scli_board->players_sd.second, "W", 1, 0);
                    n = send(client->scli_board->players_sd.second, &(end_game_size_str.front()), 3, 0);
                    n = send(client->scli_board->players_sd.second, &(end_game_message.front()), end_game_message.size(), 0);
                    cout << "     👤 Send to [ " << client->scli_board->players_sd.second << " ]" << '\n';
                    cout << "Message: " << client_buffer << endl;
                    for (int k = 0; k < myServer.s_clients.size(); ++k)
                    {
                        if (client->scli_board->players_sd.first == myServer.s_clients[k]->scli_socketFD)
                            myServer.s_clients[k]->scli_available = true;
                        else if (client->scli_board->players_sd.second == myServer.s_clients[k]->scli_socketFD)
                            myServer.s_clients[k]->scli_available = true;
                    }
                    
            }
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
