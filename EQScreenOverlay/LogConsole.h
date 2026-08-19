#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>

void OpenConsole();
void CloseConsole();
void ConsolePrintf(const char* fmt, ...);

class LogConsole {
public:
    LogConsole();
    ~LogConsole();

    bool Open(int charsWidth = 120, int charsHeight = 30, int fontSize = 16);
    void Close();
    void Printf(const char* fmt, ...);
    void VPrintf(const char* fmt, va_list args);
    void Print(const char* text);

    bool IsOpen() const { return m_hWnd != NULL; }

    void CopySelectionToClipboard();
    std::string GetSelectedText() const;

private:
    enum { WM_LOG_PRINT = WM_USER + 100, WM_LOG_CLOSE = WM_USER + 101 };

    struct PrintData {
        char text[1];
    };

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI ThreadProc(LPVOID lpParam);

    HWND    m_hWnd;
    HANDLE  m_hThread;
    int     m_charsWidth;
    int     m_charsHeight;
    int     m_fontSize;
    HFONT   m_hFont;
    int     m_charWidth;
    int     m_charHeight;

    std::vector<std::string> m_lines;
    std::string m_currentLine;
    int     m_cursorCol;
    size_t  m_maxLines;
    int     m_scrollOffset;
    int     m_selAnchorLine, m_selAnchorCol;
    int     m_selLine, m_selCol;
    bool    m_selecting;

    void OnPaint(HDC hdc);
    void OnSize(int cx, int cy);
    void OnVScroll(int code, int pos);
    void OnMouseWheel(int delta);
    void OnLButtonDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnLButtonUp();
    void OnPrint(const char* text);

    void AddText(const char* text);
    void ScrollTo(int offset);
    void EnsureVisible();
    void UpdateScrollBar();
    void CharFromPoint(int x, int y, int& line, int& col) const;
    void GetSelRange(int& startLine, int& startCol, int& endLine, int& endCol) const;
    void RecalcCharDimensions(HDC hdc);

    static bool s_classRegistered;
    static const TCHAR s_windowClass[];
};
