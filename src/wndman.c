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
void _wnd_list_add(window_t** list, window_t* wnd)
{
    window_t* p = *list;
    if (!p) {
        *list = wnd;
        wnd->next_as_child = NULL;
        wnd->prev_as_child = NULL;
        return;
    }
    while (p->next_as_child) {
        p = p->next_as_child;
    }
    p->next_as_child = wnd;
    wnd->prev_as_child = p;
    wnd->next_as_child = NULL;
}
void _wnd_list_remove(window_t** list, window_t* wnd)
{
    if (!*list) {
        return;
    }
    if (*list == wnd) {
        *list = wnd->next_as_child;
        if (*list) {
            (*list)->prev_as_child = NULL;
        }
        wnd->next_as_child = NULL;
        wnd->prev_as_child = NULL;
        return;
    }
    window_t* p = *list;
    while (p->next_as_child) {
        if (p->next_as_child == wnd) {
            p->next_as_child = wnd->next_as_child;
            if (p->next_as_child) {
                p->next_as_child->prev_as_child = p;
            }
            wnd->next_as_child = NULL;
            wnd->prev_as_child = NULL;
            return;
        }
        p = p->next_as_child;
    }
}
void default_mouse_move_event_handler(windowptr_t wndptr, int event_type, window_event_t* event)
{
#ifdef DEBUG
    // printf("default_mouse_move_event_handler: wndptr=%d, x=%d, y=%d\n", wndptr, event->x, event->y);
#endif
}
void default_mouse_down_event_handler(windowptr_t wndptr, int event_type, window_event_t* event)
{
#ifdef DEBUG
    printf("default_mouse_down_event_handler: wndptr=%p, x=%d, y=%d, button=%d\n", wndptr, event->x, event->y, event->mouse_button);
#endif
}
void default_mouse_up_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_mouse_double_click_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_mouse_wheel_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_key_down_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_key_up_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_key_press_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_window_resize_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_window_move_event_handler(windowptr_t wndptr, int event_type, window_event_t* event)
{
    _offset_window(wndptr, event->dx, event->dy);
#ifdef DEBUG
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif
}
void default_window_close_event_handler(windowptr_t wndptr, int event_type, window_event_t* event)
{
    while (wndptr->parent) {
        wndptr = wndptr->parent;
    }
    destroy_window(wndptr);
#ifdef DEBUG
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif
}
void default_window_focus_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_window_lost_focus_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
void default_timer_event_handler(windowptr_t wndptr, int event_type, window_event_t* event) { }
int init_wndman()
{
    g_windows = NULL; //(int)KMALLOC(sizeof(window_t) * MAX_WINDOWS);s
    //TODO 向时钟中断注册处理消息的函数

    return 0;
}
windowptr_t create_window(char* title, int wnd_type)
{
    if (wnd_type < 0 || wnd_type > WNDTYPE_TREEVIEW) {
        return NULL;
    }
    if (wnd_type == WNDTYPE_WINDOW) {
        windowptr_t btn = _create_control("X", WNDTYPE_BUTTON);
        windowptr_t wndp = _create_control(title, WNDTYPE_WINDOW);
        wndp->width = DEFAULT_WINDOW_WIDTH;
        wndp->height = DEFAULT_WINDOW_HEIGHT;
        wndp->x = 100;
        wndp->y = 100;
        _wnd_list_add(&g_windows, wndp);
        btn->width = 20;
        btn->height = 20;
        btn->x = wndp->width - 20 - 2;
        btn->y = 3;
        show_window(btn);

        attach_window(btn, wndp);
        set_window_event_handler(btn, WND_EVENT_MOUSE_CLICK, _wndpresethandler_closebutton_clicked);
        set_window_event_handler(btn, WND_EVENT_MOUSE_UP, _wndpresethandler_closebutton_clicked);
        return wndp;
    }
    return _create_control(title, wnd_type);
}
windowptr_t _create_control(char* title, int wnd_type)
{
    // 初始化窗口的基本信息
    window_t* wndp = KMALLOC(sizeof(window_t));
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

    return wndp;
}

