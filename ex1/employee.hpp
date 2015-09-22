#ifndef employee_hpp
#define employee_hpp

#include <stdio.h>
#include <string>

struct employee {
    std::string lastname;
    std::string surname;
    float salary;
    int age;
    int clearanceLevel;
};

struct employee_cmp {
    bool operator()(const employee &a, employee &b) const {
        return a.salary < b.salary;
    }
};
#endif
