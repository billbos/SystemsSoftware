#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <unistd.h>

std::vector<int> memory_blocks;
int free_blocks = 0;
pthread_mutex_t lock;
bool run;

int allocate_memory(unsigned long number_of_blocks, int id);
void release_blocks(unsigned long number_of_blocks, int id);
void print_memory_blocks();
void sleep(int min, int max);
void* thread_func( void* arg );

struct thread_param {
	int _id;	// thread id
	int w_min;	// min number of blocks
	int w_max;	// max number of blocks
	int t_min;	// min number of waiting time
	int t_max;	// max number of waiting time
};  

//prints the memory blocks as an ASCII block:
//	+--+--+--+--+
//  |-1|-1|-1|-1|
//	+--+--+--+--+
void print_memory_blocks() {
	int size = memory_blocks.size();
	printf("+");
	for (int i = 0; i < size * 3 - 1; i++) {
		if (i % 3 == 2) {
			printf("+");
		} else {
			printf("-");
		}
	}
	printf("+\n|");

	for (int i = 0; i < size; i++) {
		if (memory_blocks[i] >= 0 && memory_blocks[i] < 10) {
			printf(" %d|", memory_blocks[i]);
		} else {
			printf("%d|", memory_blocks[i]);
		}
	}
	printf("\n");

	printf("+");
	for (int i = 0; i < size * 3 - 1; i++) {
		if (i % 3 == 2) {
			printf("+");
		} else {
			printf("-");
		}
	}
	printf("+\n\n");
}

//simulates the computation time
//also used to let the program wait a bit after each loop
//otherwise the terminal would be flooded
void sleep(int min, int max) {
	unsigned long t = min + rand() % ( max - min );
  
	struct timespec ts;
	ts.tv_sec = t / 1000;
	ts.tv_nsec = ( t % 1000 ) * 1000000;
  
	nanosleep( &ts, NULL );
}

//simulates memory allocation and deallocation
void* thread_func( void* arg ) {
	thread_param* tp = ( thread_param* )arg;
 
	run = true;

	do {
		//get the length of the block
		//has to be between w_min and w_max
		unsigned long w = tp->w_min + rand() % ( tp->w_max - tp->w_min );

		//lock the global variables and call the allocate function
		pthread_mutex_lock(&lock);
		int success = allocate_memory(w, tp->_id);
		//unlock the variables
		pthread_mutex_unlock(&lock);

		if (success) {
			//simulate computation time
			sleep(tp->t_min, w * tp->t_max);
			//lock the global variables and call the deallocate function
			pthread_mutex_lock(&lock);
			release_blocks(w, tp->_id);
			//unlock the variables
			pthread_mutex_unlock(&lock);
		}
		//wait a bit until next iteration
		sleep(tp->t_min, tp->t_max);
	}
	//run get changed to false when the user enters 'e'
	while( run );

	return NULL;
}

//allocates a number of blocks for the thread 
int allocate_memory(unsigned long number_of_blocks, int id) {
	std::cout << "Thread " << id << " wants to allocate " << number_of_blocks << " memory blocks." << std::endl;
	//we use a variable to store the number of blocks which are currently free
	//to save the user from needless iterations through the vector
	if (free_blocks < number_of_blocks) {
		std::cerr << "Not enough free memory blocks available!\n" << std::endl;
		return 0;
	} else {
		int i = 0;
		free_blocks = free_blocks - number_of_blocks;
		//get some blocks which are empty (-1) 
		//insert the thread id at that position 
		while (number_of_blocks > 0) {
			if (memory_blocks[i] == -1) {
				memory_blocks[i] = id;
				number_of_blocks--;
			}
			i++;
		}
		print_memory_blocks();
		return 1;
	}
}

//deallocates a number of blocks which are occupied by a thread
void release_blocks(unsigned long number_of_blocks, int id) {
	std::cout << "Thread " << id << " releases " << number_of_blocks << " memory blocks." << std::endl;
	int i = 0;
	free_blocks = free_blocks + number_of_blocks;
	//replace number_of_blocks elements from the vector with value = id with -1
	while (number_of_blocks > 0) {
		if (memory_blocks[i] == id) {
			memory_blocks[i] = -1;
			number_of_blocks--;
		}
		i++;
	}
	print_memory_blocks();
}


int main( int argc, char* argv[] )  {
	if( argc < 7 ) {
		std::cerr << "Not enough arguments provided. Terminating." << std::endl;
		exit( 1 );
	}
	// srand( time( 0 ) );   // uncomment to actually get pseudo-random numbers
  
	// get command-line parameters
	int p = std::stoi( argv[ 1 ] );
	int b = std::stoi( argv[ 2 ] );
	int w_min = std::stoi( argv[ 3 ] );
	int w_max = std::stoi( argv[ 4 ] );
	int t_min = std::stoi( argv[ 5 ] );
	int t_max = std::stoi( argv[ 6 ] );

	pthread_t threads[p];

	//initialize mutex
	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex init failed\n");
        return -1;
    }
  
  	//fill the memory_blocks vector with -1 to symbolize empty blocks
  	//we need to do that to be sure that no thread can use more memory than available
  	//since vectors do not have any bounds
	for (int i = 0; i < b; i++) {
		memory_blocks.push_back(-1);
	}

	//global variable to indicate the amount of blocks which are empty
  	free_blocks = b;

  	//array with thread structs
  	struct thread_param thread_params [p];

  	//initialize the structs and start the threads
 	for (int i = 0; i < p; i++) {
 		thread_params[i] = {i, w_min, w_max, t_min, t_max};
 		pthread_create(&threads[i], NULL, thread_func, (void *)(&thread_params[i]));
 	}
  
	// wait for 'e' key
	char c = '1';
	do {
		std::cin >> c;
	}
	while( c != 'e' );

	//finish the threads gracefully
	//we wait until every thread has deallocated
	//his occupied memory blocks to not have any memory leaks
	run = false;
	std::cout << "Ending simulation..." << std::endl;

	for (int i = 0; i < p; ++i) {
		pthread_join(threads[i], NULL);
	}

	std::cout << "Simulation finished!" << std::endl;
	return 0;
}
