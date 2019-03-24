// StdEn.cpp --- KeybdPlugin StdEn keyboard
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "PluginFramework.hpp"
#include <windowsx.h>
#include <imm.h>
#include <unordered_map>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static UINT s_nKeybdID = IDD_NORMAL;

#define SHIFT 1
#define CAPS 2
#define HIRA 4
#define KATA 8
#define SMALL 16
#define ALT 32
#define CTRL 64
#define SCROLL 128
static DWORD s_dwStatus = 0;

static LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[4][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % _countof(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(s_hinstDLL, nID, pszBuff, cchBuffMax))
    {
        assert(0);
    }
    return pszBuff;
}

static void MySleep(void)
{
    //Sleep(100);
}

static inline void MyKeybdEvent(WORD wVk, WORD wScan, DWORD dwFlags, ULONG_PTR dwExtra)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = wVk;
    input.ki.wScan = wScan;
    input.ki.dwFlags = dwFlags;
    input.ki.time = 0;
    input.ki.dwExtraInfo = dwExtra;
    SendInput(1, &input, sizeof(input));
}

#ifdef __cplusplus
extern "C" {
#endif

// API Name: Plugin_Load
// Purpose: The framework want to load the plugin component.
// TODO: Load the plugin component.
BOOL APIENTRY
Plugin_Load(PLUGIN *pi, LPARAM lParam)
{
    if (!pi)
    {
        assert(0);
        return FALSE;
    }
    if (pi->framework_version < FRAMEWORK_VERSION)
    {
        assert(0);
        return FALSE;
    }
    if (lstrcmpi(pi->framework_name, FRAMEWORK_NAME) != 0)
    {
        assert(0);
        return FALSE;
    }
    if (pi->framework_instance == NULL)
    {
        assert(0);
        return FALSE;
    }

    pi->plugin_version = 2;
    StringCbCopy(pi->plugin_product_name, sizeof(pi->plugin_product_name), TEXT("StdEn"));
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("StdEn.keybd"));
    StringCbCopy(pi->plugin_company, sizeof(pi->plugin_company), TEXT("Katayama Hirofumi MZ"));
    StringCbCopy(pi->plugin_copyright, sizeof(pi->plugin_copyright), TEXT("Copyright (C) 2019 Katayama Hirofumi MZ"));
    pi->plugin_instance = s_hinstDLL;
    pi->plugin_window = NULL;

    return TRUE;
}

// API Name: Plugin_Unload
// Purpose: The framework want to unload the plugin component.
// TODO: Unload the plugin component.
BOOL APIENTRY
Plugin_Unload(PLUGIN *pi, LPARAM lParam)
{
    if (pi->plugin_window)
        return pi->driver(pi, DRIVER_DESTROY, 0, 0);

    return FALSE;
}

static inline BOOL IsCapsLocked()
{
    return (GetKeyState(VK_CAPITAL) & 1);
}

static inline BOOL IsNumLocked()
{
    return (GetKeyState(VK_NUMLOCK) & 1);
}

