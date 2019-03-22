#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cassert>
#include "MSmoothLayout.hpp"
#include "PluginFramework.hpp"

static const TCHAR s_szName[] = TEXT("KeybdSystem");
static HINSTANCE s_hInst = NULL;
static HWND s_hMainWnd = NULL;
static HWND s_hChildWnd = NULL;
static HWND s_hSizeGrip = NULL;
static MSmoothLayout s_layout;
static HWND s_hwndTarget = NULL;
static PLUGIN s_plugin;

void ModifyStyleEx(HWND hwnd, DWORD dwRemove, DWORD dwAdd)
{
    DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exstyle &= ~dwRemove;
    exstyle |= dwAdd;
    SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndParent = GetParent(hwnd);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        PostMessage(hwndParent, uMsg, wParam, lParam);
        break;
    case WM_NCHITTEST:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR)HTTRANSPARENT);
        return TRUE;
    }
    return 0;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    PF_ActOne(&s_plugin, ACTION_COMMAND, MAKEWPARAM(id, codeNotify), (LPARAM)hwndCtl);
}

void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    MoveWindow(s_hChildWnd, 0, 0, cx, cy, TRUE);

    s_layout.OnSize();

    RECT rc;
    GetClientRect(s_hChildWnd, &rc);
    MoveWindow(s_hSizeGrip,
        rc.right - GetSystemMetrics(SM_CXVSCROLL),
        rc.bottom - GetSystemMetrics(SM_CYHSCROLL),
        GetSystemMetrics(SM_CXVSCROLL),
        GetSystemMetrics(SM_CYHSCROLL),
        TRUE);
}

LRESULT APIENTRY PF_Driver(struct PLUGIN *pi, UINT uFunc, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = s_plugin.framework_window;
    assert(IsWindow(hwnd));

    switch (uFunc)
    {
    case DRIVER_NONE:
        assert(0);
        return FALSE;

    case DRIVER_RECREATE:
        {
            BOOL bHadChild = IsWindow(s_hChildWnd);
            RECT rcWnd;
            GetWindowRect(hwnd, &rcWnd);

            {
                DestroyWindow(s_hSizeGrip);
                s_hSizeGrip = NULL;

                DestroyWindow(s_hChildWnd);
                pi->plugin_window = s_hChildWnd = NULL;
            }

            s_hChildWnd = CreateDialog(s_plugin.plugin_instance, MAKEINTRESOURCE(wParam),
                                       hwnd, DialogProc);
            if (!s_hChildWnd)
                return FALSE;

            s_layout.init(s_hChildWnd);

            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                DWORD style = WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP;
                DWORD exstyle = 0;
                s_hSizeGrip = CreateWindowEx(exstyle, TEXT("SCROLLBAR"), NULL,
                    style, 0, 0, 0, 0,
                    s_hChildWnd, (HMENU)-1, s_hInst, NULL);
            }

            {
                RECT rc;
                GetWindowRect(s_hChildWnd, &rc);
                DWORD style = GetWindowLong(hwnd, GWL_STYLE);
                DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                AdjustWindowRectEx(&rc, style, FALSE, exstyle);

                SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                             SWP_NOMOVE | SWP_NOZORDER);
            }

            pi->plugin_window = s_hChildWnd;

            if (bHadChild)
            {
                MoveWindow(hwnd, rcWnd.left, rcWnd.top,
                    rcWnd.right - rcWnd.left,
                    rcWnd.bottom - rcWnd.top, FALSE);
            }

            ShowWindow(s_hChildWnd, SW_SHOW);
        }
        return TRUE;

    case DRIVER_DESTROY:
        {
            DestroyWindow(s_hSizeGrip);
            s_hSizeGrip = NULL;

            DestroyWindow(s_hChildWnd);
            pi->plugin_window = s_hChildWnd = NULL;
        }
        return TRUE;

    case DRIVER_FINDKEY:
        {
            if (!IsWindow(s_hChildWnd))
                return 0;

            LPTSTR pszText = (LPTSTR)wParam;
            return (LRESULT)FindWindowEx(s_hChildWnd, NULL, TEXT("BUTTON"), pszText);
        }
    }

    return FALSE;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    s_hMainWnd = hwnd;
    HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);
    RemoveMenu(hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
    RemoveMenu(hSysMenu, SC_MINIMIZE, MF_BYCOMMAND);

    if (!PF_LoadOne(&s_plugin, TEXT("SimpleJP.keybd")))
        return FALSE;

    s_plugin.framework_window = hwnd;

    PF_ActOne(&s_plugin, ACTION_RECREATE, 0, 0);

    return TRUE;
}

void OnDestroy(HWND hwnd)
{
    PF_UnloadOne(&s_plugin);
    PostQuitMessage(0);
}

UINT OnNCHitTest(HWND hwnd, int x, int y)
{
    UINT ret = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
    if (ret == HTCLIENT)
        ret = HTCAPTION;
    return ret;
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_NCHITTEST, OnNCHitTest);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE, OnSize);
    case WM_NCACTIVATE:
        DefWindowProc(hwnd, uMsg, TRUE, lParam);
        return FALSE;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    s_hInst = hInstance;
    InitCommonControls();

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = s_szName;
    if (!RegisterClass(&wc))
    {
        MessageBoxA(NULL, "RegisterClass failed", NULL, MB_ICONERROR);
        return 1;
    }

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD exstyle = WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    HWND hwnd = CreateWindowEx(exstyle, s_szName, s_szName, style,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBoxA(NULL, "CreateWindowEx failed", NULL, MB_ICONERROR);
        return 2;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (s_hChildWnd && IsDialogMessage(s_hChildWnd, &msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
