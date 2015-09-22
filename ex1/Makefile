all: employee_exec

employee_exec: main.o employee.o
	g++ main.o employee.o -o employee_exec

main.o: main.cpp employee.hpp
	g++ -c main.cpp

employee.o: employee.cpp employee.hpp
	g++ -c employee.cpp

clean:
	rm -rf main.o employee.o employee_exec