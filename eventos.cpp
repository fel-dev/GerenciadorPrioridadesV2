#define _CRT_SECURE_NO_WARNINGS
#include "eventos.h"
#include "gui.h"
#include "processos.h"
#include "favoritos.h"
#include "utils.h"
#include "resource.h"
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

// Cache para prioridade original dos processos
std::map<std::wstring, DWORD> prioridadeOriginal;

static std::wstring LoadResString(UINT id) {
    wchar_t buf[256] = {};
    LoadStringW(GetModuleHandleW(nullptr), id, buf, 256);
    return buf;
}

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
        MessageBoxW(nullptr, LoadResString(IDS_MSG_LOG_ERRO).c_str(), LoadResString(IDS_MSG_ERRO).c_str(), MB_OK | MB_ICONERROR);
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
        case 0: nomeColuna = LoadResString(IDS_COL_FAVORITO); break;
        case 1: nomeColuna = LoadResString(IDS_COL_PROCESSO); break;
        case 2: nomeColuna = LoadResString(IDS_COL_PRIORIDADE); break;
        case 3: nomeColuna = LoadResString(IDS_COL_STATUS); break;
        case 4: nomeColuna = LoadResString(IDS_COL_MEMORIA); break;
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
        arquivo << favorito << L"  4 " << nome << L"  4 " << status << L"\n";
    }
    arquivo << L"=============================\n";
    arquivo.close();
    MessageBoxW(nullptr, LoadResString(IDS_MSG_LOG_SALVO).c_str(), LoadResString(IDS_MSG_OK).c_str(), MB_OK | MB_ICONINFORMATION);
}

// Função utilitária para obter o PID de um processo pelo nome
DWORD GetProcessIdByName(const wchar_t* nomeProc) {
    DWORD pid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { 0 };
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, nomeProc) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return pid;
}

