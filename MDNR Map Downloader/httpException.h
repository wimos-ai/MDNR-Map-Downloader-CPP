#ifndef HTTPEXCEPTION_H8732790
#define HTTPEXCEPTION_H8732790

#include <exception>
#include <string>
#include <windows.h>


class httpException : public std::exception {
private:
	const DWORD errorCode;
	std::string errorMsg;
public:
	httpException(const std::string& errorMsg, DWORD errorCode);
};

#endif // HTTPEXCEPTION_H8732790