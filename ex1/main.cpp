#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "employee.hpp"

std::string buffer;
std::vector<employee> data;

void parseFile(std::string fileName) {
    std::ifstream dataFile(fileName);
    if (dataFile.is_open()) {
        while (std::getline(dataFile, buffer)) {
            std::stringstream ss(buffer);
            employee emp;
            ss >> emp.surname >> emp.name >> emp.salary >> emp.age >> emp.clearanceLevel;
            data.push_back(emp);
        }
        dataFile.close();
    }
    employee_cmp compare_functor;
    std::sort(data.begin(), data.end(), compare_functor);
    std::ofstream output("sorted_db.txt");
    for (employee emp : data) {
        output << emp.surname << " " << emp.name << " " << emp.salary << " " << emp.age
        << " " << emp.clearanceLevel << std::endl;
    }
    output.close();
}

int main(int argc, const char * argv[]) {
    if (argc == 1) {
        std::cout << argv[0] << " called without input args\n";
    } else {
        for (int i = 1; i < argc; i++) {
            parseFile(argv[1]);
        }
    }
    return 0;
}
