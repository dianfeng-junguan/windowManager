#include "wndman.h"
#include <stdio.h>
#include <windows.h>

// 全局变量
HANDLE hTimer = NULL;
HWND hWnd = NULL;

// 时钟中断处理函数
VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    _on_clock_int();
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
    int wnd_type = WNDTYPE_WINDOW;
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
// 定义绘制单个窗口的函数
void draw_window(HDC hdci, window_t* wnd)
{
    // 创建一个矩形来表示窗口的位置和大小
    RECT rect, clientrect;
    PAINTSTRUCT ps;
    // 开始绘制
    hdc = BeginPaint(hWnd, &ps);

    // 定义要填充的矩形区域
    rect.left = wnd->x;
    rect.top = wnd->y;
    rect.right = wnd->x + wnd->width;
    rect.bottom = wnd->y + wnd->height;

    // 创建一个画刷，用于填充矩形
    hBrush = CreateSolidBrush(RGB(255, 0, 0)); // 创建一个红色画刷

    // 使用FillRect函数填充矩形
    FillRect(hdc, &rect, hBrush);
    DrawText(hdc, wnd->title, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

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