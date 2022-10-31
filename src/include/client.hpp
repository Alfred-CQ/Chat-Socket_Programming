#ifndef _CLIENT_HPP_
    #define _CLIENT_HPP_

    #include "config.h"
    #include "../game/tictactoe.hpp"

    class Client
    {
        public:
            /* Variables */
            int                 cli_socketFD;
            std::string         cli_nickname;

            uint                cli_port;
            std::string         cli_ip;
            struct sockaddr_in  cli_stsocketAddr;

            Client              (uint port, std::string ip);
            ~Client             ();

            /* Senders */

            bool send_Option                (char* option);
            bool send_Nickname              ();
            bool send_Message               (std::string nickname_friend, std::string message);
            bool send_Broadcast             (std::string message);
            bool send_File                  (std::string nickname_friend, std::string file_name);
            bool send_Invitation            (uint size_board);

            bool send_Server_Notification   (std::string message);
            bool send_Server_Game_Tick      (std::string selected_box);
            /* Receivers */

            bool recv_Message               ();
            bool recv_File                  ();
            bool recv_List                  ();

            bool recv_Server_Notification   ();
            bool recv_Server_Game_Settings  ();
            bool recv_Server_Game_Tick      ();
            bool recv_Server_Game_Winner    ();
            void recv_Server_Game_Inv_Tick  ();

            /* Getters */
            void get_Game_Owner_CurrTurn    (OWNER_TURN& data_Game);

            /* Setters */
            void set_nickname               (std::string nickname);
            
            /* Utils */
            void draw_Board_Game            ();
            void log_out                    ();
            
        private:

            /* Variables */
            Tictactoe* game =               nullptr;
    };

#endif