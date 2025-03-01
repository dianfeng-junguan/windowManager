#pragma once

#include "wndman.h"
#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480
#define DEFAULT_WINDOW_TITLE "Window"
typedef struct _window;
typedef struct _window_event{
    int x, y;
    int dx,dy;
    int event_type;
    int mouse_button;
    int key_code;
    int state;
    struct _window* sender;
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
typedef void (*window_event_handler_t)(struct _window* wndptr,int event_type,window_event_t* event);
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
    void (*constructor)(struct _window* wndptr);
    void (*destructor)(struct _window* wndptr);
    void* specific_data;
} window_t;
typedef struct _button_data{
    int pressed;
}button_data_t;
//文本框特有属性
typedef struct _editbox_data{
    char *text;
    long long text_buf_len;
    int cursor_pos;
    int select_start;
    int select_end;
}editbox_data_t;
//滚动条特有属性
typedef struct _scrollbar_data{
    int range;
    int value;
    int small_step;
    int large_step;
    int thumb_size;
    int thumb_pos;
}scrollbar_data_t;
//进度条特有属性
typedef struct _progressbar_data{
    int range;
    int value;
}progressbar_data_t;
//滑块特有属性
typedef struct _slider_data{
    int range;
    int value;
}slider_data_t;
//树控件特有属性
typedef struct _treeview_data{
    int selected_item;
}treeview_data_t;
typedef window_t* windowptr_t;
typedef struct _window_event_listener{
    windowptr_t wndptr;
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
#define WNDSTATE_RESIZABLE (1<<4)
#define WNDSTATE_MOVABLE (1<<5)

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
windowptr_t create_window(char* title,int wnd_type);
int destroy_window(windowptr_t wndptr);
/**
    @brief 将窗口附加到父窗口上
    @param wnd_id 要附加的窗口
    @param parent 父窗口
    @return 0 成功，-1 失败
*/
int attach_window(windowptr_t wndptr,windowptr_t parent_wndptr);
/**
    @brief 将窗口从父窗口上分离
    @param wnd_id 要分离的窗口
    @return 0 成功，-1 失败
*/
int detach_window(windowptr_t wndptr);

windowptr_t get_window_by_title(char* title);
/**
    @brief 根据坐标获取窗口
    @param x 窗口坐标x
    @param y 窗口坐标y
    @param layer 窗口层级
    @return 成功则返回窗口号(>=0)，-1 失败
*/
windowptr_t get_window_by_pos(int x,int y,int layer);

int move_window(windowptr_t wndptr,int x,int y);
int resize_window(windowptr_t wndptr,int width,int height);
int set_window_title(windowptr_t wndptr,char* title);
int get_window_title(windowptr_t wndptr,char* title);
int set_window_state(windowptr_t wndptr,int state);
int get_window_state(windowptr_t wndptr,int* state);

int set_window_text(windowptr_t wndptr,char* text);
int get_window_text(windowptr_t wndptr,char* text,int max_len);

int show_window(windowptr_t wndptr);
int hide_window(windowptr_t wndptr);

/**
    @brief 设置窗口事件处理函数
    @param wnd_id 要设置的窗口
    @param handler 事件处理函数
    @return 0 成功，-1 失败
*/
int set_window_event_handler(windowptr_t wndptr,int event_type,window_event_handler_t handler);
int send_window_event(windowptr_t wndptr,window_event_t* event);
/**
    @brief 添加窗口事件监听器
    @param wnd_id 要添加监听器的窗口
    @param event_type 要监听的事件类型
    @param listener 监听器函数
    @return 返回监听器ID，用于移除监听器
*/
int add_window_event_listener(windowptr_t wndptr,int event_type,window_event_handler_t listener);
/**
    @brief 移除窗口事件监听器
    @param wnd_id 要移除监听器的窗口
    @param event_type 要移除的事件类型
    @param listener_id 要移除的监听器ID
    @return 0 成功，-1 失败
*/
int remove_window_event_listener(windowptr_t wndptr,int event_type,int listener_id);

/**
    @brief 处理事件
*/
void deal_events();
#define CHECK_VALID_WNDID(wnd_id) if(wnd_id<0 || wnd_id>=MAX_WINDOWS) return -1;

/**
    @brief 创建控件，内部函数
*/
windowptr_t _create_control(char* title,int wnd_type);
int _offset_window(windowptr_t wndptr, int dx, int dy);
//对接外设中断的函数
void _on_mouse_down(int x, int y, int button);
void _on_mouse_up(int x, int y, int button);
void _on_mouse_move(int x, int y);
void _on_mouse_click(windowptr_t wndptr,int x, int y, int button);
void _on_key_down(int key_code);
void _on_key_up(int key_code);
void _on_key_press(int key_code);
void _on_clock_int();
void _render_windows();

int _add_to_layer_ordered(windowptr_t wndptr, int layer);

void _wnd_list_add(window_t** list, window_t* wnd);
void _wnd_list_remove(window_t** list, window_t* wnd);

void default_mouse_move_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_mouse_down_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_mouse_up_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_mouse_double_click_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_mouse_wheel_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_key_down_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_key_up_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_key_press_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_window_resize_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_window_move_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_window_close_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_window_focus_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_window_lost_focus_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);
void default_timer_event_handler(windowptr_t wndptr, int event_type, window_event_t* event);

void _wndpresethandler_closebutton_clicked(windowptr_t wndptr, int event_type, window_event_t* event);
int init_wndman();

void _editbox_ctor(windowptr_t wndptr);
void _editbox_dtor(windowptr_t wndptr);