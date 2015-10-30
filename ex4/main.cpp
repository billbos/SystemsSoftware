#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include <omp.h>

#include "image_matrix.hpp"


void median_filter_images( const std::vector< image_matrix >& input_images_,
			   std::vector< image_matrix >& output_images_,
			   const int window_size_,
			   const int n_threads_,
			   const int mode_ )
{
  // perform filtering of input_images_, selecting the appropriate algorithm based on mode_

  // ...
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
    
    // write filtered matrices to text files
    // ...
  }
  else if( mode == 3 )       // benchmark mode
  {
    double start;
    double time[ 3 ];

    // run filtering in each mode and time each execution
    // ...

    // print timing summary
    // ...
  }
  else
  {
    std::cerr << "Invalid mode. Terminating" << std::endl;
    return 1;
  }

  return 0;
}
