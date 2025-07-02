#pragma once
#include <windows.h>

// Cria e posiciona todos os controles da janela principal
void CriarControlesJanela(HWND hwnd, HWND& hBtnAlta, HWND& hBtnBaixa, HWND& hBtnAtualizar, HWND& hBtnBuscar, HWND& hListResult, HWND& hEditEntrada, HWND& hComboPrioridade, HWND& hBtnAplicarPrioridade, HWND& hBtnSalvarLog, HWND& hCheckFavoritarTodos, HWND& hBtnReverter);

// Fun��o utilit�ria para criar bot�es Unicode
HWND CriarBotaoUnicode(HWND hwnd, int x, int y, int w, int h, int id, const wchar_t* texto);

// Fun��o utilit�ria para aplicar fonte emoji
void SetFonteEmoji(HWND hWnd);