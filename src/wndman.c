#include "wndman.h"
#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define KMALLOC(size) malloc(size)
#define KFREE(ptr) free(ptr)
#else
#define KMALLOC(size) kmalloc(size, NO_ALIGN)
#define KFREE(ptr) kfree(ptr)
#endif
#ifdef DEBUG
#include <windows.h>
extern HDC hdc;
extern HDC hMemDC;
extern HBITMAP hBitmap;
extern HWND hWnd;
#endif
static int listener_id_gen = 0;
#define LISTENER_ID (listener_id_gen++)
window_t* g_wnd_layer_ordered[MAX_WINDOWS] = { 0 };
window_t* g_windows;
window_event_listener_t* g_event_listeners = NULL;
window_event_t* g_event_queue = NULL;
window_event_handler_t g_default_event_handlers[WNDEVENT_COUNT] = {
    [WND_EVENT_MOUSE_MOVE] = default_mouse_move_event_handler,
    [WND_EVENT_MOUSE_DOWN] = default_mouse_down_event_handler,
    [WND_EVENT_MOUSE_UP] = default_mouse_up_event_handler,
    [WND_EVENT_MOUSE_DOUBLE_CLICK] = default_mouse_double_click_event_handler,
    [WND_EVENT_MOUSE_WHEEL] = default_mouse_wheel_event_handler,
    [WND_EVENT_KEY_DOWN] = default_key_down_event_handler,
    [WND_EVENT_KEY_UP] = default_key_up_event_handler,
    [WND_EVENT_KEY_PRESS] = default_key_press_event_handler,
    [WND_EVENT_WINDOW_RESIZE] = default_window_resize_event_handler,
    [WND_EVENT_WINDOW_MOVE] = default_window_move_event_handler,
    [WND_EVENT_WINDOW_CLOSE] = default_window_close_event_handler,
    [WND_EVENT_WINDOW_FOCUS] = default_window_focus_event_handler,
    [WND_EVENT_WINDOW_LOST_FOCUS] = default_window_lost_focus_event_handler,
};
void default_mouse_move_event_handler(int wnd_id, int event_type, window_event_t* event)
{
#ifdef DEBUG
    // printf("default_mouse_move_event_handler: wnd_id=%d, x=%d, y=%d\n", wnd_id, event->x, event->y);
#endif
}
void default_mouse_down_event_handler(int wnd_id, int event_type, window_event_t* event)
{
#ifdef DEBUG
    printf("default_mouse_down_event_handler: wnd_id=%d, x=%d, y=%d, button=%d\n", wnd_id, event->x, event->y, event->mouse_button);
#endif
}
void default_mouse_up_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_mouse_double_click_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_mouse_wheel_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_key_down_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_key_up_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_key_press_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_window_resize_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_window_move_event_handler(int wnd_id, int event_type, window_event_t* event)
{
    _offset_window(wnd_id, event->dx, event->dy);
#ifdef DEBUG
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif
}
void default_window_close_event_handler(int wnd_id, int event_type, window_event_t* event)
{
    destroy_window(wnd_id);
}
void default_window_focus_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_window_lost_focus_event_handler(int wnd_id, int event_type, window_event_t* event) { }
void default_timer_event_handler(int wnd_id, int event_type, window_event_t* event) { }
int init_wndman()
{
    g_windows = (int)KMALLOC(sizeof(window_t) * MAX_WINDOWS);
    if (g_windows == NULL) {
        return -1;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        g_windows[i].state = 0;
    }
    //TODO 向时钟中断注册处理消息的函数

    return 0;
}
int create_window(char* title, int wnd_type)
{
    if (wnd_type < 0 || wnd_type > WNDTYPE_TREEVIEW) {
        return -1;
    }
    if (wnd_type == WNDTYPE_WINDOW) {
        int btni = _create_control("X", WNDTYPE_BUTTON);
        int wndi = _create_control(title, WNDTYPE_WINDOW);
        window_t* wndp = g_windows + wndi;
        wndp->width = DEFAULT_WINDOW_WIDTH;
        wndp->height = DEFAULT_WINDOW_HEIGHT;
        wndp->x = 100;
        wndp->y = 100;
        window_t* btnp = g_windows + btni;
        btnp->width = 20;
        btnp->height = 20;
        btnp->x = wndp->x + wndp->width - 20 - 2;
        btnp->y = wndp->y + 3;
        show_window(btni);

        attach_window(btni, wndi);
        return wndi;
    }
    return _create_control(title, wnd_type);
}
int _create_control(char* title, int wnd_type)
{
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!(g_windows[i].state & WNDSTATE_PRESENT) && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            // 初始化窗口的基本信息
            window_t* wndp = g_windows + i;
            wndp->state = WNDSTATE_PRESENT;
            wndp->x = 0;
            wndp->y = 0;
            wndp->width = 100;
            wndp->height = 100;
            wndp->type = wnd_type;
            wndp->parent = NULL;
            wndp->children = NULL;
            wndp->next_as_child = NULL;
            wndp->prev_as_child = NULL;
            // 复制窗口标题
            strncpy(wndp->title, title, sizeof(wndp->title) - 1);
            wndp->title[sizeof(wndp->title) - 1] = '\0';
            memcpy(wndp->event_handlers, g_default_event_handlers, sizeof(g_default_event_handlers));
            _add_to_layer_ordered(i, 0);
            return i;
        }
    }
    return -1;
}

