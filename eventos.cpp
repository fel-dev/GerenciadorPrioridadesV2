#define _CRT_SECURE_NO_WARNINGS
#include "eventos.h"
#include "gui.h"
#include "processos.h"
#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <tuple>
#include <algorithm>
#include <map>
#include <set>
#pragma comment(lib, "psapi.lib")

int ultimaColunaOrdenada = -1;
std::map<int, bool> ordemCrescentePorColuna;

HWND GetListaResultados(HWND hwndPai) {
    return FindWindowEx(hwndPai, nullptr, WC_LISTVIEW, nullptr);
}

void AdicionarNaLista(HWND hList, const std::wstring& favorito, const std::wstring& processo, const std::wstring& status) {
    LVITEMW item = { 0 };
    item.mask = LVIF_TEXT;
    item.iItem = ListView_GetItemCount(hList);
    item.pszText = (LPWSTR)favorito.c_str();
    ListView_InsertItem(hList, &item);
    ListView_SetItemText(hList, item.iItem, 1, (LPWSTR)processo.c_str());
    ListView_SetItemText(hList, item.iItem, 2, (LPWSTR)status.c_str());
}

void SalvarLogParaArquivo(HWND hList) {
    std::wofstream arquivo(L"log_prioridades.txt", std::ios::app);
    if (!arquivo.is_open()) {
        MessageBoxW(nullptr, L"Falha ao abrir o arquivo de log.", L"Erro", MB_OK | MB_ICONERROR);
        return;
    }
    std::time_t t = std::time(nullptr);
    struct tm tm_buf;
    errno_t err = localtime_s(&tm_buf, &t);
    wchar_t dataHora[100] = L"";
    if (err == 0) {
        wcsftime(dataHora, 100, L"%d/%m/%Y %H:%M:%S", &tm_buf);
    }
    arquivo << L"\n==== LOG EM " << dataHora << L" ====\n";
    // Nome da coluna ordenada
    std::wstring nomeColuna = L"(sem ordenação)";
    switch (ultimaColunaOrdenada) {
        case 0: nomeColuna = L"Favorito"; break;
        case 1: nomeColuna = L"Processo"; break;
        case 2: nomeColuna = L"Prioridade"; break;
        case 3: nomeColuna = L"Status"; break;
        case 4: nomeColuna = L"Memória (KB)"; break;
    }
    std::wstring direcao = L"";
    if (ultimaColunaOrdenada != -1) {
        bool crescente = ordemCrescentePorColuna[ultimaColunaOrdenada];
        direcao = crescente ? L" (crescente)" : L" (decrescente)";
    }
    arquivo << L"Ordenado por: " << nomeColuna << direcao << L"\n";
    int total = ListView_GetItemCount(hList);
    for (int i = 0; i < total; ++i) {
        wchar_t favorito[260], nome[260], status[260];
        ListView_GetItemText(hList, i, 0, favorito, 260);
        ListView_GetItemText(hList, i, 1, nome, 260);
        ListView_GetItemText(hList, i, 2, status, 260);
        arquivo << favorito << L"  " << nome << L"  " << status << L"\n";
    }
    arquivo << L"=============================\n";
    arquivo.close();
    MessageBoxW(nullptr, L"Log salvo com sucesso em 'log_prioridades.txt'.", L"OK", MB_OK | MB_ICONINFORMATION);
}

void AtualizarArquivoFavoritos(HWND hList) {
    std::wofstream favFile(L"favoritos.txt", std::ios::trunc);
    if (!favFile.is_open()) return;
    int total = ListView_GetItemCount(hList);
    for (int i = 0; i < total; ++i) {
        wchar_t estrela[8] = {};
        ListView_GetItemText(hList, i, 0, estrela, 8);
        if (wcscmp(estrela, L"⭐") == 0) {
            wchar_t nomeProc[256] = {};
            ListView_GetItemText(hList, i, 1, nomeProc, 256); // coluna 1 = Processo
            favFile << nomeProc << L"\n";
        }
    }
    favFile.close();
}

