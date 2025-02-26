#include "wndman.h"
#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define KMALLOC(size) malloc(size)
#else
#define KMALLOC(size) kmalloc(size, NO_ALIGN)
#endif
window_t* g_windows;
int init_wndman()
{
    g_windows = (window_t*)KMALLOC(sizeof(window_t) * MAX_WINDOWS);
    if (g_windows == NULL) {
        return -1;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        g_windows[i].state = 0;
    }
    return 0;
}
/**
    @brief 创建窗口
    @param wnd 要创建的窗口
    @param title 窗口标题
    @param wnd_type 窗口类型
    @return 0 成功，-1 失败
*/
int create_window(window_t** wnd, char* title, int wnd_type)
{
    if (wnd == NULL || title == NULL) {
        return -1;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!(g_windows[i].state & WNDSTATE_PRESENT) && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            // 初始化窗口的基本信息
            *wnd = g_windows + i;
            window_t* wndp = *wnd;
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
            return i;
        }
    }
    return -1;
}

/**
    @brief 销毁窗口
    @param wnd 要销毁的窗口
    @return 0 成功，-1 失败
*/
int destroy_window(window_t** wnd)
{
    if (wnd == NULL || *wnd == NULL) {
        return -1;
    }
    window_t* wndp = *wnd;
    if (wndp - g_windows < 0 || wndp - g_windows >= MAX_WINDOWS) {
        return -1;
    }
    wndp->state |= WNDSTATE_DESTROYED;
    // 这里可以添加释放窗口相关资源的代码
    // 例如，如果有动态分配的内存，需要在这里释放
    wndp->state = 0;
    return 0;
}

/**
    @brief 将窗口附加到父窗口上
    @param wnd 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(window_t* wnd, window_t* parent)
{
    if (wnd == NULL || parent == NULL) {
        return -1;
    }
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
    @param wnd 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(window_t* wnd)
{
    if (wnd == NULL) {
        return -1;
    }
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
    @param wnd 用于存储找到的窗口
    @return 0 成功，-1 失败
*/
int get_window_by_title(char* title, window_t** wnd)
{
    if (title == NULL || wnd == NULL) {
        return -1;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].state & WNDSTATE_PRESENT && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            if (strcmp(g_windows[i].title, title) == 0) {
                *wnd = g_windows + i;
                return 0;
            }
        }
    }
    return -1;
}

/**
    @brief 根据位置获取窗口
    @param x 窗口的x坐标
    @param y 窗口的y坐标
    @param wnd 用于存储找到的窗口
    @return 0 成功，-1 失败
*/
int get_window_by_pos(int x, int y, window_t** wnd)
{
    if (wnd == NULL) {
        return -1;
    }

    for (int i = 0; i < MAX_WINDOWS; i++) {
        //TODO 窗口重叠前后处理
        if (g_windows[i].state & WNDSTATE_PRESENT && !(g_windows[i].state & WNDSTATE_DESTROYED)) {
            //在范围内
            if (x >= g_windows[i].x && x < g_windows[i].x + g_windows[i].width && y >= g_windows[i].y && y < g_windows[i].y + g_windows[i].height) {
                *wnd = g_windows + i;
                return 0;
            }
        }
    }
    return -1;
}

/**
    @brief 移动窗口
    @param wnd 要移动的窗口
    @param x 新的x坐标
    @param y 新的y坐标
    @return 0 成功，-1 失败
*/
int move_window(window_t** wnd, int x, int y)
{
    if (wnd == NULL || *wnd == NULL) {
        return -1;
    }
    (*wnd)->x = x;
    (*wnd)->y = y;
    return 0;
}

/**
    @brief 调整窗口大小
    @param wnd 要调整大小的窗口
    @param width 新的宽度
    @param height 新的高度
    @return 0 成功，-1 失败
*/
int resize_window(window_t** wnd, int width, int height)
{
    if (wnd == NULL) {
        return -1;
    }
    (*wnd)->width = width;
    (*wnd)->height = height;
    return 0;
}

/**
    @brief 设置窗口标题
    @param wnd 要设置标题的窗口
    @param title 新的窗口标题
    @return 0 成功，-1 失败
*/
int set_window_title(window_t** wnd, char* title)
{
    if (wnd == NULL || title == NULL) {
        return -1;
    }
    strncpy((*wnd)->title, title, sizeof((*wnd)->title) - 1);
    (*wnd)->title[sizeof((*wnd)->title) - 1] = '\0';
    return 0;
}

/**
    @brief 获取窗口标题
    @param wnd 要获取标题的窗口
    @param title 用于存储窗口标题的缓冲区
    @return 0 成功，-1 失败
*/
int get_window_title(window_t* wnd, char* title)
{
    if (wnd == NULL || title == NULL) {
        return -1;
    }
    strncpy(title, wnd->title, sizeof(wnd->title));
    return 0;
}

/**
    @brief 设置窗口状态
    @param wnd 要设置状态的窗口
    @param state 新的窗口状态
    @return 0 成功，-1 失败
*/
int set_window_state(window_t** wnd, int state)
{
    if (wnd == NULL) {
        return -1;
    }
    (*wnd)->state = state;
    return 0;
}

/**
    @brief 获取窗口状态
    @param wnd 要获取状态的窗口
    @param state 用于存储窗口状态的指针
    @return 0 成功，-1 失败
*/
int get_window_state(window_t* wnd, int* state)
{
    if (wnd == NULL || state == NULL) {
        return -1;
    }
    *state = wnd->state;
    return 0;
}

/**
    @brief 显示窗口
    @param wnd 要显示的窗口
    @return 0 成功，-1 失败
*/
int show_window(window_t* wnd)
{
    if (wnd == NULL) {
        return -1;
    }
    // 这里可以添加显示窗口的具体逻辑
    return 0;
}

/**
    @brief 隐藏窗口
    @param wnd 要隐藏的窗口
    @return 0 成功，-1 失败
*/
int hide_window(window_t* wnd)
{
    if (wnd == NULL) {
        return -1;
    }
    // 这里可以添加隐藏窗口的具体逻辑
    return 0;
}

/**
    @brief 设置窗口事件处理函数
    @param wnd 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(window_t** wnd, window_event_handler_t handler)
{
    if (wnd == NULL || handler == NULL) {
        return -1;
    }
    // 这里可以添加设置事件处理函数的具体逻辑
    return 0;
}

/**
    @brief 发送窗口事件
    @param wnd 要发送事件的窗口
    @param event 要发送的事件
    @return 0 成功，-1 失败
*/
int send_window_event(window_t* wnd, window_event_t* event)
{
    if (wnd == NULL || event == NULL) {
        return -1;
    }
    // 这里可以添加发送事件的具体逻辑
    return 0;
}

/**
    @brief 添加窗口事件监听器
    @param wnd 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(window_t** wnd, int event_type, window_event_handler_t listener)
{
    if (wnd == NULL || listener == NULL) {
        return -1;
    }
    // 这里可以添加添加监听器的具体逻辑
    // 暂时简单返回一个假的ID
    return 1;
}

/**
    @brief 移除窗口事件监听器
    @param wnd 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(window_t** wnd, int event_type, int listener_id)
{
    if (wnd == NULL) {
        return -1;
    }
    // 这里可以添加移除监听器的具体逻辑
    return 0;
}