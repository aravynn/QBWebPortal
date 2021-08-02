#pragma once
/**
* 
* Error Reporting: Will save data to external log. 
* Will also be used for determining WHAT is saved to log, using a system of predefined level
* 
* STATIC class requires manual creation and destruction at start and end of main().
* 
*/

// use this definition to determine if error reporting has been set up.
#define ERROR_REPORTING

#include <string>	// string
#include "Enums.h"	// ErrorLevel
#include <fstream>
#include <iostream>
class ErrorReporting
{
private:
	inline static ErrorLevel m_level;	// default level ERROR
	inline static std::string m_file{ "error_log.log" };

	ErrorReporting();			// this will never be created.
	~ErrorReporting();			// likewise, destruction will have no effect.
public:
	static void writeError(std::string error, ErrorLevel level);	// write error to file
	static void setErrorLevel(ErrorLevel level);					// set error level for what is logged.
	static void clearErrorLog();									// clear the current log to ''. mostly for debug.
	static void setErrorFile(std::string filename);					// set the error file name
};

