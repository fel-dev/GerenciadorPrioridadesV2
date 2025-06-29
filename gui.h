#pragma once
#include <windows.h>
#include <string>

// IDs dos componentes da interface
#define ID_BTN_ALTA     2001
#define ID_BTN_BAIXA    2002
#define ID_BTN_ATUALIZAR 2003
#define ID_BTN_APLICAR_SELECIONADOS 2004
#define ID_BTN_BUSCAR   2005
#define ID_EDIT_ENTRADA 3001

// Função que cria a janela principal
void CriarJanela(HINSTANCE hInstance, int nCmdShow);

// Função para alterar a prioridade de um processo
bool AlterarPrioridade(const std::wstring& nomeProcesso, unsigned int prioridade);