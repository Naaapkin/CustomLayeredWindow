#pragma once
#include <cstdint>
#include <string>
#include <windows.h>
#include <windowsx.h>

namespace ZhiJ
{
    #define CAPTION_HEIGHT 30
    #define SCALER_WIDTH 8
    #define BORDER_WIDTH 1

    extern BLENDFUNCTION g_blend;
    extern POINT g_origin_point;
    extern HDC g_screen_dc;
    extern HPEN g_gray_pen;

    void Init();

    void Exit();

    void DefTitleDrawer(HWND _hWnd, HDC _hdc, const SIZE& _caption_size, HFONT _title_font);
    
    void DefFrameDrawer(HWND _hWnd, HDC _hdc, const SIZE& _size, HFONT _title_font);
    
    void RegisterWNDClass();
    
    LRESULT CustomFrameHitTest(HWND _hWnd, WPARAM wParam, LPARAM lParam);
    
    LRESULT CALLBACK BorderlessLayeredWindowProc(HWND _hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void UpdateSize(HWND _hWnd, WPARAM _wParam, LPARAM _lParam);
    
    void UpdateWindow(HWND _hWnd);
    
    void DrawLayeredWindow(HWND _hWnd, uint16_t w, uint16_t h);
    
    class CustomBorderWindow
    {
        HWND hWnd_;
        HDC hdc_;
        HFONT title_font_;
        bool is_active_;
        void(*frame_drawer)(HWND _hwnd, HDC _hdc, const SIZE& _size, HFONT _title_font) = DefFrameDrawer;
        
    public:
        CustomBorderWindow(const POINT& _pos, SIZE& _size, const std::wstring& _title);

        ~CustomBorderWindow();
        
        HWND GetHandler() const;
        
        HDC GetHDC() const;

        void Active(bool _active);
        
        void ClearClient(COLORREF _color) const;
        
        void DrawFrame() const;
        
        static RECT GetClientRect(const CustomBorderWindow& _window);
    };
}


