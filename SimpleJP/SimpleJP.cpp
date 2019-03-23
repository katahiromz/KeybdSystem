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
static UINT s_nKeybdID = IDD_LOWER;
static DWORD s_dwConv = 0;

#define SHIFT 1
#define CAPS 2
#define HIRA 4
#define KATA 8
#define SMALL 16
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

static void DoTypeDakuten(PLUGIN *pi, TCHAR ch)
{
    typedef std::unordered_map<WCHAR, WCHAR> map_type;
    static const map_type map =
    {
        // hiragana
        {0x304B, 0x304C},
        {0x304D, 0x304E},
        {0x304F, 0x3050},
        {0x3051, 0x3052},
        {0x3053, 0x3054},
        {0x3055, 0x3056},
        {0x3057, 0x3058},
        {0x3059, 0x305A},
        {0x305B, 0x305C},
        {0x305D, 0x305E},
        {0x305F, 0x3060},
        {0x3061, 0x3062},
        {0x3064, 0x3065},
        {0x3066, 0x3067},
        {0x3068, 0x3069},
        {0x306F, 0x3070},
        {0x3072, 0x3073},
        {0x3075, 0x3076},
        {0x3078, 0x3079},
        {0x307B, 0x307C},
        {0x3046, 0x3094},
        // katakana
        {0x30AB, 0x30AC},
        {0x30AD, 0x30AE},
        {0x30AF, 0x30B0},
        {0x30B1, 0x30B2},
        {0x30B3, 0x30B4},
        {0x30B5, 0x30B6},
        {0x30B7, 0x30B8},
        {0x30B9, 0x30BA},
        {0x30BB, 0x30BC},
        {0x30BD, 0x30BE},
        {0x30BF, 0x30C0},
        {0x30C1, 0x30C2},
        {0x30C4, 0x30C5},
        {0x30C6, 0x30C7},
        {0x30C8, 0x30C9},
        {0x30CF, 0x30D0},
        {0x30D2, 0x30D3},
        {0x30D5, 0x30D6},
        {0x30D6, 0x30D9},
        {0x30DB, 0x30DC},
        {0x30A6, 0x30F4},
   };

    map_type::const_iterator it = map.find(ch);
    if (it != map.end())
    {
        DoTypeBackSpace(pi);
        DoTypeOneKey(pi, it->second);
    }
    else
    {
        DoTypeOneKey(pi, ch);
    }
}

static void DoTypeHanDakuten(PLUGIN *pi, TCHAR ch)
{
    typedef std::unordered_map<WCHAR, WCHAR> map_type;
    static const map_type map =
    {
        // hiragana
        {0x306F, 0x3071},
        {0x3072, 0x3074},
        {0x3075, 0x3077},
        {0x3078, 0x307A},
        {0x307B, 0x307D},
        // katakana
        {0x30CF, 0x30D1},
        {0x30D2, 0x30D2},
        {0x30D5, 0x30D7},
        {0x30D8, 0x30DA},
        {0x30DB, 0x30DD},
   };

    map_type::const_iterator it = map.find(ch);
    if (it != map.end())
    {
        DoTypeBackSpace(pi);
        DoTypeOneKey(pi, it->second);
    }
    else
    {
        DoTypeOneKey(pi, ch);
    }
}

