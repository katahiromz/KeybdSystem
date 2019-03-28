#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <strsafe.h>
#include "MSmoothLayout.hpp"
#include "PluginFramework.hpp"
#include "resource.h"

static const TCHAR s_szName[] = TEXT("KeybdSystem");
static HINSTANCE s_hInst = NULL;
static HWND s_hMainWnd = NULL;
static HWND s_hChildWnd = NULL;
static HWND s_hSizeGrip = NULL;
static MSmoothLayout s_layout;
static HWND s_hwndTarget = NULL;
static std::vector<PLUGIN> s_plugins;
static INT s_iPlugin = 0;
static std::wstring s_strSelectedName;

static inline PLUGIN *GetCurPlugin(void)
{
    if (s_iPlugin < s_plugins.size())
        return &s_plugins[s_iPlugin];
    return NULL;
}

static LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[4][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % _countof(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(NULL, nID, pszBuff, cchBuffMax))
    {
        assert(0);
    }
    return pszBuff;
}

void ModifyStyleEx(HWND hwnd, DWORD dwRemove, DWORD dwAdd)
{
    DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exstyle &= ~dwRemove;
    exstyle |= dwAdd;
    SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
}

LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    if (pnmhdr->code == NM_CUSTOMDRAW && pnmhdr->hwndFrom)
    {
        if (LRESULT ret = PF_ActOne(GetCurPlugin(), ACTION_CUSTOMDRAW,
                                    (WPARAM)idFrom, (LPARAM)pnmhdr))
        {
            return ret;
        }

        NMCUSTOMDRAW *pcd = (NMCUSTOMDRAW *)pnmhdr;
        HWND hwndFrom = pnmhdr->hwndFrom;

        TCHAR szText[64];
        GetWindowText(hwndFrom, szText, 64);

        DWORD style = GetWindowStyle(hwndFrom);
        if ((style & BS_TYPEMASK) == BS_PUSHBUTTON ||
            (style & BS_TYPEMASK) == BS_DEFPUSHBUTTON)
        {
        }
        else if ((style & BS_PUSHLIKE) && 
            ((style & BS_TYPEMASK) == BS_AUTOCHECKBOX ||
             (style & BS_TYPEMASK) == BS_AUTORADIOBUTTON ||
             (style & BS_TYPEMASK) == BS_CHECKBOX ||
             (style & BS_TYPEMASK) == BS_RADIOBUTTON))
        {
        }
        else
        {
            return CDRF_DODEFAULT;
        }

        RECT rc;
        GetClientRect(hwndFrom, &rc);

        switch (pcd->dwDrawStage)
        {
        case CDDS_PREERASE:
        case CDDS_ITEMPREERASE:
            if (pcd->uItemState & CDIS_HOT)
            {
                if (pcd->uItemState & CDIS_SELECTED)
                    DrawFrameControl(pcd->hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED | DFCS_HOT);
                else
                    DrawFrameControl(pcd->hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_HOT);
            }
            else
            {
                if (pcd->uItemState & CDIS_SELECTED)
                    DrawFrameControl(pcd->hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
                else
                    DrawFrameControl(pcd->hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH);
            }
            return CDRF_NOTIFYPOSTERASE;
        case CDDS_POSTERASE:
        case CDDS_ITEMPOSTERASE:
            return CDRF_SKIPDEFAULT;
        case CDDS_PREPAINT:
        case CDDS_ITEMPREPAINT:
            {
                HFONT hFont = GetWindowFont(hwnd);
                assert(hFont);

                SIZE siz;
                siz.cx = (rc.right - rc.left) * 8 / 10;
                siz.cy = (rc.bottom - rc.top) * 6 / 10;

                LOGFONT lf;
                GetObject(hFont, sizeof(lf), &lf);
                lf.lfQuality = ANTIALIASED_QUALITY;
                lf.lfHeight = -std::min(siz.cx, siz.cy);

                if (lstrcmpi(lf.lfFaceName, TEXT("MS UI Gothic")) == 0)
                {
                    lf.lfWeight = FW_BOLD;
                    if (~GetFileAttributes(TEXT("C:\\Windows\\Fonts\\meiryo.ttc")))
                        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), TEXT("Meiryo"));
                    else
                        StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), TEXT("MS PGothic"));
                }

                UINT uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;

                for (INT i = 0; i < 16; ++i)
                {
                    hFont = CreateFontIndirect(&lf);

                    RECT rcText;
                    SetRectEmpty(&rcText);

                    HGDIOBJ hFontOld = SelectObject(pcd->hdc, hFont);
                    DrawText(pcd->hdc, szText, lstrlen(szText), &rcText, uFormat | DT_CALCRECT);
                    SelectObject(pcd->hdc, hFontOld);

                    SIZE sizText;
                    sizText.cx = rcText.right - rcText.left;
                    sizText.cy = rcText.bottom - rcText.top;
                    if (sizText.cx <= siz.cx && sizText.cy <= siz.cy)
                        break;

                    lf.lfHeight = lf.lfHeight * 9 / 10;
                    DeleteObject(hFont);
                }

                if (pcd->uItemState & CDIS_SELECTED)
                    OffsetRect(&rc, 1, 1);

                HGDIOBJ hFontOld = SelectObject(pcd->hdc, hFont);
                SetBkMode(pcd->hdc, TRANSPARENT);
                SetTextColor(pcd->hdc, GetSysColor(COLOR_BTNTEXT));
                DrawText(pcd->hdc, szText, lstrlen(szText), &rc, uFormat);
                SelectObject(pcd->hdc, hFontOld);

                DeleteObject(hFont);
            }
            return CDRF_SKIPDEFAULT;
        case CDDS_POSTPAINT:
        case CDDS_ITEMPOSTPAINT:
            return CDRF_SKIPDEFAULT;
        case CDDS_SUBITEM:
            return CDRF_SKIPDEFAULT;
        }

        return CDRF_DODEFAULT;
    }

    return 0;
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;
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
    case WM_TIMER:
        PF_ActOne(GetCurPlugin(), ACTION_TIMER, 0, 0);
        break;
    case WM_CONTEXTMENU:
        PostMessage(hwndParent, uMsg, wParam, lParam);
        break;
    case WM_NOTIFY:
        result = OnNotify(hwnd, (INT)wParam, (NMHDR *)lParam);
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result);
        return TRUE;
    }
    return 0;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (hwndCtl)
    {
        PF_ActOne(GetCurPlugin(), ACTION_COMMAND, MAKEWPARAM(id, codeNotify), (LPARAM)hwndCtl);
        return;
    }

    if (codeNotify == 0)
    {
        KillTimer(s_hChildWnd, 999);

        INT nIndex = id - 1;
        if (0 <= nIndex && nIndex < (INT)s_plugins.size())
        {
            PF_ActOne(GetCurPlugin(), ACTION_DESTROY, 0, 0);
            s_iPlugin = nIndex;
            PF_ActOne(GetCurPlugin(), ACTION_RECREATE, 0, 0);
            s_strSelectedName = GetCurPlugin()->plugin_product_name;
        }
    }
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
    HWND hwnd = GetCurPlugin()->framework_window;
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

            KillTimer(s_hChildWnd, 999);

            if (bHadChild)
            {
                DestroyWindow(s_hSizeGrip);
                s_hSizeGrip = NULL;

                DestroyWindow(s_hChildWnd);
                pi->plugin_window = s_hChildWnd = NULL;
            }

            s_hChildWnd = CreateDialog(GetCurPlugin()->plugin_instance,
                                       MAKEINTRESOURCE(wParam),
                                       hwnd, DialogProc);
            if (!s_hChildWnd)
                return FALSE;

            s_layout.init(s_hChildWnd);

            {
                RECT rc;
                GetClientRect(s_hChildWnd, &rc);
                DWORD style = WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP;
                DWORD exstyle = 0;
                s_hSizeGrip = CreateWindowEx(exstyle, TEXT("SCROLLBAR"), NULL, style,
                    rc.right - GetSystemMetrics(SM_CXVSCROLL),
                    rc.bottom - GetSystemMetrics(SM_CYHSCROLL),
                    GetSystemMetrics(SM_CXVSCROLL),
                    GetSystemMetrics(SM_CYHSCROLL),
                    s_hChildWnd, (HMENU)-1, s_hInst, NULL);
            }

            {
                RECT rc;
                GetWindowRect(s_hChildWnd, &rc);
                DWORD style = GetWindowLong(hwnd, GWL_STYLE);
                DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                AdjustWindowRectEx(&rc, style, FALSE, exstyle);

                INT cx = rc.right - rc.left;
                INT cy = rc.bottom - rc.top;
                SetWindowPos(hwnd, NULL, 0, 0, cx, cy,
                             SWP_NOMOVE | SWP_NOZORDER);
                if (!bHadChild)
                {
                    RECT rcWork;
                    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
                    SetWindowPos(hwnd, NULL, rcWork.left, rcWork.bottom - cy, 0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);
                }
            }

            pi->plugin_window = s_hChildWnd;

            if (bHadChild)
            {
                MoveWindow(hwnd, rcWnd.left, rcWnd.top,
                    rcWnd.right - rcWnd.left,
                    rcWnd.bottom - rcWnd.top, FALSE);
            }

            PF_ActOne(pi, ACTION_REFRESH, 0, 0);
            ShowWindow(s_hChildWnd, SW_SHOW);
            SetTimer(s_hChildWnd, 999, 100, NULL);
        }
        return TRUE;

    case DRIVER_DESTROY:
        {
            KillTimer(s_hChildWnd, 999);

            DestroyWindow(s_hSizeGrip);
            s_hSizeGrip = NULL;

            DestroyWindow(s_hChildWnd);
            pi->plugin_window = s_hChildWnd = NULL;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL DoLoadSettings(HWND hwnd)
{
    static const TCHAR s_szSubKey[] = TEXT("Software\\Katayama Hirofumi MZ\\KeybdSystem");

    s_strSelectedName.clear();

    HKEY hApp = NULL;
    RegOpenKeyEx(HKEY_CURRENT_USER, s_szSubKey, 0, KEY_READ, &hApp);
    if (hApp)
    {
        TCHAR szValue[128];
        DWORD cb = sizeof(szValue);

        szValue[0] = 0;
        RegQueryValueEx(hApp, TEXT("Selected"), NULL, NULL, (LPBYTE)szValue, &cb);
        s_strSelectedName = szValue;

        RegCloseKey(hApp);

        return TRUE;
    }

    return FALSE;
}

BOOL DoSaveSettings(HWND hwnd)
{
    HKEY hSoftware = NULL;
    RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, NULL, 0, KEY_ALL_ACCESS,
                   NULL, &hSoftware, NULL);
    if (hSoftware)
    {
        HKEY hCompany = NULL;
        RegCreateKeyEx(hSoftware, TEXT("Katayama Hirofumi MZ"), 0, NULL, 0, KEY_ALL_ACCESS,
                       NULL, &hCompany, NULL);
        if (hCompany)
        {
            HKEY hApp = NULL;
            RegCreateKeyEx(hCompany, TEXT("KeybdSystem"), 0, NULL, 0, KEY_ALL_ACCESS,
                           NULL, &hApp, NULL);
            if (hApp)
            {
                TCHAR szValue[128];
                DWORD cb;

                StringCbCopy(szValue, sizeof(szValue), s_strSelectedName.c_str());
                cb = (lstrlen(szValue) + 1) * sizeof(TCHAR);
                RegSetValueEx(hApp, TEXT("Selected"), 0, REG_SZ, (LPBYTE)szValue, cb);

                RegCloseKey(hApp);
            }
            RegCloseKey(hCompany);
        }
        RegCloseKey(hSoftware);
    }
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    s_hMainWnd = hwnd;
    HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);
    RemoveMenu(hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
    RemoveMenu(hSysMenu, SC_MINIMIZE, MF_BYCOMMAND);

    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
    *PathFindFileName(szPath) = 0;
    PathRemoveBackslash(szPath);

    DoLoadSettings(hwnd);

    if (!PF_LoadAll(s_plugins, szPath))
    {
        MessageBox(hwnd, LoadStringDx(IDS_CANTLOADPLUGINS), NULL, MB_ICONERROR);
        return FALSE;
    }

    std::sort(s_plugins.begin(), s_plugins.end(),
        [](const PLUGIN& a, const PLUGIN& b) {
            return lstrcmpiW(a.plugin_product_name, b.plugin_product_name) < 0;
        }
    );

    for (size_t i = 0; i < s_plugins.size(); ++i)
    {
        s_plugins[i].framework_window = hwnd;

        if (lstrcmpiW(s_plugins[i].plugin_product_name, s_strSelectedName.c_str()) == 0)
        {
            s_iPlugin = (INT)i;
        }
    }

    PF_ActOne(GetCurPlugin(), ACTION_RECREATE, 0, 0);

    SetWindowText(hwnd, LoadStringDx(IDS_APP_NAME));

    return TRUE;
}

