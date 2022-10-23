#include "include/client.hpp"
#include "include/tictactoe.hpp"
#include <string.h>

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
    cout << "      [F] Send a file    - [G] Accept game\n";

    while (option_send != 'R')
    {
        cout << "Your option: ";

        cin >> option_send;

        myclient.send_Option(&option_send);

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

    }

    cout << "Closing chat ...\n *** Finished program *** ";

    shutdown(myclient.cli_socketFD, SHUT_RDWR);
    close(myclient.cli_socketFD);

    return 0;
}
