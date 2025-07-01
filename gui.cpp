#include "gui.h"
#include "eventos.h"
#include "gui_layout.h"
#include "listview_logic.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <set>

#pragma comment(lib, "comctl32.lib")

extern int ultimaColunaOrdenada;
extern std::map<int, bool> ordemCrescentePorColuna;

#define IDC_LISTVIEW_RESULT 4001

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
            TratarColumnClick(hListResult, pnm->iSubItem, ordemCrescentePorColuna, ultimaColunaOrdenada);
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