// -------------------------------------------------------------------------------------------------
// Includes
// -------------------------------------------------------------------------------------------------

#include "pch.h"
#include "LogConsole.h"

#include <windowsx.h>
#include <commctrl.h>

#include "config.h"

// -------------------------------------------------------------------------------------------------
// Link
// -------------------------------------------------------------------------------------------------

#pragma comment(lib, "comctl32.lib")

// -------------------------------------------------------------------------------------------------
// Globals
// -------------------------------------------------------------------------------------------------

bool LogConsole::s_classRegistered = false;
const TCHAR LogConsole::s_windowClass[] = TEXT("EQLogConsole");

// -------------------------------------------------------------------------------------------------
// Construction
// -------------------------------------------------------------------------------------------------

#ifdef CREATE_CONSOLE

static LogConsole g_console;

void OpenConsole()
{
    g_console.Open();
}

void CloseConsole()
{
    g_console.Close();
}

void ConsolePrintf(const char* fmt, ...)
{
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_console.Print(buf);
    OutputDebugStringA(buf);
}

#else

void OpenConsole() {}
void CloseConsole() {}
void ConsolePrintf(const char* fmt, ...) {}

#endif

LogConsole::LogConsole()
    : m_hWnd(NULL)
    , m_hThread(NULL)
    , m_charsWidth(160)
    , m_charsHeight(80)
    , m_fontSize(16)
    , m_hFont(NULL)
    , m_charWidth(0)
    , m_charHeight(0)
    , m_cursorCol(0)
    , m_maxLines(5000)
    , m_scrollOffset(0)
    , m_selAnchorLine(-1), m_selAnchorCol(0)
    , m_selLine(-1), m_selCol(0)
    , m_selecting(false)
{}

LogConsole::~LogConsole()
{
    Close();
}

// -------------------------------------------------------------------------------------------------
// Open / Close
// -------------------------------------------------------------------------------------------------

bool LogConsole::Open(int charsWidth, int charsHeight, int fontSize)
{
    if (m_hWnd)
        return true;

    m_charsWidth = charsWidth;
    m_charsHeight = charsHeight;
    m_fontSize = fontSize;

    m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
    if (!m_hThread)
        return false;

    for (int i = 0; i < 500; i++)
    {
        if (m_hWnd)
            return true;
        Sleep(10);
    }

    Close();
    return false;
}

void LogConsole::Close()
{
    if (m_hWnd)
        PostMessage(m_hWnd, WM_LOG_CLOSE, 0, 0);

    if (m_hThread)
    {
        WaitForSingleObject(m_hThread, 5000);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    m_hWnd = NULL;
}

// -------------------------------------------------------------------------------------------------
// Output
// -------------------------------------------------------------------------------------------------

void LogConsole::Print(const char* text)
{
    if (!m_hWnd)
        return;

    size_t len = strlen(text) + 1;
    PrintData* pd = (PrintData*)malloc(sizeof(PrintData) + len);
    memcpy(pd->text, text, len);
    PostMessage(m_hWnd, WM_LOG_PRINT, 0, (LPARAM)pd);
}

void LogConsole::Printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    VPrintf(fmt, args);
    va_end(args);
}

void LogConsole::VPrintf(const char* fmt, va_list args)
{
    if (!m_hWnd)
        return;

    char buffer[4096];
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);

    size_t len = strlen(buffer) + 1;
    PrintData* pd = (PrintData*)malloc(sizeof(PrintData) + len);
    memcpy(pd->text, buffer, len);
    PostMessage(m_hWnd, WM_LOG_PRINT, 0, (LPARAM)pd);
}

// -------------------------------------------------------------------------------------------------
// Window thread
// -------------------------------------------------------------------------------------------------

