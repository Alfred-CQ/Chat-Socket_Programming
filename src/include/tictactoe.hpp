#ifndef _TIC_TAC_TOE_HPP_
#define _TIC_TAC_TOE_HPP_

#include "config.h"

#define SIZE_BOARD 3
#include <vector>

using namespace std;

class Tictactoe
{
public:
    /* Variables */
    int N;
    vector<vector<std::string>> board;
    std::map<string, std::pair<int, int>> boxes_map;
    std::pair<int, int> players_sd;
    string player_avatars[2] = {"🇽", "⭕"};

    int free_boxes = 0;
    int board_owner;
    int current_turn = 0;


    Tictactoe()
    {
        cout << "ALIVE FAKE" << std::endl;
    }

    void setN(int An)
    {
        N = An;
    }

    Tictactoe(int board_owner, int An)
    {
        N = An;
        cout << "ALIVE" << std::endl;
        this->board_owner = board_owner;
        cout << this->board_owner << std::endl;

        for (int i = 0; i < N; ++i)
        {
            vector<string> m(N);
            for (int j = 0; j < N; ++j)
            {
                m.push_back("00");
            }
            board.push_back(m);
        }

        setup_board();
    }

    Tictactoe(int sd_player1, int sd_player2, int An)
    {
        N = An;
        players_sd.first = sd_player1;
        players_sd.second = sd_player2;

        for (int i = 0; i < N; ++i)
        {
            vector<string> m(N);
            for (int j = 0; j < N; ++j)
            {
                m.push_back("00");
            }
            board.push_back(m);
        }

        setup_board();
    }

    ~Tictactoe()
    {
    }

    string complete_digitss(int t, bool type)
    {
        if (type)
        {
            if (t < 10)
                return ('0' + std::to_string(t));
            return std::to_string(t);
        }
        else
        {
            if (t < 10)
                return ("00" + std::to_string(t));
            else if (t < 100)
                return ('0' + std::to_string(t));
            return std::to_string(t);
        }
    }
    void setup_board()
    {
        char number_char[] = "0 ";

        for (int i = 0, k = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j, ++k)
            {
                // number_char[0] = char(k);
                board[i][j] = complete_digitss(k, 1);
                boxes_map.emplace(complete_digitss(k, 1), std::make_pair(i, j));
            }
        }
    }

    void draw_board(bool notification, string nickname, int player)
    {
        system("clear");

        cout << "░▀█▀░▀█▀░█▀▀░░░▀█▀░█▀█░█▀▀░░░▀█▀░█▀█░█▀▀░░░█▀▀░█▀█░█▄█░█▀▀\n░░█░░░█░░█░░░░░░█░░█▀█░█░░░░░░█░░█░█░█▀▀░░░█░█░█▀█░█░█░█▀▀\n░░▀░░▀▀▀░▀▀▀░░░░▀░░▀░▀░▀▀▀░░░░▀░░▀▀▀░▀▀▀░░░▀▀▀░▀░▀░▀░▀░▀▀▀\n\n";

        cout << "\t\t    " << nickname << " 👉 " << player_avatars[player] << std::endl;

        cout << "\t Player " << (current_turn == 0 ? 1 : 2) << " turn"
             << " [ " << player_avatars[current_turn] << " ]"
             << "  -  "
             << "Next turn [ " << player_avatars[!current_turn] << " ]\n\n";
        /*
                        cout << "\t\t         |      |      \n";
                        cout << "\t\t     " << board[0][0] << "  |  " << board[0][1] << "  |  " << board[0][2] << '\n';
                        cout << "\t\t   ______|______|______\n";

                        cout << "\t\t         |      |      \n";
                        cout << "\t\t     " << board[1][0] << "  |  " << board[1][1] << "  |  " << board[1][2] << '\n';
                        cout << "\t\t   ______|______|______\n";

                        cout << "\t\t         |      |      \n";
                        cout << "\t\t     " << board[2][0] << "  |  " << board[2][1] << "  |  " << board[2][2] << '\n';
                        cout << "\t\t         |      |      \n";
        */

        for (int i = 0, k = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j, ++k)
            {
                cout << board[i][j] << "  ";
            }
            cout << endl;
        }

        if (notification)
        {
            cout << "\n\t\t  _____________________\n";
            cout << "\t\t  |                   |\n";
            cout << "\t\t  |  Invalid Move " << player_avatars[player] << "  |\n";
            cout << "\t\t  |___________________|\n";
        }
    }

    bool mark_board(string m, int player_avatar)
    {
        cout << m << " " << player_avatar << " BEFORE IF INSIDE" << std::endl;
        cout << board[0][0] << " " << player_avatars[player_avatar] << " BEFORE IF INSIDE" << std::endl;
        if (board[boxes_map[m].first][boxes_map[m].second] == player_avatars[player_avatar] ||
            board[boxes_map[m].first][boxes_map[m].second] == player_avatars[!player_avatar]

        )
            return false;

        board[boxes_map[m].first][boxes_map[m].second] = player_avatars[player_avatar];

        return true;
    }

    bool check_win(int player_avatar)
    {
        // Rows and cols
        int count_in_rows{0}, count_in_cols{0};
        for (int i = 0; i < N; ++i)
        {
            count_in_rows = 0;
            count_in_cols = 0;
            for (int j = 0; j < N; ++j)
            {
                if (board[i][j] == player_avatars[player_avatar])
                    count_in_rows++;
                if (board[j][i] == player_avatars[player_avatar])
                    count_in_cols++;
            }
            if (count_in_rows == N || count_in_cols == N)
                return true;
        }

        // Diagonals
        int count_in_diag{0}, count_in_invdiag{0};
        for (int i = 0, j = 2; i < N; ++i, --j)
        {
            if (board[i][i] == player_avatars[player_avatar])
                count_in_diag++;
            if (board[i][j] == player_avatars[player_avatar])
                count_in_invdiag++;
        }

        if (count_in_diag == N || count_in_invdiag == N)
            return true;

        return false;
    }

private:
    /* data */
};

#endif
