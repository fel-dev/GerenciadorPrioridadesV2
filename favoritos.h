#pragma once
#include <windows.h>
#include <string>
#include <set>

// Lê o arquivo de favoritos.txt e retorna um set com os nomes dos processos favoritos
std::set<std::wstring> LerFavoritosArquivo();

// Salva os processos marcados como favoritos do ListView no arquivo favoritos.txt
void SalvarFavoritosArquivo(HWND hList);

// Consulta se um processo está na lista de favoritos
bool ProcessoEhFavorito(const std::set<std::wstring>& favoritos, const std::wstring& nomeProc);