DWORD WINAPI LogConsole::ThreadProc(LPVOID lpParam)
{
    LogConsole* self = (LogConsole*)lpParam;

    if (!s_classRegistered)
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = s_windowClass;
        RegisterClassEx(&wc);
        s_classRegistered = true;
    }

    HDC hdc = GetDC(NULL);

    HFONT hFont = CreateFont(
        -16,
        0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN,
        TEXT("Consolas"));

    HGDIOBJ oldFont = SelectObject(hdc, hFont);

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    self->m_charWidth = tm.tmAveCharWidth;
    self->m_charHeight = tm.tmHeight;

    SelectObject(hdc, oldFont);
    ReleaseDC(NULL, hdc);

    int clientW = self->m_charWidth * self->m_charsWidth;
    int clientH = self->m_charHeight * self->m_charsHeight;

    RECT rect = { 0, 0, clientW, clientH };
    AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, FALSE, 0);

    self->m_hWnd = CreateWindowEx(
        0,
        s_windowClass,
        TEXT("EQScreenOverlay Console"),
        WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL,
        GetModuleHandle(NULL),
        self);

    if (!self->m_hWnd)
    {
        DeleteObject(hFont);
        return 1;
    }

    self->m_hFont = hFont;

    ShowWindow(self->m_hWnd, SW_SHOWNA);
    self->UpdateScrollBar();
    UpdateWindow(self->m_hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (self->m_hWnd)
    {
        DestroyWindow(self->m_hWnd);
        self->m_hWnd = NULL;
    }

    if (self->m_hFont)
    {
        DeleteObject(self->m_hFont);
        self->m_hFont = NULL;
    }

    if (s_classRegistered)
    {
        UnregisterClass(s_windowClass, GetModuleHandle(NULL));
        s_classRegistered = false;
    }

    return 0;
}

// -------------------------------------------------------------------------------------------------
// Window procedure
// -------------------------------------------------------------------------------------------------

