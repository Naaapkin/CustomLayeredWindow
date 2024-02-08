#include "CustomBorderWindow.h"

#include <Uxtheme.h>
#include <dwmapi.h>
#include <iostream>

#pragma comment(lib, "Dwmapi.lib")

namespace ZhiJ
{
#define TEST_ENABLE
#ifdef TEST_ENABLE
#endif

    BLENDFUNCTION g_blend;
    POINT g_origin_point;
    HDC g_screen_dc;
    HPEN g_gray_pen;

    void DefTitleDrawer(HWND _hWnd, HDC _hdc, LONG _caption_width, HFONT _title_font)
    {
        SelectObject(_hdc, GetStockPen(BLACK_PEN));
        SelectObject(_hdc, GetStockBrush(NULL_BRUSH));
        SelectObject(_hdc, _title_font);
        const int length = GetWindowTextLength(_hWnd) + 1;
        auto* title = new wchar_t[length];
        GetWindowText(_hWnd, title, length);
        RECT rect{BORDER_WIDTH + 5, static_cast<LONG>(CAPTION_HEIGHT * 0.3), _caption_width - 5, static_cast<LONG>(CAPTION_HEIGHT * 0.7)};
        DrawText(_hdc, title, length, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);
        delete[] title;
    }

    void Init()
    {
        g_screen_dc = GetDC(nullptr);
        g_gray_pen = CreatePen(PS_SOLID, 1, 0x00555555);
        g_origin_point = {0, 0};
        g_blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    }

    void Exit()
    {
        DeleteObject(g_gray_pen);
        ReleaseDC(nullptr, g_screen_dc);
    }

    void DefFrameDrawer(HWND _hWnd, HDC _hdc, const SIZE& _size, HFONT _title_font) 
    {
        SelectObject(_hdc, GetStockBrush(NULL_BRUSH));
        SelectObject(_hdc, g_gray_pen);
        Rectangle(_hdc, 1, 1, _size.cx, _size.cy);
        const RECT caption{2, 2, _size.cx - 2, CAPTION_HEIGHT};
        FillRect(_hdc, &caption, GetStockBrush(WHITE_BRUSH));
        DefTitleDrawer(_hWnd, _hdc, _size.cx - 2, _title_font);
    }

    void RegisterWNDClass()
    {
        WNDCLASS wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpfnWndProc = BorderlessLayeredWindowProc;
        wc.lpszClassName = L"BorderlessLayeredWindow";
        RegisterClass(&wc);
    }

    LRESULT CustomFrameHitTest(HWND _hWnd, WPARAM wParam, LPARAM lParam)
    {
        RECT wr, cr;
        const POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        GetWindowRect(_hWnd, &wr);
        cr.left = wr.left + SCALER_WIDTH;
        cr.right = wr.right - SCALER_WIDTH;
        cr.bottom = wr.bottom - SCALER_WIDTH;
        cr.top = wr.top + SCALER_WIDTH;

        uint8_t pos_code = 0;
        if (ptMouse.x < wr.left || ptMouse.x > wr.right || ptMouse.y < wr.top || ptMouse.y > wr.bottom)
            return HTNOWHERE;
        
        if (ptMouse.x < cr.left)
            pos_code |= 0b01;
        else if (ptMouse.x > cr.right)
            pos_code |= 0b11;
        else
            pos_code |= 0b10;

        if (ptMouse.y < cr.top)
            pos_code |= 0b0100;
        else if (ptMouse.y < wr.top + CAPTION_HEIGHT)
            return HTCAPTION;
        else if (ptMouse.y > cr.bottom)
            pos_code |= 0b1100;
        else
            pos_code |= 0b1000;
        
        switch (pos_code)
        {
        case 0b0101:
            return HTTOPLEFT;
        case 0b0110:
            return HTTOP;
        case 0b0111:
            return HTTOPRIGHT;
        case 0b1001:
            return HTLEFT;
        case 0b1010:
            return HTCLIENT;
        case 0b1011:
            return HTRIGHT;
        case 0b1101:
            return HTBOTTOMLEFT;
        case 0b1110:
            return HTBOTTOM;
        case 0b1111:
            return HTBOTTOMRIGHT;
        }
        return HTNOWHERE;
    }

    LRESULT BorderlessLayeredWindowProc(HWND _hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg) {
        case WM_ACTIVATE:
            {
                constexpr MARGINS margins{BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH};
                DwmExtendFrameIntoClientArea(_hWnd, &margins);
            }
            return 0;
        case WM_CREATE:
            {
                RECT rcClient;
                GetWindowRect(_hWnd, &rcClient);
        
                // Inform application of the frame change.
                SetWindowPos(_hWnd, 
                             nullptr, 
                             rcClient.left, rcClient.top,
                             rcClient.right - rcClient.left,
                             rcClient.bottom - rcClient.top,
                             SWP_FRAMECHANGED);
                const auto cs = reinterpret_cast<CREATESTRUCT*>(lParam);
                SetWindowLongPtr(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
                return 0;
            }
        case WM_NCCALCSIZE:
            if (wParam)
            {
                return 0;
            }
            break;
        case WM_NCHITTEST:
            {
                LRESULT lr;
                if (!DwmDefWindowProc(_hWnd, uMsg, wParam, lParam, &lr))
                    lr = CustomFrameHitTest(_hWnd, wParam, lParam);
                return lr;
            }
        case WM_NCACTIVATE:
            return 1;
        case WM_SIZING:
            {
                ZhiJ::UpdateSize(_hWnd, wParam, lParam);
                return 0;
            }
        case WM_CLOSE:
            ::DestroyWindow(_hWnd);
            return 0;
        case WM_DESTROY: 
            PostQuitMessage(0);
            return 0;
        }
            
        return DefWindowProc(_hWnd, uMsg, wParam, lParam);
    }