void TratarEventoBotao(HWND hwnd, WPARAM wParam) {
    wchar_t buffer[2048] = {};
    GetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ENTRADA), buffer, 2048);
    std::wstring texto(buffer);

    if (texto.empty() && LOWORD(wParam) != ID_BTN_ATUALIZAR && LOWORD(wParam) != ID_BTN_APLICAR_SELECIONADOS && LOWORD(wParam) != ID_BTN_BUSCAR && LOWORD(wParam) != ID_BTN_APLICAR_PRIORIDADE && LOWORD(wParam) != ID_BTN_SALVAR_LOG) {
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
    if (hLista && LOWORD(wParam) != ID_BTN_APLICAR_SELECIONADOS && LOWORD(wParam) != ID_BTN_BUSCAR && LOWORD(wParam) != ID_BTN_APLICAR_PRIORIDADE && LOWORD(wParam) != ID_BTN_SALVAR_LOG) ListView_DeleteAllItems(hLista);

    switch (LOWORD(wParam)) {
    case ID_BTN_ALTA:
    case ID_BTN_BAIXA:
        for (const auto& nome : linhas) {
            bool ok = AlterarPrioridade(nome, prioridade);
            std::wstring status = ok ? L"Prioridade alterada ✅" : L"Falha ao alterar ❌";
            if (hLista) AdicionarNaLista(hLista, L"", nome, status);
        }
        break;
    case ID_BTN_ATUALIZAR: {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) break;
        PROCESSENTRY32W pe = { 0 };
        pe.dwSize = sizeof(pe);
        struct ProcInfo {
            std::wstring favorito;
            std::wstring nome;
            std::wstring prioridade;
            std::wstring status;
            SIZE_T memoria;
        };
        std::vector<ProcInfo> processos;
        if (Process32FirstW(hSnap, &pe)) {
            do {
                PROCESS_MEMORY_COUNTERS pmc = { 0 };
                SIZE_T kb = 0;
                std::wstring strPrioridade = L"(?)";
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                if (hProc) {
                    DWORD priClass = GetPriorityClass(hProc);
                    switch (priClass) {
                        case IDLE_PRIORITY_CLASS: strPrioridade = L"Baixa"; break;
                        case NORMAL_PRIORITY_CLASS: strPrioridade = L"Normal"; break;
                        case HIGH_PRIORITY_CLASS: strPrioridade = L"Alta"; break;
                        case REALTIME_PRIORITY_CLASS: strPrioridade = L"Tempo real"; break;
                        case ABOVE_NORMAL_PRIORITY_CLASS: strPrioridade = L"Acima do normal"; break;
                        case BELOW_NORMAL_PRIORITY_CLASS: strPrioridade = L"Abaixo do normal"; break;
                        default: break;
                    }
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                        kb = pmc.WorkingSetSize / 1024;
                    }
                    CloseHandle(hProc);
                }
                processos.push_back({L"", pe.szExeFile, strPrioridade, L"Rodando", kb});
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
        std::set<std::wstring> favoritos;
        std::wifstream favFile(L"favoritos.txt");
        std::wstring nome;
        while (std::getline(favFile, nome)) {
            favoritos.insert(nome);
        }
        favFile.close();
        // Ordena do maior para o menor uso de memória
        std::sort(processos.begin(), processos.end(), [](const ProcInfo& a, const ProcInfo& b) {
            return a.memoria > b.memoria;
        });
        if (hLista) ListView_DeleteAllItems(hLista);
        for (size_t i = 0; i < processos.size(); ++i) {
            LVITEMW item = { 0 };
            item.mask = LVIF_TEXT;
            item.iItem = (int)i;
            item.pszText = (LPWSTR)processos[i].favorito.c_str();
            ListView_InsertItem(hLista, &item);
            ListView_SetItemText(hLista, (int)i, 1, (LPWSTR)processos[i].nome.c_str());
            ListView_SetItemText(hLista, (int)i, 2, (LPWSTR)processos[i].prioridade.c_str());
            ListView_SetItemText(hLista, (int)i, 3, (LPWSTR)processos[i].status.c_str());
            std::wstring memStr = std::to_wstring(processos[i].memoria) + L" KB";
            ListView_SetItemText(hLista, (int)i, 4, (LPWSTR)memStr.c_str());
            // Marca favorito se estiver no arquivo
            if (favoritos.count(processos[i].nome)) {
                ListView_SetItemText(hLista, (int)i, 0, (LPWSTR)L"⭐");
            }
        }
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
                ListView_GetItemText(hLista, i, 1, buffer, 260);
                bool ok = AlterarPrioridade(buffer, prioridade);
                std::wstring status = ok ? L"Prioridade alterada ✅" : L"Falha ao alterar ❌";
                ListView_SetItemText(hLista, i, 2, (LPWSTR)status.c_str());
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
            ListView_GetItemText(hLista, i, 1, nome, 260);
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
    case ID_BTN_SALVAR_LOG: {
        HWND hLista = GetListaResultados(hwnd);
        if (hLista) {
            SalvarLogParaArquivo(hLista);
            AtualizarArquivoFavoritos(hLista);
        }
        break;
    }
    }
}