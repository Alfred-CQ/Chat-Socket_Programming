#include <sys/stat.h>

#include "../include/client.hpp"
#include "../include/utils.h"

Client::Client(uint port, std::string ip)
{
    cli_port     = port;
    cli_ip       = ip;
    cli_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (-1 == cli_socketFD)
    {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }  

    memset(&cli_stsocketAddr, 0, sizeof(struct sockaddr_in));
    cli_stsocketAddr.sin_family = AF_INET;
    cli_stsocketAddr.sin_port   = htons(cli_port);
    
    int valid_network_addr = inet_pton(AF_INET, &(cli_ip.front()) , &cli_stsocketAddr.sin_addr);

    if (0 > valid_network_addr)
    {
        perror("error: first parameter is not a valid address family");
        close(cli_socketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == valid_network_addr)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(cli_socketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(cli_socketFD, (const struct sockaddr *)&cli_stsocketAddr, sizeof(struct sockaddr_in)))
    {
        perror("Connect failed");
        close(cli_socketFD);
        exit(EXIT_FAILURE);
    }
}

Client::~Client()
{

}

void Client::get_Game_Owner_CurrTurn(OWNER_TURN& data_Game)
{
    data_Game.first     = game->board_owner;
    data_Game.second    = game->current_turn;
}

void Client::set_nickname(std::string nickname)
{
    cli_nickname = nickname;
}

void Client::draw_Board_Game()
{
    game->draw_Board(cli_nickname, game->board_owner);
}

void Client::log_out()
{
    shutdown(cli_socketFD, SHUT_RDWR);
    close(cli_socketFD);
}

/*
** 2022 October 01
**
** The design of a client chat protocol using socket programming
** 
** 
********************************************************************************
** The basic methods of the Client class shown in the lines above.
** From here the code is shown, to implement the client's protocol when sending 
** In some cases the parameters are not received by the function but 
** describe the protocol used and also the type of data and 
** the bytes needed to represent it if it is a variable, V is shown.
**
*/

/**
 * 
 * @brief This function send the client's option to the server 
 * 
 * @param option for activating different protocols and responses [ char ] [ 1 ]
 * 
 * @return A boolean to know if the option has been sent
 * 
*/ 
bool Client::send_Option(char* option)
{
    int bytes_send{0};
  
    bytes_send = send(cli_socketFD, option, 1, 0);

    if ( error_communication (bytes_send) ) return 0;

    return 1;
}

/**
 * 
 * @brief This function send the client's nickname to the server 
 * 
 * @protocol N | size_nickname | nickname
 * 
 * @param N              option to enable this protocol [ char ]        [ 1 ]
 * @param size_nickname  nickname string size           [ size_t ]      [ 2 ]
 * @param nickname       client's nickname              [ std::string ] [ V ]
 * 
 * @return A boolean to know if the nickname has been sent
 * 
*/ 
bool Client::send_Nickname()
{
    int bytes_send{0};
    std::string size_name_str = complete_digits(cli_nickname.size(), 1);

    bytes_send = send(cli_socketFD, &(size_name_str.front()), size_name_str.size(), 0);

    if ( error_communication (bytes_send) ) return 0;

    bytes_send = send(cli_socketFD, &(cli_nickname.front()), cli_nickname.size(), 0);

    if ( error_communication (bytes_send) ) return 0;

    return 1;
}

/**
 * 
 * @brief This function sends a message to another client 
 *        using the server as an intermediary
 * 
 * @protocol M | size_nickname_friend | size_message | nickname_friend | message
 * 
 * @param M                     option to enable this protocol  [ char ]        [ 1 ]
 * @param size_nickname_friend  nickname friend string size     [ size_t ]      [ 2 ]
 * @param size_message          message string size             [ size_t ]      [ 3 ]
 * @param nickname_friend       friend nickname                 [ std::string ] [ V ]
 * @param message              message to another client       [ std::string ] [ V ]
 * 
 * @return A boolean to know if the message has been sent
*/ 
bool Client::send_Message(std::string nickname_friend, std::string message)
{
    int bytes_send{0};
    std::string block_1, block_2;

    block_1 = complete_digits(nickname_friend.size(), 1) + 
                                           complete_digits(message.size(), 0);

    bytes_send = send(cli_socketFD, &(block_1.front()), 5, 0);

    if ( error_communication (bytes_send) ) return 0;

    block_2 = nickname_friend + message;

    bytes_send = send(cli_socketFD, &(block_2.front()), block_2.size(), 0);

    if ( error_communication (bytes_send) ) return 0;

    return 1;
}

/**
 * 
 * @brief This function sends a broadcast to all connected clients
 * 
 * @protocol B | size_message | message
 * 
 * @param N             option to enable this protocol  [ char ]        [ 1 ]
 * @param size_message  message string size             [ size_t ]      [ 3 ]
 * @param message       message to all clients          [ std::string ] [ V ]
 * 
 * @return A boolean to know if the broadcast message has been sent
 * 
*/ 
bool Client::send_Broadcast(std::string message)
{
    int bytes_send{0};
    std::string size_message_str = complete_digits(message.size(), 0);

    bytes_send = send(cli_socketFD, &(size_message_str.front()), size_message_str.size(), 0);

    if ( error_communication (bytes_send) ) return 0;

    bytes_send = send(cli_socketFD, &(message.front()), message.size(), 0);

    if ( error_communication (bytes_send) ) return 0;

    return 1;
}

/**
 * 
 * @brief This function sends a non-binary file to another client 
 *        using the server as an intermediate.
 * 
 * @protocol F | size_nickname_friend | size_file_name | size_file_bytes | nickname_friend | file_name | file
 * 
 * @param F                     option to enable this protocol      [ char ]          [ 1 ]
 * @param size_nickname_friend  nickname friend string size         [ size_t ]        [ 2 ]
 * @param size_file_name        file name string size               [ size_t ]        [ 2 ]
 * @param size_file_bytes       size of the file in bytes           [ size_t ]        [ 3 ]
 * @param nickname_friend       friend nickname                     [ std::string ]   [ V ]
 * @param file_name             name of the file with its extension [ std::string ]   [ V ]
 * @param file                  non-binary file                     [ std::ifstream ] [ V ]
 * 
 * @return A boolean to know if the file has been sent
 * 
*/ 
bool Client::send_File(std::string nickname_friend, std::string file_name)
{
    std::ifstream input_file;
    string block_1, block_2;
    char buffer[STR_LENGTH_MAX];

    int bytes_send{0}, size_file_bytes, packet_size{BYTES_PER_PACKET};

    input_file.open(file_name, std::ios::in);

    input_file.seekg(0, input_file.end);
    size_file_bytes = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    // block_1 
    block_1 = complete_digits(nickname_friend.size(), 1) +
              complete_digits(file_name.size(), 1) +
              complete_digits(size_file_bytes, 0);

    bytes_send = send(cli_socketFD, &(block_1.front()), 7, 0);

    // block_2
    block_2 = nickname_friend + file_name;

    bytes_send = send(cli_socketFD, &(block_2.front()), block_2.size(), 0);

    // block_3
    while (size_file_bytes > 0)
    {
        input_file.read(buffer, packet_size);

        bytes_send = send(cli_socketFD, buffer, packet_size, 0);

        size_file_bytes -= packet_size;

        if (size_file_bytes < packet_size)
            packet_size = size_file_bytes;
    }

    input_file.close();
  
    send_Server_Notification("  ???? file uploaded ");

    return 1;
}

/**
 * 
 * @brief This function sends the game settings in an invitation 
 * 
 * @protocol G | size_nickname_friend | size_board | nickname_friend
 * 
 * @param G                     option to enable this protocol  [ char ]        [ 1 ]
 * @param size_nickname_friend  nickname friend string size     [ size_t ]      [ 2 ]
 * @param size_board            dimensions of the game board    [ size_t ]      [ 1 ]
 * @param nickname_friend       friend nickname                 [ std::string ] [ V ]
 * 
 * @return A boolean to know if the inivitation has been sent
*/ 
bool Client::send_Invitation(uint size_board)
{
    int bytes_send{0};

    string sizeboard_str = std::to_string(size_board);

    bytes_send = send(cli_socketFD, &(sizeboard_str.front()), 1, 0);

    return 1;
}

/**
 * 
 * @brief This function sends a notification to the server
 * 
 * @protocol U | size_message | message
 * 
 * @param U                     option to enable this protocol      [ char ]        [ 1 ]
 * @param size_message          size message string size            [ size_t ]      [ 2 ]
 * @param message               message to server                   [ std::string ] [ V ]
 * 
 * @return A boolean to know if the message has been sent
 * 
*/ 
bool Client::send_Server_Notification(std::string message)
{
    int bytes_send{0};
    string response_size_str = complete_digits(message.size(), 0);
    
    bytes_send = send(cli_socketFD, "U", 1, 0);
    bytes_send = send(cli_socketFD, &(response_size_str.front()), 3, 0);
    bytes_send = send(cli_socketFD, &(message.front()), message.size(), 0);

    return 1;
}

/**
 * 
 * @brief This function sends the box that the player has selected
 * 
 * @protocol T | selected_box
 * 
 * @param T                     option to enable this protocol  [ char ]        [ 1 ]
 * @param selected_box          nickname friend string size     [ std::string ] [ 2 ]
 * 
 * @return A boolean to know if the selected box has been sent
*/ 
bool Client::send_Server_Game_Tick(std::string selected_box)
{
    int bytes_send{0};
    
    bytes_send = send(cli_socketFD, "T", 1, 0);
    bytes_send = send(cli_socketFD, &(selected_box.front()), 2, 0);

    return 1;
}

/*
** 2022 October 01
**
** The design of a client chat protocol using socket programming
** 
*************************************************************************
** The client's protocol when sending shown in the lines above.
** From here the code is shown, to implement the client's protocol when receiving 
** In some cases the function is not documented because
** it uses the same format as the functions that perform 
** sending, but in this case it is for receiving, only those 
** that do not have their own sending function are specified
**
*/

/**
 * 
 * @brief  This function receives all messages from the server 
 *         related to a response or status of an action.
 * 
 * @see    Client::send_Nickname
 * 
 * @return boolean if the notification was received successfully
 * 
*/ 
bool Client::recv_Server_Notification()
{
    char message_received[STR_LENGTH_MAX];
    int bytes_received{0}, size_message;
    time_t time_message = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    bytes_received = recv(cli_socketFD, message_received, 3, 0);
 
    message_received[bytes_received] = '\0';
    size_message = atoi(message_received);

    bytes_received= recv(cli_socketFD, message_received, size_message, 0);
    message_received[bytes_received] = '\0';

    cout << "\n[ ???? Server message ] " << message_received << " at " << std::ctime(&time_message) << '\n';

    return 1;
}

/**
 * 
 * @brief  This function receives the message sent by another client
 * 
 * @see    Client::send_Message
 * 
 * @return boolean if the message was received successfully
 * 
*/ 
bool Client::recv_Message()
{
    char message_received[STR_LENGTH_MAX];
    int bytes_received{0}, size_nickname_friend, size_message;
    time_t time_message = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    bytes_received = recv(cli_socketFD, message_received, 5, 0);
    message_received[bytes_received] = '\0';

    string  size_friend_nick_str(message_received, 0, 2),
            size_message_str(message_received, 2, 3);

    size_nickname_friend = atoi(&size_friend_nick_str.front());
    size_message = atoi(&size_message_str.front());

    bytes_received = recv(cli_socketFD, message_received, size_nickname_friend + size_message, 0);

    string  nickname_friend(message_received, 0, size_nickname_friend), 
            message(message_received, size_nickname_friend, size_message);

    cout << "\n\n[ ???? Message from " << nickname_friend << " ] at " << std::ctime(&time_message) << '\n'
                 << " ???? " << message << '\n';

    return 1;
}

/**
 * 
 * @brief  This function receives the file sent by another client
 * 
 * @see    Client::send_File
 * 
 * @return boolean if the file was received successfully
 * 
*/ 
bool Client::recv_File()
{
    char message_received[STR_LENGTH_MAX];

    std::ofstream out_file;
    string block_1, block_2;

    int bytes_received{0}, size_filename, size_file_bytes, packet_size{BYTES_PER_PACKET};
           
    bytes_received = recv(cli_socketFD, message_received, 5, 0);
    message_received[bytes_received] = '\0';

    string  size_filename_str(message_received, 0, 2),
            size_file_str(message_received, 2, 3);

    size_filename = atoi(&size_filename_str.front());
    size_file_bytes = atoi(&size_file_str.front());

    bytes_received = recv(cli_socketFD, message_received, size_filename, 0);
    message_received[bytes_received] = '\0';

    string filename(message_received, 0, size_filename);

    mkdir(&(cli_nickname.front()),0777);

    string path = cli_nickname + "/" + filename;

    out_file.open(path, std::ios::out);

    while (size_file_bytes > 0)
    {
        bytes_received = recv(cli_socketFD, message_received, packet_size, 0);
        message_received[bytes_received] = '\0';
        
        out_file << message_received;
        size_file_bytes -= packet_size;
        
        if (size_file_bytes < packet_size)
            packet_size = size_file_bytes;
    }

    out_file.close();

    send_Server_Notification("  ???? file downloaded successfully ");

    return 1;
}

/**
 * 
 * @brief This function receives the list of active clients in the chat
 * 
 * @protocol L | connected_clients | size_nickname_client_1 | ... | size_nickname_client_n | nickname_client_1 | ... | nickname_client_n
 * 
 * @param L                         option to enable this protocol          [ char ]        [ 1 ]
 * @param connected_clients         number of connected clients             [ size_t ]      [ 2 ]
 * @param size_nickname_client_1    the first client nickname string size   [ size_t ]      [ 2 ]
 * @param size_nickname_client_n    the n client nickname string size       [ size_t ]      [ 2 ]
 * @param nickname_client_1         first client nickname                   [ std::string ] [ V ]
 * @param nickname_client_n         n client nickname                       [ std::string ] [ V ]
 * 
 * @return boolean if the list of clients was received successfully
 * 
*/ 
bool Client::recv_List()
{
    char message_received[STR_LENGTH_MAX];
    int bytes_received{0};

    bytes_received = recv(cli_socketFD, message_received, 2, 0);
    message_received[bytes_received] = '\0';

    int clients_online = atoi(message_received), total = 0;

    bytes_received = recv(cli_socketFD, message_received, clients_online * 2, 0);
    message_received[bytes_received] = '\0';

    std::vector<int> sizes_clients;

    sizes_clients.push_back(0);

    for (int i = 0; i < bytes_received - 1; i += 2)
    {
        string t(message_received, i, 2);
        total += atoi(&t.front());
        sizes_clients.push_back(total);
    }

    bytes_received = recv(cli_socketFD, message_received, total, 0);
    message_received[bytes_received] = '\0';
    
    cout << '\n';
    for (int i = 1; i <= clients_online; ++i)
    {
        string client(message_received, sizes_clients[i - 1], sizes_clients[i] - sizes_clients[i - 1]);
        if (client == cli_nickname)
            cout << "  ???? " << client << '\n';
        else
            cout << "  ???? " << client << '\n';
    }

    return 1;
}

/**
 * 
 * @brief  This function receives the arguments of the game 
 *         from the server, to configure a local version
 * 
 * @see    Client::send_Invitation
 * 
 * @param G                     option to enable this protocol  [ char ]   [ 1 ]
 * @param player_turn           player's turn randomly          [ size_t ] [ 1 ]
 * @param size_board            dimensions of the game board    [ size_t ] [ 1 ]
 * 
 * @return boolean if the game settings was received successfully
 * 
*/ 
bool Client::recv_Server_Game_Settings()
{
    char bufferRead[STR_LENGTH];
    int bytes_received{0}, size_board;

    bytes_received = recv(cli_socketFD, bufferRead, 2, 0);
    bufferRead[bytes_received] = '\0';

    char player = bufferRead[0];
            
    size_board = atoi(&bufferRead[1]);

    cout << "CLIENT " << atoi(&player) << " " << size_board << "\n";

    game = new Tictactoe(atoi(&player), size_board);

    cout << "\n Successfully created game ??????, type [E] to enter game \n";

    return 1;
}

/**
 * 
 * @brief  This function receives the move validated by the server 
 *         and updates the local board values
 * 
 * @see    Client::send_Server_Game_Tick
 * 
 * @protocol T | movement_on_board | current_turn
 * 
 * @param T                     option to enable this protocol  [ char ]        [ 1 ]
 * @param movement_on_board     box marked on the board         [ std::string ] [ 2 ]
 * @param current_turn          board turn to update            [ size_t ]      [ 1 ]
 * 
 * @return boolean if the game tick settings was received successfully
 * 
*/ 
bool Client::recv_Server_Game_Tick()
{
    int bytes_received{0};
    char bufferRead[STR_LENGTH];
            
    bytes_received = recv(cli_socketFD, bufferRead, 3, 0);
    bufferRead[bytes_received] = '\0';
    
    string movement_on_board(bufferRead, 0 ,2);

    game->mark_Board(movement_on_board, game->current_turn);
    game->current_turn = atoi(&bufferRead[2]);
    game->draw_Board(cli_nickname, game->board_owner);

    return 1;
}

/**
 * 
 * @brief  This function receives if there was a winner 
 *         of the game or a tie
 * 
 * @protocol W | size_message | message
 * 
 * @param W                     option to enable this protocol  [ char ]        [ 1 ]
 * @param size_message          message string size             [ size_t ]      [ 3 ]
 * @param message               winner or tie message           [ std::string ] [ V ]
 * 
 * @return boolean if the message was received successfully
 * 
*/ 
bool Client::recv_Server_Game_Winner()
{
    char bufferRead[STR_LENGTH];
    int bytes_received{0}, size_message;
    
    game->draw_Board(cli_nickname, game->board_owner);
    
    bytes_received = recv(cli_socketFD, bufferRead, 3, 0);
    bufferRead[bytes_received] = '\0';
    
    size_message = atoi(bufferRead);
    
    bytes_received = recv(cli_socketFD, bufferRead, size_message, 0);
    bufferRead[bytes_received] = '\0';
    
    
    cout << "\n\t    _____________________________\n";
    cout << "\t    |                             |\n";
    cout << "\t    " << bufferRead << '\n';
    cout << "\t    |_____________________________|\n";

    return 1;
}

/**
 * 
 * @brief  This function receives an indication from the server
 *         that the selected box has been selected.
 * 
 * @param H                     option to enable this protocol  [ char ]        [ 1 ]
 * 
*/ 
void Client::recv_Server_Game_Inv_Tick()
{
    game->draw_Board(cli_nickname, game->board_owner, true);
}