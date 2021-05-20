
#include "main.h"
#include <cstring>

MyException::MyException(const char *s)
	: exception(s)
{
}

void log(const char *s)
{
	// Открыть файл log.txt на запись. Если файл существует, он будет перезаписан.
	HANDLE hfile = CreateFile("log.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
		return;
	// Запись текста s и перевода строки
	auto len = strlen(s);
	DWORD bytes_written = 0;
	WriteFile(hfile, s, len, &bytes_written, 0);
	WriteFile(hfile, "\r\n", 2, &bytes_written, 0);
}

void MyException::log()
{
	::log(what());
}
