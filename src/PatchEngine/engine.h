#pragma once

typedef struct _DIC
{
	char szChinese[512];
	int nChineseLen;
	char szKorean[512];
} DIC;

typedef struct _LOOK
{
	unsigned char lookInfo[6];
} LOOK;

int LoadDictionary(char* szFileName, DIC *dic, BOOL bNeedBlank);
