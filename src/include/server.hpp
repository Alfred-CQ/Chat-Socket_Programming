#ifndef _SERVER_HPP_
    #define _SERVER_HPP_

    #include "sclient.h"
    class Server
    {
        public:
        
            int                 server_SocketFD;
            /* Server Registers*/
            vector<SClients*>   s_clients;
            vector<Tictactoe*>  s_games;

            Server(uint port, std::string ip);
            ~Server();

            /* */
            SClients*  register_client      ();
            SClients*  message_clients      (SClients* client);
            Tictactoe* make_game            (SClients* player_1, SClients* player_2);
            bool       update_Local_Games    (SClients* client);

            /* Senders */
            bool send_broadcast             (SClients *client, bool exit = false);
            bool send_list_clients          (SClients *client);
            bool send_file                  (SClients *client);
            bool send_Notification          (string message, int client_FD);

            bool send_Game_Settings         (string settings, SClients *client);
            bool send_Game_Tick             (Tictactoe* game, string move);
            bool send_Game_Winner           (Tictactoe* game, int last_turn);
            
             /* Receivers */
            bool recv_Message               ();
            bool recv_notification          (SClients *client);

        private:
            
            /* Variables */
            // Server Connection
            uint                server_port;
            std::string         server_ip;

            struct sockaddr_in  server_SockAddr;
            
            // Clients Connection
            struct sockaddr_in  cli_addr;
            socklen_t           client = sizeof(struct sockaddr_in);

            // Server
            uint                server_clients = 0;

            // Consts 
            const int   bytes_per_packet = 100;
    };

#endif