/**
    @brief 销毁窗口
    @param wnd_id 要销毁的窗口
    @return 0 成功，-1 失败
*/
int destroy_window(int wnd_id)
{
    if (wnd_id < 0 || wnd_id >= MAX_WINDOWS) {
        return -1;
    }
    window_t* wndp = g_windows + wnd_id;
    wndp->state |= WNDSTATE_DESTROYED;
    // 这里可以添加释放窗口相关资源的代码
    // 例如，如果有动态分配的内存，需要在这里释放
    wndp->state = 0;
    return 0;
}

/**
    @brief 将窗口附加到父窗口上
    @param wnd_id 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(int wnd_id, int parent_id)
{
    if (wnd_id < 0 || wnd_id >= MAX_WINDOWS || parent_id < 0 || parent_id >= MAX_WINDOWS) {
        return -1;
    }
    window_t* parent = g_windows + parent_id;
    window_t* wnd = g_windows + wnd_id;
    if (!parent->children) {
        parent->children = wnd;
    }
    for (window_t* p = parent->children; p; p = p->next_as_child) {
        if (!p->next_as_child) {
            p->next_as_child = wnd;
            wnd->prev_as_child = p;
            wnd->next_as_child = NULL;
            wnd->parent = parent;
            return 0;
        }
    }

    return -1;
}

/**
    @brief 将窗口从父窗口上分离
    @param wnd_id 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(int wnd_id)
{
    if (wnd_id < 0 || wnd_id >= MAX_WINDOWS) {
        return -1;
    }
    window_t* wnd = g_windows + wnd_id;
    wnd->parent = NULL;
    if (wnd->prev_as_child)
        wnd->prev_as_child->next_as_child = wnd->next_as_child;
    if (wnd->next_as_child)
        wnd->next_as_child->prev_as_child = wnd->prev_as_child;
    wnd->next_as_child = NULL;
    wnd->prev_as_child = NULL;
    return 0;
}

/**
    @brief 根据标题获取窗口
    @param title 窗口标题
    @param wnd_id 用于存储找到的窗口
    @return 0 成功，-1 失败
*/
int get_window_by_title(char* title)
{
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].state & WNDSTATE_PRESENT && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            if (strcmp(g_windows[i].title, title) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/**
    @brief 根据位置获取窗口
    @param x 窗口的x坐标
    @param y 窗口的y坐标
    @param wnd_id 用于存储找到的窗口
    @return 0 成功，-1 失败
*/
int get_window_by_pos(int x, int y, int layer)
{
    for (int i = 0; i < MAX_WINDOWS; i++) {
        //TODO 窗口重叠前后处理
        if (g_windows[i].state & WNDSTATE_PRESENT && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            //在范围内
            if (x >= g_windows[i].x && x < g_windows[i].x + g_windows[i].width && y >= g_windows[i].y && y < g_windows[i].y + g_windows[i].height) {
                return i;
            }
        }
    }
    return -1;
}
int _offset_window(int wnd_id, int dx, int dy)
{
    window_t* wnd = g_windows + wnd_id;
    wnd->x += dx;
    wnd->y += dy;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].parent == wnd) {
            g_windows[i].x += dx;
            g_windows[i].y += dy;
        }
    }
    return 0;
}
/**
    @brief 移动窗口
    @param wnd_id 要移动的窗口
    @param x 新的x坐标
    @param y 新的y坐标
    @return 0 成功，-1 失败
*/
int move_window(int wnd_id, int x, int y)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    int dx = x - wnd->x;
    int dy = y - wnd->y;
    _offset_window(wnd_id, dx, dy);
    return 0;
}

