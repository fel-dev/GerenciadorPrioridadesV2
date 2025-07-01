#include "listview_logic.h"
#include <commctrl.h>
#include <cwchar>

int CALLBACK CompararItens(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    SortParams* params = (SortParams*)lParamSort;
    WCHAR texto1[256] = {}, texto2[256] = {};
    ListView_GetItemText(params->hList, lParam1, params->column, texto1, 256);
    ListView_GetItemText(params->hList, lParam2, params->column, texto2, 256);
    int resultado = 0;
    if (params->column == 4) { // Memória (coluna 4): ordenar como número
        int mem1 = _wtoi(texto1);
        int mem2 = _wtoi(texto2);
        resultado = mem1 - mem2;
    } else {
        resultado = _wcsicmp(texto1, texto2);
    }
    return params->ascending ? resultado : -resultado;
}

void TratarColumnClick(HWND hList, int col, std::map<int, bool>& ordemCrescentePorColuna, int& ultimaColunaOrdenada) {
    bool crescente = !ordemCrescentePorColuna[col];
    ordemCrescentePorColuna[col] = crescente;
    // Atualiza setas no header
    HWND hHeader = ListView_GetHeader(hList);
    for (int i = 0; i < Header_GetItemCount(hHeader); ++i) {
        HDITEM item = { 0 };
        item.mask = HDI_FORMAT;
        Header_GetItem(hHeader, i, &item);
        item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN); // limpa setas
        if (i == col) {
            item.fmt |= crescente ? HDF_SORTUP : HDF_SORTDOWN;
        }
        Header_SetItem(hHeader, i, &item);
    }
    ultimaColunaOrdenada = col;
    SortParams* params = new SortParams{ hList, col, crescente };
    ListView_SortItemsEx(hList, CompararItens, (LPARAM)params);
    delete params;
}

LRESULT TratarCustomDraw(LPNMLVCUSTOMDRAW lplvcd) {
    switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT: {
        // Pega valor da coluna Memória
        WCHAR buffer[256] = {};
        ListView_GetItemText(lplvcd->nmcd.hdr.hwndFrom, (int)lplvcd->nmcd.dwItemSpec, 4, buffer, 256);
        int memKB = _wtoi(buffer);
        // Pega valor da coluna Favorito
        WCHAR estrela[8] = {};
        ListView_GetItemText(lplvcd->nmcd.hdr.hwndFrom, (int)lplvcd->nmcd.dwItemSpec, 0, estrela, 8);
        if (wcscmp(estrela, L"?") == 0) {
            lplvcd->clrTextBk = RGB(230, 255, 230); // verde-claro para favorito
        } else if (memKB > 512000) {
            lplvcd->clrTextBk = RGB(255, 255, 200); // amarelo claro para RAM alta
        }
        return CDRF_DODEFAULT;
    }
    }
    return CDRF_DODEFAULT;
}
