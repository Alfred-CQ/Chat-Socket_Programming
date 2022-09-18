 /* Client code in C */
 
#include <sys/types.h>
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_LENGTH 256

#include <iostream>
#include <string>
#include <thread>
#include <ctime>    
#include <chrono>

using namespace std;

struct Message
{  
    char s_action;
    int s_size_name, s_size_message;
    string s_message;

    Message(char action, int size_name, int size_message, string message):
        s_action(s_action),
        s_size_name(size_name),
        s_size_message(size_message),
        s_message(message) 
        {}
};


string global_response;

void readMessage(int SocketFD)
{
    char bufferRead[STR_LENGTH];
    int N, size_friend;
    //string name_friend, message;

    while (1)
    {
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
        std::time_t time_message = chrono::system_clock::to_time_t(chrono::system_clock::now());
        cout << "\n\n[ ICM Message from " << name_friend  << "] at " << std::ctime(&time_message) << '\n'
             << " 💬 " << message << '\n';
    }

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}

string complete_digits(int t, bool type)
{
    if (type)
    {
        if (t < 10)
            return ('0' + to_string(t));
        return to_string(t);
    }
    else
    {
        if (t < 10)
            return ("00" + to_string(t));
        else if (t < 100)
            return ('0' + to_string(t));
        return to_string(t);
    }

}

int main(int argc, char *argv[])
{
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
    int n;
 
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
    
    char option = 'Z';
    string nickname, block, nick_friend, message;
    bool chat = 0;

    thread (readMessage, SocketFD).detach();

    cout << "░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀\n░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀\n░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀\n";
    
    cout << "***** Choose one of the following options *****\n";
    cout << "      [N] Setup Nickname - [M] Send a message\n";
    cout << "      [R] Reply messages - [Q] Quit\n";


    while (option != 'Q')
    {
        cout << "Your option: ";
        cin >> option;
        
        if (option == 'N') 
        {
            cout << "Write your nickname: ";
            cin >> nickname;
            block = option + to_string(nickname.size());
            
            n = send(SocketFD, block.c_str(), 3, 0);
            n = send(SocketFD, nickname.c_str(), nickname.size(), 0);

            cout << "\n✅ Successful registration\n\n";
            
            chat = 1;
        }
        else if(option == 'M' && chat)
        {
            cout << "Enter your friend's nickname: ";
            cin >> nick_friend;
            message = "-1";
            while ( message != "chau")
            {
                message.clear(); 
                cout << "Type your message: ";
                cin >> message;

                block = option + complete_digits(nick_friend.size(), 1) + complete_digits(message.size(), 0);
                //cout << "V1 "  << block << endl;
                n = send(SocketFD, block.c_str(), 6, 0);
                block.clear();
            
                block = nick_friend + message;
                 //cout << "V2 "  << block << endl;
                n = send(SocketFD, block.c_str(), nick_friend.size() + message.size(), 0);
                block.clear();
            }

        }
        else if (option == 'R' && chat) 
        {
            while ( message != "chau")
            {
                message.clear(); 
                cout << "Type your reply: ";
                cin >> message;

                block = option + complete_digits(global_response.size(), 1) + complete_digits(message.size(), 0);
                    //cout << "V1 "  << block << endl;
                n = send(SocketFD, block.c_str(), 6, 0);
                block.clear();

                block = global_response + message;
                     //cout << "V2 "  << block << endl;
                n = send(SocketFD, block.c_str(), global_response.size() + message.size(), 0);
                block.clear();
            }
        }
        else if (option == 'C' && chat)
        {
            system("clear");
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
