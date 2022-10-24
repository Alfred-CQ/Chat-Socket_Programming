#include "tictactoe.hpp"

Tictactoe::Tictactoe()
{

}

Tictactoe::Tictactoe(int board_owner, int size_board)
{
    this->board_owner = board_owner;

    set_Size_Board(size_board);

    set_Board();
}

Tictactoe::Tictactoe(int sd_player_1, int sd_player_2, int size_board)
{
    players_sd.first = sd_player_1;
    players_sd.second = sd_player_2;

    set_Size_Board(size_board);

    set_Board();
}

Tictactoe::~Tictactoe()
{

}

/* Setters */
void Tictactoe::set_Size_Board(int size_board)
{
    this->size_board = size_board;
}

void Tictactoe::set_Board()
{

    for (int i = 0; i < size_board; ++i)
    {
        vector<string> m(size_board, "00");
        board.push_back(m);
    }

    for (int i = 0, k = 0; i < size_board; ++i)
    {
        for (int j = 0; j < size_board; ++j, ++k)
        {
            board[i][j] = complete_digitss(k, 1);
            boxes_map.emplace(complete_digitss(k, 1), std::make_pair(i, j));
        }
    }
}

/* Core */
void Tictactoe::draw_Board(string nickname, int player, bool notification)
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

    for (int i = 0, k = 0; i < size_board; ++i)
    {
        for (int j = 0; j < size_board; ++j, ++k)
            cout << board[i][j] << "  ";
        cout << '\n';
    }

    if (notification)
    {
        cout << "\n\t\t  _____________________\n";
        cout << "\t\t  |                   |\n";
        cout << "\t\t  |  Invalid Move " << player_avatars[player] << "  |\n";
        cout << "\t\t  |___________________|\n";
    }
}

bool Tictactoe::mark_Board(string move, int player)
{
    if (board[boxes_map[move].first][boxes_map[move].second] == player_avatars[player] ||
        board[boxes_map[move].first][boxes_map[move].second] == player_avatars[!player] )
        return false;

    board[boxes_map[move].first][boxes_map[move].second] = player_avatars[player];

    return true;
}

bool Tictactoe::check_Win(int player_avatar)
{
    // Rows and cols
    int count_in_rows{0}, count_in_cols{0};
    for (int i = 0; i < size_board; ++i)
    {
        count_in_rows = 0;
        count_in_cols = 0;
        for (int j = 0; j < size_board; ++j)
        {
            if (board[i][j] == player_avatars[player_avatar])
                count_in_rows++;
            if (board[j][i] == player_avatars[player_avatar])
                count_in_cols++;
        }
        if (count_in_rows == size_board || count_in_cols == size_board)
            return true;
    }

    // Diagonals
    int count_in_diag{0}, count_in_invdiag{0};
    for (int i = 0, j = 2; i < size_board; ++i, --j)
    {
        if (board[i][i] == player_avatars[player_avatar])
            count_in_diag++;
        if (board[i][j] == player_avatars[player_avatar])
            count_in_invdiag++;
    }

    if (count_in_diag == size_board || count_in_invdiag == size_board)
        return true;

    return false;
}

string Tictactoe::complete_digitss(int t, bool type)
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