/**
    @brief 调整窗口大小
    @param wnd_id 要调整大小的窗口
    @param width 新的宽度
    @param height 新的高度
    @return 0 成功，-1 失败
*/
int resize_window(int wnd_id, int width, int height)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    wnd->width = width;
    wnd->height = height;
    return 0;
}

/**
    @brief 设置窗口标题
    @param wnd_id 要设置标题的窗口
    @param title 新的窗口标题
    @return 0 成功，-1 失败
*/
int set_window_title(int wnd_id, char* title)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    strncpy(wnd->title, title, sizeof(wnd->title) - 1);
    wnd->title[sizeof(wnd->title) - 1] = '\0';
    return 0;
}

/**
    @brief 获取窗口标题
    @param wnd_id 要获取标题的窗口
    @param title 用于存储窗口标题的缓冲区
    @return 0 成功，-1 失败
*/
int get_window_title(int wnd_id, char* title)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    strncpy(title, wnd->title, sizeof(wnd->title));
    return 0;
}

/**
    @brief 设置窗口状态
    @param wnd_id 要设置状态的窗口
    @param state 新的窗口状态
    @return 0 成功，-1 失败
*/
int set_window_state(int wnd_id, int state)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    wnd->state = state;
    return 0;
}
/**
    @brief 获取窗口状态
    @param wnd_id 要获取状态的窗口
    @param state 用于存储窗口状态的指针
    @return 0 成功，-1 失败
*/
int get_window_state(int wnd_id, int* state)
{
    CHECK_VALID_WNDID(wnd_id);
    window_t* wnd = g_windows + wnd_id;
    *state = wnd->state;
    return 0;
}

/**
    @brief 显示窗口
    @param wnd_id 要显示的窗口
    @return 0 成功，-1 失败
*/
int show_window(int wnd_id)
{
    CHECK_VALID_WNDID(wnd_id);
    // 这里可以添加显示窗口的具体逻辑
    g_windows[wnd_id].state |= WNDSTATE_VISIBLE;
    return 0;
}

/**
    @brief 隐藏窗口
    @param wnd_id 要隐藏的窗口
    @return 0 成功，-1 失败
*/
int hide_window(int wnd_id)
{
    CHECK_VALID_WNDID(wnd_id);
    // 这里可以添加隐藏窗口的具体逻辑
    g_windows[wnd_id].state &= ~WNDSTATE_VISIBLE;
    return 0;
}

