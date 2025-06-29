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

    if (texto.empty() && LOWORD(wParam) != ID_BTN_ATUALIZAR && LOWORD(wParam) != ID_BTN_APLICAR_SELECIONADOS && LOWORD(wParam) != ID_BTN_BUSCAR && LOWORD(wParam) != ID_BTN_APLICAR_PRIORIDADE) {
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
    if (hLista && LOWORD(wParam) != ID_BTN_APLICAR_SELECIONADOS && LOWORD(wParam) != ID_BTN_BUSCAR && LOWORD(wParam) != ID_BTN_APLICAR_PRIORIDADE) ListView_DeleteAllItems(hLista);

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
    case ID_BTN_APLICAR_SELECIONADOS: {
        // Botão antigo, pode ser removido futuramente
        break;
    }
    case ID_BTN_APLICAR_PRIORIDADE: {
        HWND hCombo = GetDlgItem(hwnd, ID_COMBO_PRIORIDADE);
        if (!hLista || !hCombo) break;
        int selecao = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
        DWORD prioridade = NORMAL_PRIORITY_CLASS;
        switch (selecao) {
            case 0: prioridade = IDLE_PRIORITY_CLASS; break;
            case 1: prioridade = NORMAL_PRIORITY_CLASS; break;
            case 2: prioridade = HIGH_PRIORITY_CLASS; break;
            case 3: prioridade = ABOVE_NORMAL_PRIORITY_CLASS; break;
            case 4: prioridade = REALTIME_PRIORITY_CLASS; break;
            default: break;
        }
        int count = ListView_GetItemCount(hLista);
        int alterados = 0;
        for (int i = 0; i < count; ++i) {
            if (ListView_GetItemState(hLista, i, LVIS_SELECTED) & LVIS_SELECTED) {
                wchar_t buffer[260];
                ListView_GetItemText(hLista, i, 0, buffer, 260);
                bool ok = AlterarPrioridade(buffer, prioridade);
                std::wstring status = ok ? L"Prioridade alterada ✅" : L"Falha ao alterar ❌";
                ListView_SetItemText(hLista, i, 1, (LPWSTR)status.c_str());
                ++alterados;
            }
        }
        std::wstring msg = L"Alterações aplicadas: " + std::to_wstring(alterados);
        MessageBoxW(hwnd, msg.c_str(), L"Concluído", MB_OK | MB_ICONINFORMATION);
        break;
    }
    case ID_BTN_BUSCAR: {
        HWND hLista = GetListaResultados(hwnd);
        if (!hLista) break;
        wchar_t buffer[260] = {};
        GetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ENTRADA), buffer, 260);
        std::wstring alvo(buffer);
        if (alvo.empty()) {
            MessageBoxW(hwnd, L"Digite o nome do processo para buscar.", L"Aviso", MB_OK | MB_ICONWARNING);
            break;
        }
        // Adiciona .exe se necessário
        if (alvo.size() > 0 && alvo.find(L".exe") == std::wstring::npos) {
            alvo += L".exe";
        }
        int total = ListView_GetItemCount(hLista);
        bool encontrado = false;
        for (int i = 0; i < total; ++i) {
            wchar_t nome[260];
            ListView_GetItemText(hLista, i, 0, nome, 260);
            if (_wcsicmp(nome, alvo.c_str()) == 0) {
                ListView_SetItemState(hLista, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(hLista, i, FALSE);
                encontrado = true;
                break;
            }
        }
        if (!encontrado) {
            MessageBoxW(hwnd, (L"Processo \"" + alvo + L"\" não encontrado.").c_str(),
                        L"Não localizado", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }
    }
}