LRESULT CALLBACK LogConsole::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LogConsole* self = (LogConsole*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return 0;
    }

    case WM_LOG_PRINT:
    {
        PrintData* pd = (PrintData*)lParam;
        if (self)
            self->OnPrint(pd->text);
        free(pd);
        return 0;
    }

    case WM_PAINT:
    {
        if (!self) break;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        self->OnPaint(hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SIZE:
        if (self) self->OnSize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_VSCROLL:
        if (self) self->OnVScroll(LOWORD(wParam), HIWORD(wParam));
        return 0;

    case WM_MOUSEWHEEL:
        if (self) self->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;

    case WM_LBUTTONDOWN:
        if (self) self->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        SetCapture(hWnd);
        return 0;

    case WM_MOUSEMOVE:
        if (self && (wParam & MK_LBUTTON))
            self->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        if (self) self->OnLButtonUp();
        ReleaseCapture();
        return 0;

    case WM_LOG_CLOSE:
        if (self)
        {
            DestroyWindow(hWnd);
            self->m_hWnd = NULL;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        return 0;

    case WM_MOUSEACTIVATE:
        return MA_ACTIVATE;

    case WM_KEYDOWN:
        if (self)
        {
            if ((GetKeyState(VK_CONTROL) & 0x8000))
            {
                switch (wParam)
                {
                case 'C':
                    self->CopySelectionToClipboard();
                    return 0;

                case 'A':
                {
                    if (!self->m_lines.empty())
                    {
                        self->m_selAnchorLine = 0;
                        self->m_selAnchorCol = 0;

                        self->m_selLine = (int)self->m_lines.size() - 1;
                        self->m_selCol =
                            (int)self->m_lines.back().size();

                        InvalidateRect(hWnd, NULL, TRUE);
                    }
                    return 0;
                }
                }
            }
        }
        break;

    case WM_RBUTTONUP:
    {
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, 1, TEXT("Copy"));

        POINT pt;
        GetCursorPos(&pt);

        int cmd = TrackPopupMenu(
            menu,
            TPM_RETURNCMD | TPM_NONOTIFY,
            pt.x,
            pt.y,
            0,
            hWnd,
            NULL);

        DestroyMenu(menu);

        if (cmd == 1 && self)
        {
            self->CopySelectionToClipboard();
        }

        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------------------------------------
// OnPaint
// -------------------------------------------------------------------------------------------------

void LogConsole::OnPaint(HDC hdc)
{
    RECT client;
    GetClientRect(m_hWnd, &client);

    SelectObject(hdc, m_hFont);

    int visRows = client.bottom / m_charHeight;
    int visCols = client.right / m_charWidth;
    if (visRows < 1) visRows = 1;
    if (visCols < 1) visCols = 1;

    int totalLines = (int)m_lines.size();

    // Sanitize scroll offset
    int maxOffset = totalLines - visRows;
    if (maxOffset < 0) maxOffset = 0;
    if (m_scrollOffset > maxOffset)
    {
        m_scrollOffset = maxOffset;
        SCROLLINFO si = { sizeof(si), SIF_POS, 0, 0, 0, m_scrollOffset, 0 };
        SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
    }
    if (m_scrollOffset < 0)
        m_scrollOffset = 0;

    // Determine selection range
    bool hasSel = (m_selAnchorLine >= 0) &&
        (m_selAnchorLine != m_selLine || m_selAnchorCol != m_selCol);
    int ss = 0, sc = 0, es = 0, ec = 0;
    if (hasSel)
        GetSelRange(ss, sc, es, ec);

    for (int row = 0; row < visRows; row++)
    {
        int lineIdx = m_scrollOffset + row;
        int y = row * m_charHeight;

        if (lineIdx >= totalLines)
        {
            RECT r = { 0, y, client.right, y + m_charHeight };
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(0, 0, 0));
            ExtTextOut(hdc, 0, y, ETO_OPAQUE, &r, TEXT(""), 0, NULL);
            continue;
        }

        const std::string& line = m_lines[lineIdx];
        int lineLen = (int)line.size();
        int drawLen = (lineLen < visCols) ? lineLen : visCols;

        // Text and background colors
        COLORREF txtColor = RGB(255, 255, 255);
        COLORREF bgColor = RGB(0, 0, 0);
        COLORREF selTxt = RGB(0, 0, 0);
        COLORREF selBg = RGB(192, 192, 192);

        // Draw in up to three segments: before selection, selection, after selection
        int segments[3][2] = {};
        int segCount = 0;

        if (!hasSel || lineIdx < ss || lineIdx > es)
        {
            segments[0][0] = 0;
            segments[0][1] = drawLen;
            segCount = 1;
        }
        else if (lineIdx == ss && lineIdx == es)
        {
            // Single line, partial selection
            if (sc > 0)
            {
                segments[0][0] = 0; segments[0][1] = sc;
                segCount++;
            }
            segments[segCount][0] = sc; segments[segCount][1] = ec;
            segCount++;
            if (ec < drawLen)
            {
                segments[segCount][0] = ec; segments[segCount][1] = drawLen;
                segCount++;
            }
        }
        else if (lineIdx == ss)
        {
            if (sc > 0)
            {
                segments[0][0] = 0; segments[0][1] = sc;
                segCount++;
            }
            segments[segCount][0] = sc; segments[segCount][1] = drawLen;
            segCount++;
        }
        else if (lineIdx == es)
        {
            segments[segCount][0] = 0; segments[segCount][1] = ec;
            segCount++;
            if (ec < drawLen)
            {
                segments[segCount][0] = ec; segments[segCount][1] = drawLen;
                segCount++;
            }
        }
        else
        {
            // Entire line selected
            segments[0][0] = 0; segments[0][1] = drawLen;
            segCount = 1;
        }

        for (int s = 0; s < segCount; s++)
        {
            int segStart = segments[s][0];
            int segEnd = segments[s][1];
            int segLen = segEnd - segStart;
            if (segLen <= 0) continue;

            bool selected = hasSel &&
                ((lineIdx > ss && lineIdx < es) ||
                    (lineIdx == ss && lineIdx == es && segStart >= sc && segEnd <= ec) ||
                    (lineIdx == ss && lineIdx < es && segStart >= sc) ||
                    (lineIdx == es && lineIdx > ss && segEnd <= ec));

            RECT r = {
                segStart * m_charWidth,
                y,
                segEnd * m_charWidth,
                y + m_charHeight
            };

            SetTextColor(hdc, selected ? selTxt : txtColor);
            SetBkColor(hdc, selected ? selBg : bgColor);

            ExtTextOutA(hdc, r.left, y, ETO_OPAQUE | ETO_CLIPPED, &r,
                line.c_str() + segStart, segLen, NULL);
        }

        // Clear unused portion of row
        if (drawLen < visCols)
        {
            RECT r = { drawLen * m_charWidth, y, visCols * m_charWidth, y + m_charHeight };
            SetTextColor(hdc, txtColor);
            SetBkColor(hdc, bgColor);
            ExtTextOut(hdc, r.left, y, ETO_OPAQUE, &r, TEXT(""), 0, NULL);
        }
    }
}

// -------------------------------------------------------------------------------------------------
// Event handlers
// -------------------------------------------------------------------------------------------------

void LogConsole::OnSize(int cx, int cy)
{
    UpdateScrollBar();
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void LogConsole::OnMouseWheel(int delta)
{
    RECT client;
    GetClientRect(m_hWnd, &client);
    int visRows = client.bottom / m_charHeight;
    if (visRows < 1) visRows = 1;

    int totalLines = (int)m_lines.size();
    int maxOffset = totalLines - visRows;
    if (maxOffset < 0) maxOffset = 0;

    int step = (delta > 0) ? -3 : 3;
    m_scrollOffset += step;

    if (m_scrollOffset > maxOffset)
        m_scrollOffset = maxOffset;
    if (m_scrollOffset < 0)
        m_scrollOffset = 0;

    UpdateScrollBar();
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void LogConsole::OnLButtonDown(int x, int y)
{
    CharFromPoint(x, y, m_selAnchorLine, m_selAnchorCol);
    m_selLine = m_selAnchorLine;
    m_selCol = m_selAnchorCol;
    m_selecting = true;
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void LogConsole::OnMouseMove(int x, int y)
{
    if (!m_selecting)
        return;

    RECT client;
    GetClientRect(m_hWnd, &client);
    if (x < client.left) x = client.left;
    if (x >= client.right) x = client.right - 1;
    if (y < client.top) y = client.top;
    if (y >= client.bottom) y = client.bottom - 1;

    CharFromPoint(x, y, m_selLine, m_selCol);
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void LogConsole::OnLButtonUp()
{
    m_selecting = false;
}

void LogConsole::OnVScroll(int code, int pos)
{
    RECT client;
    GetClientRect(m_hWnd, &client);
    int visRows = client.bottom / m_charHeight;
    if (visRows < 1) visRows = 1;

    int totalLines = (int)m_lines.size();
    int maxOffset = totalLines - visRows;
    if (maxOffset < 0) maxOffset = 0;

    switch (code)
    {
    case SB_LINEUP:      m_scrollOffset -= 1;    break;
    case SB_LINEDOWN:    m_scrollOffset += 1;    break;
    case SB_PAGEUP:      m_scrollOffset -= visRows; break;
    case SB_PAGEDOWN:    m_scrollOffset += visRows; break;
    case SB_THUMBTRACK:  m_scrollOffset = pos;   break;
    case SB_TOP:         m_scrollOffset = 0;     break;
    case SB_BOTTOM:      m_scrollOffset = maxOffset; break;
    }

    if (m_scrollOffset > maxOffset) m_scrollOffset = maxOffset;
    if (m_scrollOffset < 0) m_scrollOffset = 0;

    UpdateScrollBar();
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void LogConsole::OnPrint(const char* text)
{
    AddText(text);
    InvalidateRect(m_hWnd, NULL, TRUE);
    UpdateWindow(m_hWnd);
}

// -------------------------------------------------------------------------------------------------
// Text buffer management
// -------------------------------------------------------------------------------------------------

void LogConsole::AddText(const char* text)
{
    int wrapAt = m_charsWidth;
    if (wrapAt < 1) wrapAt = 80;

    int totalBefore = (int)m_lines.size();
    bool wasAtBottom = true;

    {
        RECT client;
        GetClientRect(m_hWnd, &client);
        int visRows = client.bottom / m_charHeight;
        if (visRows < 1) visRows = 1;

        if (totalBefore > visRows)
            wasAtBottom = (m_scrollOffset >= totalBefore - visRows);
    }

    for (const char* p = text; *p; p++)
    {
        char c = *p;

        if (c == '\n')
        {
            m_lines.push_back(m_currentLine);
            m_currentLine.clear();
            m_cursorCol = 0;
        }
        else if (c == '\r')
        {
            m_cursorCol = 0;
        }
        else
        {
            if (m_cursorCol >= wrapAt)
            {
                m_lines.push_back(m_currentLine);
                m_currentLine.clear();
                m_cursorCol = 0;
            }
            m_currentLine += c;
            m_cursorCol++;
        }
    }

    while (m_lines.size() > m_maxLines)
        m_lines.erase(m_lines.begin());

    UpdateScrollBar();

    int totalAfter = (int)m_lines.size();

    if (wasAtBottom)
    {
        RECT client;
        GetClientRect(m_hWnd, &client);
        int visRows = client.bottom / m_charHeight;
        if (visRows < 1) visRows = 1;

        m_scrollOffset = totalAfter - visRows;
        if (m_scrollOffset < 0) m_scrollOffset = 0;
    }
}

// -------------------------------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------------------------------

void LogConsole::CharFromPoint(int x, int y, int& line, int& col) const
{
    col = x / m_charWidth;
    int row = y / m_charHeight;
    line = m_scrollOffset + row;

    if (col < 0) col = 0;
    if (line < 0) line = 0;

    int totalLines = (int)m_lines.size();
    if (line >= totalLines && totalLines > 0)
        line = totalLines - 1;
}

void LogConsole::GetSelRange(int& startLine, int& startCol,
    int& endLine, int& endCol) const
{
    if (m_selAnchorLine < m_selLine ||
        (m_selAnchorLine == m_selLine && m_selAnchorCol <= m_selCol))
    {
        startLine = m_selAnchorLine; startCol = m_selAnchorCol;
        endLine = m_selLine;       endCol = m_selCol;
    }
    else
    {
        startLine = m_selLine;       startCol = m_selCol;
        endLine = m_selAnchorLine; endCol = m_selAnchorCol;
    }
}

void LogConsole::UpdateScrollBar()
{
    RECT client;
    GetClientRect(m_hWnd, &client);
    int visRows = client.bottom / m_charHeight;
    if (visRows < 1) visRows = 1;

    int totalLines = (int)m_lines.size();
    bool overflow = totalLines > visRows;

    ShowScrollBar(m_hWnd, SB_VERT, overflow);

    if (overflow)
    {
        SCROLLINFO si = {};
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = totalLines - 1;
        si.nPage = visRows;
        si.nPos = m_scrollOffset;
        SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
    }
}

std::string LogConsole::GetSelectedText() const
{
    if (m_selAnchorLine < 0)
        return {};

    int ss, sc, es, ec;
    GetSelRange(ss, sc, es, ec);

    if (ss == es && sc == ec)
        return {};

    std::string result;

    for (int line = ss; line <= es; ++line)
    {
        if (line < 0 || line >= (int)m_lines.size())
            continue;

        const std::string& str = m_lines[line];

        int start = 0;
        int end = (int)str.size();

        if (line == ss)
            start = sc;

        if (line == es)
            end = ec;

        if (start < 0) start = 0;
        if (end > (int)str.size()) end = (int)str.size();

        if (end > start)
            result += str.substr(start, end - start);

        if (line != es)
            result += "\r\n";
    }

    return result;
}

void LogConsole::CopySelectionToClipboard()
{
    std::string text = GetSelectedText();
    if (text.empty())
        return;

    if (!OpenClipboard(m_hWnd))
        return;

    EmptyClipboard();

    SIZE_T size = text.size() + 1;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hMem)
    {
        void* ptr = GlobalLock(hMem);
        if (ptr)
        {
            memcpy(ptr, text.c_str(), size);
            GlobalUnlock(hMem);

            SetClipboardData(CF_TEXT, hMem);
        }
        else
        {
            GlobalFree(hMem);
        }
    }

    CloseClipboard();
}
