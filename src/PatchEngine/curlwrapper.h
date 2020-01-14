#pragma once
#include <windows.h>
BOOL OpenUrl(char* url, char* header1, char* header2, char* header3, char* field, OUT PBYTE response, OUT PDWORD size);