static void DoTypeBackSpace(PLUGIN *pi)
{
    MyKeybdEvent(VK_BACK, 0, 0, 0);
    MySleep();
    MyKeybdEvent(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
    MySleep();
}

static void DoTypeOneKey(PLUGIN *pi, TCHAR ch)
{
    if (IsCapsLocked())
    {
        if (IsCharLower(ch))
            ch = (TCHAR)(INT_PTR)CharUpper((LPTSTR)(INT_PTR)ch);
        else if (IsCharUpper(ch))
            ch = (TCHAR)(INT_PTR)CharLower((LPTSTR)(INT_PTR)ch);
    }

    SHORT s = VkKeyScanEx(ch, GetKeyboardLayout(0));
    char wVk = LOBYTE(s);
    char flags = HIBYTE(s);
    if (wVk == -1 && flags == -1)
    {
        MyKeybdEvent(0, ch, KEYEVENTF_UNICODE, 0);
        MySleep();
        MyKeybdEvent(0, ch, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, 0);
        MySleep();
        return;
    }

    if ((flags & 4) || (s_dwStatus & ALT))
    {
        MyKeybdEvent(VK_MENU, 0, 0, 0);
        MySleep();
    }
    if ((flags & 2) || (s_dwStatus & CTRL))
    {
        MyKeybdEvent(VK_CONTROL, 0, 0, 0);
        MySleep();
    }
    if ((flags & 1) || (s_dwStatus & SHIFT))
    {
        MyKeybdEvent(VK_SHIFT, 0, 0, 0);
        MySleep();
    }

    MyKeybdEvent(wVk, 0, 0, 0);
    MySleep();
    MyKeybdEvent(wVk, 0, KEYEVENTF_KEYUP, 0);

    if ((flags & 1) || (s_dwStatus & SHIFT))
    {
        MySleep();
        MyKeybdEvent(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    }
    if ((flags & 2) || (s_dwStatus & CTRL))
    {
        MySleep();
        MyKeybdEvent(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    }
    if ((flags & 4) || (s_dwStatus & ALT))
    {
        MySleep();
        MyKeybdEvent(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    }

    MySleep();
}

static BOOL CheckButtonText(const TCHAR *text, UINT ids, UINT vk)
{
    if (lstrcmpi(text, LoadStringDx(ids)) == 0)
    {
        MyKeybdEvent(vk, 0, 0, 0);
        MySleep();
        MyKeybdEvent(vk, 0, KEYEVENTF_KEYUP, 0);
        return TRUE;
    }
    return FALSE;
}

static void
OnCommandEx(PLUGIN *pi, HWND hDlg, UINT id, UINT codeNotify,
            HWND hwndCtl, const TCHAR *text)
{
    if (hwndCtl == NULL)
        return;

    if (CheckButtonText(text, IDS_UP, VK_UP))
        return;
    if (CheckButtonText(text, IDS_DOWN, VK_DOWN))
        return;
    if (CheckButtonText(text, IDS_LEFT, VK_LEFT))
        return;
    if (CheckButtonText(text, IDS_RIGHT, VK_RIGHT))
        return;

    if (text[1] == 0 || lstrcmpi(text, TEXT("&&")) == 0)
    {
        DoTypeOneKey(pi, text[0]);
        s_dwStatus &= ~(SHIFT | CTRL | ALT);
        return;
    }

    if (CheckButtonText(text, IDS_ENTER, VK_RETURN))
        return;
    if (CheckButtonText(text, IDS_CAPS, VK_CAPITAL))
    {
        s_dwStatus &= ~SHIFT;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_SHIFT)) == 0)
    {
        if (s_dwStatus & SHIFT)
            s_dwStatus &= ~SHIFT;
        else
            s_dwStatus |= SHIFT;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_SCROLL)) == 0)
    {
        if (s_dwStatus & SCROLL)
            s_dwStatus &= ~SCROLL;
        else
            s_dwStatus |= SCROLL;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_CTRL)) == 0)
    {
        if (s_dwStatus & CTRL)
            s_dwStatus &= ~CTRL;
        else
            s_dwStatus |= CTRL;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_ALT)) == 0)
    {
        if (s_dwStatus & ALT)
            s_dwStatus &= ~ALT;
        else
            s_dwStatus |= ALT;
        return;
    }
    if (CheckButtonText(text, IDS_BS, VK_BACK))
        return;
    if (CheckButtonText(text, IDS_DEL, VK_DELETE))
        return;
    if (CheckButtonText(text, IDS_F1, VK_F1))
        return;
    if (CheckButtonText(text, IDS_F2, VK_F2))
        return;
    if (CheckButtonText(text, IDS_F3, VK_F3))
        return;
    if (CheckButtonText(text, IDS_F4, VK_F4))
        return;
    if (CheckButtonText(text, IDS_F5, VK_F5))
        return;
    if (CheckButtonText(text, IDS_F6, VK_F6))
        return;
    if (CheckButtonText(text, IDS_F7, VK_F7))
        return;
    if (CheckButtonText(text, IDS_F8, VK_F8))
        return;
    if (CheckButtonText(text, IDS_F9, VK_F9))
        return;
    if (CheckButtonText(text, IDS_F10, VK_F10))
        return;
    if (CheckButtonText(text, IDS_F11, VK_F11))
        return;
    if (CheckButtonText(text, IDS_F12, VK_F12))
        return;
    if (CheckButtonText(text, IDS_TAB, VK_TAB))
        return;
    if (CheckButtonText(text, IDS_WIN, VK_LWIN))
        return;
    if (CheckButtonText(text, IDS_APPS, VK_APPS))
        return;
    if (CheckButtonText(text, IDS_INS, VK_INSERT))
        return;
    if (CheckButtonText(text, IDS_HOME, VK_HOME))
        return;
    if (CheckButtonText(text, IDS_END, VK_END))
        return;
    if (CheckButtonText(text, IDS_PGUP, VK_PRIOR))
        return;
    if (CheckButtonText(text, IDS_PGDN, VK_NEXT))
        return;
    if (CheckButtonText(text, IDS_ESC, VK_ESCAPE))
        return;
    if (CheckButtonText(text, IDS_PRINT, VK_SNAPSHOT))
        return;
    if (CheckButtonText(text, IDS_BREAK, VK_PAUSE))
        return;
}

