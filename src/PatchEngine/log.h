#pragma once
#include <Windows.h>
#include <stdio.h>

void LOG(int nColor, LPCSTR lpszLog, ...);
void LOGW(int nColor, LPWSTR lpszLog, ...);
void LOGHEX(PBYTE pbHex, DWORD dwSize);
int isReadableMemory(LPVOID pMemoryAddr);

__forceinline void GetOffset(int depth)
{
	DWORD dwRetn;

	DWORD dwEbp;
	__asm push eax
	__asm mov dwEbp, ebp
	__asm pop eax

	LOG(11, "*S-----------------------------------------\r\n");
	for (int i = 0; i < depth; i++)
	{
		dwEbp = *(DWORD*)dwEbp;
		int b = isReadableMemory((LPVOID)dwEbp);
		if (b != 0)
		{
			continue;
		}
		dwRetn = *(DWORD*)(dwEbp + 4);
		LOG(11, "누가 호출 %x\r\n", dwRetn);
	}
	LOG(11, "*E-----------------------------------------\r\n");
}
