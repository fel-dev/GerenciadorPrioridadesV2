#include "eventos.h"
#include "gui.h"
#include "processos.h"
#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

HWND GetListaResultados(HWND hwndPai) {
    return FindWindowEx(hwndPai, nullptr, WC_LISTVIEW, nullptr);
}

void AdicionarNaLista(HWND hList, const std::wstring& processo, const std::wstring& status) {
    LVITEMW item = { 0 };
    item.mask = LVIF_TEXT;
    item.iItem = ListView_GetItemCount(hList);
    item.pszText = (LPWSTR)processo.c_str();
    ListView_InsertItem(hList, &item);
    ListView_SetItemText(hList, item.iItem, 1, (LPWSTR)status.c_str());
}

void TratarEventoBotao(HWND hwnd, WPARAM wParam) {
    wchar_t buffer[2048] = {};
    GetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ENTRADA), buffer, 2048);
    std::wstring texto(buffer);

    if (texto.empty() && LOWORD(wParam) != ID_BTN_ATUALIZAR) {
        MessageBoxW(hwnd, L"Digite ao menos um nome de processo.", L"Aviso", MB_OK | MB_ICONWARNING);
        return;
    }

    std::vector<std::wstring> linhas;
    size_t pos = 0;
    while ((pos = texto.find(L"\r\n")) != std::wstring::npos) {
        linhas.push_back(texto.substr(0, pos));
        texto.erase(0, pos + 2);
    }
    if (!texto.empty()) linhas.push_back(texto);

    DWORD prioridade = 0;
    switch (LOWORD(wParam)) {
    case ID_BTN_ALTA:  prioridade = HIGH_PRIORITY_CLASS; break;
    case ID_BTN_BAIXA: prioridade = IDLE_PRIORITY_CLASS; break;
    }

    HWND hLista = GetListaResultados(hwnd);
    if (hLista) ListView_DeleteAllItems(hLista);

    switch (LOWORD(wParam)) {
    case ID_BTN_ALTA:
    case ID_BTN_BAIXA:
        for (const auto& nome : linhas) {
            bool ok = AlterarPrioridade(nome, prioridade);
            std::wstring status = ok ? L"Prioridade alterada ✅" : L"Falha ao alterar ❌";
            if (hLista) AdicionarNaLista(hLista, nome, status);
        }
        break;
    case ID_BTN_ATUALIZAR: {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) break;
        PROCESSENTRY32W pe = { 0 };
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                std::wstring nome(pe.szExeFile);
                if (hLista) AdicionarNaLista(hLista, nome, L"Rodando");
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
        break;
    }
    }
}