static void
OnCommand(PLUGIN *pi, WPARAM wParam, LPARAM lParam)
{
    UINT id = LOWORD(wParam);
    UINT codeNotify = HIWORD(wParam);
    HWND hwndCtl = (HWND)lParam;

    TCHAR szText[32];
    GetWindowText(hwndCtl, szText, ARRAYSIZE(szText));

    OnCommandEx(pi, pi->plugin_window, id, codeNotify, hwndCtl, szText);
}

void OnRefresh(PLUGIN *pi)
{
    UINT nNewKeybdID = 0;
    if (IsCapsLocked())
    {
        if (s_dwStatus & SHIFT)
            nNewKeybdID = IDD_CAPITAL_SHIFTED;
        else
            nNewKeybdID = IDD_CAPITAL;
    }
    else
    {
        if (s_dwStatus & SHIFT)
            nNewKeybdID = IDD_SHIFTED;
        else
            nNewKeybdID = IDD_NORMAL;
    }
    if (s_nKeybdID != nNewKeybdID)
    {
        s_nKeybdID = nNewKeybdID;
        pi->driver(pi, DRIVER_RECREATE, nNewKeybdID, s_dwStatus);
    }

    HWND hwndAlt1 = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_ALT));
    HWND hwndAlt2 = FindWindowEx(pi->plugin_window, hwndAlt1, TEXT("BUTTON"), LoadStringDx(IDS_ALT));
    if (s_dwStatus & ALT)
    {
        if (hwndAlt1)
            Button_SetCheck(hwndAlt1, BST_CHECKED);
        if (hwndAlt2)
            Button_SetCheck(hwndAlt2, BST_CHECKED);
    }
    else
    {
        if (hwndAlt1)
            Button_SetCheck(hwndAlt1, BST_UNCHECKED);
        if (hwndAlt2)
            Button_SetCheck(hwndAlt2, BST_UNCHECKED);
    }

    HWND hwndCtrl1 = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_CTRL));
    HWND hwndCtrl2 = FindWindowEx(pi->plugin_window, hwndCtrl1, TEXT("BUTTON"), LoadStringDx(IDS_CTRL));
    if (s_dwStatus & CTRL)
    {
        if (hwndCtrl1)
            Button_SetCheck(hwndCtrl1, BST_CHECKED);
        if (hwndCtrl2)
            Button_SetCheck(hwndCtrl2, BST_CHECKED);
    }
    else
    {
        if (hwndCtrl1)
            Button_SetCheck(hwndCtrl1, BST_UNCHECKED);
        if (hwndCtrl2)
            Button_SetCheck(hwndCtrl2, BST_UNCHECKED);
    }

    HWND hwndShift1 = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_SHIFT));
    HWND hwndShift2 = FindWindowEx(pi->plugin_window, hwndShift1, TEXT("BUTTON"), LoadStringDx(IDS_SHIFT));
    if (s_dwStatus & SHIFT)
    {
        if (hwndShift1)
            Button_SetCheck(hwndShift1, BST_CHECKED);
        if (hwndShift2)
            Button_SetCheck(hwndShift2, BST_CHECKED);
    }
    else
    {
        if (hwndShift1)
            Button_SetCheck(hwndShift1, BST_UNCHECKED);
        if (hwndShift2)
            Button_SetCheck(hwndShift2, BST_UNCHECKED);
    }

    HWND hwndCaps = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_CAPS));
    if (IsCapsLocked())
    {
        s_dwStatus |= CAPS;
        if (hwndCaps)
            Button_SetCheck(hwndCaps, BST_CHECKED);
    }
    else
    {
        s_dwStatus &= ~CAPS;
        if (hwndCaps)
            Button_SetCheck(hwndCaps, BST_UNCHECKED);
    }


    HWND hwndNum = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_NUM));
    if (IsNumLocked())
    {
        if (hwndNum)
            Button_SetCheck(hwndNum, BST_CHECKED);
    }
    else
    {
        if (hwndNum)
            Button_SetCheck(hwndNum, BST_UNCHECKED);
    }
}

// API Name: Plugin_Act
// Purpose: Act something on the plugin.
// TODO: Act something on the plugin.
LRESULT APIENTRY
Plugin_Act(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam)
{
    switch (uAction)
    {
    case ACTION_NONE:
        assert(0);
        break;
    case ACTION_RECREATE:
        return pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
    case ACTION_DESTROY:
        s_nKeybdID = IDD_NORMAL;
        return pi->driver(pi, DRIVER_DESTROY, wParam, lParam);
    case ACTION_COMMAND:
        OnCommand(pi, wParam, lParam);
        break;
    case ACTION_REFRESH:
        OnRefresh(pi);
        break;
    case ACTION_TIMER:
        OnRefresh(pi);
        break;
    }
    return 0;
}

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        s_hinstDLL = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif
