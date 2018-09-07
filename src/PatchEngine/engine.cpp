#include <windows.h>
#include <stdlib.h>
#include <locale.h>
#include "detours\detours.h"
#include "Log.h"

typedef struct _DIC
{
	char szChinese[512];
	int nChineseLen;
	char szKorean[512];
} DIC;

int LoadDictionary(char* szFileName, DIC *dic)
{
	char szCurrentDirectory[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\");
	strcat(szCurrentDirectory, szFileName);

	int nDicCount = 0;
	FILE *fp;
	// UTF8로 저장된 사전파일
	fp = fopen(szCurrentDirectory, "r");
	if (NULL == fp)
	{
		LOG(5, "사전 파일이 없음. 프로세스를 종료한 후 사전파일을 복사해주세요~\n");
		Sleep(100000);
		return 0;
	}
	char buf[512];
	while (!feof(fp))
	{
		fgets(buf, sizeof(buf), fp);
		if (strstr(buf, "$$$$")) continue;
		if (strstr(buf, "%%%%")) break;
		char* context = NULL;
		char* token = strtok_s(buf, "=", &context);
		char* token2 = strtok_s(NULL, "=", &context);

		WCHAR szUnicode[200] = L"";

		// UTF-8을 유니코드로 변환
		MultiByteToWideChar(CP_UTF8, 0, token, strlen(token), szUnicode, sizeof(szUnicode));
		// 유니코드를 BIG5로 변환
		WideCharToMultiByte(950, 0, szUnicode, lstrlenW(szUnicode), dic[nDicCount].szChinese, sizeof(dic[nDicCount].szChinese), NULL, NULL);
		dic[nDicCount].nChineseLen = strlen(dic[nDicCount].szChinese);

		ZeroMemory(szUnicode, sizeof(szUnicode));

		// UTF-8을 유니코드로 변환
		MultiByteToWideChar(CP_UTF8, 0, token2, strlen(token2), szUnicode, sizeof(szUnicode));
		// 유니코드를 8비트 ASCII 한국어로 변환

		WideCharToMultiByte(949, 0, szUnicode, lstrlenW(szUnicode), dic[nDicCount].szKorean, sizeof(dic[nDicCount].szKorean), NULL, NULL);
		dic[nDicCount].szKorean[strlen(dic[nDicCount].szKorean) - 1] = 0; // \n 삭제

		// 한글 공백 채워넣기
		if (dic[nDicCount].nChineseLen > strlen(dic[nDicCount].szKorean))
		{
			int nBlank = dic[nDicCount].nChineseLen - strlen(dic[nDicCount].szKorean);

			for (int j = 0; j < nBlank; j++)
			{
				strcat(dic[nDicCount].szKorean, " ");
			}
		}

		nDicCount++;
	}
	fclose(fp);

	LOG(13, "사전에서 %d 문장 읽음.\n", nDicCount);

	// 내림차순 정렬	
	for (int i = 0; i < nDicCount - 1; i++)
	{
		for (int j = i + 1; j < nDicCount; j++)
		{
			char szTempChinese[512];
			int nTempChineseLen = 0;
			char szTempKorean[512];
			if (dic[i].nChineseLen < dic[j].nChineseLen)
			{
				nTempChineseLen = dic[j].nChineseLen;
				dic[j].nChineseLen = dic[i].nChineseLen;
				dic[i].nChineseLen = nTempChineseLen;

				strcpy(szTempChinese, dic[j].szChinese);
				strcpy(dic[j].szChinese, dic[i].szChinese);
				strcpy(dic[i].szChinese, szTempChinese);

				strcpy(szTempKorean, dic[j].szKorean);
				strcpy(dic[j].szKorean, dic[i].szKorean);
				strcpy(dic[i].szKorean, szTempKorean);
			}
		}
	}

	return nDicCount;
}

size_t StringSearch(IN BYTE* pbBuffer, IN size_t BufSize, IN size_t Offset, IN BYTE* pbString, IN size_t StringSize)
{
	size_t ret = (size_t)-1;
	size_t i = 0;
	for (i = Offset; i <= BufSize - StringSize; i++)
	{
		size_t j = 0;
		for (j = 0; j < StringSize; j++)
		{
			if (pbBuffer[i + j] != pbString[j])
			{
				break;
			}

		}
		if (j == StringSize)
		{
			ret = i;
			break;
		}
	}
	return ret;
}

HFONT(WINAPI* TrueCreateFontA)(
	_In_ int cHeight,
	_In_ int cWidth,
	_In_ int cEscapement,
	_In_ int cOrientation,
	_In_ int cWeight,
	_In_ DWORD bItalic,
	_In_ DWORD bUnderline,
	_In_ DWORD bStrikeOut,
	_In_ DWORD iCharSet,
	_In_ DWORD iOutPrecision,
	_In_ DWORD iClipPrecision,
	_In_ DWORD iQuality,
	_In_ DWORD iPitchAndFamily,
	_In_opt_ LPCSTR pszFaceName) = CreateFontA;

HFONT WINAPI HookCreateFontA(
	_In_ int cHeight,
	_In_ int cWidth,
	_In_ int cEscapement,
	_In_ int cOrientation,
	_In_ int cWeight,
	_In_ DWORD bItalic,
	_In_ DWORD bUnderline,
	_In_ DWORD bStrikeOut,
	_In_ DWORD iCharSet,
	_In_ DWORD iOutPrecision,
	_In_ DWORD iClipPrecision,
	_In_ DWORD iQuality,
	_In_ DWORD iPitchAndFamily,
	_In_opt_ LPCSTR pszFaceName)
{
	LOG(6, "한글 폰트 패치 완료! %d %d %d %s\n", cHeight, cWidth, cWeight, pszFaceName);
	if (cHeight == 12) cHeight = 13;
	return TrueCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut,
		HANGUL_CHARSET, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}

void PrintLogo()
{
	LOG(10, "______           _   _      ______          _            _\n");
	LOG(11, "|  _  \\         | | | |     |  ___|        | |          (_)\n");
	LOG(12, "| | | |___ _ __ | |_| |__   | |_ __ _ _ __ | |_ __ _ ___ _  __ _\n");
	LOG(13, "| | | / _ \\ '_ \\| __| '_ \\  |  _/ _` | '_ \\| __/ _` / __| |/ _` |\n");
	LOG(14, "| |/ /  __/ |_) | |_| | | | | || (_| | | | | || (_| \\__ \\ | (_| |\n");
	LOG(15, "|___/ \\___| .__/ \\__|_| |_| \\_| \\__,_|_| |_|\\__\\__,_|___/_|\\__,_|\n");
	LOG(15, "| |\n");
	LOG(15, "|_|\n");
	LOG(10, " _   __                            ______     _       _\n");
	LOG(11, "| | / /                            | ___ \\   | |     | |\n");
	LOG(12, "| |/ /  ___  _ __ ___  __ _ _ __   | |_/ /_ _| |_ ___| |__\n");
	LOG(13, "|    \\ / _ \\| '__/ _ \\/ _` | '_ \\  |  __/ _` | __/ __| '_ \\ \n");
	LOG(14, "| |\\  \\ (_) | | |  __/ (_| | | | | | | | (_| | || (__| | | |\n");
	LOG(15, "\\_| \\_/\\___/|_|  \\___|\\__,_|_| |_| \\_|  \\__,_|\\__\\___|_| |_|\n");
	LOG(15, "\n");
	LOG(10, "개발 : codetronik, 초코\n");
}

void Hooking()
{
	if (DetourIsHelperProcess())
	{
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)TrueCreateFontA, HookCreateFontA);
	LONG error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		LOG(12, "후킹 실패!! 프로세스를 강제 종료해주세요.\n");
		Sleep(100000);
	}
	else
	{
		LOG(13, "후킹 성공!!\n");
	}

}

