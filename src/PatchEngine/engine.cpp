#include <windows.h>
#include <stdlib.h>
#include <locale.h>
#include "engine.h"
#include "hook.h"
#include "Log.h"
#include "trans.h"
#include "unzip.h"
#include "curlwrapper.h"

void Updater()
{
	BOOL bRet = FALSE;
	BYTE* response = (BYTE*)malloc(2000000);

	char szCurrentDirectory[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\");
	strcat(szCurrentDirectory, "korean_skill.txt");
		
	FILE* fp;
	fp = fopen(szCurrentDirectory, "r");
	if (NULL == fp)
	{
		LOG(5, "사전 파일이 존재하지 않습니다. 서버에서 사전 파일을 다운로드 합니다.\n");
		goto DOWNLOAD;
	}
	char buf[30] = { 0, };
	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	char* version = strstr(buf, "$ ");
	if (version == NULL)
	{
		LOG(5, "사전 파일에 버전 정보가 없습니다. 서버에서 사전 파일을 갱신합니다.\n");
		goto DOWNLOAD;
	}
	int now_version = atoi(version + 2);
	LOG(8, "현재 사전 파일 버전 %d\n", now_version);
	
	DWORD size = 0;

	BOOL bSuccess = OpenUrl("http://52.79.240.233/api/v1/dictionary/version", NULL, NULL, NULL, NULL, response, &size);
	if (FALSE == bSuccess)
	{
		LOG(5, "사전 서버와의 연결에 실패하였습니다.\n");
		goto EXIT_ERROR;
	}

	// 응답에서 버전 파싱
	char* dl_version = strstr((char*)response, ":");
	char* dl_version_e = strstr((char*)response, "}");
	
	char ver[10] = { 0, };
	strncpy(ver, dl_version + 1, dl_version_e + 1 - dl_version - 1);
	int dl_ver = atoi(ver);
	LOG(8, "서버 사전 파일 버전 %d\n", dl_ver);
	if (dl_ver <= now_version)
	{
		// 최신 버전. 업데이트 스킵
		LOG(9, "사전 파일이 최신 버전입니다.\n");
		goto EXIT;
	}
	LOG(13, "사전 파일을 최신 버전으로 갱신합니다.\n");
DOWNLOAD:
	bSuccess = OpenUrl("http://52.79.240.233/api/v1/dict-file", NULL, NULL, NULL, NULL, response, &size);
	if (FALSE == bSuccess)
	{
		LOG(5, "사전 서버와의 연결에 실패하였습니다.\n");
		goto EXIT_ERROR;
	}
	
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\");
	strcat(szCurrentDirectory, "dictionary.zip");

	fp = fopen(szCurrentDirectory, "wb");
	fwrite(response, 1, size, fp);
	fclose(fp);
	
	HZIP hz = NULL;
	hz = OpenZip(szCurrentDirectory, 0);
	if (NULL == hz)
	{
		LOG(5, "사전 압축파일을 해제할 수 없습니다.\n");
		goto EXIT_ERROR;
	}
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	ZRESULT zResult = 0;
	zResult = SetUnzipBaseDir(hz, szCurrentDirectory, MAX_PATH);

	ZIPENTRY ze;
	zResult = GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int zi = 0; zi < numitems; zi++)
	{
		ZIPENTRY ze;
		zResult = GetZipItem(hz, zi, &ze); 
		zResult = UnzipItem(hz, zi, ze.name);	
	}
	CloseZip(hz);
	goto EXIT;
EXIT_ERROR:
EXIT:
	if (response) free(response);	
}

