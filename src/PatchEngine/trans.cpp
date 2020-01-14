#define CURL_STATICLIB
#include "log.h"
#include <strsafe.h>
#include "curlwrapper.h"
WCHAR szDialogStack[100][1000] = { 0, };

char pa_id[100] = { 0, };
char pa_secret[100] = { 0, };
void PushDialogue(WCHAR* pszDialogue)
{
	for (int i = 0; i < 100; i++)
	{
		if (0 == wcscmp(szDialogStack[i], L""))
		{
			StringCchCopyW(szDialogStack[i], 1000, pszDialogue);
			break;
		}
	}
}

BOOL TransPapago(WCHAR* pszDialogue)
{
	// ���İ� ������ �Ų����� �ϱ� ���� ��翡�� \n�� ������
	WCHAR* split = NULL;
	while (split = wcsstr(pszDialogue, L"\\n"))
	{
		if (split == NULL)
		{
			break;
		}
		WCHAR temp1[1000] = { 0, };
		WCHAR temp2[1000] = { 0, };
		wcsncpy_s(temp1, pszDialogue, split - pszDialogue);
	
		StringCchPrintfW(temp2, 1000, L"%s%s", temp1, split + 2);
		StringCchCopyW(pszDialogue, 1000, temp2);
	}

	// papago post�����Ϳ� ��縦 ����
	WCHAR szPostData[512] = { 0, };
	StringCchPrintfW(szPostData, 512, L"source=zh-TW&target=ko&text=%s", pszDialogue);

	// �����ڵ带 UTF8�� ��ȯ
	char szUTF8Dialogue[512] = { 0, };
	WideCharToMultiByte(CP_UTF8, 0, szPostData, lstrlenW(szPostData), szUTF8Dialogue, sizeof(szUTF8Dialogue), NULL, NULL);

	char header1[255] = "Content-Type: application/x-www-form-urlencoded; charset=UTF-8";
	char header2[255] = "X-Naver-Client-Id: ";
	char header3[255] = "X-Naver-Client-Secret: ";
	strcat(header2, pa_id);
	strcat(header3, pa_secret);

	BYTE response[1024] = { 0, };
	DWORD size = 0;
	BOOL bSuccess = OpenUrl("https://openapi.naver.com/v1/papago/n2mt", header1, header2, header3, szUTF8Dialogue, response, &size);
	if (FALSE == bSuccess)
	{
		return FALSE;
	}

	// curl ���䰪�� �����ڵ�� ��ȯ
	WCHAR szUnicodeResponse[1000] = { 0, };
	MultiByteToWideChar(CP_UTF8, 0, (char*)response, size, szUnicodeResponse, sizeof(szUnicodeResponse) / sizeof(WCHAR));

	
	// ���İ� ���䰪���� ���� ����
	WCHAR* start = wcsstr(szUnicodeResponse, L"Text\"");
	WCHAR* end = wcsstr(szUnicodeResponse, L"\"}}");

	if (NULL == start)
	{
		return FALSE;
	}

	int copyLen = end - start - 7;
	WCHAR szTrans[1000] = { 0, };
	wcsncpy_s(szTrans, 1000, start + 7, copyLen);

	// �����ڵ带 8��Ʈ ASCII �ѱ���� ��ȯ
	char szAnsiResponse[1000] = { 0, };
	WideCharToMultiByte(949, 0, szTrans, lstrlenW(szTrans), szAnsiResponse, sizeof(szAnsiResponse), NULL, NULL);
	LOG(12, "���� ��� : %s\n", szAnsiResponse);

}

void PapagoThread()
{
	// ���İ� ���� id, secret ����
	char szCurrentDirectory[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
	strcat(szCurrentDirectory, "\\");
	strcat(szCurrentDirectory, "papago.txt");

	FILE* fp = fopen(szCurrentDirectory, "r");
	
	if (NULL == fp)
	{
		return;
	}
	char buf[512] = { 0, };
	while (!feof(fp))
	{
		fgets(buf, sizeof(buf), fp);
		if (strstr(buf, "$$$$")) continue;
		if (strstr(buf, "%%%%")) break;
	
		char* context = NULL;
		char* token = strtok_s(buf, ":", &context);
		char* token2 = strtok_s(NULL, ":", &context);
		strcpy(pa_id, token);
		strcpy(pa_secret, token2);
		pa_secret[strlen(pa_secret) - 1] = 0; // \n ����
	}
	if (fp)	fclose(fp);

	while (1)
	{
		Sleep(100);
		for (int i = 0; i < 100; i++)
		{
			if (0 != wcscmp(szDialogStack[i], L""))
			{
				TransPapago(szDialogStack[i]);
				StringCchCopyW(szDialogStack[i], 1000, L""); // ���� �� delete
			}
		}
	}
}