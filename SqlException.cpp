#include "SqlException.h"

SqlException::SqlException(char* exception)
{
	std::string errorMsg = exception;
	sqlite3_free(exception);
	this->_exception = "SQL Exception: " + errorMsg;
}

char const* SqlException::what() const throw()
{
	return this->_exception.c_str();
}
