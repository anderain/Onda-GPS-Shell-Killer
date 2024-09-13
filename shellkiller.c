#include "windows.h"
#include <commdlg.h>
#include <commctrl.h>

#define APP_NAME _T("ShellKiller")

#define IDC_BTN_KILLSHELL   1001
#define IDC_BTN_EXIT        1002

HINSTANCE           hInst;
HWND                hWndBtnKillShell;
HWND                hWndBtnExit;

ATOM                MyRegisterClass     (HINSTANCE hInstance, LPTSTR szWindowClass);
BOOL                InitInstance        (HINSTANCE, int);
LRESULT CALLBACK    WndProc             (HWND, UINT, WPARAM, LPARAM);
void                Launch              (PTCHAR exePath);

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPTSTR    lpCmdLine,
                    int       nCmdShow) {
    MSG msg;

    if (!InitInstance (hInstance, nCmdShow))  {
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0))  {
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass) {
    WNDCLASS    wc;

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = (WNDPROC) WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = 0;
    wc.hbrBackground    = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = szWindowClass;

    return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND    hWnd;
    RECT	winRect;
    TCHAR   szTitle[]       = APP_NAME;
    TCHAR   szWindowClass[] = APP_NAME;
    int     winWidth;
    int     winHeight;
    int     buttonWidth;
    int     buttonHeight;
    int     gutter = 12;

    hInst = hInstance;

    MyRegisterClass(hInstance, szWindowClass);

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd) {    
        return FALSE;
    }

    GetWindowRect(hWnd, &winRect);

    winWidth = winRect.right - winRect.left;
    winHeight = winRect.bottom - winRect.top;
    buttonWidth = 80;
    buttonHeight = 24;

    hWndBtnKillShell = CreateWindow(
                    _T("BUTTON"), _T("Kill Shell"),
                    WS_CHILD | WS_VISIBLE | WS_BORDER,
                    winWidth / 2 - buttonWidth - gutter / 2, (winHeight + buttonHeight) / 2,
                    buttonWidth, buttonHeight,
                    hWnd, (HMENU)IDC_BTN_KILLSHELL, hInst,  NULL);

    hWndBtnExit = CreateWindow(
                    _T("BUTTON"), _T("Exit"),
                    WS_CHILD | WS_VISIBLE | WS_BORDER,
                    winWidth / 2 + gutter / 2, (winHeight + buttonHeight) / 2,
                    buttonWidth, buttonHeight,
                    hWnd, (HMENU)IDC_BTN_EXIT, hInst,  NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

BOOL KillShell(const TCHAR* wndTitle) {
    HWND    hwnd = FindWindow(NULL, wndTitle);
    DWORD   processId;
    int     success;
    HANDLE  hProcess;

    if (!hwnd) {
        MessageBox(NULL, L"Window not found", L"Failed", MB_OK);
        return FALSE;
    }

    GetWindowThreadProcessId(hwnd, &processId);
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        MessageBox(NULL, L"Failed to open process", L"Failed", MB_OK);
        return FALSE;
    }

    success = TerminateProcess(hProcess, 0);
    if (!success) {
        MessageBox(NULL, L"Failed to terminate process", L"Failed", MB_OK);
    }

    CloseHandle(hProcess);
    return success;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int             wmId;
    int             wmEvent;
    HDC             hdc;
    PAINTSTRUCT     ps;
    RECT            clientRect;
    static TCHAR    msg[] = _T("Onda GPS Shell Killer");
    static int      msgLength = sizeof(msg) / sizeof(msg[0]) - 1;
    SIZE            textSize;
    int             x, y;

    switch (message) {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);

            GetClientRect(hWnd, &clientRect);
            GetTextExtentPoint32(hdc, msg, msgLength, &textSize);
            x = (clientRect.right - textSize.cx) / 2;
            y = (clientRect.bottom - textSize.cy) / 2;
            ExtTextOut(hdc, x, y, 0, NULL, msg, msgLength, NULL);

            EndPaint(hWnd, &ps);
            break;
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            switch (wmId) {
                case IDC_BTN_EXIT:
                    PostQuitMessage(0);
                    break;
                case IDC_BTN_KILLSHELL:
                    Launch(_T("\\Windows\\explorer.exe"));
                    KillShell(_T("SHELLEX_LYG"));
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

void Launch(PTCHAR exePath) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcess(exePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        TCHAR errMsg[500];
        wsprintf(errMsg, _T("Failed to launch %s, error code: %d\n"), exePath, GetLastError());
        MessageBox(NULL, errMsg, _T("Error"), MB_OK);
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
