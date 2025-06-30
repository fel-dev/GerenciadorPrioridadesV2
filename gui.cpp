#include "gui.h"
#include "eventos.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <map>

#pragma comment(lib, "comctl32.lib")

#define IDC_LISTVIEW_RESULT 4001

struct SortParams {
    HWND hList;
    int column;
    bool ascending;
};

std::map<int, bool> ordemCrescentePorColuna;

int CALLBACK CompararItens(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    SortParams* params = (SortParams*)lParamSort;
    WCHAR texto1[256] = {}, texto2[256] = {};
    ListView_GetItemText(params->hList, lParam1, params->column, texto1, 256);
    ListView_GetItemText(params->hList, lParam2, params->column, texto2, 256);
    int resultado = 0;
    if (params->column == 3) { // Memória (coluna 3): ordenar como número
        int mem1 = _wtoi(texto1);
        int mem2 = _wtoi(texto2);
        resultado = mem1 - mem2;
    } else {
        resultado = _wcsicmp(texto1, texto2);
    }
    return params->ascending ? resultado : -resultado;
}

// Implementação do WndProc e CriarJanela
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnAlta, hBtnBaixa, hBtnAtualizar, hBtnBuscar, hListResult, hEditEntrada;
    static HWND hComboPrioridade, hBtnAplicarPrioridade, hBtnSalvarLog;
    switch (msg) {
    case WM_CREATE: {
        // Campo de texto (EDIT)
        hEditEntrada = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
            20, 20, 450, 60,
            hwnd, (HMENU)ID_EDIT_ENTRADA, nullptr, nullptr);

        // Botão Buscar
        hBtnBuscar = CreateWindowW(L"BUTTON", L"🔍 Buscar",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            480, 20, 110, 30,
            hwnd, (HMENU)ID_BTN_BUSCAR, nullptr, nullptr);

        // ComboBox de prioridade
        hComboPrioridade = CreateWindowW(L"COMBOBOX", nullptr,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            20, 95, 200, 300,
            hwnd, (HMENU)ID_COMBO_PRIORIDADE, nullptr, nullptr);
        SendMessageW(hComboPrioridade, CB_ADDSTRING, 0, (LPARAM)L"Baixa (IDLE)");
        SendMessageW(hComboPrioridade, CB_ADDSTRING, 0, (LPARAM)L"Normal");
        SendMessageW(hComboPrioridade, CB_ADDSTRING, 0, (LPARAM)L"Alta");
        SendMessageW(hComboPrioridade, CB_ADDSTRING, 0, (LPARAM)L"Acima do normal");
        SendMessageW(hComboPrioridade, CB_ADDSTRING, 0, (LPARAM)L"Tempo real");
        SendMessageW(hComboPrioridade, CB_SETCURSEL, 2, 0); // seleciona "Alta" por padrão

        // Botão Aplicar Prioridade
        hBtnAplicarPrioridade = CreateWindowW(L"BUTTON", L" Aplicar Prioridade",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            230, 95, 150, 30,
            hwnd, (HMENU)ID_BTN_APLICAR_PRIORIDADE, nullptr, nullptr);

        // Botão Atualizar Processos
        hBtnAtualizar = CreateWindowW(L"BUTTON", L"🔃 Atualizar Processos",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            390, 95, 150, 30,
            hwnd, (HMENU)ID_BTN_ATUALIZAR, nullptr, nullptr);

        // Botão Salvar Log
        hBtnSalvarLog = CreateWindowW(L"BUTTON", L"💾 Salvar Log",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            550, 95, 150, 30,
            hwnd, (HMENU)ID_BTN_SALVAR_LOG, nullptr, nullptr);

        // Linha divisória visual
        CreateWindowW(L"STATIC", nullptr,
            WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
            20, 130, 680, 1,
            hwnd, nullptr, nullptr, nullptr);

        // ListView (abaixo dos botões)
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);
        hListResult = CreateWindowW(WC_LISTVIEW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER,
            20, 140, 680, 280,
            hwnd, (HMENU)IDC_LISTVIEW_RESULT, nullptr, nullptr);

        // Estilo grid e full row select
        ListView_SetExtendedListViewStyle(hListResult,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Colunas
        LVCOLUMNW col = { 0 };
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        col.pszText = (LPWSTR)L"Processo";
        col.cx = 220;
        ListView_InsertColumn(hListResult, 0, &col);

        col.pszText = (LPWSTR)L"Prioridade";
        col.cx = 120;
        ListView_InsertColumn(hListResult, 1, &col);

        col.pszText = (LPWSTR)L"Status";
        col.cx = 180;
        ListView_InsertColumn(hListResult, 2, &col);

        col.pszText = (LPWSTR)L"Memória (KB)";
        col.cx = 130;
        ListView_InsertColumn(hListResult, 3, &col);

        // Item de exemplo
        LVITEMW item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.pszText = (LPWSTR)L"chrome.exe";
        ListView_InsertItem(hListResult, &item);
        ListView_SetItemText(hListResult, 0, 1, (LPWSTR)L"Normal");
        ListView_SetItemText(hListResult, 0, 2, (LPWSTR)L"Prioridade alterada ✅");
        ListView_SetItemText(hListResult, 0, 3, (LPWSTR)L"123456 KB");
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
        if (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK) {
            NMLISTVIEW* pnm = (NMLISTVIEW*)lParam;
            int col = pnm->iSubItem;
            bool ordemAtual = ordemCrescentePorColuna[col];
            ordemCrescentePorColuna[col] = !ordemAtual;
            SortParams* params = new SortParams{ hListResult, col, !ordemAtual };
            ListView_SortItemsEx(hListResult, CompararItens, (LPARAM)params);
            delete params;
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