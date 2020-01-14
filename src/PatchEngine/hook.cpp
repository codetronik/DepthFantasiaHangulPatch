#include <windows.h>
#include <stdlib.h>
#include "engine.h"
#include "log.h"
#include "detours\detours.h"
#include "trans.h"
#include <time.h>

DIC* dialogue_dic;
int dialogue_nDicCount;
DIC* fieldobject_dic;
int fieldobject_nDicCount;
DIC* select_dic;
int select_nDicCount;
DIC* monster_dic;
int monster_nDicCount;
DIC* skilldescription_dic;
int skilldescription_nDicCount;
DIC* skill_dic;
int skill_nDicCount;
DIC* job_dic;
int job_nDicCount;
DIC* item_dic;
int item_nDicCount;

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
	if (cHeight == 10)
	{
		return TrueCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut,
			HANGUL_CHARSET, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
	}
	if (cHeight == 12)
	{
		return TrueCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut,
			HANGUL_CHARSET, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "고딕");
	}
	if (cHeight == 14)
	{
		return TrueCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut,
			HANGUL_CHARSET, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "고딕");
	}
	if (cHeight == 16)
	{
		return TrueCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut,
			HANGUL_CHARSET, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "고딕");
	}


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
	   
	BOOL bTrans = FALSE;

	for (int i = 0; i < fieldobject_nDicCount; i++)
	{
		if (0 == strncmp(fieldobject_dic[i].szChinese, str, fieldobject_dic[i].nChineseLen))
		{
			strncpy(str, fieldobject_dic[i].szKorean, fieldobject_dic[i].nChineseLen);
			bTrans = TRUE;
			break;
		}
	}

	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
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
void __stdcall TransMonster(void* esi, void* edi, void* eax);
__declspec(naked) void HOOK_GetMonster()
{
	__asm {
		__asm pushad
		__asm pushfd
		push eax
		push edi
		push esi
		call TransMonster
		__asm popfd
		__asm popad
		jmp[pTrampoline2]
	}
}
void __stdcall TransMonster(void* esi, void *edi, void *eax)
{
//	LOG(11, "%d\n", edi);
//	LOG(11, "%d\n", eax);

	//LOGHEX((PBYTE)edi, 16);
//	LOGHEX((PBYTE)eax, 16);

	/* 서버에서 받아온 패킷
		03 F3 00 00 00 06 00 00 00 04 00 00 00 04 07 BA  
		F1 BD BF BD BB 00                                
	*/
	char *str = (char*)esi + 15;
//	LOGHEX((PBYTE)esi, 128);

//	char *len2 = (char*)esi + 5;
	char *name_len = (char*)esi + 14;

	char* look = (char*)esi + 14 + *name_len + 18;
	char* look2 = (char*)esi + 14 + *name_len + 19;
	char* look3 = (char*)esi + 14 + *name_len + 20;
	LOG(12, "몬스터 외형 : %02x %02x %02x\n", *look, *look2, *look3);
		
	if (*look == 0x00) // 플레이어만
	{
		char szCurrentDirectory[MAX_PATH] = { 0, };
		GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
		strcat(szCurrentDirectory, "\\");
		strcat(szCurrentDirectory, "character_look.txt");

		FILE* fp = fopen(szCurrentDirectory, "r");
		if (NULL != fp)
		{
			char buf[512] = { 0, };
			while (!feof(fp))
			{
				fgets(buf, sizeof(buf), fp);

				if (strstr(buf, "$$$$")) continue;
				if (strstr(buf, "%%%%")) break;
				if (strstr(buf, "#")) continue;
				// 주석처리
				char* comment = strstr(buf, " //");
				if (comment)
				{
					comment = 0x0;
				}

				char* context = NULL;
				char* token = strtok_s(buf, ":", &context);
				char* token2 = strtok_s(NULL, ":", &context);
				char* token3 = strtok_s(NULL, ":", &context);
				*look = strtol(token, NULL, 16);
				*look2 = strtol(token2, NULL, 16);
				*look3 = strtol(token3, NULL, 16);			
			}			
		}
		if (fp)	fclose(fp);		
	}
	
	DWORD *monster_hp = (DWORD*)((DWORD)esi + 15 + *name_len + 4);
	
	// 서버에서 받아온 몬스터 체력은 big endian이므로 little endian으로 변환
	DWORD monhp = (*monster_hp & 0x000000ff) << 24u | (*monster_hp & 0x0000ff00) << 8u 
				| (*monster_hp & 0x00ff0000) >> 8u | (*monster_hp & 0xff000000) >> 24u;
		
	BOOL bTrans = FALSE;

	for (int i = 0; i < monster_nDicCount; i++)
	{
		// 사전의 중국어 이름과 메모리의 이름 비교
		if (0 == strncmp(monster_dic[i].szChinese, str, monster_dic[i].nChineseLen))
		{
			strncpy(str, monster_dic[i].szKorean, monster_dic[i].nChineseLen);

			LOG(11, "몬스터 : %s (MAX HP: %d)\n", str, monhp);
			bTrans = TRUE;
			break;
		}
	}
	
	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(11, L"(미번역) 몬스터 : %s (MAX HP : %d)\n", szUnicode, monhp);

		// 화면에 중국어로 표시
		char temp[200] = { 0, };
		WideCharToMultiByte(949, 0, szUnicode, lstrlenW(szUnicode), temp, sizeof(temp), NULL, NULL);
		strncpy((char*)str, temp, strlen(temp));
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

	BOOL bTrans = FALSE;

	for (int i = 0; i < select_nDicCount; i++)
	{
		if (0 == strncmp(select_dic[i].szChinese, str, select_dic[i].nChineseLen))
		{
			strncpy(str, select_dic[i].szKorean, select_dic[i].nChineseLen);
			bTrans = TRUE;
			break;
		}
	}

	if (FALSE == bTrans)
	{
		// big5->유니코드로 변환
		WCHAR szUnicode[512] = L"";
		MultiByteToWideChar(950, 0, str, strlen(str), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
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
	MultiByteToWideChar(950, 0, a4, strlen(a4), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
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
	BOOL bTrans = FALSE;

	for (int i = 0; i < dialogue_nDicCount; i++)
	{
		if (0 == strncmp(dialogue_dic[i].szChinese, a6, dialogue_dic[i].nChineseLen))
		{
			// 글자 잔상 방지를 위해 NULL을 2개 넣음
			memset(a6, 0, strlen(dialogue_dic[i].szKorean) + 2);
			memcpy(a6, dialogue_dic[i].szKorean, strlen(dialogue_dic[i].szKorean));
			a7 = strlen(dialogue_dic[i].szKorean) + 2;		
			bTrans = TRUE;
			break;
		}
	}
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, a6, strlen(a6), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(11, L"대사 : %s\n", szUnicode);
		PushDialogue(szUnicode);

	}
	return CHAT_AND_DIALOGUE(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

typedef int(__thiscall* _S_3ED90)(void* pThis, const char* a2, void* a3, void* a4);
_S_3ED90 S_3ED90;
int __fastcall Hook_S_3ED90(void* pThis, void* Unknown, const char* a2, void* a3, void* a4)
{
	GetOffset(4);
	WCHAR szUnicode[300] = L"";
	MultiByteToWideChar(950, 0, a2, strlen(a2), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
	LOGW(15, L"%s\n", szUnicode);
//	if (strlen(a2)>10)	PushDialogue(szUnicode);
	return S_3ED90(pThis, a2, a3, a4);
}

typedef int(__cdecl* _S_CE7D6)(void* a1, void* a2, void* a3, void* a4);
_S_CE7D6 S_CE7D6;
int __cdecl Hook_S_CE7D6(void* a1, void* a2, void* a3, void* a4)
{
	WCHAR szUnicode[200] = L"";
	MultiByteToWideChar(950, 0, (char*)a3, strlen((char*)a3), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
	LOGW(11, L":>>> %s\n", szUnicode);
	return S_CE7D6(a1, a2, a3, a4);
}
typedef int(__cdecl* _S_D2587)(void* a1, void* a2, void* a3);
_S_D2587 S_D2587;
int __cdecl Hook_S_D2587(void* a1, void* a2, void* a3)
{
	WCHAR szUnicode[200] = L"";
	MultiByteToWideChar(950, 0, (char*)a2, strlen((char*)a2), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
	LOGW(11, L":>>> %s\n", szUnicode);
	return S_D2587(a1, a2, a3);
}

// 스킬 설명 번역 (필드)
/*
.text:004238E2                 push    eax
.text:004238E3                 mov     eax, [ebx+40h]
.text:004238E6                 push    eax
.text:004238E7                 push    offset aSSpLd   ; "%s  SP%ld"
.text:004238EC                 push    ecx
.text:004238ED                 call    sub_4CE7D6
.text:004238F2                 lea     edi, [esp+120h+var_100]
*/
PDETOUR_TRAMPOLINE pTrampoline4 = NULL;
void __stdcall TransSkillDescription(void* edi);
__declspec(naked) void HOOK_GetSkillDescription()
{
	__asm {
		__asm pushad
		__asm pushfd
		push edi
		call TransSkillDescription
		__asm popfd
		__asm popad
		jmp[pTrampoline4]
	}
}

// 스킬 설명 번역 (전투중)
/*
.text:00439655                 push    offset aSSpLd   ; "%s  SP%ld"
.text:0043965A                 push    eax
.text:0043965B                 call    sub_4CE7D6
.text:00439660                 lea     edi, [esp+128h+sz]
*/
PDETOUR_TRAMPOLINE pTrampoline7 = NULL;
__declspec(naked) void HOOK_GetSkillDescription2()
{
	__asm {
		__asm pushad
		__asm pushfd
		push edi
		call TransSkillDescription
		__asm popfd
		__asm popad
		jmp[pTrampoline7]
	}
}

void __stdcall TransSkillDescription(void* edi)
{
	BOOL bTrans = FALSE;

	for (int i = 0; i < skilldescription_nDicCount; i++)
	{		
		if (0 == strncmp(skilldescription_dic[i].szChinese, (char*)edi, skilldescription_dic[i].nChineseLen))
		{
			// edi의 구조 : "%s  SP%ld"
			// SP%ld를 잘라낸뒤 번역한 것 뒤에 붙인다.
			char* sp = strstr((char*)edi + strlen((char*)edi) - 7, "SP"); // SP를 소모 SP10 : 이런 경우를 대비해 뒤의 SP만 취함
			char new_sp[10] = { 0, };
			strcpy_s(new_sp, 10, sp);
			strcpy((char*)edi, skilldescription_dic[i].szKorean);
			strcat((char*)edi, " ");
			strcat((char*)edi, new_sp);
			bTrans = TRUE;
			break;
		}
	}
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)edi, strlen((char*)edi), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(14, L"%s\n", szUnicode);
	}
}

// 스킬 창 눌렀을 때
/*
.text:004231FB                 lea     ecx, [esp+1B8h+var_140]
.text:004231FF                 lea     esi, [ebp+0CCh]
.text:00423205                 push    0Ch
.text:00423207                 push    ecx
.text:00423208                 mov     ecx, esi
.text:0042320A                 call    sub_43ED90
*/
PDETOUR_TRAMPOLINE pTrampoline5 = NULL;
void __stdcall TransSkill(void* ecx);
__declspec(naked) void HOOK_GetSkill()
{	
	__asm {
		pushad
		pushfd
		push ecx
		call TransSkill
		popfd
		popad
		jmp[pTrampoline5]
	}
}

/*
.text:00422F5C                 lea     ecx, [esp+1B8h+var_140]
.text:00422F60                 push    0Ch
.text:00422F62                 push    ecx
.text:00422F63                 mov     ecx, ebx
.text:00422F65                 call    sub_43ED90
*/
PDETOUR_TRAMPOLINE pTrampoline33 = NULL;
__declspec(naked) void HOOK_GetSkill3()
{
	__asm {
		pushad
		pushfd
		push ecx
		call TransSkill
		popfd
		popad
		jmp[pTrampoline33]
	}
}

void __stdcall TransSkill(void* ecx)
{
	WCHAR szUnicode[200] = L"";
	MultiByteToWideChar(950, 0, (char*)ecx, strlen((char*)ecx), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));

	BOOL bTrans = FALSE;
	for (int i = 0; i < skill_nDicCount; i++)
	{
		if (0 == strncmp(skill_dic[i].szChinese, (char*)ecx, skill_dic[i].nChineseLen))
		{
			if (strstr((char*)ecx, "    "))
			{
				// 스킬 단축키 지정시 "%s         %d" 형태임
				char skill[34] = { 0x00, };
				memset(skill, 0x20, 33);
				skill[32] = *((char*)ecx + 32);
				memcpy((char*)skill, skill_dic[i].szKorean, strlen(skill_dic[i].szKorean));
				strcpy((char*)ecx, skill);
			}
			else
			{
				strcpy((char*)ecx, skill_dic[i].szKorean);
			}
		
			bTrans = TRUE;	
			break;
		}
	}
	
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)ecx, strlen((char*)ecx), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(15, L"%s\n", szUnicode);
	}
	
}


PDETOUR_TRAMPOLINE pTrampoline6 = NULL;
void __stdcall TransSkill2(void* edx);
__declspec(naked) void HOOK_GetSkill2()
{
	__asm {
		pushad
		pushfd
		push edx
		call TransSkill2
		popfd
		popad
		jmp[pTrampoline6]
	}
}

void __stdcall TransSkill2(void* ecx)
{		
	BOOL bTrans = FALSE;
	for (int i = 0; i < skill_nDicCount; i++)
	{
		if (0 == strncmp(skill_dic[i].szChinese, (char*)ecx, skill_dic[i].nChineseLen))
		{
			strcpy((char*)ecx, skill_dic[i].szKorean);
			bTrans = TRUE;			
			break;
		}
	}
	
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)ecx, strlen((char*)ecx), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(15, L"%s\n", szUnicode);	
	}

}


