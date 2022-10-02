#ifndef _CLIENT_HPP_
    #define _CLIENT_HPP_

    #include "config.h"

    class Client
    {
        public:
            /* Variables */
            int cli_socketFD;
            std::string cli_nickname;

            uint cli_port;
            std::string cli_ip;
            struct sockaddr_in cli_stsocketAddr;

            Client(uint port, std::string ip);
            ~Client();

            /* Senders */
            bool send_Option(char* option);
            bool send_Nickname();
            bool send_Message(std::string nickname_friend, std::string message);
            bool send_Broadcast(std::string message);
            bool send_File(std::string nickname_friend, std::string file_name);

            /* Receivers */
            bool recv_Server();
            bool recv_Message();
            bool recv_List();
            bool recv_File();

            /* Setters */
            void set_nickname(std::string nickname);
            
            /* Utils */
            
        private:
        
    };

    
#endif