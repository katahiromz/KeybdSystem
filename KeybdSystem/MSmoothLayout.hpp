// MSmoothLayout.hpp --- Win32 smooth layout
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.

#ifndef MSMOOTH_LAYOUT_HPP_
#define MSMOOTH_LAYOUT_HPP_     2   // Version 2

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#if __cplusplus >= 201103L
    #include <unordered_map>
#else
    #include <map>
#endif

//////////////////////////////////////////////////////////////////////////////

class MSmoothLayout
{
public:
    MSmoothLayout();
    MSmoothLayout(HWND hwndParent);

    void init(HWND hwndParent);

    void OnSize(LPCRECT prcTo = NULL);
    void OnSize(INT x, INT y, INT cx, INT cy);
    void OnSize(INT cx, INT cy);

protected:
    HWND m_hwndParent;
#if __cplusplus >= 201103L
    typedef std::unordered_map<HWND, RECT> map_type;
#else
    typedef std::map<HWND, RECT> map_type;
#endif
    RECT m_rcFrom;
    map_type m_hwnd2rc;

    void map_rect(RECT& rc, const RECT &rcFrom, const RECT &rcTo);
};

//////////////////////////////////////////////////////////////////////////////

inline MSmoothLayout::MSmoothLayout() : m_hwndParent(NULL)
{
}

inline MSmoothLayout::MSmoothLayout(HWND hwndParent)
{
    init(hwndParent);
}

inline void MSmoothLayout::init(HWND hwndParent)
{
    m_hwndParent = hwndParent;
    GetClientRect(hwndParent, &m_rcFrom);

    m_hwnd2rc.clear();

    for (HWND hChild = ::GetTopWindow(hwndParent);
         hChild != NULL;
         hChild = GetWindow(hChild, GW_HWNDNEXT))
    {
        if (GetParent(hChild) != hwndParent)
            continue;

        RECT rc;
        GetWindowRect(hChild, &rc);

        MapWindowPoints(NULL, hwndParent, (LPPOINT)&rc, 2);
        m_hwnd2rc[hChild] = rc;
    }
}

inline void MSmoothLayout::OnSize(INT x, INT y, INT cx, INT cy)
{
    RECT rc;
    rc.left = x;
    rc.top = y;
    rc.right = x + cx;
    rc.bottom = y + cy;
    OnSize(&rc);
}

inline void MSmoothLayout::OnSize(INT cx, INT cy)
{
    OnSize(0, 0, cx, cy);
}

inline void MSmoothLayout::map_rect(RECT& rcItem, const RECT &rcFrom, const RECT &rcTo)
{
    SIZE sizFrom;
    sizFrom.cx = rcFrom.right - rcFrom.left;
    sizFrom.cy = rcFrom.bottom - rcFrom.top;
    if (sizFrom.cx == 0 || sizFrom.cy == 0)
        return;

    SIZE sizTo;
    sizTo.cx = rcTo.right - rcTo.left;
    sizTo.cy = rcTo.bottom - rcTo.top;

    SIZE sizItem;
    sizItem.cx = rcItem.right - rcItem.left;
    sizItem.cy = rcItem.bottom - rcItem.top;

    OffsetRect(&rcItem, -rcFrom.left, -rcFrom.top);

    rcItem.left = rcItem.left * sizTo.cx / sizFrom.cx;
    rcItem.top = rcItem.top * sizTo.cy / sizFrom.cy;
    rcItem.right = rcItem.right * sizTo.cx / sizFrom.cx;
    rcItem.bottom = rcItem.bottom * sizTo.cy / sizFrom.cy;

    OffsetRect(&rcItem, rcTo.left, rcTo.top);
}

inline void MSmoothLayout::OnSize(LPCRECT prcTo)
{
    if (m_hwndParent == NULL)
        return;

    RECT rcTo;
    if (prcTo == NULL)
    {
        GetClientRect(m_hwndParent, &rcTo);
        prcTo = &rcTo;
    }

    HDWP hDWP = BeginDeferWindowPos((INT)m_hwnd2rc.size());
    if (!hDWP)
        return;

    for (map_type::const_iterator it = m_hwnd2rc.begin();
         it != m_hwnd2rc.end();
         ++it)
    {
        HWND hwnd = it->first;
        RECT rcItem = it->second;
        map_rect(rcItem, m_rcFrom, *prcTo);
        hDWP = DeferWindowPos(hDWP, hwnd, NULL,
            rcItem.left, rcItem.top,
            rcItem.right - rcItem.left, rcItem.bottom - rcItem.top,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }

    EndDeferWindowPos(hDWP);
}

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MSMOOTH_LAYOUT_HPP_
