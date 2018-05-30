#include "Helpers.h"
#include <vector>
#include <Psapi.h>

#define INRANGE(x, a, b)	(x >= a && x <= b)
#define GETBITS(x)			(INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define GETBYTE(x)			(GETBITS(x[0]) << 4 | GETBITS(x[1]))

//MISCELLANEOUS
void Helpers::Log(char* szMessage)
{
	std::cout << "[+] " << szMessage << std::endl;
}
void Helpers::LogChar(char szMessage)
{
	std::cout << "[+] " << szMessage << std::endl;
}
void Helpers::LogInt(char* szMessage, int iAdress)
{
	std::cout << "[+] " << szMessage << ": " << std::defaultfloat << iAdress << std::endl;
}
void Helpers::LogAddress(char* szName, int64_t iAddress)
{
	std::cout << "[+] " << szName << ": 0x" << std::hex << iAddress << std::endl;
}
void Helpers::LogFloat(char* szName, float fValue)
{
	std::cout << "[+] " << szName << ": " << std::defaultfloat << fValue << std::endl;
}
void Helpers::LogString(char* szName, std::string sValue)
{
	std::cout << "[+] " << szName << ": " << sValue << std::endl;
}
void Helpers::LogError(char* szMessage)
{
	std::cout << "[Error] " << szMessage << std::endl;
}
void Helpers::LogSuccess(char* szMessage)
{
	std::cout << "[Success] " << szMessage << std::endl;
}

// CONVERTERS
std::string Helpers::VariableText(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	char buffer[2048];
	vsprintf(buffer, format, argptr);

	va_end(argptr);

	return buffer;
}

void Helpers::GetResolution(UINT horizontal, UINT vertical)
{
	RECT desktop;
	const HWND hDesktop = FindWindowA(NULL, "Rainbow Six");
	GetWindowRect(hDesktop, &desktop);
	horizontal = desktop.right;
	vertical = desktop.bottom;
}