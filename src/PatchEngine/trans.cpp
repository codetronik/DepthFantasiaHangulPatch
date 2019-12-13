#define CURL_STATICLIB
#include "log.h"
#include <curl.h>
#include <strsafe.h>

WCHAR szDialogStack[100][1000] = { 0, };

struct string 
{
	char* ptr;
	size_t len;
};

size_t WriteCallback(void* ptr, size_t size, size_t nmemb, struct string* s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	memcpy((char*)s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

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
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (NULL == curl)
	{
		return FALSE;
	}
	
	// 파파고 번역을 매끄럽게 하기 위해 대사에서 \n을 삭제함
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

	// papago post데이터에 대사를 붙임
	WCHAR szPostData[512] = { 0, };
	StringCchPrintfW(szPostData, 512, L"source=zh-TW&target=ko&text=%s", pszDialogue);

	// 유니코드를 UTF8로 변환
	char szUTF8Dialogue[512] = { 0, };
	WideCharToMultiByte(CP_UTF8, 0, szPostData, lstrlenW(szPostData), szUTF8Dialogue, sizeof(szUTF8Dialogue), NULL, NULL);
	
	// curl https response buffer 초기화
	struct string s;
	s.len = 0;
	s.ptr = (char*)malloc(1);

	struct curl_slist* list = NULL;

	curl_easy_setopt(curl, CURLOPT_URL, "https://openapi.naver.com/v1/papago/n2mt"); 
	list = curl_slist_append(list, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	list = curl_slist_append(list, "X-Naver-Client-Id: SieKKQkciwUN83a2alSp");
	list = curl_slist_append(list, "X-Naver-Client-Secret: JpiYzwscWO");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list); 
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L); // POST
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szUTF8Dialogue); 
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(szUTF8Dialogue));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

	res = curl_easy_perform(curl); 
	curl_slist_free_all(list); 

	if (res != CURLE_OK)
	{
		return FALSE;
	}
	// curl 응답값을 유니코드로 변환
	WCHAR szUnicodeResponse[1000] = { 0, };
	MultiByteToWideChar(CP_UTF8, 0, (char*)s.ptr, s.len, szUnicodeResponse, sizeof(szUnicodeResponse) / sizeof(WCHAR));

	// 파파고 응답값에서 번역 추출
	WCHAR* start = wcsstr(szUnicodeResponse, L"Text\"");
	WCHAR* end = wcsstr(szUnicodeResponse, L"\"}}");

	if (NULL == start)
	{
		return FALSE;
	}

	int copyLen = end - start - 7;
	WCHAR szTrans[1000] = { 0, };
	wcsncpy_s(szTrans, 1000, start + 7, copyLen);

	// 유니코드를 8비트 ASCII 한국어로 변환
	char szAnsiResponse[1000] = { 0, };
	WideCharToMultiByte(949, 0, szTrans, lstrlenW(szTrans), szAnsiResponse, sizeof(szAnsiResponse), NULL, NULL);
	LOG(12, "번역 대사 : %s\n", szAnsiResponse);
	free(s.ptr);
	curl_easy_cleanup(curl);
	curl_global_cleanup(); 
}

void PapagoThread()
{
	while (1)
	{
		Sleep(100);
		for (int i = 0; i < 100; i++)
		{
			if (0 != wcscmp(szDialogStack[i], L""))
			{
				TransPapago(szDialogStack[i]);
				StringCchCopyW(szDialogStack[i], 1000, L""); // 번역 후 delete
			}
		}
	}
}