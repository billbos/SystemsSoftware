#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sstream>

// count the occurrences of pattern_ inside the filename_ and write the result (a single number) to a file called "result_PID.txt",
// where PID is the process id of the calling process. NOTE: must use execl to perform the count and write to file!

void occurrences_in_file( const std::string& filename_, const std::string& pattern_ )
{
  int my_pid = getpid();
  std::string result = "result-" + std::to_string(my_pid) + ".txt";

  std::string search_command = "grep -o '" + pattern_ + "'" + " " + filename_ + " | wc -l > " + result;
  const char * search_command_c = search_command.c_str();
  // generate string containing the command to be passed as argument to /bin/sh

  if (execl("/bin/sh", "/bin/sh", "-c", search_command_c, (char*)0) < 0) {
  		std::cerr << "Error: execl failed";
  		exit(EXIT_FAILURE);
  }

  // call exec() to execute the command in search_command as argument to /bin/sh

  // check correct termination
}


// open a text file, read partial count of occurrences and return it as an integer

int read_occurrences_file( const std::string& filename_ )
{
	int result;
	std::cout << filename_ << std::endl;
	std::string buffer;
	std::ifstream dataFile(filename_);
    if (dataFile.is_open()) {
        while (std::getline(dataFile, buffer)) {
            std::stringstream ss(buffer);
            std::cout << "ss" << std::endl;
        }
        dataFile.close();
    }
}


// entry point of the application

int main( int argc, char* argv[] )
{
  // check parameters
  if (argc < 3) {
  	std::cerr << "Error: parameters are missing" << std::endl;
  	exit(EXIT_FAILURE);
  }

  std::string pattern( argv[ 1 ] );  
  //argc has one default argument to invoke the name and one argument is the pattern
  int files_count = argc - 2; 

  int* status = new int[ files_count ];
  pid_t* pids = new pid_t[ files_count ];

  // spawn files_count processes
  for( int f = 0; f < files_count; f++ )
  {
    // call fork and invoke occurrences_in_file() from child process
    pids[f] = fork();
    if (pids[f] == -1) {
    	std::cerr << "Error on fork()" << std::endl;
    	exit(EXIT_FAILURE);
    }
    if (pids[f] == 0) {
    	pids[f] = getpid();
    	occurrences_in_file(argv[f + 2], argv[1]);
    }
  }

  // wait for termination and check termination
  /*for( int f = 0; f < files_count; f++ )
  {
  	wait(&status[f]);

  	if (WIFEXITED(status[f])) {
  		std::cout << "Child terminated normally" << std::endl;
  		return 0;
  	}
  }*/
  

  // open results files, compute overall number of occurrences and print it to standard output
  // ...
  int result = 0;

  for(int i = 0; i < files_count; i++) {
  	std::string filename = "result-" + std::to_string(pids[i]) + ".txt";
  	result += read_occurrences_file(filename);
  }
  
  delete[] status;
  delete[] pids;
  
  return 0;
}
