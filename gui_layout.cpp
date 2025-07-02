#include "gui_layout.h"
#include "gui.h"
#include "resource.h"
#include <commctrl.h>
#include <string>

// Função utilitária para carregar string de recurso
static std::wstring LoadResString(UINT id) {
    wchar_t buf[256] = {};
    LoadStringW(GetModuleHandleW(nullptr), id, buf, 256);
    return buf;
}

// Função utilitária para criar botões Unicode
HWND CriarBotaoUnicode(HWND hwnd, int x, int y, int w, int h, int id, const wchar_t* texto) {
    return CreateWindowW(L"BUTTON", texto, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, h, hwnd, (HMENU)id, nullptr, nullptr);
}

// Função utilitária para aplicar fonte emoji
void SetFonteEmoji(HWND hWnd) {
    static HFONT hFontEmoji = nullptr;
    if (!hFontEmoji) {
        hFontEmoji = CreateFontW(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI Emoji"
        );
    }
    SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);
}

void CriarControlesJanela(HWND hwnd, HWND& hBtnAlta, HWND& hBtnBaixa, HWND& hBtnAtualizar, HWND& hBtnBuscar, HWND& hListResult, HWND& hEditEntrada, HWND& hComboPrioridade, HWND& hBtnAplicarPrioridade, HWND& hBtnSalvarLog, HWND& hCheckFavoritarTodos) {
    // Campo de texto (EDIT)
    hEditEntrada = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
        20, 20, 450, 60,
        hwnd, (HMENU)ID_EDIT_ENTRADA, nullptr, nullptr);

    // Botão Buscar (Unicode)
    hBtnBuscar = CriarBotaoUnicode(hwnd, 480, 20, 110, 30, ID_BTN_BUSCAR, L"🔍 Buscar");
    SetFonteEmoji(hBtnBuscar);

    // Checkbox Favoritar todos com o mesmo nome (Unicode)
    hCheckFavoritarTodos = CreateWindowW(L"BUTTON", L"⭐ Favoritar grupo",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_LEFTTEXT,
        480, 60, 220, 30,
        hwnd, (HMENU)ID_CHECK_FAVORITAR_TODOS, nullptr, nullptr);
    SendMessageW(hCheckFavoritarTodos, BM_SETCHECK, BST_CHECKED, 0);
    SetFonteEmoji(hCheckFavoritarTodos);

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

    // Botão Aplicar Prioridade (Unicode)
    hBtnAplicarPrioridade = CriarBotaoUnicode(hwnd, 230, 95, 150, 30, ID_BTN_APLICAR_PRIORIDADE, L"🆗 Aplicar Prioridade");
    SetFonteEmoji(hBtnAplicarPrioridade);

    // Botão Atualizar Processos (Unicode)
    hBtnAtualizar = CriarBotaoUnicode(hwnd, 390, 95, 150, 30, ID_BTN_ATUALIZAR, L"🔃 Atualizar Processos");
    SetFonteEmoji(hBtnAtualizar);

    // Botão Salvar Log (Unicode)
    hBtnSalvarLog = CriarBotaoUnicode(hwnd, 550, 95, 150, 30, ID_BTN_SALVAR_LOG, L"📎 Salvar Log");
    SetFonteEmoji(hBtnSalvarLog);

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

    // Aplica fonte emoji na ListView
    HFONT hFontEmoji = CreateFontW(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI Emoji"
    );
    SendMessageW(hListResult, WM_SETFONT, (WPARAM)hFontEmoji, TRUE);

    // Estilo grid e full row select
    ListView_SetExtendedListViewStyle(hListResult,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Colunas
    LVCOLUMNW col = { 0 };
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    std::wstring colFavorito = LoadResString(IDS_COL_FAVORITO);
    col.pszText = (LPWSTR)colFavorito.c_str();
    col.cx = 40;
    ListView_InsertColumn(hListResult, 0, &col);

    std::wstring colProcesso = LoadResString(IDS_COL_PROCESSO);
    col.pszText = (LPWSTR)colProcesso.c_str();
    col.cx = 220;
    ListView_InsertColumn(hListResult, 1, &col);

    std::wstring colPrioridade = LoadResString(IDS_COL_PRIORIDADE);
    col.pszText = (LPWSTR)colPrioridade.c_str();
    col.cx = 120;
    ListView_InsertColumn(hListResult, 2, &col);

    std::wstring colStatus = LoadResString(IDS_COL_STATUS);
    col.pszText = (LPWSTR)colStatus.c_str();
    col.cx = 180;
    ListView_InsertColumn(hListResult, 3, &col);

    std::wstring colMemoria = LoadResString(IDS_COL_MEMORIA);
    col.pszText = (LPWSTR)colMemoria.c_str();
    col.cx = 130;
    ListView_InsertColumn(hListResult, 4, &col);

    // Item de exemplo
    LVITEMW item = { 0 };
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    std::wstring exemploColuna = LoadResString(IDS_EXEMPLO_COLUNA);
    item.pszText = (LPWSTR)exemploColuna.c_str();
    ListView_InsertItem(hListResult, &item);
    std::wstring exemploProc = LoadResString(IDS_EXEMPLO_PROCESSO);
    std::wstring exemploPrior = LoadResString(IDS_EXEMPLO_PRIORIDADE);
    std::wstring exemploStatus = LoadResString(IDS_EXEMPLO_STATUS);
    std::wstring exemploMem = LoadResString(IDS_EXEMPLO_MEMORIA);
    ListView_SetItemText(hListResult, 0, 1, (LPWSTR)exemploProc.c_str());
    ListView_SetItemText(hListResult, 0, 2, (LPWSTR)exemploPrior.c_str());
    ListView_SetItemText(hListResult, 0, 3, (LPWSTR)exemploStatus.c_str());
    ListView_SetItemText(hListResult, 0, 4, (LPWSTR)exemploMem.c_str());
}
