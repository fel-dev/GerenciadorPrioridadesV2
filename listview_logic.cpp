#include "listview_logic.h"
#include "utils.h"
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
        resultado = equals_ignore_case(texto1, texto2) ? 0 : _wcsicmp(texto1, texto2);
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
        if (wcscmp(estrela, L"⭐") == 0) {
            lplvcd->clrTextBk = RGB(230, 255, 230); // verde-claro para favorito
        } else if (memKB > 512000) {
            lplvcd->clrTextBk = RGB(255, 255, 200); // amarelo claro para RAM alta
        }
        return CDRF_NEWFONT;
    }
    }
    return CDRF_DODEFAULT;
}

// Altera o título (texto) de uma coluna do ListView em tempo de execução.
// Útil para mudar dinamicamente o nome de uma coluna, por exemplo, após uma ação do usuário.
void ListView_SetColumnTitle(HWND hList, int col, const std::wstring& title) {
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT; // Indica que queremos alterar apenas o texto
    lvc.pszText = const_cast<LPWSTR>(title.c_str()); // Converte std::wstring para LPWSTR
    ListView_SetColumn(hList, col, &lvc); // Aplica a alteração na coluna desejada
}

// Altera a largura de uma coluna do ListView em tempo de execução.
// Pode ser usado para ajustar o layout conforme o conteúdo ou preferências do usuário.
void ListView_SetColumnWidthEx(HWND hList, int col, int width) {
    ListView_SetColumnWidth(hList, col, width); // Define a largura da coluna
}

// "Mostra" ou "esconde" uma coluna do ListView.
// Não existe esconder coluna nativamente, então aqui simulamos escondendo ao definir largura zero.
// Útil para interfaces que permitem ao usuário personalizar quais colunas deseja ver.
void ListView_ShowColumn(HWND hList, int col, bool show) {
    HWND hHeader = ListView_GetHeader(hList);
    if (!hHeader) return;
    LONG style = GetWindowLong(hList, GWL_STYLE);
    if (show) {
        style |= LVS_REPORT; // Garante que o modo relatório está ativo
    } else {
        // Simula esconder a coluna ajustando a largura para zero
        ListView_SetColumnWidth(hList, col, 0);
        return;
    }
    SetWindowLong(hList, GWL_STYLE, style);
}

// Trata o clique duplo no ListView para marcar/desmarcar favoritos em grupo ou individualmente.
void TratarDoubleClick(HWND hwnd, HWND hListResult, LPNMITEMACTIVATE pnm, bool favoritarTodos, void (*AtualizarArquivoFavoritos)(HWND)) {
    if (pnm->iSubItem == 0 && pnm->iItem >= 0) {
        wchar_t atual[8] = {}, nomeAlvo[260] = {};
        ListView_GetItemText(hListResult, pnm->iItem, 0, atual, 8);
        ListView_GetItemText(hListResult, pnm->iItem, 1, nomeAlvo, 260);
        std::wstring nomeAlvoStr = nomeAlvo;
        trim_right(nomeAlvoStr);
        bool marcar = (wcscmp(atual, L"⭐") != 0);
        int total = ListView_GetItemCount(hListResult);
        if (favoritarTodos) {
            for (int i = 0; i < total; ++i) {
                wchar_t nome[260];
                ListView_GetItemText(hListResult, i, 1, nome, 260);
                std::wstring nomeStr = nome;
                trim_right(nomeStr);
                if (equals_ignore_case(nomeStr, nomeAlvoStr)) {
                    ListView_SetItemText(hListResult, i, 0, marcar ? (LPWSTR)L"⭐" : (LPWSTR)L"");
                    ListView_RedrawItems(hListResult, i, i);
                }
            }
        } else {
            ListView_SetItemText(hListResult, pnm->iItem, 0, marcar ? (LPWSTR)L"⭐" : (LPWSTR)L"");
            ListView_RedrawItems(hListResult, pnm->iItem, pnm->iItem);
        }
        AtualizarArquivoFavoritos(hListResult);
    }
}
