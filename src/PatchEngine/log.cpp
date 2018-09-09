#include <stdlib.h>
#include "Log.h"

#define LOG_ON

void LOG(int nColor, LPCSTR lpszLog, ...)
{
#if defined(LOG_ON)
	DWORD dwWritten = 0;
	char szBuffer[2500] = { 0, };
	va_list fmtList;
	va_start(fmtList, lpszLog);
	vsprintf_s(szBuffer, sizeof(szBuffer), lpszLog, fmtList);
	va_end(fmtList);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nColor);
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), szBuffer, lstrlenA(szBuffer), &dwWritten, NULL);
#endif	
}
void LOGW(int nColor, LPWSTR lpszLog, ...)
{
	DWORD dwWritten = 0;
	WCHAR szBuffer[2500] = { 0, };
	va_list fmtList;
	va_start(fmtList, lpszLog);
	vswprintf_s(szBuffer, lpszLog, fmtList);
	va_end(fmtList);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nColor);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), szBuffer, lstrlenW(szBuffer), &dwWritten, NULL);
}


void LOGHEX(PBYTE pbHex, DWORD dwSize)
{
#if defined(LOG_ON)
	char* szBuffer = (char*)malloc(25000);
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
	printf("%s", szBuffer);
	free(szBuffer);
#endif
}