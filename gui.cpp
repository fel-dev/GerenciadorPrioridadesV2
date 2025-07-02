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

// Trata atalhos de teclado na janela principal
void TratarAtalhos(HWND hwnd, WPARAM wParam) {
    if (GetKeyState(VK_CONTROL) & 0x8000) {
        switch (wParam) {
            case 'R':
                TratarEventoBotao(hwnd, ID_BTN_ATUALIZAR);
                break;
            case 'L':
                TratarEventoBotao(hwnd, ID_BTN_SALVAR_LOG);
                break;
            case 'F':
                MessageBoxW(hwnd, L"[TODO] Mostrar apenas favoritos 😄", L"Atalho Ctrl+F", MB_OK);
                break;
            case 'M':
                MessageBoxW(hwnd, L"[TODO] Ativar Modo Jogo! 🕹️", L"Atalho Ctrl+M", MB_OK);
                break;
        }
    } else if (wParam == VK_ESCAPE) {
        PostQuitMessage(0);
    }
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
    static HWND hBtnReverter;
    static HBRUSH hBrushLight = nullptr;
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        CriarControlesJanela(hwnd, hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada, hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog, hCheckFavoritarTodos, hBtnReverter);
        SetWindowTextW(hwnd, LoadResString(IDS_TITULO_JANELA).c_str());
        // Ícone
        HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        // Menu
        HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
        SetMenu(hwnd, hMenu);
        // Cria brush para cor light (cinza claro)
        hBrushLight = CreateSolidBrush(RGB(243,244,246));
        break;
    }
    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, hBrushLight);
        return 1;
    }
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(243,244,246));
        SetTextColor(hdc, RGB(30,30,30));
        return (LRESULT)hBrushLight;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
            case IDM_EXPORT_LOG:
                TratarEventoBotao(hwnd, ID_BTN_SALVAR_LOG);
                break;
            case IDM_ABOUT:
                MessageBoxW(hwnd, LoadResString(IDS_SOBRE_MSG).c_str(), LoadResString(IDS_SOBRE_TITULO).c_str(), MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_EXIT:
                PostQuitMessage(0);
                break;
            default: {
                int wmId = LOWORD(wParam);
                if (wmId == 2001 || wmId == 2002 || wmId == ID_BTN_ATUALIZAR || wmId == ID_BTN_BUSCAR || wmId == ID_BTN_APLICAR_PRIORIDADE || wmId == ID_BTN_SALVAR_LOG) {
                    TratarEventoBotao(hwnd, wParam);
                } else if (wmId == ID_BTN_REVERTER || wmId == IDM_REVERTER_PROCESSO) {
                    // Chama a função de reversão para todos os selecionados ou processo do menu
                    HWND hLista = hListResult;
                    if (!hLista) break;
                    int count = ListView_GetItemCount(hLista);
                    int revertidos = 0;
                    for (int i = 0; i < count; ++i) {
                        if (ListView_GetItemState(hLista, i, LVIS_SELECTED) & LVIS_SELECTED) {
                            wchar_t buffer[260];
                            ListView_GetItemText(hLista, i, 1, buffer, 260);
                            std::wstring nomeProc(buffer);
                            extern std::map<std::wstring, DWORD> prioridadeOriginal;
                            auto it = prioridadeOriginal.find(nomeProc);
                            if (it != prioridadeOriginal.end()) {
                                DWORD pid = GetProcessIdByName(buffer);
                                HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
                                bool ok = false;
                                if (hProc) {
                                    ok = SetPriorityClass(hProc, it->second);
                                    CloseHandle(hProc);
                                }
                                std::wstring status = ok ? L"Prioridade revertida ⏪" : L"Falha ao reverter";
                                // Atualiza ListView
                                std::wstring priorStr;
                                switch (it->second) {
                                    case IDLE_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_BAIXA); break;
                                    case NORMAL_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_NORMAL); break;
                                    case HIGH_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_ALTA); break;
                                    case REALTIME_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_TEMPO_REAL); break;
                                    case ABOVE_NORMAL_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_ACIMA); break;
                                    case BELOW_NORMAL_PRIORITY_CLASS: priorStr = LoadResString(IDS_PRIORIDADE_ABAIXO); break;
                                    default: priorStr = L"(?)"; break;
                                }
                                ListView_SetItemText(hLista, i, 2, (LPWSTR)priorStr.c_str());
                                ListView_SetItemText(hLista, i, 3, (LPWSTR)status.c_str());
                                ++revertidos;
                            }
                        }
                    }
                    std::wstring msg = L"Revertidos: " + std::to_wstring(revertidos);
                    MessageBoxW(hwnd, msg.c_str(), L"Reverter alterações", MB_OK | MB_ICONINFORMATION);
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
    case WM_CONTEXTMENU: {
        if ((HWND)wParam == hListResult) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, IDM_REVERTER_PROCESSO, L"⏪ Reverter este processo");
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            if (pt.x == -1 && pt.y == -1) {
                // Tecla de contexto: pega posição do item selecionado
                int iSel = ListView_GetNextItem(hListResult, -1, LVNI_SELECTED);
                if (iSel != -1) {
                    RECT rc;
                    ListView_GetItemRect(hListResult, iSel, &rc, LVIR_BOUNDS);
                    pt.x = rc.left;
                    pt.y = rc.bottom;
                    ClientToScreen(hListResult, &pt);
                } else {
                    GetCursorPos(&pt);
                }
            }
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
            return 0;
        }
        break;
    }
    case WM_KEYDOWN:
        TratarAtalhos(hwnd, wParam);
        break;
    case WM_SIZE: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int margemX = 20;
        int margemY = 20;
        int larguraBtn = 200;
        int alturaBtn = 30;
        // Ajusta botão Reverter
        if (hBtnReverter) {
            SetWindowPos(hBtnReverter, nullptr,
                margemX,
                rc.bottom - alturaBtn - margemY,
                larguraBtn, alturaBtn,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        // Ajusta ListView
        if (hListResult) {
            SetWindowPos(hListResult, nullptr,
                20, 140,
                rc.right - 40, rc.bottom - 140 - alturaBtn - 2 * margemY,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    case WM_DESTROY:
        if (hBrushLight) DeleteObject(hBrushLight);
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
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, // Remove maximizar e redimensionamento
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