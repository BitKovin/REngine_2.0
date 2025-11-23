#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdio.h>

#include <string>

class Logger
{
public:

	static void Log(std::string message)
	{

#ifndef DISTRIBUTION

		printf("%s \n", message.c_str());

#endif // !DISTRIBUTION
	}

private:

};


#endif