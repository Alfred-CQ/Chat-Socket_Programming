#include "include/client.hpp"
#include "game/tictactoe.hpp"

bool stop_game = false;

Tictactoe* game = nullptr;

char option_send = 'N';

void readMessage(Client* client)
{
    int n;
    char option = '1';

    while (option != 'R')
    {

        n = recv(client->cli_socketFD, &option, 1, 0);

        if (option == 'S')
            client->recv_Server();
        else if (option == 'M')
            client->recv_Message();
        else if (option == 'L')
            client->recv_List();
        else if (option == 'F')
            client->recv_File();
        else if (option == 'G')
        {
            char bufferRead[STR_LENGTH];
            
            n = recv(client->cli_socketFD, bufferRead, 2, 0);
            bufferRead[n] = '\0';

            char player = bufferRead[0];
            
            int size_board = atoi(&bufferRead[1]);
            
            cout << player << "  " << size_board << endl;

            game = new Tictactoe(atoi(&player), size_board);
        }
        else if (option == 'O')
        {
            game->draw_Board(client->cli_nickname, game->board_owner, true);
        }
           else if (option == 'K')
        {
            char bufferRead[STR_LENGTH];
            
            n = recv(client->cli_socketFD, bufferRead, 3, 0);
            bufferRead[n] = '\0';
            string move(bufferRead, 0 ,2);

            game->mark_Board(move, game->current_turn);

            game->current_turn = atoi(&bufferRead[2]);

            game->draw_Board(client->cli_nickname, game->board_owner);
            cout << game->board_owner << " - " << game->current_turn << std::endl;
        }else if (option == 'W')
        {
             char bufferRead[STR_LENGTH];
            game->draw_Board(client->cli_nickname, game->board_owner);
            
            n = recv(client->cli_socketFD, bufferRead, 3, 0);
            bufferRead[n] = '\0';
            int size_message = atoi(bufferRead);

            n = recv(client->cli_socketFD, bufferRead, size_message, 0);
            bufferRead[n] = '\0';
            stop_game = true;
            option_send = '0';
            
            cout << "\n\t    _____________________________\n";
            cout << "\t    |                             |\n";
            cout << "\t    " << bufferRead << '\n';
            cout << "\t    |_____________________________|\n";
        }
            
        option = '1';
    }
    
    shutdown(client->cli_socketFD, SHUT_RDWR);
    close(client->cli_socketFD);
}

int main(int argc, char *argv[])
{
    
    Client myclient(45000, "127.0.0.1");

    string nickname;

    std::thread(readMessage, &myclient).detach();

    system("clear");
    cout << "░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀\n░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀\n░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀\n";

    myclient.send_Option(&option_send);

    cout << " 👥 Write your nickname to get started: ";
    cin >> nickname;

    myclient.set_nickname(nickname);
    myclient.send_Nickname();
    
    sleep(1.5);
    cout << "***** " << nickname << " choose one of the following options *****\n";
    cout << "      [M] Send a message - [B] Broadcast\n";
    cout << "      [L] List of users  - [R] Close session\n";
    cout << "      [F] Send a file\n";
    cout << "      [I] Invite game    - [G] Accept game  - [E] Enter game\n";

    while (option_send != 'R')
    {
        if (option_send != 'E')
        {
            cout << "Your option: ";

            cin >> option_send;

            myclient.send_Option(&option_send);
        }

        if (option_send == 'M')
        {
            string nick_friend, message;

            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;

            cout << "Type your message: ";
            cin >> message;

            myclient.send_Message(nick_friend, message);
        }
        else if (option_send == 'B')
        {
            string message;

            cout << "Write your message: ";
            cin >> message;

            myclient.send_Broadcast(message);
        }
        else if (option_send == 'R')
        {
            string message = " 🏃 " + nickname + " left the chat";

            myclient.send_Broadcast(message);
        }
        else if (option_send == 'F')
        {
            string nick_friend, file_name;

            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;
            cout << "Type file name: ";
            cin >> file_name;

            myclient.send_File(nick_friend, file_name);
        }
        else if (option_send == 'I')
        {
            string nick_friend, message;

            cout << "Enter your friend's nickname to invite: ";
            cin >> nick_friend;

            cout << "Type your invitation message: ";
            cin >> message;

            myclient.send_Message(nick_friend, message);
        }
        else if (option_send == 'G')
        {
            string nick_friend, response_invitation;
            int sizeboard;
            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;

            cout << "Type your response (YES/NO): ";
            cin >> response_invitation;

            myclient.send_Message(nick_friend, response_invitation);
            
            if (response_invitation == "YES")
            {
                cout << "Size of Board (1-9): "; 
                cin >> sizeboard;

                if (sizeboard > 9)
                {
                    cout << "Invalid Board Size\n";
                    continue;
                }

                myclient.send_Invitation(nick_friend, sizeboard);

                option_send = 'E';

            }
        }
        else if (option_send == 'E')
        {
            int n;
            string selected_box;
            string block;
            
            sleep(5);
            cout << "DEPLOY " << game->board_owner << " " <<  game->current_turn << std::endl;
            game->draw_Board(myclient.cli_nickname, game->board_owner);
            while (!stop_game)
            {
                if (game->board_owner == game->current_turn)
                {
                    sleep(1);

                    if (stop_game == true) break;

                    cout << "\n\t       Player [ " << (game->board_owner == 0 ? 1 : 2) << " ] choose a box: ";
                    cin >> selected_box;

                    n = send(myclient.cli_socketFD, "P", 1, 0);
                    n = send(myclient.cli_socketFD, &(selected_box.front()), 2, 0);
                    sleep(1);
                    selected_box.clear();
                }  
            }  
            option_send = '0';
        }

    }

    cout << "Closing chat ...\n *** Finished program *** ";

    shutdown(myclient.cli_socketFD, SHUT_RDWR);
    close(myclient.cli_socketFD);

    return 0;
}