/*
.text:004533CF                 mov     ecx, [esi+4]
.text:004533D2                 push    ecx
.text:004533D3                 push    eax
.text:004533D4                 push    offset aSS      ; "%s%s"
.text:004533D9                 push    offset dword_8E9F34
.text:004533DE                 call    sub_4CE7D6
*/
char* g_ecx;
PDETOUR_TRAMPOLINE pTrampoline66 = NULL;
void __stdcall TransSkill4(void* ecx);
__declspec(naked) void HOOK_GetSkill4()
{
	__asm {
		pushad
		pushfd
		push ecx
		call TransSkill4
		popfd
		popad
		mov ecx, g_ecx;
		jmp[pTrampoline66]
	}
}
void __stdcall TransSkill4(void* ecx)
{
	BOOL bTrans = FALSE;
	for (int i = 0; i < skill_nDicCount; i++)
	{
		if (0 == strncmp(skill_dic[i].szChinese, (char*)ecx, skill_dic[i].nChineseLen))
		{
			g_ecx = (char*)malloc(30);
			strcpy((char*)g_ecx, skill_dic[i].szKorean);
			bTrans = TRUE;
			break;
		}
	}

	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)ecx, strlen((char*)ecx), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(15, L"%s\n", szUnicode);
	}

}


PDETOUR_TRAMPOLINE pTrampoline8 = NULL;
void __stdcall TransJob(void* edx);
__declspec(naked) void HOOK_GetJob()
{
	__asm {
		pushad
		pushfd
		push edx
		call TransJob
		popfd
		popad
		jmp[pTrampoline8]
	}
}

