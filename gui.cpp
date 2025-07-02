#include "gui.h"
#include "eventos.h"
#include "gui_layout.h"
#include "listview_logic.h"
#include "favoritos.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>
#include <set>

#pragma comment(lib, "comctl32.lib")

extern int ultimaColunaOrdenada;
extern std::map<int, bool> ordemCrescentePorColuna;

#define IDC_LISTVIEW_RESULT 4001

// Função para atualizar favoritos (agora definida em favoritos.cpp)
void AtualizarArquivoFavoritos(HWND hList) {
    SalvarFavoritosArquivo(hList);
}

static std::wstring LoadResString(UINT id) {
    wchar_t buf[256] = {};
    LoadStringW(GetModuleHandleW(nullptr), id, buf, 256);
    return buf;
}

// Implementação do WndProc e CriarJanela
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada;
    static HWND hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog;
    static HWND hCheckFavoritarTodos;
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        CriarControlesJanela(hwnd, hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada, hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog, hCheckFavoritarTodos);
        SetWindowTextW(hwnd, LoadResString(IDS_TITULO_JANELA).c_str());
        // Ícone
        HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        // Menu
        HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
        SetMenu(hwnd, hMenu);
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
            case IDM_EXPORT_LOG:
                TratarEventoBotao(hwnd, ID_BTN_SALVAR_LOG);
                break;
            case IDM_ABOUT:
                MessageBoxW(hwnd, L"Gerenciador de Prioridades V2\nLicença: MIT\nAutor: Felipe + Copilot", L"Sobre", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_EXIT:
                PostQuitMessage(0);
                break;
            default: {
                int wmId = LOWORD(wParam);
                if (wmId == 2001 || wmId == 2002 || wmId == ID_BTN_ATUALIZAR || wmId == ID_BTN_BUSCAR || wmId == ID_BTN_APLICAR_PRIORIDADE || wmId == ID_BTN_SALVAR_LOG) {
                    TratarEventoBotao(hwnd, wParam);
                }
                break;
            }
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
            return TratarCustomDraw(lplvcd);
        } else if (pnmhdr->code == NM_DBLCLK && pnmhdr->idFrom == IDC_LISTVIEW_RESULT) {
            LPNMITEMACTIVATE pnm = (LPNMITEMACTIVATE)lParam;
            bool favoritarTodos = (IsDlgButtonChecked(hwnd, ID_CHECK_FAVORITAR_TODOS) == BST_CHECKED);
            TratarDoubleClick(hwnd, hListResult, pnm, favoritarTodos, AtualizarArquivoFavoritos);
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
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, classeJanela, LoadResString(IDS_TITULO_JANELA).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 740, 500,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, LoadResString(IDS_MSG_LOG_ERRO).c_str(), LoadResString(IDS_MSG_ERRO).c_str(), MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}