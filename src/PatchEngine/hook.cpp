#include <windows.h>
#include <stdlib.h>
#include "engine.h"
#include "log.h"
#include "detours\detours.h"

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


/* esi에 NPC, 플레이어,게시판등 화면에 보여지는 사물 이름 출력
	00477309   52               PUSH EDX
	0047730A   51               PUSH ECX
	0047730B   50               PUSH EAX
	0047730C   57               PUSH EDI
	0047730D   56               PUSH ESI
	0047730E   E8 1DDC0300      CALL ddf.004B4F30
*/
PDETOUR_TRAMPOLINE pTrampoline3 = NULL;
void __stdcall TransFieldObject(void* esi);
__declspec(naked) void HOOK_GetFieldObject()
{
	__asm {
		__asm pushad
		__asm pushfd
		push esi
		call TransFieldObject
		__asm popfd
		__asm popad
		jmp[pTrampoline3]
	}
}
void __stdcall TransFieldObject(void* esi)
{
	/* 서버에서 받아온 패킷
		00 65 00 1E 8D E3 00 5B 99 38 00 00 04 29 00 00
		02 CB 00 00 04 29 00 00 02 CB 0B BB 45 B8 A8 AA      0B = 길이
		BA A9 B1 AD FB 00
	*/
	char *object = (char*)esi + 3;

	/* 대륙에서 1E가 아닌 값들이 발견
	// NPC, 게시판 등
	if (object[0] != 0x1E)
	{
		return;
	}
	*/

	char *str;
	str = (char*)esi + 27;

	DIC* dic = (DIC*)malloc(sizeof(DIC) * 800);
	int nDicCount = LoadDictionary("korean_fieldobject.txt", dic, TRUE);

	BOOL bTrans = FALSE;

	for (int i = 0; i < nDicCount; i++)
	{
		if (0 == strncmp(dic[i].szChinese, str, dic[i].nChineseLen))
		{
			strncpy(str, dic[i].szKorean, dic[i].nChineseLen);
			bTrans = TRUE;
		}
	}

	free(dic);

	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode));
		//	LOG(11, "len %d len2 %d\n", *len, *len2);
		LOGW(11, L"필드 오브젝트 : %s\n", szUnicode);
	}

}

/* esi에 몬스터 이름
	0047ADD0   8D8C24 F8000000  LEA ECX,DWORD PTR SS:[ESP+0xF8]
	0047ADD7   8D9424 60120000  LEA EDX,DWORD PTR SS:[ESP+0x1260]
	0047ADDE   51               PUSH ECX
	0047ADDF   52               PUSH EDX
	0047ADE0   50               PUSH EAX
	0047ADE1   57               PUSH EDI
	0047ADE2   56               PUSH ESI
	0047ADE3   E8 48A10300      CALL ddf.004B4F30                        ; 몬스터가 나타날때 여기 탐
*/
PDETOUR_TRAMPOLINE pTrampoline2 = NULL;
void __stdcall TransMonster(void* esi);
__declspec(naked) void HOOK_GetMonster()
{
	__asm {
		__asm pushad
		__asm pushfd
		push esi
		call TransMonster
		__asm popfd
		__asm popad
		jmp[pTrampoline2]
	}
}
void __stdcall TransMonster(void* esi)
{

	/* 서버에서 받아온 패킷
		03 F3 00 00 00 06 00 00 00 04 00 00 00 04 07 BA  
		F1 BD BF BD BB 00                                
	*/
	char *str = (char*)esi + 15;

//	char *len2 = (char*)esi + 5;
//	char *len = (char*)esi + 14;
		
	DIC* dic = (DIC*)malloc(sizeof(DIC) * 800);
	int nDicCount = LoadDictionary("korean_monster.txt", dic, TRUE);
	BOOL bTrans = FALSE;

	for (int i = 0; i < nDicCount; i++)
	{
		if (0 == strncmp(dic[i].szChinese, str, dic[i].nChineseLen))
		{
		//	strcpy(str, dic[i].szKorean);
		//	*len = strlen(dic[i].szKorean);
			strncpy(str, dic[i].szKorean, dic[i].nChineseLen);
			bTrans = TRUE;
		}
	}
	free(dic);
	
	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode));
		//	LOG(11, "len %d len2 %d\n", *len, *len2);
		LOGW(11, L"몬스터 : %s\n", szUnicode);
	}
}