// 두번 탐. 1번째는 edx에 중국어 2번째는 edx에 한국어
void __stdcall TransJob(void* edx)
{


	BOOL bTrans = FALSE;
	for (int i = 0; i < job_nDicCount; i++)
	{	
		if (0 == strncmp(job_dic[i].szKorean, (char*)edx, strlen(job_dic[i].szKorean))) // 2번째 처리
		{
			bTrans = TRUE;
			break;
		}
		else if (0 == strncmp(job_dic[i].szChinese, (char*)edx, job_dic[i].nChineseLen)) // 1번째 처리
		{			
			strcpy((char*)edx, job_dic[i].szKorean);
			bTrans = TRUE;
			break;
		}
	}
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)edx, strlen((char*)edx), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(19, L"(job) %s %d\n", szUnicode, strlen((char*)edx));
	
	}
}

// ID, PW 자동 입력
char id[30];
char pw[30];
DWORD s1 = 0;
DWORD s2 = 0;
typedef int(__cdecl* _S_74BE0)(void* a1, void* a2);
_S_74BE0 S_74BE0;
int __cdecl Hook_S_74BE0(void* a1, void* a2)
{
	s1 = timeGetTime(); // 5초이상 미 로그인시 id/pw새로 지정
	if (s2 == 0 || s1 - s2 > 5000)
	{
		if (0 == strcmp((char*)a1, "1"))
		{
			strcpy((char*)a1, "Corea");
			strcpy((char*)a2, "Corea");
			strcpy((char*)id, "Corea");
			strcpy((char*)pw, "Corea");
		}
		else if (0 == strcmp((char*)a1, "2"))
		{
			strcpy((char*)a1, "Korea");
			strcpy((char*)a2, "Korea");
			strcpy((char*)id, "Korea");
			strcpy((char*)pw, "Korea");
		}
		else if (strlen((char*)a1) > 2 && strlen((char*)a2) > 2) // 오토로그인 사용하지 않음
		{
			strcpy((char*)id, (char*)a1);
			strcpy((char*)pw, (char*)a2);
		}
		else
		{
			char szCurrentDirectory[MAX_PATH] = { 0, };
			GetCurrentDirectoryA(MAX_PATH, szCurrentDirectory);
			strcat(szCurrentDirectory, "\\");
			strcat(szCurrentDirectory, "login.txt");

			FILE* fp = fopen(szCurrentDirectory, "r");
			if (NULL == fp)
			{
				goto EXIT;
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
				token2[strlen(token2) - 1] = 0; // \n 제거
			//	LOG(13, "ID :%s PW : %s\n", buf, token, token2);
				strcpy((char*)a1, token);
				strcpy((char*)a2, token2);
				strcpy((char*)id, token);
				strcpy((char*)pw, token2);
			}
			fclose(fp);
		}
	}
	else
	{
		strcpy((char*)a1, id);
		strcpy((char*)a2, pw);
	}

EXIT:
	s2 = timeGetTime();
	return S_74BE0(a1, a2);
}

