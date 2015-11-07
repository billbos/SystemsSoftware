all: med_filt

med_filt: main.o image_matrix.o
	g++-5 -fopenmp main.o image_matrix.o -o med_filt

main.o: main.cpp image_matrix.hpp
	g++-5 -fopenmp -c main.cpp

image_matrix.o: image_matrix.cpp image_matrix.hpp
	g++-5 -c image_matrix.cpp

clean:
	rm -rf *.o med_filt
