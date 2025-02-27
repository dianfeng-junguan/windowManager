#pragma once

#include "wndman.h"
typedef struct _window_event{
    int x, y;
    int dx,dy;
    int event_type;
    int mouse_button;
    int key_code;
    int state;
    int sender;
    int timer_id;
    struct _window_event* next;
} window_event_t;
enum{
    WND_EVENT_MOUSE_MOVE=0,
    WND_EVENT_MOUSE_DOWN ,
    WND_EVENT_MOUSE_UP ,
    WND_EVENT_MOUSE_CLICK ,
    WND_EVENT_MOUSE_DOUBLE_CLICK ,
    WND_EVENT_MOUSE_WHEEL ,
    WND_EVENT_KEY_DOWN ,
    WND_EVENT_KEY_UP ,
    WND_EVENT_KEY_PRESS ,
    WND_EVENT_WINDOW_RESIZE ,
    WND_EVENT_WINDOW_MOVE ,
    WND_EVENT_WINDOW_CLOSE ,
    WND_EVENT_WINDOW_FOCUS ,
    WND_EVENT_WINDOW_LOST_FOCUS ,
    WND_EVENT_TIMER,
};
#define WNDEVENT_COUNT 16
typedef void (*window_event_handler_t)(int wnd_id,int event_type,window_event_t* event);
typedef struct _window{
    int x, y, width, height;
    int type;
    char title[128];
    int state;
    struct _window* next_as_child;
    struct _window* prev_as_child;
    struct _window* children;
    struct _window* parent;
    window_event_handler_t event_handlers[WNDEVENT_COUNT];
} window_t;
typedef struct _window_event_listener{
    int wnd_id;
    int listener_id;
    int event_type;
    window_event_handler_t listener;
    struct _window_event_listener* next;
}window_event_listener_t;
#define MAX_WINDOWS 100
#define WNDSTATE_PRESENT (1<<0)
#define WNDSTATE_VISIBLE (1<<1)
#define WNDSTATE_FOCUSED (1<<2)
#define WNDSTATE_DESTROYED (1<<3)

#define WNDTYPE_WINDOW 0
#define WNDTYPE_BUTTON 1
#define WNDTYPE_LABEL 2
#define WNDTYPE_EDITBOX 3
#define WNDTYPE_LISTBOX 4
#define WNDTYPE_COMBOBOX 5
#define WNDTYPE_SCROLLBAR 6
#define WNDTYPE_PROGRESSBAR 7
#define WNDTYPE_SLIDER 8
#define WNDTYPE_MENU 9
#define WNDTYPE_MENUBAR 10
#define WNDTYPE_TOOLBAR 11
#define WNDTYPE_STATUSBAR 12
#define WNDTYPE_TABCONTROL 13
#define WNDTYPE_TREEVIEW 14
/**
    @brief 创建窗口
    @param title 窗口标题
    @param wnd_type 窗口类型
    @return 成功则返回窗口号(>=0)，-1 失败
*/
int create_window(char* title,int wnd_type);
int destroy_window(int wnd_id);
/**
    @brief 将窗口附加到父窗口上
    @param wnd_id 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(int wnd_id,int parent_id);
/**
    @brief 将窗口从父窗口上分离
    @param wnd_id 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(int wnd_id);

int get_window_by_title(char* title);
/**
    @brief 根据坐标获取窗口
    @param x 窗口坐标x
    @param y 窗口坐标y
    @param layer 窗口层级
    @return 成功则返回窗口号(>=0)，-1 失败
*/
int get_window_by_pos(int x,int y,int layer);

int move_window(int wnd_id,int x,int y);
int resize_window(int wnd_id,int width,int height);
int set_window_title(int wnd_id,char* title);
int get_window_title(int wnd_id,char* title);
int set_window_state(int wnd_id,int state);
int get_window_state(int wnd_id,int* state);

int show_window(int wnd_id);
int hide_window(int wnd_id);

/**
    @brief 设置窗口事件处理函数
    @param wnd_id 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(int wnd_id,int event_type,window_event_handler_t handler);
int send_window_event(int wnd_id,window_event_t* event);
/**
    @brief 添加窗口事件监听器
    @param wnd_id 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(int wnd_id,int event_type,window_event_handler_t listener);
/**
    @brief 移除窗口事件监听器
    @param wnd_id 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(int wnd_id,int event_type,int listener_id);

/**
    @brief 处理事件
*/
void deal_events();
#define CHECK_VALID_WNDID(wnd_id) if(wnd_id<0 || wnd_id>=MAX_WINDOWS) return -1;

//对接外设中断的函数
void _on_mouse_down(int x, int y, int button);
void _on_mouse_up(int x, int y, int button);
void _on_mouse_move(int x, int y);
void _on_mouse_click(int wnd_id,int x, int y, int button);
void _on_key_down(int key_code);
void _on_key_up(int key_code);
void _on_key_press(int key_code);
void _on_clock_int();
void _render_windows();

int _add_to_layer_ordered(int wnd_id, int layer);

void default_mouse_move_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_mouse_down_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_mouse_up_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_mouse_double_click_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_mouse_wheel_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_key_down_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_key_up_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_key_press_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_window_resize_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_window_move_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_window_close_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_window_focus_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_window_lost_focus_event_handler(int wnd_id, int event_type, window_event_t* event);
void default_timer_event_handler(int wnd_id, int event_type, window_event_t* event);

int init_wndman();