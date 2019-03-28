// PluginFramework.hpp --- PluginFramework
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.
#ifndef PLUGIN_FRAMEWORK_HPP_
#define PLUGIN_FRAMEWORK_HPP_

#include "KeybdPlugin.hpp"
#include <vector>

enum ACTION
{
    ACTION_NONE = 0,
    ACTION_RECREATE,
    ACTION_DESTROY,
    ACTION_COMMAND,
    ACTION_REFRESH,
    ACTION_TIMER,
    ACTION_CUSTOMDRAW
};

enum DRIVER_FUNCTION
{
    DRIVER_NONE = 0,
    DRIVER_RECREATE,
    DRIVER_DESTROY
};

BOOL PF_LoadOne(PLUGIN *pi, const TCHAR *pathname);
INT PF_LoadAll(std::vector<PLUGIN>& pis, const TCHAR *dir);
BOOL PF_IsLoaded(const PLUGIN *pi);
LRESULT PF_ActOne(PLUGIN *pi, UINT uAction, WPARAM wParam, LPARAM lParam);
LRESULT PF_ActAll(std::vector<PLUGIN>& pis, UINT uAction, WPARAM wParam, LPARAM lParam);
BOOL PF_UnloadOne(PLUGIN *pi);
BOOL PF_UnloadAll(std::vector<PLUGIN>& pis);

LRESULT APIENTRY PF_Driver(struct PLUGIN *pi, UINT uFunc, WPARAM wParam, LPARAM lParam);

#endif  // ndef PLUGIN_FRAMEWORK_HPP_
