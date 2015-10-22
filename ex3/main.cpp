#include <iostream>
#include <vector>
#include <fstream>

#include <pthread.h>

#include "image_matrix.hpp"


// function that performs the median filtering on pixel p(r_,c_) of input_image_,
// using a window of size window_size_
// the function returns the new filtered value p'(r_,c_)
float median_filter_pixel( const image_matrix& input_image_,
						   int r_,
						   int c_,
						   int window_size_ )
{
	int n_rows = input_image_.get_n_rows();
	int n_cols = input_image_.get_n_cols();
	
	float filtered_value;
	std::vector<float> window_vector;
	
	for (int i = std::max(0, r_ - window_size_/2); i <= r_ + window_size_/2; i++) {
		if (i >= n_rows) {
			break;
		}
		for (int j = std::max(0, c_ - window_size_/2); j <= c_ + window_size_/2; j++) {
			if (j >= n_cols) {
				break;
			}
			window_vector.push_back(input_image_.get_pixel(i, j));    
		}
	}

	std::sort(window_vector.begin(), window_vector.end());
	if (window_vector.size() % 2 != 0) {
		filtered_value = window_vector[window_vector.size() / 2 + 1];
	} else {
		filtered_value = (window_vector[window_vector.size() / 2 - 1] + window_vector[window_vector.size() / 2])/2;
	}
	
	return filtered_value;
}

// struct passed to each thread, containing the information necessary
// to process its assigned range
struct task
{
	const image_matrix& input_image;
	image_matrix& filtered_image;
	int firstRow, lastRow, window_size;
};

// function run by each thread
void* func( void* arg ) 
{
  	task* t_arg = ( task* )arg;
  	
  	int n_cols = t_arg->input_image.get_n_cols();
  	for( int r = t_arg->firstRow; r < t_arg->lastRow; r++ ) {
	  	for( int c = 0; c < n_cols; c++ ) {
			float p_rc_filt = median_filter_pixel( t_arg->input_image, r, c, t_arg->window_size );
			t_arg->filtered_image.set_pixel( r, c, p_rc_filt );
	  	}
	}
  	pthread_exit( NULL );
}


// read input image from file filename_ into image_in_
bool read_input_image( const std::string& filename_, image_matrix& image_in_ )
{
  bool ret = false;
  std::ifstream is( filename_.c_str() );
  if( is.is_open() )
  {
	int n_rows;
	int n_cols;

	is >> n_rows;
	is >> n_cols;

	image_in_.resize( n_rows, n_cols );

	for( int r = 0; r < n_rows; r++ )
	{
	  for( int c = 0; c < n_cols; c++ )
	  {
		float value;
		is >> value;
		image_in_.set_pixel( r, c, value );
	  }
	}
	is.close();
	ret = true;
  }
  return ret;
}


// write filtered image_out_ to file filename_
bool write_filtered_image( const image_matrix& image_out_ )
{
  bool ret = false;
  std::ofstream os( "filtered.txt" );
  if( os.is_open() )
  {
	int n_rows = image_out_.get_n_rows();
	int n_cols = image_out_.get_n_cols();

	os << n_rows << std::endl;
	os << n_cols << std::endl;

	for( int r = 0; r < n_rows; r++ )
	{
	  for( int c = 0; c < n_cols; c++ )
	  {
		os << image_out_.get_pixel( r, c ) << " ";
	  }
	  os << std::endl;
	}
	os.close();
	ret = true;
  }
  return ret;
}



int main( int argc, char* argv[] )
{
  if( argc < 5 )
  {
	std::cerr << "Not enough arguments provided to " << argv[ 0 ] << ". Terminating." << std::endl;
	return 1;
  }

  // get input arguments
  std::string input_filename( argv[ 1 ] );
  int window_size = std::stoi( argv[ 2 ] );
  int n_threads = std::stoi( argv[ 3 ] );
  int mode = std::stoi( argv[ 4 ] );

  std::cout << argv[ 0 ] << " called with parameters " << input_filename << " " << window_size << " " << n_threads << " " << mode << std::endl;

  // input and the filtered image matrices
  image_matrix input_image;
  image_matrix filtered_image;
   
  // read input matrix
  read_input_image( input_filename, input_image );

  // get dimensions of the image matrix
  int n_rows = input_image.get_n_rows();
  int n_cols = input_image.get_n_cols();

  filtered_image.resize( n_rows, n_cols );

  // start with the actual processing
  if( mode == 0 )
  {
	// ******    SERIAL VERSION     ******

	for( int r = 0; r < n_rows; r++ )
	{
	  for( int c = 0; c < n_cols; c++ )
	  {
		float p_rc_filt = median_filter_pixel( input_image, r, c, window_size );
		filtered_image.set_pixel( r, c, p_rc_filt );
	  }
	}
	// ***********************************
  }
  else if( mode == 1 )
  {
	// ******   PARALLEL VERSION    ******

	// declaration of pthread_t variables for the threads
  	pthread_t threads [n_threads];
	// declaration of the struct variables to be passed to the threads
  	std::vector<task> tasks;
  	for (int i = 0; i < n_threads; i++) {
  		task newTask = {input_image, filtered_image};
  		tasks.push_back(newTask);
  	}

	// create threads
	for(int i = 0; i < n_threads; i++) {
		if (i != n_threads - 1) {
			tasks[i].firstRow = n_rows / n_threads * i;
			tasks[i].lastRow = tasks[i].firstRow + n_rows / n_threads;
		} else {
			tasks[i].firstRow = n_rows / n_threads * i;
			tasks[i].lastRow = n_rows;
		}
		tasks[i].window_size = window_size;
		pthread_create(&threads[i], NULL, func, (void *)(&tasks[i]));
	}

	// wait for termination of threads
	for (int i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	// ***********************************
  }
  else
  {
	std::cerr << "Invalid mode. Terminating" << std::endl;
	return 1;
  }

  // write filtered matrix to file
  write_filtered_image( filtered_image );

  return 0;
}
