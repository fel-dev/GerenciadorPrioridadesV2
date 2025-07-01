#include "gui.h"
#include "eventos.h"
#include "gui_layout.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <set>

#pragma comment(lib, "comctl32.lib")

extern int ultimaColunaOrdenada;
extern std::map<int, bool> ordemCrescentePorColuna;

#define IDC_LISTVIEW_RESULT 4001

struct SortParams {
    HWND hList;
    int column;
    bool ascending;
};

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

// Função para atualizar favoritos (definida em eventos.cpp)
void AtualizarArquivoFavoritos(HWND hList);

// Implementação do WndProc e CriarJanela
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada;
    static HWND hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog;
    static HWND hCheckFavoritarTodos;
    switch (msg) {
    case WM_CREATE: {
        CriarControlesJanela(hwnd, hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada, hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog, hCheckFavoritarTodos);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == 2001 || wmId == 2002 || wmId == ID_BTN_ATUALIZAR || wmId == ID_BTN_BUSCAR || wmId == ID_BTN_APLICAR_PRIORIDADE || wmId == ID_BTN_SALVAR_LOG) {
            TratarEventoBotao(hwnd, wParam);
        }
        break;
    }
    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->code == LVN_COLUMNCLICK) {
            NMLISTVIEW* pnm = (NMLISTVIEW*)lParam;
            HWND hList = hListResult;
            int col = pnm->iSubItem;
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
        } else if (pnmhdr->code == NM_CUSTOMDRAW && pnmhdr->idFrom == IDC_LISTVIEW_RESULT) {
            LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
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
                return CDRF_DODEFAULT;
            }
            }
        } else if (pnmhdr->code == NM_DBLCLK && pnmhdr->idFrom == IDC_LISTVIEW_RESULT) {
            LPNMITEMACTIVATE pnm = (LPNMITEMACTIVATE)lParam;
            if (pnm->iSubItem == 0 && pnm->iItem >= 0) {
                // Se checkbox estiver marcado, marca/desmarca todos os processos com o mesmo nome
                bool favoritarTodos = (IsDlgButtonChecked(hwnd, ID_CHECK_FAVORITAR_TODOS) == BST_CHECKED);
                wchar_t atual[8] = {}, nomeAlvo[260] = {};
                ListView_GetItemText(hListResult, pnm->iItem, 0, atual, 8);
                ListView_GetItemText(hListResult, pnm->iItem, 1, nomeAlvo, 260);
                std::wstring nomeAlvoStr = nomeAlvo;
                nomeAlvoStr.erase(nomeAlvoStr.find_last_not_of(L" \t\n\r") + 1);
                bool marcar = (wcscmp(atual, L"⭐") != 0);
                int total = ListView_GetItemCount(hListResult);
                if (favoritarTodos) {
                    for (int i = 0; i < total; ++i) {
                        wchar_t nome[260];
                        ListView_GetItemText(hListResult, i, 1, nome, 260);
                        std::wstring nomeStr = nome;
                        nomeStr.erase(nomeStr.find_last_not_of(L" \t\n\r") + 1);
                        if (_wcsicmp(nomeStr.c_str(), nomeAlvoStr.c_str()) == 0) {
                            ListView_SetItemText(hListResult, i, 0, marcar ? (LPWSTR)L"⭐" : (LPWSTR)L"");
                        }
                    }
                } else {
                    ListView_SetItemText(hListResult, pnm->iItem, 0, marcar ? (LPWSTR)L"⭐" : (LPWSTR)L"");
                }
                AtualizarArquivoFavoritos(hListResult);
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CriarJanela(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t* classeJanela = L"JanelaPrincipalV2";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = classeJanela;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, classeJanela, L"Gerenciador de Prioridades — V2",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 740, 500,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Falha ao criar janela!", L"Erro", MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}