void Patch(DWORD dwMemAddr, DWORD dwMemSize, DIC *dic, int nDicCount)
{
	for (int i = 0; i < nDicCount; i++)
	{
		LOG(14, "<%s>", dic[i].szKorean);
		LOG(10, " 검색중..\n");

		size_t Offset = 0;

		do {
			size_t StartOffset = StringSearch((BYTE*)dwMemAddr, dwMemSize, Offset, (BYTE*)dic[i].szChinese, strlen(dic[i].szChinese));

			// 찾을 수 없다면
			if ((size_t)-1 == StartOffset)
			{
				break;
			}
			LOG(11, "0x%x\t", StartOffset);
			Offset = StartOffset + dic[i].nChineseLen;
			strncpy((char*)((DWORD)dwMemAddr + StartOffset), dic[i].szKorean, dic[i].nChineseLen);

		} while (Offset < dwMemSize);
		LOG(12, "\n");
	}
}

void Start()
{
	HMODULE hMod = NULL;

	if (AllocConsole())
	{
		FILE *stream = NULL;
		freopen_s(&stream, "CONOUT$", "w", stdout);

		SetConsoleTitle("한글패치 Log Console");
		setlocale(LC_ALL, "korean");
	}

	PrintLogo();
	LOG(7, "process id - %d(%x)\n", GetCurrentProcessId(), GetCurrentProcessId());
	Hooking();

	// 문자열이 모여있는 메모리를 RWE로 변경함
	DWORD OldProtect;
	DWORD dwThirdServerStringMem = 0x4EB000;
	DWORD dwThirdServerMemSize = 0x446000;
	VirtualProtectEx(GetCurrentProcess(), (void*)dwThirdServerStringMem, dwThirdServerMemSize, PAGE_EXECUTE_WRITECOPY, &OldProtect);

	DIC* dic = (DIC*)malloc(sizeof(DIC) * 800);
	int nDicCount = LoadDictionary("korean.txt", dic);
	Patch(dwThirdServerStringMem, dwThirdServerMemSize, dic, nDicCount);
	free(dic);

	// 아이템은 서버에서 받아오므로 수시로 메모리 검색해야 함 
	dic = (DIC*)malloc(sizeof(DIC) * 800);
	nDicCount = LoadDictionary("korean_item.txt", dic);
	while (1)
	{
		Sleep(10000);
		Patch(dwThirdServerStringMem, dwThirdServerMemSize, dic, nDicCount);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DWORD dwThread;
		HANDLE hDlgThread = 0;
		hDlgThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Start, 0, 0, &dwThread);

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

