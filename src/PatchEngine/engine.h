#pragma once

typedef struct _DIC
{
	char szChinese[512];
	int nChineseLen;
	char szKorean[512];
} DIC;

int LoadDictionary(char* szFileName, DIC *dic, BOOL bNeedBlank);
