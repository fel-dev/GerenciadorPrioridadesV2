#pragma once
#include <windows.h>
#include <commctrl.h>
#include <map>
#include <string>

struct SortParams {
    HWND hList;
    int column;
    bool ascending;
};

int CALLBACK CompararItens(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// Função para tratar o clique na coluna do ListView (LVN_COLUMNCLICK)
void TratarColumnClick(HWND hList, int col, std::map<int, bool>& ordemCrescentePorColuna, int& ultimaColunaOrdenada);

// Função para tratar custom draw do ListView (NM_CUSTOMDRAW)
LRESULT TratarCustomDraw(LPNMLVCUSTOMDRAW lplvcd);

// Manipulação dinâmica de colunas do ListView
void ListView_SetColumnTitle(HWND hList, int col, const std::wstring& title);
void ListView_SetColumnWidthEx(HWND hList, int col, int width);
void ListView_ShowColumn(HWND hList, int col, bool show);
