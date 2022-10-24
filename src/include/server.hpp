#ifndef _SERVER_HPP_
    #define _SERVER_HPP_

    #include "config.h"

    #include "tictactoe.hpp"

    struct SClients
    {
        int         scli_socketFD;
        string      scli_nickname;
        Tictactoe*  scli_board = nullptr;
        bool        scli_available = true;

        SClients(int socketFD, string nickname) :   scli_socketFD(socketFD),
                                                    scli_nickname(nickname) {}
    };

    class Server
    {
        public:
            /* Server Connection*/
            uint        server_port;
            std::string server_ip;
            int         server_SocketFD;
            struct sockaddr_in server_SockAddr;
            
            /* Clients Connection*/
            struct sockaddr_in cli_addr;
            socklen_t          client = sizeof(struct sockaddr_in);

            /* Server Registers*/
            vector<SClients*>   s_clients;
            vector<Tictactoe*>  s_games;

            Server(uint port, std::string ip);
            ~Server();

            /* */
            SClients*   register_client();
            bool        message_clients(SClients* client);

            /* Senders */
            bool send_broadcast(SClients *client, bool exit = false);
            bool send_list_clients(SClients *client);
            bool send_file(SClients *client);
            bool send_notification(string message, int client_FD);
            
             /* Receivers */
            bool recv_Message();
            bool recv_notification(SClients *client);

        private:
            /* data */  
            int bytes_per_packet = 100;
    };

#endif
