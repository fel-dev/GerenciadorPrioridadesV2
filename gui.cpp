#include "gui.h"
#include "eventos.h"
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

// Implementação do WndProc e CriarJanela
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnAlta, hBtnBaixa, hListResult;
    switch (msg) {
    case WM_CREATE: {
        hBtnAlta = CreateWindowW(L"BUTTON", L"Prioridade Alta",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 20, 140, 30,
            hwnd, (HMENU)2001, nullptr, nullptr);
        hBtnBaixa = CreateWindowW(L"BUTTON", L"Prioridade Baixa",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 60, 140, 30,
            hwnd, (HMENU)2002, nullptr, nullptr);

        // ListView
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);
        hListResult = CreateWindowW(WC_LISTVIEW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
            20, 110, 580, 200,
            hwnd, nullptr, nullptr, nullptr);

        // Estilo grid e full row select
        ListView_SetExtendedListViewStyle(hListResult,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Colunas
        LVCOLUMNW col = { 0 };
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        col.pszText = (LPWSTR)L"Processo";
        col.cx = 300;
        ListView_InsertColumn(hListResult, 0, &col);

        col.pszText = (LPWSTR)L"Status";
        col.cx = 250;
        ListView_InsertColumn(hListResult, 1, &col);

        // Item de exemplo
        LVITEMW item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.pszText = (LPWSTR)L"chrome.exe";
        ListView_InsertItem(hListResult, &item);
        ListView_SetItemText(hListResult, 0, 1, (LPWSTR)L"Prioridade alterada ✅");
        break;
    }
    case WM_COMMAND:
        TratarEventoBotao(hwnd, wParam);
        break;
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
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Falha ao criar janela!", L"Erro", MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}