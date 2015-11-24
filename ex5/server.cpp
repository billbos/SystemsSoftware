#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stddef.h>

//we populate the playfield array with spaces 
//to be able to display it nicely
char play_field [9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int counter = 0;

struct player {
    //socket descriptor on which to listen
	int id;
    //player token ('x' or 'o')
	char token;
};

//checks if the current game is finished
//(either 3 tokens of one kind in a row or a full playfield)
int didGameFinish(int position, char token) {
    //check columns (vertical)
    //get column 0, 1 or 2
    int column = position % 3;
    for (int i = 0; i < 3; i++) {
        //we have to multiply i by 3 to chump to the next row
        //if it doesn't contain the same token, the game is not finished
        if (play_field[column + 3*i] != token) {
            break;
        }
        if (i == 2) {
            return 1;
        }
    }

    //check rows (horizontal)
    //get row 0,1 or 2
    int row = position / 3;
    for (int i = 0; i < 3; i++) {
        //we have to multiply the row by 3 to get the correct positions
        if (play_field[3*row + i] != token) {
            break;
        }
        if (i == 2) {
            return 1;
        }
    }

    //check diagonal top to bottom (positions 0, 4 and 8)
    for (int i = 0; i < 9; i = i + 4){
        if (play_field[i] != token) {
            break;
        }
        if (i == 8) {
            return 1;
        }
    }

    //check diagnonal bottom to top (positions 2, 4 and 6)
    for (int i = 6; i > 1; i = i - 2) {
        if (play_field[i] != token) {
            break;
        }
        if (i == 2) {
            return 1;
        }
    }

    //if the counter hits 9, our playfield is full
    //if we reach this point, noone has one and the playfield is full
    //therefore it's a tie
    if (counter == 9) {
        return -1;
    }
    return 0;
}

//since tcp decides how the messages are sent (as one message or as a set of messages), 
//we need to be able to recognize the single messages, even if there are sent as a set of messages
//we do that by sending them with a header, which contains the length of the messages as illustrated below
//                       _____________________________________
//  set of messages:    | length | message | length | message |
//                       -------------------------------------
void write_to_client(std::string message, int client) {
    int header = message.length();
    std::string message_with_header = std::to_string(header) + message;
    write(client, message_with_header.c_str(), message_with_header.length());
}

int main(int argc, char* argv[]) {
    //path on which we create the file which handles the socket connection
    std::string path;

    if (argc != 2) {
        std::cerr << "Wrong amount of arguments provided!" << std::endl;
        return -1;
    } else {
        path = argv[1];
    }

	struct player player_x;
    struct player player_o;

    //buffer for the messages we receive from the players
    char player_message[100];

    // server socket
    int fd_s;
    struct sockaddr_un addr_s;
    socklen_t addr_s_len;

    // client socket
    int client_a;
    struct sockaddr_un addr_client_a;
    socklen_t addr_client_a_len;

    int client_b;
    struct sockaddr_un addr_client_b;
    socklen_t addr_client_b_len;

    // ***   create socket   ***
    fd_s = socket( AF_UNIX, SOCK_STREAM, 0 );
    if( fd_s < 0 ) {
        std::cerr << "Error in socket()" << std::endl;
        return -1;
    }

    // ***   set address and bind socket   ***
    addr_s.sun_family = AF_UNIX;
    strcpy( addr_s.sun_path, path.c_str() );
    addr_s_len = offsetof( struct sockaddr_un, sun_path ) + strlen( addr_s.sun_path );
  
    //we unlink first
    //if there is a file, it will be deleted
    //if not, nothing happens
    unlink( path.c_str() );
    if( bind( fd_s, ( struct sockaddr* )&addr_s, addr_s_len ) < 0 ) {
        std::cerr << "Error in bind()" << std::endl;
        return -1;
    }

    // ***   listen for connections   ***
    if( listen(fd_s, 1) < 0 ) {
        std::cerr << "Error in listen()" << std::endl;
        return -1;
    }

    // ***   accept an incoming connection   ***
    addr_client_a_len = sizeof( struct sockaddr_un );

    client_a = accept(fd_s, (struct sockaddr* )&addr_client_a, &addr_client_a_len );

    //identify player x
    write_to_client("Hello! You're player X! Please wait until a second player joins the game...", client_a);

    player_x.id = client_a;
    player_x.token = 'x';

    addr_client_b_len = sizeof(struct sockaddr_un);

    //identify player o
    client_b = accept(fd_s, (struct sockaddr* )&addr_client_b, &addr_client_b_len);

    write_to_client("Hello! You're player O!", client_b);

    player_o.id = client_b;
    player_o.token = 'o';

    if( client_a < 0 || client_b < 0 ) {
        std::cerr << "Error in accept()" << std::endl;
        return -1;
    }

    //start the game
    write_to_client("Both clients have connected to the server. Let the game begin!", client_a);
    write_to_client("Both clients have connected to the server. Let the game begin!", client_b);
        
    //represents a turn
    //therefore it wont finish until it reaches 9 (playfield full)
    //or somenone breaks it
    while (counter < 9) {
    	struct player current_player;
    	struct player waiting_player;

        //decide which player is the one currently playing
        //and which is the one currently waiting
    	if (counter % 2 == 0) {
    		current_player = player_x;
    		waiting_player = player_o;
    	} else {
    		current_player = player_o;
    		waiting_player = player_x;
    	}

        write_to_client(std::string("Please wait while player ") + current_player.token + " is making his move!", waiting_player.id);

        write_to_client("It's your turn! Please enter the position (0-8) to place your token:", current_player.id);

        //get the move the player currently playing did make
        int position = read(current_player.id, player_message, 80);
        player_message[position] = '\0';

        //parse it to an int
        //we already know that it has to be a valid move
        //so we just add it to the playfield
        int move = std::stoi(player_message);
        play_field[move] = current_player.token;

        write_to_client(std::string("Player ") + current_player.token + " has done his move.", client_a);
        write_to_client(std::string("Player ") + current_player.token + " has done his move.", client_b);

        //send the updated playfield to both clients
        write_to_client(std::string("playfield:") + std::string(play_field), client_a);
        write_to_client(std::string("playfield:") + std::string(play_field), client_b);

        //if the game is finished, we finish the program
        if (didGameFinish(move, current_player.token)) {
            write_to_client(std::string("Player ") + current_player.token + " has won! Congratulations!", client_a);
            write_to_client(std::string("Player ") + current_player.token + " has won! Congratulations!", client_b);

            close(client_a);
            close(client_b);
            close( fd_s );

            unlink( path.c_str() );

            return 0;
        }
        
        counter++;
    }

    //if we hit this point, it has to be a tie
    if (counter == 9) {
        write_to_client("It's a tie!", client_a);
        write_to_client("It's a tie!", client_b);
    }

    // ***   close sockets   ***
    close(client_a);
    close(client_b);
    close( fd_s );

    unlink( path.c_str() );

    return 0;
}