/**
    @brief 设置窗口事件处理函数
    @param wnd_id 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(int wnd_id, int event_type, window_event_handler_t handler)
{
    CHECK_VALID_WNDID(wnd_id);
    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    g_windows[wnd_id].event_handlers[event_type] = handler;
    return wnd_id * WNDEVENT_COUNT + event_type;
}

/**
    @brief 发送窗口事件
    @param wnd_id 要发送事件的窗口
    @param event 要发送的事件
    @return 0 成功，-1 失败
*/
int send_window_event(int wnd_id, window_event_t* event)
{
    CHECK_VALID_WNDID(wnd_id);
    if (event->event_type < 0 || event->event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    window_event_t* new_event = (window_event_t*)KMALLOC(sizeof(window_event_t));
    if (new_event == NULL) {
        return -1;
    }
    memcpy(new_event, event, sizeof(window_event_t));
    if (!g_event_queue) {
        g_event_queue = new_event;
    } else {
        window_event_t* tail = g_event_queue;
        for (; tail->next; tail = tail->next) {
        }
        tail->next = new_event;
        new_event->next = NULL;
    }
    return 0;
}

/**
    @brief 添加窗口事件监听器
    @param wnd_id 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(int wnd_id, int event_type, window_event_handler_t listenerfunc)
{
    CHECK_VALID_WNDID(wnd_id);
    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    window_event_listener_t* listener = (window_event_listener_t*)KMALLOC(sizeof(window_event_listener_t));
    if (listener == NULL) {
        return -1;
    }
    listener->wnd_id = wnd_id;
    listener->event_type = event_type;
    listener->listener = listenerfunc;
    listener->next = g_event_listeners;
    g_event_listeners = listener;
    return listener->listener_id = LISTENER_ID;
}

/**
    @brief 移除窗口事件监听器
    @param wnd_id 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(int wnd_id, int event_type, int listener_id)
{
    CHECK_VALID_WNDID(wnd_id);
    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    for (window_event_listener_t *p = g_event_listeners, *prev = p; p; prev = p, p = p->next) {
        if (p->wnd_id == wnd_id && p->event_type == event_type && p->listener_id == listener_id) {
            prev->next = p->next;
            p->event_type = -1;
            KFREE(p);
            if (p == g_event_listeners) {
                g_event_listeners = p->next;
            }
            return 0;
        }
    }
    return -1;
}
void deal_events()
{
    window_event_t* event = g_event_queue;
    while (event) {
        //执行handler
        window_t* wnd = g_windows + event->sender;
        if (wnd->event_handlers[event->event_type]) {
            wnd->event_handlers[event->event_type](event->sender, event->event_type, event);
        }
        //执行listener
        for (window_event_listener_t* listener = g_event_listeners; listener; listener = listener->next) {
            if (listener->wnd_id == event->sender && listener->event_type == event->event_type) {
                listener->listener(event->sender, event->event_type, event);
            }
        }
        window_event_t* next = event->next;
        KFREE(event);
        event = next;
    }
    g_event_queue = NULL;
}
static int clicked_times = 0;
int mouse_down_x, mouse_down_y, mouse_down_button;
int last_mouse_move_x, last_mouse_move_y;
int g_setted_mouse_consecutive_click_max_time = 20;
static int mouse_click_timer = 0, last_mouse_down_wnd_id = -1, last_mouse_down_button = -1;
int g_focused_wnd_id = -1;
static int last_key_code_down = -1;
static int mouse_down_flag = 0;
void _on_clock_int()
{
    if (++mouse_click_timer >= g_setted_mouse_consecutive_click_max_time) {
        mouse_click_timer = 0;
        clicked_times = 0;
    }
    deal_events();
#ifndef DEBUG
    _render_windows();
#endif
}
void _on_mouse_down(int x, int y, int button)
{
    mouse_down_x = x;
    mouse_down_y = y;
    mouse_down_button = button;
    mouse_down_flag = 1;
    last_mouse_down_button = button;
    int wnd_id = last_mouse_down_wnd_id = get_window_by_pos(x, y, 0);
    g_focused_wnd_id = wnd_id;
    window_event_t event = {
        .event_type = WND_EVENT_MOUSE_DOWN,
        .sender = wnd_id,
        .mouse_button = button,
        .x = x,
        .y = y,
    };
    send_window_event(wnd_id, &event);
}
void _on_mouse_up(int x, int y, int button)
{
    mouse_down_flag = 0;
    int wnd_id = get_window_by_pos(x, y, 0);
    window_event_t event = {
        .event_type = WND_EVENT_MOUSE_UP,
        .sender = wnd_id,
        .mouse_button = button,
        .x = x,
        .y = y,
    };
    send_window_event(wnd_id, &event);
    if (wnd_id == last_mouse_down_wnd_id && button == last_mouse_down_button) {
        _on_mouse_click(wnd_id, x, y, button);
    }
    if (mouse_click_timer < g_setted_mouse_consecutive_click_max_time) {
        clicked_times++;
    } else {
        clicked_times = 0;
    }
}
void _on_mouse_move(int x, int y)
{
    int wnd_id = get_window_by_pos(x, y, 0);
    window_event_t event = {
        .event_type = WND_EVENT_MOUSE_MOVE,
        .sender = wnd_id,
        .x = x,
        .y = y,
    };
    send_window_event(wnd_id, &event);
    if (mouse_down_flag && g_windows[wnd_id].type == WNDTYPE_WINDOW) {
        window_event_t window_move_event = {
            .event_type = WND_EVENT_WINDOW_MOVE,
            .sender = wnd_id,
            .x = x - mouse_down_x,
            .y = y - mouse_down_y,
            .dx = x - last_mouse_move_x,
            .dy = y - last_mouse_move_y,
        };
        send_window_event(wnd_id, &window_move_event);
    }
    last_mouse_move_x = x;
    last_mouse_move_y = y;
}
void _on_mouse_click(int wnd_id, int x, int y, int button)
{
    window_event_t clicked_event = {
        .event_type = WND_EVENT_MOUSE_CLICK,
        .sender = wnd_id,
        .mouse_button = button,
        .x = x,
        .y = y,
    };
    send_window_event(wnd_id, &clicked_event);
    //调整窗口在g_wnd_layer_ordered中的位置，让获得焦点的窗口位于最前面
    window_t* wnd = g_windows + wnd_id;
    while (wnd->type != WNDTYPE_WINDOW && wnd->parent) {
        wnd = wnd->parent;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_wnd_layer_ordered[i] == wnd) {
            for (int j = i; j < MAX_WINDOWS - 1; j++) {
                g_wnd_layer_ordered[j] = g_wnd_layer_ordered[j + 1];
            }
        }
    }
    for (int i = 0; i < MAX_WINDOWS - 1; i++) {
        g_wnd_layer_ordered[i + 1] = g_wnd_layer_ordered[i];
    }
    g_wnd_layer_ordered[0] = wnd;
}
void _on_key_down(int key_code)
{
    last_key_code_down = key_code;
    window_event_t event = {
        .event_type = WND_EVENT_KEY_DOWN,
        .sender = g_focused_wnd_id,
        .key_code = key_code,
    };
    send_window_event(g_focused_wnd_id, &event);
}
void _on_key_up(int key_code)
{
    window_event_t event = {
        .event_type = WND_EVENT_KEY_UP,
        .sender = g_focused_wnd_id,
        .key_code = key_code,
    };
    send_window_event(g_focused_wnd_id, &event);
    if (last_key_code_down == key_code) {
        _on_key_press(key_code);
    }
}
void _on_key_press(int key_code)
{
    window_event_t event = {
        .event_type = WND_EVENT_KEY_PRESS,
        .sender = g_focused_wnd_id,
        .key_code = key_code,
    };
    send_window_event(g_focused_wnd_id, &event);
}
int _add_to_layer_ordered(int wnd_id, int layer)
{
    if (layer < 0 || layer >= MAX_WINDOWS) {
        return -1;
    }
    for (int i = MAX_WINDOWS - 1; i > 0; i--) {
        g_wnd_layer_ordered[i] = g_wnd_layer_ordered[i - 1];
    }
    g_wnd_layer_ordered[layer] = g_windows + wnd_id;
    return 0;
}
void _render_windows()
{

#ifdef DEBUG
    for (int i = 0; i < MAX_WINDOWS; i++) {
        window_t* wnd = g_wnd_layer_ordered[i];
        if (wnd && wnd->state & WNDSTATE_PRESENT && !(wnd->state & WNDSTATE_DESTROYED)
            && wnd->state & WNDSTATE_VISIBLE) {
            switch (wnd->type) {
            case WNDTYPE_WINDOW: {
                extern void draw_window(HDC hdc, window_t * wnd);
                draw_window(hMemDC, wnd);
                break;
            }
            case WNDTYPE_BUTTON: {
                extern void draw_button(HDC hdc, window_t * wnd);
                draw_button(hMemDC, wnd);
                break;
            }
            }
        }
    }
#endif
}