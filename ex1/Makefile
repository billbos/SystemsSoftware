all: employee_exec

employee_exec: main.o
	g++ main.o -o employee_exec

main.o: main.cpp employee.hpp
	g++ -c main.cpp

clean:
	rm -rf main.o sorted_db.txt employee_exec