/**
    @brief 销毁窗口
    @param wndptr 要销毁的窗口
    @return 0 成功，-1 失败
*/
int destroy_window(windowptr_t wndptr)
{
    if (!wndptr) {
        return -1;
    }
    wndptr->state |= WNDSTATE_DESTROYED;
    wndptr->state = 0;
    if (wndptr->children) {
        window_t* p = wndptr->children;
        while (p) {
            window_t* q = p->next_as_child;
            destroy_window(p);
            p = q;
        }
    }
    if (!wndptr->parent) {
        //是顶层窗口，需要从窗口列表中移除
        _wnd_list_remove(&g_windows, wndptr);
    }
    KFREE(wndptr);

    return 0;
}

/**
    @brief 将窗口附加到父窗口上
    @param wndptr 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(windowptr_t wndptr, windowptr_t parent_wndptr)
{
    if (!wndptr || !parent_wndptr) {
        return -1;
    }
    if (!parent_wndptr->children) {
        parent_wndptr->children = wndptr;
    }
    for (window_t* p = parent_wndptr->children; p; p = p->next_as_child) {
        if (!p->next_as_child) {
            p->next_as_child = wndptr;
            wndptr->prev_as_child = p;
            wndptr->next_as_child = NULL;
            wndptr->parent = parent_wndptr;
            return 0;
        }
    }

    return -1;
}

/**
    @brief 将窗口从父窗口上分离
    @param wndptr 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(windowptr_t wnd)
{
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
    @param wndptr 用于存储找到的窗口
    @return 0 成功，-1 失败
*/
windowptr_t get_window_by_title(char* title)
{
    //链表查找
    for (windowptr_t p; p; p = p->next_as_child) {
        if (strcmp(p->title, title) == 0) {
            return p;
        }
    }
    return NULL;
}
windowptr_t _get_collided_window(windowptr_t wnd, int x, int y)
{
    //TODO 子控件发送消息
    if (wnd && wnd->state & WNDSTATE_PRESENT && !(wnd->state & WNDSTATE_DESTROYED) && wnd->state & WNDSTATE_VISIBLE) {
        //在范围内
        if (wnd->children) {
            for (windowptr_t child = wnd->children; child; child = child->next_as_child) {
                windowptr_t collided = _get_collided_window(child, x - wnd->x, y - wnd->y);
                if (collided) {
                    return collided;
                }
            }
        }
        if (x >= wnd->x && x < wnd->x + wnd->width && y >= wnd->y && y < wnd->y + wnd->height) {
            return wnd;
        }
    }
    return NULL;
}
windowptr_t get_window_by_pos(int x, int y, int layer)
{
    window_t* wnd = g_windows;
    for (; wnd; wnd = wnd->next_as_child) {
        windowptr_t collided = _get_collided_window(wnd, x, y);
        if (collided) {
            return collided;
        }
    }
    return NULL;
}
int _offset_window(windowptr_t wnd, int dx, int dy)
{
    wnd->x += dx;
    wnd->y += dy;
    return 0;
}
/**
    @brief 移动窗口
    @param wndptr 要移动的窗口
    @param x 新的x坐标
    @param y 新的y坐标
    @return 0 成功，-1 失败
*/
int move_window(windowptr_t wndptr, int x, int y)
{
    int dx = x - wndptr->x;
    int dy = y - wndptr->y;
    _offset_window(wndptr, dx, dy);
    return 0;
}

/**
    @brief 调整窗口大小
    @param wndptr 要调整大小的窗口
    @param width 新的宽度
    @param height 新的高度
    @return 0 成功，-1 失败
*/
int resize_window(windowptr_t wndptr, int width, int height)
{
    wndptr->width = width;
    wndptr->height = height;
    return 0;
}

/**
    @brief 设置窗口标题
    @param wndptr 要设置标题的窗口
    @param title 新的窗口标题
    @return 0 成功，-1 失败
*/
int set_window_title(windowptr_t wndptr, char* title)
{
    strncpy(wndptr->title, title, sizeof(wndptr->title) - 1);
    wndptr->title[sizeof(wndptr->title) - 1] = '\0';
    return 0;
}

