// SimpleJP.cpp --- KeybdPlugin SimpleJP keyboard
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
static UINT s_nKeybdID = IDD_NUMPAD;
static DWORD s_dwConv = 0;

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

#ifndef IMC_GETOPENSTATUS
    #define IMC_GETOPENSTATUS 0x0005
#endif
#ifndef IMC_SETCONVERSIONMODE
    #define IMC_SETCONVERSIONMODE 0x0002
#endif

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
    StringCbCopy(pi->plugin_product_name, sizeof(pi->plugin_product_name), TEXT("SimpleNumPad"));
    StringCbCopy(pi->plugin_filename, sizeof(pi->plugin_filename), TEXT("SimpleNumPad.keybd"));
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

static void DoTypeBackSpace(PLUGIN *pi)
{
    MyKeybdEvent(VK_BACK, 0, 0, 0);
    MySleep();
    MyKeybdEvent(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
    MySleep();
}

static void DoTypeOneKey(PLUGIN *pi, TCHAR ch)
{
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

    if (lstrcmpi(text, LoadStringDx(IDS_UP)) == 0)
    {
        MyKeybdEvent(VK_UP, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_UP, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_DOWN)) == 0)
    {
        MyKeybdEvent(VK_DOWN, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
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
        return;
    }

    if (lstrcmpi(text, LoadStringDx(IDS_ENTER)) == 0)
    {
        MyKeybdEvent(VK_RETURN, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_BS)) == 0)
    {
        MyKeybdEvent(VK_BACK, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_ESC)) == 0)
    {
        MyKeybdEvent(VK_ESCAPE, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_PAUSE)) == 0)
    {
        MyKeybdEvent(VK_PAUSE, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_PAUSE, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_HOME)) == 0)
    {
        MyKeybdEvent(VK_HOME, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_END)) == 0)
    {
        MyKeybdEvent(VK_END, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_END, 0, KEYEVENTF_KEYUP, 0);
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

void OnRefresh(PLUGIN *pi)
{
    if (GetKeyState(VK_CAPITAL) & 1)
    {
        // Unlock CapsLock
        keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
        keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
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
    if (s_dwStatus & CAPS)
    {
        if (hwndCaps)
            Button_SetCheck(hwndCaps, BST_CHECKED);
    }
    else
    {
        if (hwndCaps)
            Button_SetCheck(hwndCaps, BST_UNCHECKED);
    }

    HWND hwndHira = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_HIRAGANA));
    if (s_dwStatus & HIRA)
    {
        if (hwndHira)
            Button_SetCheck(hwndHira, BST_CHECKED);
    }
    else
    {
        if (hwndHira)
            Button_SetCheck(hwndHira, BST_UNCHECKED);
    }

    HWND hwndKata = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_KATAKANA));
    if (s_dwStatus & KATA)
    {
        if (hwndKata)
            Button_SetCheck(hwndKata, BST_CHECKED);
    }
    else
    {
        if (hwndKata)
            Button_SetCheck(hwndKata, BST_UNCHECKED);
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
        s_nKeybdID = IDD_NUMPAD;
        return pi->driver(pi, DRIVER_DESTROY, wParam, lParam);
    case ACTION_COMMAND:
        OnCommand(pi, wParam, lParam);
        break;
    case ACTION_REFRESH:
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