static void
OnCommandEx(PLUGIN *pi, HWND hDlg, UINT id, UINT codeNotify,
            HWND hwndCtl, const TCHAR *text)
{
    static WCHAR s_chOld = 0;
    if (hwndCtl == NULL)
        return;

    if (lstrcmpi(text, LoadStringDx(IDS_LEFT)) == 0)
    {
        MyKeybdEvent(VK_LEFT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
        s_chOld = 0;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_RIGHT)) == 0)
    {
        MyKeybdEvent(VK_RIGHT, 0, 0, 0);
        MySleep();
        MyKeybdEvent(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        s_chOld = 0;
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_SMALL)) == 0)
    {
        switch (s_nKeybdID)
        {
        case IDD_HIRAGANA:
            s_nKeybdID = IDD_HIRAGANA_SMALL;
            s_dwStatus |= SMALL;
            break;
        case IDD_KATAKANA:
            s_nKeybdID = IDD_KATAKANA_SMALL;
            s_dwStatus |= SMALL;
            break;
        case IDD_HIRAGANA_SMALL:
            s_nKeybdID = IDD_HIRAGANA;
            s_dwStatus &= ~SMALL;
            break;
        case IDD_KATAKANA_SMALL:
            s_nKeybdID = IDD_KATAKANA;
            s_dwStatus &= ~SMALL;
            break;
        }
        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        s_chOld = 0;
        return;
    }

    if (text[1] == 0)
    {
        if (s_chOld && text[0] == 0x309B)
        {
            DoTypeDakuten(pi, text[0]);
        }
        else if (s_chOld && text[0] == 0x309C)
        {
            DoTypeHanDakuten(pi, text[0]);
        }
        else
        {
            DoTypeOneKey(pi, text[0]);
        }
        if (s_dwStatus & SHIFT)
        {
            s_dwStatus &= ~SHIFT;
            if (s_dwStatus & CAPS)
                s_nKeybdID = IDD_UPPER;
            else
                s_nKeybdID = IDD_LOWER;
            pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        }
        else if (s_dwStatus & SMALL)
        {
            switch (s_nKeybdID)
            {
            case IDD_HIRAGANA_SMALL:
                s_nKeybdID = IDD_HIRAGANA;
                s_dwStatus &= ~SMALL;
                pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
                break;
            case IDD_KATAKANA_SMALL:
                s_nKeybdID = IDD_KATAKANA;
                s_dwStatus &= ~SMALL;
                pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
                break;
            }
        }
        s_chOld = text[0];
        return;
    }

    s_chOld = 0;

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
        s_dwStatus &= ~(SHIFT | CAPS | HIRA | KATA | SMALL);
        s_dwStatus |= HIRA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_KATAKANA)) == 0)
    {
        s_nKeybdID = IDD_KATAKANA;
        s_dwConv = IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA;
        ImeOnOff(pi, TRUE);
        s_dwStatus &= ~(SHIFT | CAPS | HIRA | KATA | SMALL);
        s_dwStatus |= KATA;
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_ABC)) == 0)
    {
        s_nKeybdID = IDD_LOWER;
        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        s_dwStatus &= ~(SHIFT | CAPS | HIRA | KATA | SMALL);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_DIGITS)) == 0)
    {
        s_nKeybdID = IDD_DIGITS;
        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        s_dwStatus &= ~(SHIFT | CAPS | HIRA | KATA | SMALL);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_CAPS)) == 0)
    {
        if (s_dwStatus & CAPS)
            s_dwStatus &= ~CAPS;
        else
            s_dwStatus |= CAPS;

        if (s_nKeybdID == IDD_LOWER)
            s_nKeybdID = IDD_UPPER;
        else
            s_nKeybdID = IDD_LOWER;

        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
        return;
    }
    if (lstrcmpi(text, LoadStringDx(IDS_SHIFT)) == 0)
    {
        if (s_dwStatus & SHIFT)
            s_dwStatus &= ~SHIFT;
        else
            s_dwStatus |= SHIFT;

        if (s_nKeybdID == IDD_LOWER)
            s_nKeybdID = IDD_UPPER;
        else
            s_nKeybdID = IDD_LOWER;

        s_dwConv = 0;
        ImeOnOff(pi, FALSE);
        pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
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
        return pi->driver(pi, DRIVER_RECREATE, s_nKeybdID, s_dwStatus);
    case ACTION_DESTROY:
        return pi->driver(pi, DRIVER_DESTROY, wParam, lParam);
    case ACTION_COMMAND:
        OnCommand(pi, wParam, lParam);
        break;
    case ACTION_REFRESH:
        {
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
                Button_SetCheck(hwndCaps, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndCaps, BST_UNCHECKED);
            }

            HWND hwndHira = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_HIRAGANA));
            if (s_dwStatus & HIRA)
            {
                Button_SetCheck(hwndHira, BST_CHECKED);
            }
            else
            {
                Button_SetCheck(hwndHira, BST_UNCHECKED);
            }

            HWND hwndKata = FindWindowEx(pi->plugin_window, NULL, TEXT("BUTTON"), LoadStringDx(IDS_KATAKANA));
            if (s_dwStatus & KATA)
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
