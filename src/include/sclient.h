#ifndef _SCLIENT_H_
    #define _SCLIENT_H_

        #include "config.h"
        #include "../game/tictactoe.hpp"

        class Tictactoe;

        struct SClients
        {
            int         scli_id, scli_socketFD;
            string      scli_nickname;
            Tictactoe*  scli_board = nullptr;
            bool        scli_available = true;

            SClients(int id, int socketFD, string nickname) :   scli_id(id), 
                                                                scli_socketFD(socketFD),
                                                                scli_nickname(nickname) {}
        };

#endif