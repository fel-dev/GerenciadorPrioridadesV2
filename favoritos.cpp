#include "favoritos.h"
#include "utils.h"
#include <fstream>
#include <cwchar>
#include <vector>
#include <algorithm>
#include <commctrl.h>

std::set<std::wstring> LerFavoritosArquivo() {
    std::set<std::wstring> favoritos;
    std::wifstream favFile(L"favoritos.txt");
    std::wstring nome;
    while (std::getline(favFile, nome)) {
        favoritos.insert(nome);
    }
    favFile.close();
    return favoritos;
}

void SalvarFavoritosArquivo(HWND hList) {
    std::wofstream favFile(L"favoritos.txt", std::ios::trunc);
    if (!favFile.is_open()) return;
    int total = ListView_GetItemCount(hList);
    for (int i = 0; i < total; ++i) {
        wchar_t estrela[8] = {};
        ListView_GetItemText(hList, i, 0, estrela, 8);
        if (wcscmp(estrela, L"⭐") == 0) {
            wchar_t nomeProc[256] = {};
            ListView_GetItemText(hList, i, 1, nomeProc, 256);
            favFile << nomeProc << L"\n";
        }
    }
    favFile.close();
}

bool ProcessoEhFavorito(const std::set<std::wstring>& favoritos, const std::wstring& nomeProc) {
    return favoritos.find(nomeProc) != favoritos.end();
}
