// SimpleJP.cpp --- KeybdPlugin SimpleJP keyboard
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#include "PluginFramework.hpp"
#include <windowsx.h>
#include <imm.h>
#include <cassert>
#include <strsafe.h>
#include "resource.h"

static HINSTANCE s_hinstDLL;
static UINT s_nKeybdID = IDD_LOWER;
static DWORD s_dwConv = 0;

#define SHIFT 1
#define CAPS 2
#define HIRA 4
#define KATA 8
static DWORD s_dwFlags = 0;

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

#ifndef IMC_GETOPENSTATUS
    #define IMC_GETOPENSTATUS 0x0005
#endif
#ifndef IMC_SETCONVERSIONMODE
    #define IMC_SETCONVERSIONMODE 0x0002
#endif

static void ImeOnOff(PLUGIN *pi, BOOL bOn)
{
    HWND hwnd = GetForegroundWindow();
    HWND hwndIME = ImmGetDefaultIMEWnd(hwnd);
    BOOL bOpen = SendMessage(hwndIME, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0);

    if (bOpen != bOn)
    {
        if (!bOn)
        {
            
        }
        MyKeybdEvent(VK_MENU, 0, 0, 0);
        MyKeybdEvent(VK_KANJI, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_KANJI, 0, KEYEVENTF_KEYUP, 0);
        MyKeybdEvent(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    }

    SendMessage(hwndIME, WM_IME_CONTROL, IMC_SETCONVERSIONMODE, s_dwConv);
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

static void DoTypeOneKey(PLUGIN *pi, TCHAR ch)
{
    SHORT s = VkKeyScanEx(ch, GetKeyboardLayout(0));
    char wVk = LOBYTE(s);
    char flags = HIBYTE(s);
    if (wVk == -1 && flags == -1)
    {
        HWND hwnd = GetForegroundWindow();
        ImeOnOff(pi, TRUE);

        MyKeybdEvent(0, ch, KEYEVENTF_UNICODE, 0);
        MySleep();
        MyKeybdEvent(0, ch, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, 0);
        MySleep();
        return;
    }
    else
    {
        ImeOnOff(pi, FALSE);
    }

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
    MySleep();
}

static void
OnCommandEx(PLUGIN *pi, HWND hDlg, UINT id, UINT codeNotify,
            HWND hwndCtl, const TCHAR *text)
{
    if (hwndCtl == NULL)
        return;

    if (lstrcmpi(text, LoadStringDx(IDS_LEFT)) == 0)
    {
        MyKeybdEvent(VK_LEFT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_RIGHT)) == 0)
    {
        MyKeybdEvent(VK_RIGHT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }

    if (text[1] == 0)
    {
        DoTypeOneKey(pi, text[0]);
        if (s_dwFlags & SHIFT)
        {
            s_dwFlags &= ~SHIFT;
            if (s_dwFlags & CAPS)
                s_nKeybdID = IDD_UPPER;
            else
                s_nKeybdID = IDD_LOWER;
            pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        }
        return;
    }

    if (lstrcmpi(text, LoadStringDx(IDS_ENTER)) == 0)
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
        s_dwConv = IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE;
        ImeOnOff(pi, TRUE);
        s_dwFlags &= ~(SHIFT | CAPS | HIRA | KATA);
        s_dwFlags |= HIRA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_KATAKANA)) == 0)
    {
        s_nKeybdID = IDD_KATAKANA;
        s_dwConv = IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA;
        ImeOnOff(pi, TRUE);
        s_dwFlags &= ~(SHIFT | CAPS | HIRA | KATA);
        s_dwFlags |= KATA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_ABC)) == 0)
    {
        s_nKeybdID = IDD_LOWER;
        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        s_dwFlags &= ~(SHIFT | CAPS | HIRA | KATA);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_DIGITS)) == 0)
    {
        s_nKeybdID = IDD_DIGITS;
        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        s_dwFlags &= ~(SHIFT | CAPS | HIRA | KATA);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_CAPS)) == 0)
    {
        if (s_dwFlags & CAPS)
            s_dwFlags &= ~CAPS;
        else
            s_dwFlags |= CAPS;

        if (s_nKeybdID == IDD_LOWER)
            s_nKeybdID = IDD_UPPER;
        else
            s_nKeybdID = IDD_LOWER;

        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_SHIFT)) == 0)
    {
        if (s_dwFlags & SHIFT)
            s_dwFlags &= ~SHIFT;
        else
            s_dwFlags |= SHIFT;

        if (s_nKeybdID == IDD_LOWER)
            s_nKeybdID = IDD_UPPER;
        else
            s_nKeybdID = IDD_LOWER;

        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_CONV)) == 0)
    {
        MyKeybdEvent(VK_CONVERT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_CONVERT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_NOCONV)) == 0)
    {
        MyKeybdEvent(VK_NONCONVERT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_NONCONVERT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_NEXTCAND)) == 0)
    {
        MyKeybdEvent(VK_CONVERT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_CONVERT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_PREVCAND)) == 0)
    {
        MyKeybdEvent(VK_SHIFT, 0, 0, 0);
        MyKeybdEvent(VK_CONVERT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_CONVERT, 0, KEYEVENTF_KEYUP, 0);
        MyKeybdEvent(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_BS)) == 0)
    {
        MyKeybdEvent(VK_BACK, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_DEL)) == 0)
    {
        MyKeybdEvent(VK_DELETE, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
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
        return pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwFlags);
    case ACTION_DESTROY:
        return pi->driver(pi, DRIVER_DESTROY, wParam, lParam);
    case ACTION_COMMAND:
        OnCommand(pi, wParam, lParam);
        break;
    case ACTION_REFRESH:
        {
            HWND hwndShift = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_SHIFT));
            if (s_dwFlags & SHIFT)
            {
                Button_SetCheck(hwndShift, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndShift, BST_UNCHECKED);
            }

            HWND hwndCaps = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_CAPS));
            if (s_dwFlags & CAPS)
            {
                Button_SetCheck(hwndCaps, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndCaps, BST_UNCHECKED);
            }

            HWND hwndHira = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_HIRAGANA));
            if (s_dwFlags & HIRA)
            {
                Button_SetCheck(hwndHira, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndHira, BST_UNCHECKED);
            }

            HWND hwndKata = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_KATAKANA));
            if (s_dwFlags & KATA)
            {
                Button_SetCheck(hwndKata, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndKata, BST_UNCHECKED);
            }
        }
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
