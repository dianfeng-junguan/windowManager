#include "wndman.h"
#include <stdio.h>
#include <windows.h>

// 全局变量
HANDLE hTimer = NULL;
HWND hWnd = NULL;

// 时钟中断处理函数
VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    // _on_clock_int();
}

// 鼠标按下处理函数
LRESULT CALLBACK MouseDownHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    int button = (wParam & MK_LBUTTON) ? 1 : ((wParam & MK_RBUTTON) ? 2 : 0);
    _on_mouse_down(x, y, button);
    return 0;
}

// 鼠标抬起处理函数
LRESULT CALLBACK MouseUpHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    int button = (wParam & MK_LBUTTON) ? 1 : ((wParam & MK_RBUTTON) ? 2 : 0);
    _on_mouse_up(x, y, button);
    return 0;
}

// 键盘按下处理函数
LRESULT CALLBACK KeyPressHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _on_key_press(wParam);
    return 0;
}

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_LBUTTONDOWN:
        return MouseDownHandler(hwnd, uMsg, wParam, lParam);
    case WM_LBUTTONUP:
        return MouseUpHandler(hwnd, uMsg, wParam, lParam);
    case WM_MOUSEMOVE:
        _on_mouse_move(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KEYDOWN:
        return KeyPressHandler(hwnd, uMsg, wParam, lParam);
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        _render_windows();
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
// 自定义事件处理函数示例
void custom_event_handler(int wnd_id, int event_type, window_event_t* event)
{
    printf("Custom event handler called for window %d, event type %d\n", wnd_id, event_type);
}
HDC hdc;

// 创建一个画刷来填充窗口背景
HBRUSH hBrush;
HDC hMemDC;
HBITMAP hBitmap;
int main()
{
    // 初始化 wndman
    if (init_wndman() != 0) {
        printf("初始化 wndman 失败\n");
        return -1;
    }

    // 创建窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "TestWindowClass";
    if (!RegisterClass(&wc)) {
        printf("注册窗口类失败\n");
        return -1;
    }

    // 创建窗口
    hWnd = CreateWindow(wc.lpszClassName, "Test Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, wc.hInstance, NULL);
    if (!hWnd) {
        printf("创建窗口失败\n");
        return -1;
    }

    // 显示窗口
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    hdc = GetDC(GetDesktopWindow());
    hBrush = CreateSolidBrush(RGB(0, 255, 255)); // 白色背景
    // 创建内存设备上下文和兼容位图
    hMemDC = CreateCompatibleDC(hdc);
    hBitmap = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    SelectObject(hMemDC, hBitmap);

    // 创建定时器
    // CREATE_TIMER_QUERY timerQuery;
    // ZeroMemory(&timerQuery, sizeof(timerQuery));
    // timerQuery.Size = sizeof(timerQuery);
    // timerQuery.Period = 1000; // 1 秒
    if (!CreateTimerQueueTimer(&hTimer, NULL, TimerCallback, NULL, 1000, 1000, WT_EXECUTEDEFAULT)) {
        printf("创建定时器失败\n");
        return -1;
    }

    // 测试 create_window
    char title[] = "Test Window";
    int wnd_type = WNDTYPE_BUTTON;
    int wnd_id = create_window(title, wnd_type);
    if (wnd_id < 0) {
        printf("Failed to create window\n");
        return -1;
    }
    printf("Window created with ID: %d\n", wnd_id);

    // 测试 move_window
    if (move_window(wnd_id, 100, 100) != 0) {
        printf("Failed to move window\n");
    } else {
        printf("Window moved successfully\n");
    }

    // 测试 show_window
    if (show_window(wnd_id) != 0) {
        printf("Failed to show window\n");
    } else {
        printf("Window shown successfully\n");
    }

    // 测试 set_window_event_handler
    int event_type = WND_EVENT_WINDOW_MOVE;
    if (set_window_event_handler(wnd_id, event_type, custom_event_handler) < 0) {
        printf("Failed to set window event handler\n");
    } else {
        printf("Window event handler set successfully\n");
    }
    //还原默认事件处理函数
    set_window_event_handler(wnd_id, event_type, default_window_move_event_handler);

    // 测试 add_window_event_listener
    event_type = WND_EVENT_MOUSE_DOWN;
    int listener_id = 0; // 这里需要根据实际情况设置正确的 listener_id
    if ((listener_id = add_window_event_listener(wnd_id, event_type, custom_event_handler)) < 0) {
        printf("Failed to add window event listener\n");
    } else {
        printf("Window event listener added successfully\n");
    }
    // 创建一个窗口事件来测试 send_window_event
    window_event_t event = {
        .event_type = event_type,
        .x = 0,
        .y = 0,
        .sender = wnd_id
    };
    if (send_window_event(wnd_id, &event) != 0) {
        printf("Failed to send window event\n");
    } else {
        printf("Window event sent successfully\n");
    }

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        _on_clock_int();
    }

    // 测试 remove_window_event_listener
    if (remove_window_event_listener(wnd_id, event_type, listener_id) != 0) {
        printf("Failed to remove window event listener\n");
    } else {
        printf("Window event listener removed successfully\n");
    }
    // 测试 detach_window
    if (detach_window(wnd_id) != 0) {
        printf("Failed to detach window\n");
    } else {
        printf("Window detached successfully\n");
    }

    // 测试 destroy_window
    if (destroy_window(wnd_id) != 0) {
        printf("Failed to destroy window\n");
    } else {
        printf("Window destroyed successfully\n");
    }
    DeleteObject(hBrush);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(GetDesktopWindow(), hdc);
    // 销毁定时器
    if (hTimer) {
        DeleteTimerQueueTimer(NULL, hTimer, NULL);
    }

    return 0;
}

// Win2000 风格标题栏颜色
const COLORREF TITLEBAR_COLOR = RGB(128, 128, 255);
// Win2000 风格边框颜色
const COLORREF BORDER_COLOR = RGB(0, 0, 128);
// 定义绘制单个窗口的函数
void draw_window(HDC hdci, window_t* wnd)
{
    // 创建一个矩形来表示窗口的位置和大小
    RECT rect, clientrect, windowRect;
    PAINTSTRUCT ps;
    HDC hdcMem;
    HBITMAP hBitmap;
    // 开始绘制
    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &clientrect);
    GetWindowRect(hWnd, &windowRect);

    // 创建内存设备上下文
    hdcMem = CreateCompatibleDC(hdc);
    hBitmap = CreateCompatibleBitmap(hdc, clientrect.right, clientrect.bottom);
    SelectObject(hdcMem, hBitmap);

    // 定义要填充的矩形区域
    rect.left = wnd->x;
    rect.top = wnd->y;
    rect.right = wnd->x + wnd->width;
    rect.bottom = wnd->y + wnd->height;

    // 在内存设备上下文中绘制背景
    FillRect(hdci, &clientrect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // 绘制 Win2000 风格边框
    HPEN borderPen = CreatePen(PS_SOLID, 2, BORDER_COLOR);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdci, GetStockObject(NULL_BRUSH));
    HPEN oldPen = (HPEN)SelectObject(hdci, borderPen);
    Rectangle(hdci, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(hdci, oldPen);
    SelectObject(hdci, oldBrush);
    DeleteObject(borderPen);

    // 绘制 Win2000 风格标题栏
    HBRUSH titleBarBrush = CreateSolidBrush(TITLEBAR_COLOR);
    RECT titleBarRect = { rect.left, rect.top, rect.right, rect.top - 22 };
    FillRect(hdci, &titleBarRect, titleBarBrush);
    DeleteObject(titleBarBrush);

    // 在内存设备上下文中绘制矩形
    Rectangle(hdci, rect.left, rect.top, rect.right, rect.bottom);

    // 创建一个画刷，用于填充矩形
    hBrush = CreateSolidBrush(RGB(255, 0, 0));
    DrawText(hdci, wnd->title, -1, &rect, DT_SINGLELINE | DT_CENTER);
    // 将内存设备上下文的内容复制到屏幕设备上下文
    // BitBlt(hdc, 0, 0, clientrect.right, clientrect.bottom, hdci, 0, 0, SRCCOPY);

    // 清理资源
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);

    // 删除画刷，释放资源
    DeleteObject(hBrush);

    // 结束绘制
    EndPaint(hWnd, &ps);
    /* 
    GetClientRect(hWnd, &clientrect);
    rect.left = clientrect.left + wnd->x;
    rect.top = clientrect.top + wnd->y;
    rect.right = clientrect.left + wnd->x + wnd->width;
    rect.bottom = clientrect.top + wnd->y + wnd->height;
    extern HBRUSH hBrush;
    FillRect(hdci, &rect, hBrush);

    // 绘制窗口标题
    // BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), hMemDC, 0, 0, SRCCOPY);
    BitBlt(hdc, clientrect.left, clientrect.top, clientrect.right - clientrect.left, clientrect.bottom - clientrect.top, hMemDC, 0, 0, SRCCOPY); */
}