void OnDestroy(HWND hwnd)
{
    DoSaveSettings(hwnd);
    PF_UnloadAll(s_plugins);
    PostQuitMessage(0);
}

UINT OnNCHitTest(HWND hwnd, int x, int y)
{
    UINT ret = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
    if (ret == HTCLIENT)
        ret = HTCAPTION;
    return ret;
}

void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    lpMinMaxInfo->ptMinTrackSize.x = 400;
    lpMinMaxInfo->ptMinTrackSize.y = 180;
}

void DoShowContextMenu(HWND hwnd, INT x, INT y)
{
    HMENU hMenu = CreatePopupMenu();

    if (x == 0xFFFF && y == 0xFFFF)
    {
        POINT pt;
        GetCursorPos(&pt);
        x = pt.x;
        y = pt.y;
    }

    for (size_t i = 0; i < s_plugins.size(); ++i)
    {
        PLUGIN& pl = s_plugins[i];
        InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION, i + 1, pl.plugin_product_name);
    }

    INT nCount = (INT)s_plugins.size();
    CheckMenuRadioItem(hMenu, 0, nCount - 1, s_iPlugin, MF_BYPOSITION);

    SetForegroundWindow(hwnd);
    UINT uFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD;
    INT nCmd = TrackPopupMenu(hMenu, uFlags, x, y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    PostMessage(hwnd, WM_COMMAND, nCmd, 0);

    DestroyMenu(hMenu);
}

