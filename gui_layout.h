#pragma once
#include <windows.h>

// Cria e posiciona todos os controles da janela principal
void CriarControlesJanela(HWND hwnd, HWND& hBtnAlta, HWND& hBtnBaixa, HWND& hBtnAtualizar, HWND& hBtnBuscar, HWND& hListResult, HWND& hEditEntrada, HWND& hComboPrioridade, HWND& hBtnAplicarPrioridade, HWND& hBtnSalvarLog, HWND& hCheckFavoritarTodos);