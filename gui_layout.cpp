#include "gui_layout.h"
#include "gui.h"
#include <commctrl.h>
#include <string>

void CriarControlesJanela(HWND hwnd, HWND& hBtnAlta, HWND& hBtnBaixa, HWND& hBtnAtualizar, HWND& hBtnBuscar, HWND& hListResult, HWND& hEditEntrada, HWND& hComboPrioridade, HWND& hBtnAplicarPrioridade, HWND& hBtnSalvarLog, HWND& hCheckFavoritarTodos) {
    // Campo de texto (EDIT)
    hEditEntrada = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
        20, 20, 450, 60,
        hwnd, (HMENU)ID_EDIT_ENTRADA, nullptr, nullptr);

    // Botão Buscar
    hBtnBuscar = CreateWindowW(L"BUTTON", L"?? Buscar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        480, 20, 110, 30,
        hwnd, (HMENU)ID_BTN_BUSCAR, nullptr, nullptr);

    // Checkbox Favoritar todos com o mesmo nome
    hCheckFavoritarTodos = CreateWindowW(L"BUTTON", L"⭐ Favoritar grupo",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        480, 60, 220, 30,
        hwnd, (HMENU)ID_CHECK_FAVORITAR_TODOS, nullptr, nullptr);
    // Deixa o checkbox marcado por padrão
    SendMessageW(hCheckFavoritarTodos, BM_SETCHECK, BST_CHECKED, 0);

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
    hBtnAplicarPrioridade = CreateWindowW(L"BUTTON", L"🆗 Aplicar Prioridade",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        230, 95, 150, 30,
        hwnd, (HMENU)ID_BTN_APLICAR_PRIORIDADE, nullptr, nullptr);

    // Botão Atualizar Processos
    hBtnAtualizar = CreateWindowW(L"BUTTON", L"🔃 Atualizar Processos",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        390, 95, 150, 30,
        hwnd, (HMENU)ID_BTN_ATUALIZAR, nullptr, nullptr);

    // Botão Salvar Log
    hBtnSalvarLog = CreateWindowW(L"BUTTON", L"📎 Salvar Log",
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

    col.pszText = (LPWSTR)L"?";
    col.cx = 40;
    ListView_InsertColumn(hListResult, 0, &col);

    col.pszText = (LPWSTR)L"Processo";
    col.cx = 220;
    ListView_InsertColumn(hListResult, 1, &col);

    col.pszText = (LPWSTR)L"Prioridade";
    col.cx = 120;
    ListView_InsertColumn(hListResult, 2, &col);

    col.pszText = (LPWSTR)L"Status";
    col.cx = 180;
    ListView_InsertColumn(hListResult, 3, &col);

    col.pszText = (LPWSTR)L"Memória (KB)";
    col.cx = 130;
    ListView_InsertColumn(hListResult, 4, &col);

    // Item de exemplo
    LVITEMW item = { 0 };
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    item.pszText = (LPWSTR)L"";
    ListView_InsertItem(hListResult, &item);
    ListView_SetItemText(hListResult, 0, 1, (LPWSTR)L"chrome.exe");
    ListView_SetItemText(hListResult, 0, 2, (LPWSTR)L"Normal");
    ListView_SetItemText(hListResult, 0, 3, (LPWSTR)L"Prioridade alterada 🆗");
    ListView_SetItemText(hListResult, 0, 4, (LPWSTR)L"123456 KB");
}