    void UpdateSize(HWND _hWnd, WPARAM _wParam, LPARAM _lParam)
    {
        const auto ptr = GetWindowLongPtr(_hWnd, GWLP_USERDATA);
        const auto* blw = reinterpret_cast<CustomBorderWindow*>(ptr);
        const auto rect = reinterpret_cast<RECT*>(_lParam);
        POINT pos = {rect->left, rect->top};
        SIZE size = {rect->right - rect->left, rect->bottom - rect->top - 1};
        blw->DrawFrame();
        blw->ClearClient(0);
        UpdateLayeredWindow(_hWnd, nullptr, &pos, &size, blw->GetHDC(), &g_origin_point, RGB(255, 255, 255), &g_blend, ULW_OPAQUE);
    }

    void UpdateWindow(HWND _hWnd)
    {
        const auto ptr = GetWindowLongPtr(_hWnd, GWLP_USERDATA);
        const auto* blw = reinterpret_cast<CustomBorderWindow*>(ptr);
        UpdateLayeredWindow(_hWnd, nullptr, nullptr, nullptr, blw->GetHDC(), &g_origin_point, RGB(255, 255, 255), &g_blend, ULW_OPAQUE);
    }

    void CustomBorderWindow::DrawFrame() const
    {
        if (!hWnd_) return;
        RECT rect;
        GetWindowRect(hWnd_, &rect);
        const SIZE size = {rect.right - rect.left, rect.bottom - rect.top};
        frame_drawer(hWnd_, hdc_, size, title_font_);
    }

    CustomBorderWindow::CustomBorderWindow(const POINT& _pos, SIZE& _size, const std::wstring& _title)
    {
        hWnd_ = CreateWindowExW(
        WS_EX_LAYERED,
        L"BorderlessLayeredWindow",
        L"BorderlessLayeredWindow",
        WS_VISIBLE | WS_SIZEBOX | WS_SYSMENU,
        _pos.x > 0 ? _pos.x : CW_USEDEFAULT, _pos.y > 0 ? _pos.y : CW_USEDEFAULT,
        _size.cx, _size.cy,
        nullptr, nullptr,
        GetModuleHandleW(nullptr),
        this);

        hdc_ = CreateCompatibleDC(g_screen_dc);
        const HBITMAP hBmp = CreateCompatibleBitmap(g_screen_dc,1920,1080);
        SelectObject(hdc_,hBmp);

        constexpr LONG font_size = static_cast<LONG>(-0.4 * CAPTION_HEIGHT);
        title_font_ = CreateFont(
            font_size, font_size / 2, 0, 0, 400,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
            DEFAULT_QUALITY,  //默认输出质量
            FF_DONTCARE,  //不指定字体族*/
            L"微软雅黑"  //字体名
        );
        
        ClearClient(0);
        DrawFrame();
        
        RECT rect;
        GetWindowRect(hWnd_, &rect);
        POINT pos = {rect.left, rect.top};
        UpdateLayeredWindow(hWnd_, nullptr, &pos, &_size, hdc_, &g_origin_point, RGB(0, 0, 0), &g_blend, ULW_OPAQUE);
    }

    CustomBorderWindow::~CustomBorderWindow()
    {
        const auto hbm = static_cast<HBITMAP>(GetCurrentObject(hdc_, OBJ_BITMAP));
        DeleteObject(hbm);
        ReleaseDC(nullptr, hdc_);
    }

    RECT CustomBorderWindow::GetClientRect(const CustomBorderWindow& _window)
    {
        RECT cr;
        GetWindowRect(_window.GetHandler(), &cr);
        cr.right -= cr.left + 1;
        cr.bottom -= cr.top  + 1;
        cr.left = 1;
        cr.top = CAPTION_HEIGHT;
        return cr;
    }

    HWND CustomBorderWindow::GetHandler() const
    {
        return hWnd_;
    }

    HDC CustomBorderWindow::GetHDC() const
    {
        return hdc_;
    }

    void CustomBorderWindow::Active(bool _active)
    {
        is_active_ = _active;
    }

    void CustomBorderWindow::ClearClient(COLORREF _color) const
    {
        const HBRUSH brush = CreateSolidBrush(_color);
        RECT rect;
        GetWindowRect(hWnd_, &rect);
        rect.right -= rect.left + BORDER_WIDTH;
        rect.bottom -= rect.top + BORDER_WIDTH;
        rect.left = BORDER_WIDTH + 1;
        rect.top = CAPTION_HEIGHT;
        FillRect(hdc_, &rect, brush);
        DeleteBrush(brush);
    }
}

