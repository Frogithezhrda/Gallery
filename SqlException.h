#include <exception>
#include <string>
#include "sqlite3.h"

class SqlException : public std::exception
{
public:
	/*
	* constructor and cleans the memory of the exception
	* @param what exception
	*/
	SqlException(char* exception);
	/*
	* lets us see what is the exception
	* @return what was the exception
	*/
	char const* what() const throw();
private:
	std::string _exception;
};
