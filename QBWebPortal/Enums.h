#pragma once

#include <string>

// error level for error reporting
// error level is done by lowest displayed. therefore, NOTICE and ALL are interchangeable.
enum class ErrorLevel {
	NONE,
	ERROR,
	WARNING,
	NOTICE,
	ALL
};

/**
*
* Struct for the passage of data. will store 3 types (int, double, string) to pass back collected data.
* Enum for determining stype to pass back as well. 
*
*/

enum class DPO {
    STRING,
    NUMBER,
    DOUBLE,
    UNINITIALIZED
};

struct DataPass {
    std::string str;
    long long num{ 0 };
    double dbl{ 0.0 };
    DPO o{ DPO::UNINITIALIZED };
    bool notEqual = false; // manually set after declaration, will force the SQL call to state != as opposed to =
    // quick constructors will reduce calls in main code.
    DataPass(std::string s) : str{ s } { o = DPO::STRING; }
    DataPass(long long n) : num{ n } { o = DPO::NUMBER; }
    DataPass(double d) : dbl{ d } { o = DPO::DOUBLE; }
    DataPass() {} // default blank constructor.
};