// 서버에서 막 받아온 따끈따끈한 데이터를 처리
PDETOUR_TRAMPOLINE pTrampoline9 = NULL;
void __stdcall TransItem(char* str);
__declspec(naked) void HOOK_GetItemName()
{
	__asm {
		pushad
		pushfd
		mov eax, [esp+0x30]
		push eax
		call TransItem
		popfd
		popad
		jmp[pTrampoline9]
	}
}
PDETOUR_TRAMPOLINE pTrampoline10 = NULL;
__declspec(naked) void HOOK_GetItemDescription()
{
	__asm {
		pushad
		pushfd
		mov eax, [esp + 0x30]
		push eax
		call TransItem
		popfd
		popad
		jmp[pTrampoline10]
	}
}

void __stdcall TransItem(char* str)
{
	
	BOOL bTrans = FALSE;
	for (int i = 0; i < item_nDicCount; i++)
	{	
		char* wild = strstr(item_dic[i].szChinese, "#");
		if (wild)
		{
			// 와일드 캐릭터 이전까지 비교
			int len_w = wild - 1 - item_dic[i].szChinese;
			if (0 == strncmp(item_dic[i].szChinese, str, len_w))
			{
				char backup[10] = { 0, };
				strcpy(backup, str + len_w + 1);
				strcpy((char*)str, item_dic[i].szKorean);
				strcat((char*)str, backup);
				bTrans = TRUE;
				break;
			}
		}
		else if (0 == strncmp(item_dic[i].szChinese, str, item_dic[i].nChineseLen))
		{
				strcpy((char*)str, item_dic[i].szKorean);
				bTrans = TRUE;
				break;
			
		}
	}
	if (FALSE == bTrans)
	{
		//big5->unicode
		WCHAR szUnicode[200] = L"";
		MultiByteToWideChar(950, 0, (char*)str, strlen((char*)str), szUnicode, sizeof(szUnicode) / sizeof(WCHAR));
		LOGW(19, L"(item) %s\n", szUnicode);

		// 화면에 중국어로 표시
		char temp[200] = { 0, };
		WideCharToMultiByte(949, 0, szUnicode, lstrlenW(szUnicode), temp, sizeof(temp), NULL, NULL);
		strcpy((char*)str, temp);
	}
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
	
	dialogue_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(dialogue_dic, sizeof(DIC) * 50000);
	fieldobject_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(fieldobject_dic, sizeof(DIC) * 50000);
	select_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(select_dic, sizeof(DIC) * 50000);
	monster_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(monster_dic, sizeof(DIC) * 50000);
	skilldescription_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(skilldescription_dic, sizeof(DIC) * 50000);
	skill_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(skill_dic, sizeof(DIC) * 50000);
	job_dic = (DIC*)malloc(sizeof(DIC) * 120);
	ZeroMemory(job_dic, sizeof(DIC) * 120);
	item_dic = (DIC*)malloc(sizeof(DIC) * 50000);
	ZeroMemory(item_dic, sizeof(DIC) * 50000);

	dialogue_nDicCount = LoadDictionary("korean_dialogue.txt", dialogue_dic, FALSE);
	fieldobject_nDicCount = LoadDictionary("korean_fieldobject.txt", fieldobject_dic, TRUE);
	select_nDicCount = LoadDictionary("korean_select.txt", select_dic, TRUE);
	monster_nDicCount = LoadDictionary("korean_monster.txt", monster_dic, TRUE);
	skilldescription_nDicCount = LoadDictionary("korean_skilldescription.txt", skilldescription_dic, FALSE);
	skill_nDicCount = LoadDictionary("korean_skill.txt", skill_dic, FALSE);
	job_nDicCount = LoadDictionary("korean_job.txt", job_dic, FALSE);
	item_nDicCount = LoadDictionary("korean_item.txt", item_dic, FALSE);

	CHAT_AND_DIALOGUE = (_CHAT_AND_DIALOGUE)(DWORD)0x485060;
	DetourAttach(&(PVOID&)CHAT_AND_DIALOGUE, HOOK_CHAT_AND_DIALOGUE);
//	DetourAttach(&(PVOID&)S_4B4F30, HOOK_S_4B4F30);

	DWORD dwSelectAddr = 0x4798EF;
	DetourAttachEx(&(PVOID&)dwSelectAddr, HOOK_GetSelect, &pTrampoline, NULL, NULL);
	DWORD dwMonsterAddr = 0x47ADDE;
	DetourAttachEx(&(PVOID&)dwMonsterAddr, HOOK_GetMonster, &pTrampoline2, NULL, NULL);
	DWORD dwFieldObjectAddr = 0x477309;
	DetourAttachEx(&(PVOID&)dwFieldObjectAddr, HOOK_GetFieldObject, &pTrampoline3, NULL, NULL);
	S_3ED90 = (_S_3ED90)(DWORD)0x43ED90;
//	DetourAttach(&(PVOID&)S_3ED90, Hook_S_3ED90); // 화면에 텍스트 뿌려주는 부분
	S_CE7D6 = (_S_CE7D6)(DWORD)0x4CE7D6;
//	DetourAttach(&(PVOID&)S_CE7D6, Hook_S_CE7D6); // sprintf
	S_D2587 = (_S_D2587)(DWORD)0x4D2587;
//	DetourAttach(&(PVOID&)S_D2587, Hook_S_D2587); // 

	DWORD dwSkillDescription = 0x4238F6;
	DetourAttachEx(&(PVOID&)dwSkillDescription, HOOK_GetSkillDescription, &pTrampoline4, NULL, NULL); // 스킬 메뉴
	DWORD dwSkillDescription2 = 0x439664;
	DetourAttachEx(&(PVOID&)dwSkillDescription2, HOOK_GetSkillDescription2, &pTrampoline7, NULL, NULL); // 전투중 스킬설명
	DWORD dwSkill = 0x423207;
	DetourAttachEx(&(PVOID&)dwSkill, HOOK_GetSkill, &pTrampoline5, NULL, NULL); // 습득기술 메뉴
	DWORD dwSkill3 = 0x422F62;
	DetourAttachEx(&(PVOID&)dwSkill3, HOOK_GetSkill3, &pTrampoline33, NULL, NULL); // 사용스킬 메뉴
	DWORD dwSkill2 = 0x436CFF;
	DetourAttachEx(&(PVOID&)dwSkill2, HOOK_GetSkill2, &pTrampoline6, NULL, NULL); // 전투중 스킬
	DWORD dwSkill4 = 0x4533D2;
	DetourAttachEx(&(PVOID&)dwSkill4, HOOK_GetSkill4, &pTrampoline66, NULL, NULL); // 전투중 말풍선 스킬
	DWORD dwJob = 0x41DF5C;
	DetourAttachEx(&(PVOID&)dwJob, HOOK_GetJob, &pTrampoline8, NULL, NULL); // 직업
	DWORD dwItemName = 0x478836;
	DetourAttachEx(&(PVOID&)dwItemName, HOOK_GetItemName, &pTrampoline9, NULL, NULL); // 아이템 이름
	DWORD dwItemDescription = 0x478859;
	DetourAttachEx(&(PVOID&)dwItemDescription, HOOK_GetItemDescription, &pTrampoline10, NULL, NULL); // 아이템 설명
	S_74BE0 = (_S_74BE0)(DWORD)0x474BE0;
	DetourAttach(&(PVOID&)S_74BE0, Hook_S_74BE0); // 자동 로그인

	LONG error = DetourTransactionCommit();

	if (NO_ERROR != error)
	{
		LOG(12, "후킹 실패!! 프로세스를 강제 종료해주세요.\n");
		Sleep(100000);
	}
	else
	{
		//LOG(13, "후킹 성공!!\n");
	}
}
