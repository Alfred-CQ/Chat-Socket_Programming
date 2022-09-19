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



string global_response;

void readMessage(int SocketFD)
{
    int n, size_message;
    char option, bufferRead[STR_LENGTH];

    while (1)
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
            int N, size_friend;
            bzero(bufferRead, STR_LENGTH);
            N = recv(SocketFD, bufferRead, STR_LENGTH, 0);
            size_friend = atoi(&bufferRead[0]);
            string name_friend(bufferRead, 1, size_friend), message(bufferRead, size_friend + 1, N - size_friend + 1);

            if (N <= 0)
            {
                perror("ERROR reading registration");
                break;
            }
            global_response = name_friend;

            cout << "\n\n[ ICM Message from " << name_friend << "] at " << std::ctime(&time_message) << '\n'
                 << " 💬 " << message << '\n';
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

    string nickname, block, nick_friend, message;
    bool chat = 0;

    thread(readMessage, SocketFD).detach();

    cout << "░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀\n░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀\n░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀\n";

    // Setup Nickname

    n = send(SocketFD, &option, 1, 0);

    cout << " 👥 Write your nickname to get started: ";
    cin >> nickname;
    block = complete_digits(nickname.size(), 1);

    n = send(SocketFD, &(block.front()), block.size(), 0);
    n = send(SocketFD, &(nickname.front()), nickname.size(), 0);

    chat = 1;
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

        if (option == 'M' && chat)
        {
            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;

            message = "-1";
            while (message != "chau")
            {
                message.clear();

                cout << "Type your message: ";
                cin >> message;

                block = complete_digits(nick_friend.size(), 1) + complete_digits(message.size(), 0);

                n = send(SocketFD, &option, 1, 0);
                n = send(SocketFD, block.c_str(), 5, 0);

                block.clear();

                block = nick_friend + message;

                n = send(SocketFD, &(block.front()), block.size(), 0);

                block.clear();
            }
            message.clear();
        }
        else if (option == 'X' && chat)
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
        else if (option == 'C' && chat)
        {
            system("clear");
        }
        else if (option == 'B' && chat)
        {
            cout << "Write your message: ";
            cin >> message;
            block = complete_digits(message.size(), 0);

            n = send(SocketFD, &(block.front()), block.size(), 0);
            n = send(SocketFD, &(message.front()), message.size(), 0);

            chat = 1;
            block.clear();
            message.clear();
        }
        else if (option == 'L')
        {
            
            char bufferRead[STR_LENGTH];

            int N = recv(SocketFD, bufferRead, 2, 0);

            bufferRead[N] = '\0';

            int tam = atoi(bufferRead);

            N = recv(SocketFD, bufferRead, tam * 2, 0);
            bufferRead[N] = '\0';

            vector<int> sizes_clients;
            sizes_clients.push_back(0);
            int total = 0;
            for (int i = 0; i < N - 1; i += 2)
            {
                string t(bufferRead, i, 2);
                total += atoi(&t.front());
                sizes_clients.push_back(total);
            }

            N = recv(SocketFD, bufferRead, total, 0);
            bufferRead[N] = '\0';

            for (int i = 1; i <= tam; ++i)
            {
                string temp(bufferRead, sizes_clients[i - 1], sizes_clients[i] - sizes_clients[i - 1]);
                cout << temp << endl;
            }
        }
        else
        {
            cout << "\n ❌ Please register first\n\n";
        }
        block.clear();
        message.clear();
    }
   
    printf("Closing chat ...\n *** Finished program *** ");

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
