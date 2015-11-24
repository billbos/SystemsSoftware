#include <iostream>
#include <algorithm>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stddef.h>

//stores the moves each player has done
char play_field[8];

//sets every char from a string until position to 0
//used for the string which stores the server message
//such that every time we get a message, we're sure it
//only contains this message and nothing else
void clear_string(char *string, int position) {
    char *begin = string;
    char *end = begin + position;
    std::fill(begin, end, 0);
}

//draws the tic tac toe playfield
void draw_play_field() {
    std::cout << "\nPlayfield: \n" << std::endl;
    for (int i = 0; i < 9; i = i + 3) {
        std::cout << play_field[i] << '|' << play_field[i+1] << '|' << play_field[i+2] << std::endl;
        if (i != 6) {
            std::cout << '-' << '+' << '-' << '+' << '-' << std::endl;
        }
    }
    std::cout << '\n' << std::endl;
}

//breaks up a message into its submessages
//since tcp decides how the messages are sent (as one message or as a set of messages), 
//we need to be able to recognize the single messages, even if there are sent as a set of messages
//we do that by sending them with a header, which contains the length of the messages as illustrated below
//      
//                                  break it up here
//                                         |
//                                         v
//                       _____________________________________
//  set of messages:    | length | message | length | message |
//                       -------------------------------------
// break up removes this header and returns the cleaned message
std::string break_up_message(std::string message, int header, int header_length) {
    //we start from header_length (to exclude it) until the endposition, indicated
    //by the header
    std::string fixed_message = message.substr(header_length, header);

    return fixed_message;
}

//cheks if the turn is valid
//we do this on client side
//otherwise we would have to pass a ton of 
//messages between the client and the server
int check_turn_validity(std::string turn) {
    int position;
    //we try to convert the turn (position 0-8) to an int 
    try {
        position = std::stoi(turn);
    //we don't care about the exceptions, therefore we catch any
    //and return 0 to indicate a failure
    } catch (...) {
        return 0;
    }
    
    //if the position the user has given is inside 0 and 8 and isn't already
    //occupied by an x or o, it is a valid position
    if (position >= 0 && position <= 8) {
        if (play_field[position] != 'x' && play_field[position] != 'o') {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    //path in which we create the connection file
    std::string path;

    if (argc != 2) {
        std::cerr << "Wrong amount of arguments provided!" << std::endl;
        return -1;
    } else {
        path = argv[1];
    }


    int server_connection;
	
    struct sockaddr_un addr_s;
    socklen_t addr_s_len;

    //used for the messages we get from the server
    //we need a relatively high buffer since TCP
    //thinks it is ok to append a second message to
    //the first one if they are sent right after one another
    //to the same client
    char status_message[200];

    char message_to_server[80];

    // ***   create socket   ***
    server_connection = socket( AF_UNIX, SOCK_STREAM, 0 );
    if( server_connection < 0 ) {
        std::cerr << "Error in socket()" << std::endl;
        return -1;
    }


    // ***   establish connection   ***
    addr_s.sun_family = AF_UNIX;
    strcpy( addr_s.sun_path, path.c_str());
    addr_s_len = sizeof( addr_s );
    if( connect( server_connection, ( struct sockaddr* )&addr_s, addr_s_len )  <  0 ) {
        std::cerr << "Error in connect()" << std::endl;
        return -1;
    }

    //we listen to the server unless told otherwise
    while (true) {
        //read from the server
        //if there's a message, position counts to the end of the message
        int position = read(server_connection, status_message, 200);
        //we create a c++ string from the c string since it is more convenient to use
        std::string message_from_server = std::string(status_message);
        
        if (position > 0) {
            //flag which indicates if there are submessages inside the message received from the server
            //there is always at least one submessage into the message (the message itself)
            int has_next = 1;

            while (has_next) {
                //parses the header of the string to an int
                //std::stoi even works if the input is something like 10xDvdsfg
                //so it is perfect for our needs
                int header = std::stoi(message_from_server);

                //get the length of the header (how many digits it contains)
                //we need that for the starting point of each submessage
                int header_length = std::to_string(header).length();

                //get the cleaned message
                std::string headerless_message = break_up_message(message_from_server, header, header_length);

                //if the message contains the substring "playfield"
                //it contains the updated playfield
                //it is in the format: playfield: o xx ox o
                //since we do not want to print it that way, we only print the
                //headerless message if it doesn't contain the playfield update
                if (headerless_message.find("playfield") != std::string::npos) {
                    int pre_length = strlen("playfield:");
                    for (int i = pre_length; i < headerless_message.length(); i++) {
                        play_field[i - pre_length] = headerless_message[i];
                    }
                    draw_play_field();
                } else {
                    std::cout << headerless_message << std::endl;
                }

                //if the message contains the substring "turn"
                //we are supposed to make our move
                if (headerless_message.find("turn") != std::string::npos) {
                    std::string user_input;
                    //gets the user input
                    getline(std::cin, user_input);
                    //if the user input is rubbish, we want a new one
                    while (!check_turn_validity(user_input)) {
                        std::cout << "Your position is not valid. Please enter a new position:" << std::endl;
                        getline(std::cin, user_input);
                    }
                
                    //send the user input to the server
                    strcpy(message_to_server, user_input.c_str());
                    write(server_connection, message_to_server, strlen(message_to_server));
                }

                //if the message contains either Congratulations or tie
                //the game's finished
                //we close the connection and exit
                if ((headerless_message.find("Congratulations") != std::string::npos) || (headerless_message.find("tie") != std::string::npos)) {
                    close(server_connection);
                    return 0;
                }

                //we update the message from the server and exclude the substring which we appropriately managed above
                //if the position (number of read chars) is equal to header + header_length, we got only one message
                //otherwise, there are other submessages in the message
                message_from_server = message_from_server.substr(header + header_length, message_from_server.length()); 
                position = position - (header + header_length);
                has_next = position;
            }

            //we make our buffer ready for the next message
            clear_string(status_message, position);
        }
    }
}