/* 
	esi에 선택지 주소
*/
void __stdcall TransSelect(void* esi)
{
	// 서버에서 받아온 패킷
	/*
		01 E0 00 00 00 03 22 A9 B9 A8 BD AF 53 B4 B5 AE 
		71 A5 5F B3 A1 2C AC DD AC DD BD E6 AA BA AA 46  
		A6 E8 2C A8 53 A8 C6 2C 00                       

		오리진벨 길드사무관? 의 2번째 선택지에서는 시그내처 없이 바로 나옴
	*/
	char *str;
	// 시그내처 체크
	if (0 == memcmp((char*)esi, "\x01\xe0",2))
	{
		str = (char*)esi + 7;
	}
	else
	{
		str = (char*)esi;
	}
	   
	DIC* dic = (DIC*)malloc(sizeof(DIC) * 800);
	int nDicCount = LoadDictionary("korean_select.txt", dic, TRUE);
	BOOL bTrans = FALSE;

	for (int i = 0; i < nDicCount; i++)
	{
		if (0 == strncmp(dic[i].szChinese, str, dic[i].nChineseLen))
		{
			strncpy(str, dic[i].szKorean, dic[i].nChineseLen);
			bTrans = TRUE;
		}

	}
	free(dic);

	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode));
		LOGW(11, L"선택지 : %s\n", szUnicode);
	}

}

PDETOUR_TRAMPOLINE pTrampoline = NULL;

__declspec(naked) void HOOK_GetSelect()
{
	__asm {
		__asm pushad
		__asm pushfd
		push esi
		call TransSelect
		__asm popfd
		__asm popad
		jmp[pTrampoline]
	}
}


// 0017DE7C   0047ADE8  RETURN to ddf.0047ADE8 from ddf.004B4F30
// 한글: 0x4BC520
// ddf.exe : 0x4B4F30
typedef size_t(__cdecl* _S_4B4F30)(void* a1, void* a2, void* a3, char* a4, void* a5);
_S_4B4F30 S_4B4F30;
size_t __cdecl HOOK_S_4B4F30(void* a1, void* a2, void* a3, char* a4, void* a5)
{
	//big5->unicode
	WCHAR szUnicode[200] = L"";
	MultiByteToWideChar(950, 0, a4, strlen(a4), szUnicode, sizeof(szUnicode));
	LOGW(11, L"%s\n", szUnicode);
	return S_4B4F30(a1, a2, a3, a4, a5);
}


typedef int(__cdecl* _CHAT_AND_DIALOGUE)(void* a1, void* a2, void* a3, void* a4, void* a5, char* a6, int a7, void* a8, void* a9, void* a10, void* a11, void* a12, void* a13);
_CHAT_AND_DIALOGUE CHAT_AND_DIALOGUE;
/* 대사, 채팅 번역
	a6 : 대사
	a7 : 대사 길이
*/
int __cdecl HOOK_CHAT_AND_DIALOGUE(void* a1, void* a2, void* a3, void* a4, void* a5, char* a6, int a7, void* a8, void* a9, void* a10, void* a11, void* a12, void* a13)
{

	DIC* dic = (DIC*)malloc(sizeof(DIC) * 800);
	int nDicCount = LoadDictionary("korean_dialogue.txt", dic, FALSE);
	BOOL bTrans = FALSE;

	for (int i = 0; i < nDicCount; i++)
	{
		if (0 == strncmp(dic[i].szChinese, a6, dic[i].nChineseLen))
		{
			// 글자 잔상 방지를 위해 NULL을 2개 넣음
			memset(a6, 0, strlen(dic[i].szKorean) + 2);
			memcpy(a6, dic[i].szKorean, strlen(dic[i].szKorean));
			a7 = strlen(dic[i].szKorean) + 2;		
			bTrans = TRUE;
		}
	}
	free(dic);
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, a6, strlen(a6), szUnicode, sizeof(szUnicode));
		LOGW(11, L"대사 : %s\n", szUnicode);
	}
	return CHAT_AND_DIALOGUE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
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
	
	CHAT_AND_DIALOGUE = (_CHAT_AND_DIALOGUE)(DWORD)0x485060;
	DetourAttach(&(PVOID&)CHAT_AND_DIALOGUE, HOOK_CHAT_AND_DIALOGUE);
//	DetourAttach(&(PVOID&)S_4B4F30, HOOK_S_4B4F30);

	DWORD dwSelectAddr = 0x4798EF;
	DetourAttachEx(&(PVOID&)dwSelectAddr, HOOK_GetSelect, &pTrampoline, NULL, NULL);
	DWORD dwMonsterAddr = 0x47ADDE;
	DetourAttachEx(&(PVOID&)dwMonsterAddr, HOOK_GetMonster, &pTrampoline2, NULL, NULL);
	DWORD dwFieldObjectAddr = 0x477309;
	DetourAttachEx(&(PVOID&)dwFieldObjectAddr, HOOK_GetFieldObject, &pTrampoline3, NULL, NULL);
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
