#include "gui.h"
#include <windows.h>

// Remove implementações duplicadas, apenas chama CriarJanela e WinMain

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CriarJanela(hInstance, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}