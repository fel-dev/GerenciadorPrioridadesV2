#pragma once
#include <windows.h>
#include <map>

struct SortParams {
    HWND hList;
    int column;
    bool ascending;
};

int CALLBACK CompararItens(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// Função para tratar o clique na coluna do ListView (LVN_COLUMNCLICK)
void TratarColumnClick(HWND hList, int col, std::map<int, bool>& ordemCrescentePorColuna, int& ultimaColunaOrdenada);