/**
    @brief 获取窗口标题
    @param wndptr 要获取标题的窗口
    @param title 用于存储窗口标题的缓冲区
    @return 0 成功，-1 失败
*/
int get_window_title(windowptr_t wndptr, char* title)
{

    window_t* wnd = wndptr;
    strncpy(title, wnd->title, sizeof(wnd->title));
    return 0;
}

/**
    @brief 设置窗口状态
    @param wndptr 要设置状态的窗口
    @param state 新的窗口状态
    @return 0 成功，-1 失败
*/
int set_window_state(windowptr_t wndptr, int state)
{

    window_t* wnd = wndptr;
    wnd->state = state;
    return 0;
}
/**
    @brief 获取窗口状态
    @param wndptr 要获取状态的窗口
    @param state 用于存储窗口状态的指针
    @return 0 成功，-1 失败
*/
int get_window_state(windowptr_t wndptr, int* state)
{

    window_t* wnd = wndptr;
    *state = wnd->state;
    return 0;
}

/**
    @brief 显示窗口
    @param wndptr 要显示的窗口
    @return 0 成功，-1 失败
*/
int show_window(windowptr_t wndptr)
{

    // 这里可以添加显示窗口的具体逻辑
    wndptr->state |= WNDSTATE_VISIBLE;
    return 0;
}

/**
    @brief 隐藏窗口
    @param wndptr 要隐藏的窗口
    @return 0 成功，-1 失败
*/
int hide_window(windowptr_t wndptr)
{

    // 这里可以添加隐藏窗口的具体逻辑
    wndptr->state &= ~WNDSTATE_VISIBLE;
    return 0;
}