// 绘制仿 Win2000 风格按钮的函数
void DrawWin2000Button(HDC hdc, int x, int y, int width, int height, const char* text)
{
    // 定义 Win2000 风格按钮的颜色
    COLORREF borderColor = RGB(0, 0, 128);
    COLORREF lightBorderColor = RGB(255, 255, 255);
    COLORREF darkBorderColor = RGB(0, 0, 0);
    COLORREF backgroundColor = RGB(212, 208, 200);

    // 绘制按钮的外边框
    HPEN borderPen = CreatePen(PS_SOLID, 1, borderColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    Rectangle(hdc, x, y, x + width, y + height);
    SelectObject(hdc, oldPen);
    DeleteObject(borderPen);

    // 绘制按钮的内边框（高光和阴影）
    HPEN lightPen = CreatePen(PS_SOLID, 1, lightBorderColor);
    HPEN darkPen = CreatePen(PS_SOLID, 1, darkBorderColor);

    // 高光部分
    SelectObject(hdc, lightPen);
    MoveToEx(hdc, x + 1, y + 1, NULL);
    LineTo(hdc, x + width - 2, y + 1);
    LineTo(hdc, x + 1, y + height - 2);

    // 阴影部分
    SelectObject(hdc, darkPen);
    MoveToEx(hdc, x + width - 2, y + 1, NULL);
    LineTo(hdc, x + width - 2, y + height - 2);
    LineTo(hdc, x + 1, y + height - 2);

    DeleteObject(lightPen);
    DeleteObject(darkPen);

    // 填充按钮的背景
    HBRUSH backgroundBrush = CreateSolidBrush(backgroundColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, backgroundBrush);
    RECT buttonRect = { x + 2, y + 2, x + width - 2, y + height - 2 };
    FillRect(hdc, &buttonRect, backgroundBrush);
    SelectObject(hdc, oldBrush);
    DeleteObject(backgroundBrush);

    // 设置文本颜色和背景模式
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);

    // 绘制按钮上的文本
    RECT textRect = { x + 5, y + 5, x + width - 5, y + height - 5 };
    DrawTextA(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 定义绘制单个窗口的函数
void draw_button(HDC hdci, window_t* wnd)
{
    DrawWin2000Button(hdci, wnd->x, wnd->y, wnd->width, wnd->height, wnd->title);
}