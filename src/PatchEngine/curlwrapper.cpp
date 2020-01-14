#define CURL_STATICLIB
#include <curl.h>
#include <strsafe.h>
#include "log.h"
struct string
{
	char* ptr;
	size_t len;
};

static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, struct string* s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	memcpy((char*)s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

BOOL OpenUrl(char* url, char* header1, char* header2, char* header3, char *field, OUT PBYTE response, OUT PDWORD size)
{
	BOOL bRet = FALSE;
	CURL* curl;
	CURLcode res;
	struct curl_slist* list;
	// curl https response buffer √ ±‚»≠
	struct string s;
	s.len = 0;
	s.ptr = (char*)malloc(1);

	curl = curl_easy_init();
	if (NULL == curl)
	{
		bRet = FALSE;
		goto EXIT_ERROR;
	}
	   	 	
	curl_easy_setopt(curl, CURLOPT_URL, url);

	if (NULL != header1)
	{	
		list = curl_slist_append(list, header1);
		list = curl_slist_append(list, header2);
		list = curl_slist_append(list, header3);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}
	if (strstr(url, "https"))
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	if (NULL != field)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L); // POST
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, field);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(field));
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		bRet = FALSE;
		goto EXIT_ERROR;
	}
	memcpy(response, s.ptr, s.len);
	*size = s.len;
	bRet = TRUE;
	goto EXIT;
EXIT_ERROR:
EXIT:
	free(s.ptr);
	curl_slist_free_all(list);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return bRet;
}