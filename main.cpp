#include "gui.h"
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

// ID de botão só pra exemplo
#define ID_BTN_TESTAR 1001

// Forward do WndProc
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Implementação da função chamada no main
void CriarJanela(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t* classeJanela = L"JanelaPrincipalV2";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = classeJanela;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, classeJanela, L"Gerenciador de Prioridades — V2",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Falha ao criar janela!", L"Erro", MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

// Nosso WndProc básico com botão de teste
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnTeste;

    switch (msg) {
    case WM_CREATE:
        hBtnTeste = CreateWindowW(L"BUTTON", L"Testar V2",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 20, 120, 30,
            hwnd, (HMENU)ID_BTN_TESTAR, nullptr, nullptr);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_TESTAR) {
            MessageBoxW(hwnd, L"Funcionando com sucesso! 🚀", L"V2 online", MB_OK | MB_ICONINFORMATION);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Adiciona WinMain para entrada do programa Windows
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CriarJanela(hInstance, nCmdShow);
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}