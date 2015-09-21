#ifndef employee_hpp
#define employee_hpp

#include <stdio.h>
#include <string>

class employee {
public:
    employee(std::string lastname_, std::string surname_, float salary_, int age_, int clearanceLevel_);
    bool operator < (const employee &other) const {
        return _salary < other._salary;
    }

private:
    std::string _lastname;
    std::string _surname;
    float _salary;
    int _clearanceLevel;
    int _age;
};

#endif /* employee_hpp */
