#include "CustomBorderWindow.h"

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
    ZhiJ::Init();
    ZhiJ::RegisterWNDClass();
    SIZE size = {400, 300};
    ZhiJ::CustomBorderWindow borderless_layered_window{{-1, -1}, size, L"BorderlessLayeredWindowProc"};
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ZhiJ::Exit();
    return 0;
}
