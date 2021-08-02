#include "ErrorReporting.h"

ErrorReporting::ErrorReporting()
{
	// initializer will have no effect, since we'll start the required functions manually via function call. 
}

ErrorReporting::~ErrorReporting()
{
	// destructor also has no effect, since it must manually destroy file function. 
}

void ErrorReporting::writeError(std::string error, ErrorLevel level)
{
	std::ofstream errFile(m_file);

	if (errFile.is_open()) {
		if (level > m_level) {
			errFile << error << '\n';
		}
	}

	errFile.close();
}

void ErrorReporting::setErrorLevel(ErrorLevel level)
{	// statically set error level.
	m_level = level; 
}

void ErrorReporting::clearErrorLog()
{
	std::ofstream errFile(m_file, std::ios::trunc);
	errFile.close();
}

void ErrorReporting::setErrorFile(std::string filename)
{
	m_file = filename;
}

