#include "include/client.hpp"

bool stop_game = false;

char option_send = 'N';

void read_Message(Client*);
void write_Message(Client*);

int main(int argc, char *argv[])
{
    Client myclient(45000, "127.0.0.1");

    string nickname;

    std::thread(read_Message, &myclient).detach();

    system("clear");
    cout << "░█░█░█▀▀░█░░░█▀▀░█▀█░█▄█░█▀▀\n░█▄█░█▀▀░█░░░█░░░█░█░█░█░█▀▀\n░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀\n";

    cout << " 👥 Write your nickname to get started: ";
    cin >> nickname;

    myclient.set_nickname(nickname);
    myclient.send_Option(&option_send); myclient.send_Nickname();
    
    sleep(1.5);
    cout << "***** " << nickname << " choose one of the following options *****\n";
    cout << "      [M] Send a message - [B] Broadcast\n      [L] List of users  - [R] Close session\n";
    cout << "      [F] Send a file\n";
    cout << "      [I] Invite game    - [G] Accept game  - [E] Enter game\n";

    while (option_send != 'R')
    {
        write_Message(&myclient);
    }

    cout << "Closing chat ...\n *** Finished program *** ";

    myclient.log_out();

    return 0;
}

void read_Message(Client* client)
{
    char option = '1';

    while (option != 'R')
    {
        recv(client->cli_socketFD, &option, 1, 0);

        if (option == 'S')
            client->recv_Server_Notification();
        else if (option == 'M')
            client->recv_Message();
        else if (option == 'L')
            client->recv_List();
        else if (option == 'F')
            client->recv_File();
        else if (option == 'G')
            client->recv_Server_Game_Settings();
        else if (option == 'T')
            client->recv_Server_Game_Tick();
        else if (option == 'H')
            client->recv_Server_Game_Inv_Tick();
        else if (option == 'W')
        {
            client->recv_Server_Game_Winner();

            stop_game = true;
            option_send = '0';
        }
    }
}

void write_Message(Client* client)
{
    string nick_friend, message, file_name, response_invitation, selected_box;

    if (option_send != 'E')
    {
        cout << "Your option: ";                    cin >> option_send;
        client->send_Option(&option_send);
    }
    if (option_send == 'M' || option_send == 'I')
    {
        cout << "Enter your friend's nickname: ";   cin >> nick_friend;
        cout << "Type your message/invitation: ";   cin >> message;
        client->send_Message(nick_friend, message);
    }
    else if (option_send == 'B')
    {
        cout << "Write your message: ";             cin >> message;
        client->send_Broadcast(message);
    }
    else if (option_send == 'R')
    {
        message = " 🏃 " + client->cli_nickname + " left the chat";
        client->send_Broadcast(message);
        sleep(2);
    }
    else if (option_send == 'F')
    {
        cout << "Enter your friend's nickname: ";   cin >> nick_friend;
        cout << "Type file name: ";                 cin >> file_name;
        client->send_File(nick_friend, file_name);
    }
    else if (option_send == 'G')
    {
        cout << "Enter your friend's nickname: ";   cin >> nick_friend;
        cout << "Type your response (YES/NO): ";    cin >> response_invitation;
        client->send_Message(nick_friend, response_invitation);
        
        if (response_invitation == "YES")
        {
            int sizeboard;
            cout << "Size of Board (1-9): ";        cin >> sizeboard;
            client->send_Invitation(sizeboard);
        }
    }
    else if (option_send == 'E')
    {
        OWNER_TURN data_Game;
        client->draw_Board_Game();
        
        while (!stop_game)
        {
            client->get_Game_Owner_CurrTurn(data_Game);
            if (data_Game.first == data_Game.second)
            {
                sleep(1.5);
                
                if (stop_game == true) break;
                cout << "\n\t       Player [ " << (data_Game.first == 0 ? 1 : 2) << " ] choose a box: ";
                cin >> selected_box;
                client->send_Server_Game_Tick(selected_box);
                
                sleep(1.5);
            }  
        }  
        option_send = '0';
    }
}
