// KeybdPlugin.h --- Keyboard System Plugin interface
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#ifndef KEYBD_PLUGIN_HPP_
#define KEYBD_PLUGIN_HPP_
// TODO: Rename this file

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

// TODO: Change me!
#ifndef FRAMEWORK_NAME
    #define FRAMEWORK_NAME TEXT("KeybdSystem")
#endif

// TODO: Change me!
#ifndef FRAMEWORK_SPEC
    #define FRAMEWORK_SPEC TEXT("*.keybd")
#endif

// TODO: Change me!
#ifndef FRAMEWORK_VERSION
    #define FRAMEWORK_VERSION 0
#endif

struct PLUGIN;
struct PLUGIN_FRAMEWORK_IMPL;

typedef BOOL (APIENTRY *PLUGIN_LOAD)(struct PLUGIN *pi, LPARAM lParam);
typedef BOOL (APIENTRY *PLUGIN_UNLOAD)(struct PLUGIN *pi, LPARAM lParam);
typedef LRESULT (APIENTRY *PLUGIN_ACT)(struct PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);

typedef LRESULT (APIENTRY *PLUGIN_DRIVER)(struct PLUGIN *pi, UINT uFunc, WPARAM wParam, LPARAM lParam);

enum DRIVER_FUNCTION
{
    DRIVER_NONE = 0,
    DRIVER_RECREATE,
    DRIVER_DESTROY,
    DRIVER_FINDKEY
};

// NOTE: This structure must be a POD (Plain Old Data).
typedef struct PLUGIN
{
    // Don't change:
    DWORD framework_version;
    TCHAR framework_name[32];
    HINSTANCE framework_instance;
    HWND framework_window;
    TCHAR plugin_pathname[MAX_PATH];
    struct PLUGIN_FRAMEWORK_IMPL *framework_impl;
    PLUGIN_DRIVER driver;

    // Please fill them in Plugin_Load:
    DWORD plugin_version;
    TCHAR plugin_product_name[64];
    TCHAR plugin_filename[32];
    TCHAR plugin_company[64];
    TCHAR plugin_copyright[128];
    HINSTANCE plugin_instance;

    // Use freely:
    HWND plugin_window;
    void *p_user_data;
    LPARAM l_user_data;

    // TODO: Add more members and version up...
} PLUGIN;

#ifdef __cplusplus
extern "C" {
#endif

// API Name: Plugin_Load
// Purpose: The framework want to load the plugin component.
// TODO: Load the plugin component.
BOOL APIENTRY Plugin_Load(PLUGIN *pi, LPARAM lParam);

// API Name: Plugin_Unload
// Purpose: The framework want to unload the plugin component.
// TODO: Unload the plugin component.
BOOL APIENTRY Plugin_Unload(PLUGIN *pi, LPARAM lParam);

// API Name: Plugin_Act
// Purpose: Act something on the plugin.
// TODO: Act something on the plugin.
LRESULT APIENTRY Plugin_Act(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);

// TODO: Add more APIs

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // ndef KEYBD_PLUGIN_HPP_
