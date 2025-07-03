#pragma once
#include <windows.h>

#define ID_BTN_SALVAR_LOG 2007
#define ID_CHECK_AGRUPAR 5001

void TratarEventoBotao(HWND hwnd, WPARAM wParam);
DWORD GetProcessIdByName(const wchar_t* nomeProc);