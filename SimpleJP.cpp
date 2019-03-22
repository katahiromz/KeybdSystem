// SimpleJP.cpp --- KeybdPlugin SimpleJP keyboard
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "PluginFramework.hpp"
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static UINT s_nKeybdID = IDD_LOWER;

LPTSTR LoadStringDx(INT nID)
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
    StringCbCopy(pi->plugin_product_name, sizeof(pi->plugin_product_name), TEXT("SimpleJP"));
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("SimpleJP.keybd"));
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

static void MySleep(void)
{
    Sleep(100);
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

static void DoTypeOneKey(TCHAR ch)
{
    SHORT s = VkKeyScanEx(ch, GetKeyboardLayout(0));
    BYTE wVk = LOBYTE(s);
    BYTE flags = HIBYTE(s);

    if (flags & 4)
    {
        MyKeybdEvent(VK_MENU, 0, 0, 0);
        MySleep();
    }
    if (flags & 2)
    {
        MyKeybdEvent(VK_CONTROL, 0, 0, 0);
        MySleep();
    }
    if (flags & 1)
    {
        MyKeybdEvent(VK_SHIFT, 0, 0, 0);
        MySleep();
    }

    MyKeybdEvent(wVk, 0, 0, 0);
    MySleep();
    MyKeybdEvent(wVk, 0, KEYEVENTF_KEYUP, 0);

    if (flags & 1)
    {
        MySleep();
        MyKeybdEvent(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    }
    if (flags & 2)
    {
        MySleep();
        MyKeybdEvent(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    }
    if (flags & 4)
    {
        MySleep();
        MyKeybdEvent(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    }
}

static void
OnCommandEx(PLUGIN *pi, HWND hDlg, UINT id, UINT codeNotify,
            HWND hwndCtl, const TCHAR *text)
{
    if (hwndCtl == NULL)
        return;

    if (text[1] == 0)
        DoTypeOneKey(text[0]);

    if (lstrcmpi(text, TEXT("Enter")) == 0)
    {
        MyKeybdEvent(VK_RETURN, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_KANA)) == 0 ||
        lstrcmpi(text, LoadStringDx(IDS_HIRAGANA)) == 0)
    {
        s_nKeybdID = IDD_HIRAGANA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_KATAKANA)) == 0)
    {
        s_nKeybdID = IDD_KATAKANA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_ABC)) == 0)
    {
        s_nKeybdID = IDD_LOWER;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_DIGITS)) == 0)
    {
        s_nKeybdID = IDD_DIGITS;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, 0);
        return;
    }
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
        return pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, lParam);
    case ACTION_DESTROY:
        return pi->driver(pi, DRIVER_DESTROY, wParam, lParam);
    case ACTION_COMMAND:
        OnCommand(pi, wParam, lParam);
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
