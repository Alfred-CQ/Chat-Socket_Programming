/* Client code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <ctime>
#include <chrono>
#include <vector>
#include <map>

#include "include/utils.h"

#define STR_LENGTH 256

using namespace std;

map<char, string> actions = {
    {'M', "User message"},
    {'S', "Server message"},
};

string global_response, nickname;

void readMessage(int SocketFD)
{
    int n, size_friend_nick, size_message;
    char option, bufferRead[STR_LENGTH];

    while (option != 'R')
    {

        n = recv(SocketFD, &option, 1, 0);

        time_t time_message = chrono::system_clock::to_time_t(chrono::system_clock::now());

        if (option == 'S')
        {
            n = recv(SocketFD, bufferRead, 3, 0);
            bufferRead[n] = '\0';
            size_message = atoi(bufferRead);

            n = recv(SocketFD, bufferRead, size_message, 0);
            bufferRead[n] = '\0';

            cout << "\n[ 🌱 Server message ]" << bufferRead << " at " << std::ctime(&time_message) << '\n';
        }
        else if (option == 'M')
        {
            n = recv(SocketFD, bufferRead, 5, 0);
            bufferRead[n] = '\0';

            string size_friend_nick_str(bufferRead, 0, 2),
                size_message_str(bufferRead, 2, 3);

            size_friend_nick = atoi(&size_friend_nick_str.front());
            size_message = atoi(&size_message_str.front());

            n = recv(SocketFD, bufferRead, size_friend_nick + size_message, 0);
            
            string nickname_friend(bufferRead, 0, size_friend_nick), message(bufferRead, size_friend_nick, size_message);

            //global_response = name_friend;

            cout << "\n\n[ 📬 Message from " << nickname_friend << " ] at " << std::ctime(&time_message) << '\n'
                 << " 💬 " << message << '\n';
        }
        else if (option == 'L')
        {
            n = recv(SocketFD, bufferRead, 2, 0);
            bufferRead[n] = '\0';

            int clients_online = atoi(bufferRead), total = 0;

            n = recv(SocketFD, bufferRead, clients_online * 2, 0);
            bufferRead[n] = '\0';

            vector<int> sizes_clients;
            sizes_clients.push_back(0);
            
            for (int i = 0; i < n - 1; i += 2)
            {
                string t(bufferRead, i, 2);
                total += atoi(&t.front());
                sizes_clients.push_back(total);
            }

            n = recv(SocketFD, bufferRead, total, 0);
            bufferRead[n] = '\0';
            cout << '\n';
            for (int i = 1; i <= clients_online; ++i)
            {
                string client(bufferRead, sizes_clients[i - 1], sizes_clients[i] - sizes_clients[i - 1]);
                if ( client == nickname)
                    cout <<  "  🟢 " << client << endl;
                else
                    cout <<  "  👤 " << client << endl;
            }
        }
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(AF_INET, SOCK_STREAM, 0); // IPPROTO_TCP
    int n;
    char option = 'N';

    if (-1 == SocketFD)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

    if (0 > Res)
    {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    string block, nick_friend, message;

    thread(readMessage, SocketFD).detach();

    cout << "░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀\n░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀\n░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀\n";

    // Setup Nickname

    n = send(SocketFD, &option, 1, 0);

    cout << " 👥 Write your nickname to get started: ";
    cin >> nickname;
    block = complete_digits(nickname.size(), 1);

    n = send(SocketFD, &(block.front()), block.size(), 0);
    n = send(SocketFD, &(nickname.front()), nickname.size(), 0);

    block.clear();

    sleep(2);
    cout << "***** " << nickname << " choose one of the following options *****\n";
    cout << "      [M] Send a message - [B] Broadcast\n";
    cout << "      [L] List of users  - [R] Close session\n";
    cout << "      [F] Send a file    - [X] Reply message\n";

    while (option != 'R')
    {
        cout << "Your option: ";

        cin >> option;

        n = send(SocketFD, &option, 1, 0);

        if (option == 'M')
        {
            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;

            //message = "-1";
            //while (message != "chau")
            //{
              //  message.clear();

                cout << "Type your message: ";
                cin >> message;

                block = complete_digits(nick_friend.size(), 1) + complete_digits(message.size(), 0);

                
                n = send(SocketFD, block.c_str(), 5, 0);

                block.clear();

                block = nick_friend + message;

                n = send(SocketFD, &(block.front()), block.size(), 0);

                block.clear();
            //}
            message.clear();
        }
        else if (option == 'X')
        {
            while (message != "chau")
            {
                message.clear();
                cout << "Type your reply: ";
                cin >> message;

                block = option + complete_digits(global_response.size(), 1) + complete_digits(message.size(), 0);

                n = send(SocketFD, block.c_str(), 6, 0);
                block.clear();

                block = global_response + message;

                n = send(SocketFD, block.c_str(), global_response.size() + message.size(), 0);
                block.clear();
            }
        }
        else if (option == 'C')
        {
            system("clear");
        }
        else if (option == 'B')
        {
            cout << "Write your message: ";
            cin >> message;
            block = complete_digits(message.size(), 0);

            n = send(SocketFD, &(block.front()), block.size(), 0);
            n = send(SocketFD, &(message.front()), message.size(), 0);

            block.clear();
            message.clear();
        }
        else if (option == 'R')
        {
            message = " 🏃 " + nickname + " left the chat";
            block = complete_digits(message.size(), 0);

            n = send(SocketFD, &(block.front()), block.size(), 0);
            n = send(SocketFD, &(message.front()), message.size(), 0);

            block.clear();
            message.clear();
        }
        block.clear();
        message.clear();
    }

    printf("Closing chat ...\n *** Finished program *** ");

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
