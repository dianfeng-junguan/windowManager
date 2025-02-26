#pragma once
typedef struct _window{
    int x, y, width, height;
    int type;
    char title[128];
    int state;
    struct _window* next_as_child;
    struct _window* prev_as_child;
    struct _window* children;
    struct _window* parent;
} window_t;
#define MAX_WINDOWS 100
#define WNDSTATE_PRESENT 1<<0
#define WNDSTATE_VISIBLE 1<<1
#define WNDSTATE_FOCUSED 1<<2
#define WNDSTATE_DESTROYED 1<<3

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
    @param wnd 要创建的窗口
    @param title 窗口标题
    @param wnd_type 窗口类型
    @return 0 成功，-1 失败
*/
int create_window(window_t** wnd,char* title,int wnd_type);
int destroy_window(window_t** wnd);
/**
    @brief 将窗口附加到父窗口上
    @param wnd 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(window_t* wnd,window_t* parent);
/**
    @brief 将窗口从父窗口上分离
    @param wnd 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(window_t* wnd);

int get_window_by_title(char* title,window_t** wnd);
int get_window_by_pos(int x,int y,window_t** wnd);

int move_window(window_t** wnd,int x,int y);
int resize_window(window_t** wnd,int width,int height);
int set_window_title(window_t** wnd,char* title);
int get_window_title(window_t* wnd,char* title);
int set_window_state(window_t** wnd,int state);
int get_window_state(window_t* wnd,int* state);

int show_window(window_t* wnd);
int hide_window(window_t* wnd);

typedef struct _window_event{
    int x, y;
    int event_type;
    int mouse_button;
    int key_code;
    int state;
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
};
typedef void (*window_event_handler_t)(window_t* wnd,int event_type,window_event_t* event);
/**
    @brief 设置窗口事件处理函数
    @param wnd 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(window_t** wnd,window_event_handler_t handler);
int send_window_event(window_t* wnd,window_event_t* event);
/**
    @brief 添加窗口事件监听器
    @param wnd 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(window_t** wnd,int event_type,window_event_handler_t listener);
/**
    @brief 移除窗口事件监听器
    @param wnd 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(window_t** wnd,int event_type,int listener_id);