void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
    if (hwndContext == s_hChildWnd)
        return;

    UINT ret = FORWARD_WM_NCHITTEST(hwnd, xPos, yPos, DefWindowProc);
    if (ret == HTCAPTION)
    {
        FORWARD_WM_CONTEXTMENU(hwnd, hwndContext, xPos, yPos, DefWindowProc);
        return;
    }
    DoShowContextMenu(hwnd, xPos, yPos);
}

void OnNCRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
    UINT ret = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
    if (ret == HTCAPTION)
    {
        FORWARD_WM_NCRBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, DefWindowProc);
        return;
    }
    DoShowContextMenu(hwnd, x, y);
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
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
        HANDLE_MSG(hwnd, WM_CONTEXTMENU, OnContextMenu);
        HANDLE_MSG(hwnd, WM_NCRBUTTONDOWN, OnNCRButtonDown);
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

    WNDCLASSEX wcx;
    ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcx.lpszClassName = s_szName;
    wcx.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(1), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    if (!RegisterClassEx(&wcx))
    {
        MessageBoxA(NULL, "RegisterClassEx failed", NULL, MB_ICONERROR);
        return 1;
    }

    RECT rcWork;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD exstyle = WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    HWND hwnd = CreateWindowEx(exstyle, s_szName, s_szName, style,
        rcWork.left, rcWork.bottom, 0, 0,
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
