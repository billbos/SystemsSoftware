#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "employee.hpp"

std::string buffer;
std::vector<employee> data;

void parseFile(std::string fileName) {
    std::ifstream fin(fileName);
    if (fin.is_open()) {
        while (std::getline(fin, buffer)) {
            std::stringstream ss(buffer);
            employee emp;
            ss >> emp.lastname >> emp.surname >> emp.salary >> emp.age >> emp.clearanceLevel;
            data.push_back(emp);
        }
    }
    employee_cmp compare_functor;
    std::sort(data.begin(), data.end(), compare_functor);
    std::ofstream output("sorted_db.txt");
    for (auto emp : data) {
        output << emp.lastname << " " << emp.surname << " " << emp.salary << " " << emp.age
        << " " << emp.clearanceLevel << std::endl;
    }
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
