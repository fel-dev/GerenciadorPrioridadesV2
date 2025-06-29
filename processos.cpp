#include "processos.h"
#include <windows.h>
#include <tlhelp32.h>
#include <string>

bool AlterarPrioridade(const std::wstring& nomeProcesso, unsigned int prioridade) {
    bool alterado = false;
    std::wstring nomeAlvo = nomeProcesso;
    // Remove .exe do final se o usuário não digitou
    if (nomeAlvo.size() > 4 && nomeAlvo.substr(nomeAlvo.size() - 4) != L".exe") {
        nomeAlvo += L".exe";
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(hSnap, &pe)) {
        do {
            // Comparação case-insensitive
            if (_wcsicmp(pe.szExeFile, nomeAlvo.c_str()) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc) {
                    if (SetPriorityClass(hProc, prioridade)) {
                        alterado = true;
                    }
                    CloseHandle(hProc);
                }
            }
        } while (Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return alterado;
}