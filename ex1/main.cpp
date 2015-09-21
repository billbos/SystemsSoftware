#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include "employee.hpp"

std::string buf;
std::vector<employee> data;

void readFile(std::string fileName) {
    std::string lastname;
    std::string surname;
    float salary;
    int age;
    int clearanceLevel;
    
    std::ifstream fin("/Users/lukasvollenweider/Desktop/exercise1/sample_input.txt");
    if (fin.is_open()) {
        while (std::getline(fin, buf)) {
            std::stringstream ss(buf);
            ss >> lastname >> surname >> salary >> age >> clearanceLevel;
            employee emp = employee(lastname, surname, salary, age, clearanceLevel);
            data.push_back(emp);
        }
    }
    std::sort(data.begin(), data.end());
    /*std::ofstream output_file("./example.txt");
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(data.begin(), data.end(), output_iterator);*/
    std::ofstream outputFile("program3data.txt");
    outputFile << "writing to file";
}

int main(int argc, const char * argv[]) {
    if (argc == 1) {
        std::cout << argv[0] << " called without input args\n";
    } else {
        for (int i = 1; i < argc; i++) {
            readFile(argv[1]);
        }
    }
    return 0;
}


