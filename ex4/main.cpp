#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include <string>
#include <sstream>

#include <omp.h>

#include "image_matrix.hpp"

bool write_filtered_image( const std::string& filename_, const image_matrix& image_out_ );

float median_filter_pixel( const image_matrix& input_image_,
						   int r_,
						   int c_,
						   int window_size_ )
{
	int n_rows = input_image_.get_n_rows();
	int n_cols = input_image_.get_n_cols();
	
	float filtered_value;
	std::vector<float> window_vector;
	
	//for every p(r,c), we begin at r - window_size/2 and stop at r + window_size/2
	//since there is a possibility to start at a location which is out of bound (i.e. p(r,c) is at the left edge)
	//we only allow starting points which are greater than or equal to 0
	for (int i = std::max(0, r_ - window_size_/2); i <= r_ + window_size_/2; i++) {
		//if we are to overstep the right corner, we stop and go to the next row
		if (i >= n_rows) { break; }
		//the same logic as above applies here, only now for columns instead of rows
		for (int j = std::max(0, c_ - window_size_/2); j <= c_ + window_size_/2; j++) {
			if (j >= n_cols) { break; }
			window_vector.push_back(input_image_.get_pixel(i, j));    
		}
	}

	//we need to sort the vector to calculate the median
	std::sort(window_vector.begin(), window_vector.end());
	if (window_vector.size() % 2 != 0) {
		filtered_value = window_vector[window_vector.size() / 2];
	} else {
		filtered_value = (window_vector[window_vector.size() / 2 - 1] + window_vector[window_vector.size() / 2])/2;
	}
	
	return filtered_value;
}

void serialExecution(const std::vector<image_matrix>& input_images_,
			   std::vector<image_matrix>& output_images_, int window_size_, int n_threads_, int mode_) {
  //if mode == 1, we use the parallel approach at an image level to fix the wrong pixels
  //we use chunksize 1 such that every thread does not more than one iteration at a time
  //we don't need to define any scope for the variables since each thread accesses its own image from
  //input_images_, which has scoped "shared" by default. Local variables are by default private so we're save
	#pragma omp parallel for num_threads(n_threads_) if(mode_ == 1) schedule(static, 1)
	for (int i = 0; i < input_images_.size(); i++) {
		int n_rows = input_images_[i].get_n_rows();
		int n_cols = input_images_[i].get_n_cols();

		for( int r = 0; r < n_rows; r++ ) {
	  		for( int c = 0; c < n_cols; c++ ) {
			float p_rc_filt = median_filter_pixel( input_images_[i], r, c, window_size_);
			output_images_[i].set_pixel( r, c, p_rc_filt );		  	
			}
		}
	}
}

void parallelExecution(const std::vector<image_matrix>& input_images_,
			   std::vector<image_matrix>& output_images_, int window_size_, int n_threads_) {
  //parallel at pixel level
  //therefore, we loop through every image from input_images_
  //and do the fixing stuff with multiple threads
	for (int i = 0; i < input_images_.size(); i++) {
		int n_rows = input_images_[i].get_n_rows();
		int n_cols = input_images_[i].get_n_cols();

		int chunkSize = n_rows / n_threads_;

    //again: local variables are by default private and output_images_ is by default shared so no 
    //need to define any scope for the variables
		#pragma omp parallel for num_threads(n_threads_) schedule(static, chunkSize)
		for( int r = 0; r < n_rows; r++ ) {
	  		for( int c = 0; c < n_cols; c++ ) {
			      float p_rc_filt = median_filter_pixel( input_images_[i], r, c, window_size_);
			      output_images_[i].set_pixel( r, c, p_rc_filt );		  	
			  }
		}
	}
}

void median_filter_images( const std::vector<image_matrix>& input_images_,
			   std::vector<image_matrix>& output_images_,
			   const int window_size_,
			   const int n_threads_,
			   const int mode_ )
{
  // perform filtering of input_images_, selecting the appropriate algorithm based on mode_
	switch (mode_) {
    //mode 1 and 2 are basically the same, except that mode 2 is a parallel approach of mode 1 so 
    //we fall through case 0 and check inside the pragma statement if we should use the parallel approach or not
		case 0: 
		case 1: serialExecution(input_images_, output_images_, window_size_, n_threads_, mode_);
				break;
		case 2: parallelExecution(input_images_, output_images_, window_size_, n_threads_);
				break;
	}
}

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



bool write_filtered_image( const std::string& filename_, const image_matrix& image_out_ )
{
  bool ret = false;
  std::ofstream os( filename_.c_str() );
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
  int window_size = atoi( argv[ 1 ] );
  int n_threads = atoi( argv[ 2 ] );
  int mode = atoi( argv[ 3 ] );

  int input_images_count = argc - 4;
  std::vector< std::string > filenames;
  for( std::size_t f = 0; f < input_images_count; f++ )
  {
    filenames.push_back( argv[ 4 + f ] );
  }

  // input and filtered image matrices
  std::vector< image_matrix > input_images;
  std::vector< image_matrix > filtered_images;
  input_images.resize( input_images_count );
  filtered_images.resize( input_images_count );

  // read input matrices
  for( int i = 0; i < input_images_count; i++ )
  {
    read_input_image( filenames[ i ], input_images[ i ] );

    // resize output matrix
    int n_rows = input_images[ i ].get_n_rows();
    int n_cols = input_images[ i ].get_n_cols();

    filtered_images[ i ].resize( n_rows, n_cols );
  }

  // ***   start actual filtering   ***
  
  if( mode < 3 )             // serial, parallel at image level or parallel at pixel level   
  {
    // invoke appropriate filtering routine based on selected mode
    // ...
    median_filter_images(input_images, filtered_images, window_size, n_threads, mode);
    // write filtered matrices to text files
    // ...
    for (int i = 0; i < filtered_images.size(); i++) {
      write_filtered_image("OUT_" + filenames[i], filtered_images[i]);
    }
  }
  else if( mode == 3 )       // benchmark mode
  {
    double start;
    double end;
    double time[ 3 ];
    // run filtering in each mode and time each execution
    // ...
    for (int i = 0; i < 3; i++) {
      //since we can't stop the timer, we calculate the starting time from the current time (after the 
      //execution of the method) to get the benchmark
  		start = omp_get_wtime();
    	median_filter_images(input_images, filtered_images, window_size, n_threads, i);
    	end = omp_get_wtime();
    	time[i] = end - start;
          // print timing summary
          // we use Mode 1 - 4 instead of the input modes 0 - 3
    	std::cout << "Mode" << i + 1  << ": " << time[i] << std::endl;
    }
  }
  else
  {
    std::cerr << "Invalid mode. Terminating" << std::endl;
    return 1;
  }

  return 0;
}
