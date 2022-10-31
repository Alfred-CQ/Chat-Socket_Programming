#ifndef _TIC_TAC_TOE_HPP_
    #define _TIC_TAC_TOE_HPP_

    #include "../include/sclient.h"

    struct SClients;
    class Tictactoe
    {
        public:
            
            /* Variables */
            SClients*             players[2] = { nullptr };
            
            string  player_avatars[2] = {"🇽", "⭕"};
            int     free_boxes = 0, board_owner, current_turn = 0;

            Tictactoe   ();
            Tictactoe   (int board_owner, int size_board);
            Tictactoe   (SClients* player_1, SClients* player_2, int size_board);
           ~Tictactoe   ();

            /* Core */
            void draw_Board     (string nickname, int player, bool notification = false);
            bool mark_Board     (string move, int player);
            bool check_Win      (int player_avatar);

            /* Setters */
            void set_Size_Board (int size_board);
            void set_Board      ();

            /* Getters */
            uint get_Size_Board ();

            /* Utils */
            string complete_digitss(int t, bool type);

        private:
        
            int                         size_board;
            vector<vector<string>>      board;
            map<string, pair<int, int>> boxes_map;
            
    };

#endif