/**
    @brief 设置窗口事件处理函数
    @param wndptr 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(windowptr_t wndptr, int event_type, window_event_handler_t handler)
{

    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    wndptr->event_handlers[event_type] = handler;
    return 1;
}

/**
    @brief 发送窗口事件
    @param wndptr 要发送事件的窗口
    @param event 要发送的事件
    @return 0 成功，-1 失败
*/
int send_window_event(windowptr_t wndptr, window_event_t* event)
{

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
    @param wndptr 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(windowptr_t wndptr, int event_type, window_event_handler_t listenerfunc)
{

    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    window_event_listener_t* listener = (window_event_listener_t*)KMALLOC(sizeof(window_event_listener_t));
    if (listener == NULL) {
        return -1;
    }
    listener->wndptr = wndptr;
    listener->event_type = event_type;
    listener->listener = listenerfunc;
    listener->next = g_event_listeners;
    g_event_listeners = listener;
    return listener->listener_id = LISTENER_ID;
}

/**
    @brief 移除窗口事件监听器
    @param wndptr 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(windowptr_t wndptr, int event_type, int listener_id)
{

    if (event_type < 0 || event_type >= WNDEVENT_COUNT) {
        return -1;
    }
    for (window_event_listener_t *p = g_event_listeners, *prev = p; p; prev = p, p = p->next) {
        if (p->wndptr == wndptr && p->event_type == event_type && p->listener_id == listener_id) {
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
        window_t* wnd = event->sender;
        if (wnd->event_handlers[event->event_type]) {
            wnd->event_handlers[event->event_type](event->sender, event->event_type, event);
        }
        //执行listener
        for (window_event_listener_t* listener = g_event_listeners; listener; listener = listener->next) {
            if (listener->wndptr == event->sender && listener->event_type == event->event_type) {
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
static int mouse_click_timer = 0;
windowptr_t last_mouse_down_wndptr = NULL;
int last_mouse_down_button = -1;
windowptr_t g_focused_wndptr = NULL;
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
    windowptr_t wndptr = last_mouse_down_wndptr = get_window_by_pos(x, y, 0);
    g_focused_wndptr = wndptr;
    if (wndptr) {
        window_event_t event = {
            .event_type = WND_EVENT_MOUSE_DOWN,
            .sender = wndptr,
            .mouse_button = button,
            .x = x,
            .y = y,
        };
        send_window_event(wndptr, &event);
    }
}
void _on_mouse_up(int x, int y, int button)
{
    mouse_down_flag = 0;
    windowptr_t wndptr = get_window_by_pos(x, y, 0);
    if (wndptr) {
        window_event_t event = {
            .event_type = WND_EVENT_MOUSE_UP,
            .sender = wndptr,
            .mouse_button = button,
            .x = x,
            .y = y,
        };
        send_window_event(wndptr, &event);
    }
    if (wndptr == last_mouse_down_wndptr && button == last_mouse_down_button) {
        _on_mouse_click(wndptr, x, y, button);
    }
    if (mouse_click_timer < g_setted_mouse_consecutive_click_max_time) {
        clicked_times++;
    } else {
        clicked_times = 0;
    }
}
void _on_mouse_move(int x, int y)
{
    windowptr_t wndptr = get_window_by_pos(x, y, 0);
    if (wndptr) {
        window_event_t event = {
            .event_type = WND_EVENT_MOUSE_MOVE,
            .sender = wndptr,
            .x = x,
            .y = y,
        };
        send_window_event(wndptr, &event);
        if (mouse_down_flag && wndptr->type == WNDTYPE_WINDOW) {
            window_event_t window_move_event = {
                .event_type = WND_EVENT_WINDOW_MOVE,
                .sender = wndptr,
                .x = x - mouse_down_x,
                .y = y - mouse_down_y,
                .dx = x - last_mouse_move_x,
                .dy = y - last_mouse_move_y,
            };
            send_window_event(wndptr, &window_move_event);
        }
    }
    last_mouse_move_x = x;
    last_mouse_move_y = y;
}
void _on_mouse_click(windowptr_t wndptr, int x, int y, int button)
{
    window_event_t clicked_event = {
        .event_type = WND_EVENT_MOUSE_CLICK,
        .sender = wndptr,
        .mouse_button = button,
        .x = x,
        .y = y,
    };
    send_window_event(wndptr, &clicked_event);
    window_t* wnd = wndptr;
    while (wnd->type != WNDTYPE_WINDOW && wnd->parent) {
        wnd = wnd->parent;
    }
    //调整窗口在g_windows中的位置，让获得焦点的窗口位于最前面
    _wnd_list_remove(&g_windows, wnd);
    wnd->next_as_child = g_windows;
    wnd->prev_as_child = NULL;
    g_windows = wnd;

    g_focused_wndptr = wndptr;
}
void _on_key_down(int key_code)
{
    last_key_code_down = key_code;
    window_event_t event = {
        .event_type = WND_EVENT_KEY_DOWN,
        .sender = g_focused_wndptr,
        .key_code = key_code,
    };
    send_window_event(g_focused_wndptr, &event);
}
void _on_key_up(int key_code)
{
    window_event_t event = {
        .event_type = WND_EVENT_KEY_UP,
        .sender = g_focused_wndptr,
        .key_code = key_code,
    };
    send_window_event(g_focused_wndptr, &event);
    if (last_key_code_down == key_code) {
        _on_key_press(key_code);
    }
}
void _on_key_press(int key_code)
{
    window_event_t event = {
        .event_type = WND_EVENT_KEY_PRESS,
        .sender = g_focused_wndptr,
        .key_code = key_code,
    };
    send_window_event(g_focused_wndptr, &event);
}
//绘制窗口和子窗口
void _render_window(window_t* wndi)
{
    window_t* wnd = wndi;
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
    if (wnd->children) {
        for (window_t* child = wnd->children; child; child = child->next_as_child) {
            _render_window(child);
        }
    }
}
void _render_windows()
{
    window_t* wnd = g_windows;
    for (; wnd; wnd = wnd->next_as_child) {
        _render_window(wnd);
    }
}
void _wndpresethandler_closebutton_clicked(windowptr_t wndptr, int event_type, window_event_t* event)
{
#ifdef DEBUG
    printf("close button clicked\n");
#endif
    window_t* wnd = wndptr;
    windowptr_t parent_wndp = wnd->parent;
    window_event_t close_event = {
        .event_type = WND_EVENT_WINDOW_CLOSE,
        .sender = wndptr,
    };
    send_window_event(parent_wndp, &close_event);
}