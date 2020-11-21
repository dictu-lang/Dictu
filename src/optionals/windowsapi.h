// This is a thin wrapper around certain headers of the windows API which resolves a conflict between winnt.h and the scanner.
// winnt.h adds an enum member called TokenType into the global scope, conflicting with TokenType in the scanner.
// All instances of TokenType within the windows API is replaced by WinntTokenType.
// Window's TokenType is documented here: https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-token_information_class
#ifdef _WIN32
#define TokenType WinntTokenType
// Work around to ensure inet_ntop is included (comes from Vista+)
#if (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#undef TokenType
#endif