void TratarEventoBotao(HWND hwnd, WPARAM wParam) {
    wchar_t buffer[2048] = {};
    GetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ENTRADA), buffer, 2048);
    std::wstring texto(buffer);

    if (texto.empty() && LOWORD(wParam) != ID_BTN_ATUALIZAR && LOWORD(wParam) != ID_BTN_APLICAR_SELECIONADOS && LOWORD(wParam) != ID_BTN_BUSCAR && LOWORD(wParam) != ID_BTN_APLICAR_PRIORIDADE && LOWORD(wParam) != ID_BTN_SALVAR_LOG) {
        MessageBoxW(hwnd, LoadResString(IDS_MSG_DIGITE_NOME).c_str(), LoadResString(IDS_MSG_AVISO).c_str(), MB_OK | MB_ICONWARNING);
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
            std::wstring prioridadeStr;
            switch (prioridade) {
                case HIGH_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_ALTA); break;
                case IDLE_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_BAIXA); break;
                case NORMAL_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_NORMAL); break;
                case REALTIME_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_TEMPO_REAL); break;
                case ABOVE_NORMAL_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_ACIMA); break;
                case BELOW_NORMAL_PRIORITY_CLASS: prioridadeStr = LoadResString(IDS_PRIORIDADE_ABAIXO); break;
                default: prioridadeStr = L"(?)"; break;
            }
            std::wstring status = ok ? LoadResString(IDS_MSG_PRIORIDADE_OK) : LoadResString(IDS_MSG_PRIORIDADE_ERRO);
            if (hLista) {
                LVITEMW item = { 0 };
                item.mask = LVIF_TEXT;
                item.iItem = ListView_GetItemCount(hLista);
                item.pszText = (LPWSTR)L"";
                ListView_InsertItem(hLista, &item);
                ListView_SetItemText(hLista, item.iItem, 1, (LPWSTR)nome.c_str());
                ListView_SetItemText(hLista, item.iItem, 2, (LPWSTR)prioridadeStr.c_str()); // Prioridade
                ListView_SetItemText(hLista, item.iItem, 3, (LPWSTR)status.c_str()); // Status
            }
        }
        break;
    case ID_BTN_ATUALIZAR: {
        // Limpa o cache de prioridades originais
        prioridadeOriginal.clear();
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
                    prioridadeOriginal[pe.szExeFile] = priClass; // Salva prioridade original
                    switch (priClass) {
                        case IDLE_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_BAIXA); break;
                        case NORMAL_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_NORMAL); break;
                        case HIGH_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_ALTA); break;
                        case REALTIME_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_TEMPO_REAL); break;
                        case ABOVE_NORMAL_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_ACIMA); break;
                        case BELOW_NORMAL_PRIORITY_CLASS: strPrioridade = LoadResString(IDS_PRIORIDADE_ABAIXO); break;
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
        std::set<std::wstring> favoritos = LerFavoritosArquivo();
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
            std::wstring memStr = to_wstring_with_suffix(processos[i].memoria, L" KB");
            ListView_SetItemText(hLista, (int)i, 4, (LPWSTR)memStr.c_str());
            // Marca favorito se estiver no arquivo
            if (ProcessoEhFavorito(favoritos, processos[i].nome)) {
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
        std::wstring prioridadeStr = LoadResString(IDS_PRIORIDADE_NORMAL);
        switch (selecao) {
            case 0: prioridade = IDLE_PRIORITY_CLASS; prioridadeStr = LoadResString(IDS_PRIORIDADE_BAIXA); break;
            case 1: prioridade = NORMAL_PRIORITY_CLASS; prioridadeStr = LoadResString(IDS_PRIORIDADE_NORMAL); break;
            case 2: prioridade = HIGH_PRIORITY_CLASS; prioridadeStr = LoadResString(IDS_PRIORIDADE_ALTA); break;
            case 3: prioridade = ABOVE_NORMAL_PRIORITY_CLASS; prioridadeStr = LoadResString(IDS_PRIORIDADE_ACIMA); break;
            case 4: prioridade = REALTIME_PRIORITY_CLASS; prioridadeStr = LoadResString(IDS_PRIORIDADE_TEMPO_REAL); break;
            default: break;
        }
        int count = ListView_GetItemCount(hLista);
        int alterados = 0;
        for (int i = 0; i < count; ++i) {
            if (ListView_GetItemState(hLista, i, LVIS_SELECTED) & LVIS_SELECTED) {
                wchar_t buffer[260];
                ListView_GetItemText(hLista, i, 1, buffer, 260);
                std::wstring nomeProc(buffer);
                // Salva prioridade original antes de alterar
                if (prioridadeOriginal.find(nomeProc) == prioridadeOriginal.end()) {
                    DWORD prioridadeAtual = NORMAL_PRIORITY_CLASS;
                    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetProcessIdByName(nomeProc.c_str()));
                    if (hProc) {
                        prioridadeAtual = GetPriorityClass(hProc);
                        CloseHandle(hProc);
                    }
                    prioridadeOriginal[nomeProc] = prioridadeAtual;
                }
                bool ok = AlterarPrioridade(buffer, prioridade);
                std::wstring status = ok ? LoadResString(IDS_MSG_PRIORIDADE_OK) : LoadResString(IDS_MSG_PRIORIDADE_ERRO);
                ListView_SetItemText(hLista, i, 2, (LPWSTR)prioridadeStr.c_str()); // Prioridade
                ListView_SetItemText(hLista, i, 3, (LPWSTR)status.c_str()); // Status
                ++alterados;
            }
        }
        std::wstring msg = L"Alterações aplicadas: " + std::to_wstring(alterados);
        MessageBoxW(hwnd, msg.c_str(), LoadResString(IDS_MSG_CONCLUIDO).c_str(), MB_OK | MB_ICONINFORMATION);
        break;
    }
    case ID_BTN_BUSCAR: {
        HWND hLista = GetListaResultados(hwnd);
        if (!hLista) break;
        wchar_t buffer[260] = {};
        GetWindowTextW(GetDlgItem(hwnd, ID_EDIT_ENTRADA), buffer, 260);
        std::wstring alvo(buffer);
        if (alvo.empty()) {
            MessageBoxW(hwnd, LoadResString(IDS_MSG_DIGITE_NOME).c_str(), LoadResString(IDS_MSG_AVISO).c_str(), MB_OK | MB_ICONWARNING);
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
            if (equals_ignore_case(nome, alvo)) {
                ListView_SetItemState(hLista, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(hLista, i, FALSE);
                encontrado = true;
                break;
            }
        }
        if (!encontrado) {
            wchar_t msg[512];
            swprintf(msg, 512, LoadResString(IDS_MSG_PROC_NAO_ENCONTRADO).c_str(), alvo.c_str());
            MessageBoxW(hwnd, msg, LoadResString(IDS_MSG_NAO_LOCALIZADO).c_str(), MB_OK | MB_ICONINFORMATION);
        }
        // Foca novamente no campo de texto após buscar
        HWND hEdit = GetDlgItem(hwnd, ID_EDIT_ENTRADA);
        if (hEdit) SetFocus(hEdit);
        break;
    }
    case ID_BTN_SALVAR_LOG: {
        HWND hLista = GetListaResultados(hwnd);
        if (hLista) {
            SalvarLogParaArquivo(hLista);
            SalvarFavoritosArquivo(hLista);
        }
        break;
    }
    case ID_BTN_REVERTER: {
        HWND hLista = GetListaResultados(hwnd);
        if (!hLista) break;
        int count = ListView_GetItemCount(hLista);
        int revertidos = 0;
        for (int i = 0; i < count; ++i) {
            if (ListView_GetItemState(hLista, i, LVIS_SELECTED) & LVIS_SELECTED) {
                wchar_t buffer[260];
                ListView_GetItemText(hLista, i, 1, buffer, 260);
                std::wstring nomeProc(buffer);
                auto it = prioridadeOriginal.find(nomeProc);
                if (it != prioridadeOriginal.end()) {
                    bool ok = AlterarPrioridade(buffer, it->second);
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
        break;
    }
    }
}