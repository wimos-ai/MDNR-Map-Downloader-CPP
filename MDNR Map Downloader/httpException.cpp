#include "httpException.h"


httpException::httpException(const std::string& errorMsg, DWORD errorCode): errorMsg(errorMsg), errorCode(errorCode) {

}