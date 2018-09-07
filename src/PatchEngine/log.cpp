#include <stdlib.h>
#include "Log.h"

void LOG(int nColor, LPCSTR lpszLog, ...)
{
	DWORD dwWritten = 0;
	char* szBuffer = (char*)malloc(5000);
	va_list fmtList;
	va_start(fmtList, lpszLog);
	vsprintf_s(szBuffer, 5000, lpszLog, fmtList);
	va_end(fmtList);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nColor);
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), szBuffer, lstrlenA(szBuffer), &dwWritten, NULL);
	free(szBuffer);
}

void LOGHEX(PBYTE pbHex, DWORD dwSize)
{
	char* szBuffer = (char*)malloc(5000000);
	for (int i = 0; i < dwSize; i++)
	{
		if (i % 16 == 0)
		{
			strcat(szBuffer, "\r\n");
		}
		char temp[5] = { 0, };
		wsprintfA(temp, "%02x ", *(pbHex + i));
		strcat(szBuffer, temp);
	}
	strcat(szBuffer, "\r\n---------------------------\r\n");
	LOG(FOREGROUND_BLUE, szBuffer);
	free(szBuffer);
}