int LoadDictionary(char* szFileName, DIC *dic, BOOL bNeedBlank)
{
	char szCurrentDirectory[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\");
	strcat(szCurrentDirectory, szFileName);

	int nDicCount = 0;
	FILE *fp;
	// UTF8로 저장된 사전파일
	// UTF8저장시 첫 3바이트에 0xef 0xbb 0xbf가 저장되기 때문에 이를 버려야한다.
	fp = fopen(szCurrentDirectory, "r");
	if (NULL == fp)
	{
		LOG(5, "사전 파일이 없음. 프로세스를 종료한 후 사전파일을 복사해주세요~\n");
		Sleep(100000);
		return 0;
	}
	char buf[512] = { 0, };
	while (!feof(fp))
	{	
		fgets(buf, sizeof(buf), fp);
		
		if (strstr(buf, "$$$$")) continue;
		if (strstr(buf, "%%%%")) break;
		char* context = NULL;
		char* token = strtok_s(buf, "=", &context);
		char* token2 = strtok_s(NULL, "=", &context);

		// 주석처리
		char *comment = strstr(token2, " //");
		if (comment)
		{
			comment = 0x0;
		}
		WCHAR szUnicode[512] = L"";
		
		// UTF-8을 유니코드로 변환
		MultiByteToWideChar(CP_UTF8, 0, token, strlen(token), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));

		// 유니코드를 BIG5로 변환
		WideCharToMultiByte(950, 0, szUnicode, lstrlenW(szUnicode), dic[nDicCount].szChinese, sizeof(dic[nDicCount].szChinese), NULL, NULL);
		dic[nDicCount].nChineseLen = strlen(dic[nDicCount].szChinese);	
		ZeroMemory(szUnicode, sizeof(szUnicode));

		// UTF-8을 유니코드로 변환
		MultiByteToWideChar(CP_UTF8, 0, token2, strlen(token2), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		
		// 유니코드를 8비트 ASCII 한국어로 변환
		WideCharToMultiByte(949, 0, szUnicode, lstrlenW(szUnicode), dic[nDicCount].szKorean, sizeof(dic[nDicCount].szKorean), NULL, NULL);
		dic[nDicCount].szKorean[strlen(dic[nDicCount].szKorean) - 1] = 0; // \n 삭제
		
		// 한글 공백 채워넣기
		if (bNeedBlank)
		{
			if (dic[nDicCount].nChineseLen > strlen(dic[nDicCount].szKorean))
			{
				int nBlank = dic[nDicCount].nChineseLen - strlen(dic[nDicCount].szKorean);

				for (int j = 0; j < nBlank; j++)
				{
					strcat(dic[nDicCount].szKorean, " ");
				}
			}
		}
		nDicCount++;
	}
	fclose(fp);

//	LOG(13, "사전에서 %d 문장 읽음.\n", nDicCount);

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

void Trans(DWORD dwMemAddr, DWORD dwMemSize, DIC *dic, int nDicCount)
{
	for (int i = 0; i < nDicCount; i++)
	{
	//	LOG(14, "<%s>", dic[i].szKorean);
	//	LOG(10, " 검색중..\n");

		// "\x00문자열\x00" 형식으로 찾길 원하는 경우
		int copy_len = dic[i].nChineseLen;
		int plus_offset = 0;
		if (dic[i].szChinese[0] == 0x30)
		{
			dic[i].szChinese[0] = 0x00;
			copy_len = copy_len - 1;
			plus_offset++;
		}
		if (dic[i].szChinese[dic[i].nChineseLen - 1] == 0x30)
		{
			dic[i].szChinese[dic[i].nChineseLen - 1] = 0x00;
			copy_len = copy_len - 1;
		}
		size_t Offset = 0;

		do {
			size_t StartOffset = StringSearch((BYTE*)dwMemAddr, dwMemSize, Offset, (BYTE*)dic[i].szChinese, dic[i].nChineseLen);

			// 찾을 수 없다면
			if ((size_t)-1 == StartOffset)
			{
				break;
			}
		
			Offset = StartOffset + dic[i].nChineseLen;
			
			// 한번만 찾고 break
			if (dic[i].szChinese[0] == 0x00 || dic[i].szChinese[dic[i].nChineseLen - 1] == 0x00) 
			{
				memcpy((char*)((DWORD)dwMemAddr + StartOffset+plus_offset), dic[i].szKorean, copy_len);
				break;
			}
			else
			{
				strncpy((char*)((DWORD)dwMemAddr + StartOffset), dic[i].szKorean, dic[i].nChineseLen);
			}

		} while (Offset < dwMemSize);
	//	LOG(12, "\n");
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
	
	Updater();
	Hooking();

	DWORD OldProtect;
	// ID, PW를 입력하지 않아도 확인 버튼이 눌러지게
	DWORD* pCheckBox = (DWORD*)0x434F6D;
	VirtualProtectEx(GetCurrentProcess(), (void*)pCheckBox, 8, PAGE_EXECUTE_WRITECOPY, &OldProtect);
	// mov eax, 0x1
	// nop
	// nop
	// nop
	memcpy(pCheckBox, "\xB8\x01\x00\x00\x00\x90\x90\x90", 8);
	
	// 문자열이 모여있는 메모리를 RWE로 변경함
	DWORD dwThirdServerStringMem = 0x4EB000;
	DWORD dwThirdServerMemSize = 0x446000;
	VirtualProtectEx(GetCurrentProcess(), (void*)dwThirdServerStringMem, dwThirdServerMemSize, PAGE_EXECUTE_WRITECOPY, &OldProtect);
	
	DIC* dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(dic, sizeof(DIC) * 50000);
	int nDicCount = LoadDictionary("korean.txt", dic, TRUE);
	Trans(dwThirdServerStringMem, dwThirdServerMemSize, dic, nDicCount);
	free(dic);

	// papago thread
	DWORD dwThread2 = 0;
	HANDLE hDlgThread2 = 0;
	hDlgThread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PapagoThread, 0, 0, &dwThread2);

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

