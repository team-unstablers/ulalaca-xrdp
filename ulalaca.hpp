#ifndef ULALACA_HPP
#define ULALACA_HPP

#include <memory>
#include <functional>


#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
#include "guid.h"
#include "xrdp_client_info.h"

struct XrdpEvent {
public:
    enum Type {
        KEY_DOWN = 15,
        KEY_UP = 16,
        KEY_SYNCHRONIZE_LOCK = 17,
        
        MOUSE_MOVE = 100,
        MOUSE_BUTTON_LEFT_UP = 101,
        MOUSE_BUTTON_LEFT_DOWN = 102,
        MOUSE_BUTTON_RIGHT_UP = 103,
        MOUSE_BUTTON_RIGHT_DOWN = 104,
        MOUSE_BUTTON_MIDDLE_UP = 105,
        MOUSE_BUTTON_MIDDLE_DOWN = 106,
        
        MOUSE_WHEEL_UP = 107,
        MOUSE_UNKNOWN_1 = 108,
        MOUSE_WHEEL_DOWN = 109,
        MOUSE_UNKNOWN_2 = 110,
        
        INVALIDATE_REQUEST = 200,
        
        CHANNEL_EVENT = 0x5555
    };
    
    bool isKeyEvent() const;
    bool isMouseEvent() const;
    
    Type type;
    
    long param1;
    long param2;
    long param3;
    long param4;
};


class XrdpStream {
public:
    using Stream = stream;
    using StreamDeleter = std::function<void(Stream *)>;
    
    XrdpStream(size_t size);
    
    explicit operator Stream const *() const;
    
    template<typename T>
    XrdpStream &operator<< (T value);
    
private:
    std::unique_ptr<Stream, StreamDeleter> _stream;
};

extern "C" {
    
struct XrdpUlalaca {
    int size; /* size of this struct */
    int version; /* internal version */
    /* client functions */
    int (*mod_start)(XrdpUlalaca *v, int w, int h, int bpp);
    int (*mod_connect)(XrdpUlalaca *v);
    int (*mod_event)(XrdpUlalaca *v, int msg, long param1, long param2,
                     long param3, long param4);
    int (*mod_signal)(XrdpUlalaca *v);
    int (*mod_end)(XrdpUlalaca *v);
    int (*mod_set_param)(XrdpUlalaca *v, const char *name, const char *value);
    int (*mod_session_change)(XrdpUlalaca *v, int, int);
    int (*mod_get_wait_objs)(XrdpUlalaca *v, tbus *read_objs, int *rcount,
                             tbus *write_objs, int *wcount, int *timeout);
    int (*mod_check_wait_objs)(XrdpUlalaca *v);
    int (*mod_frame_ack)(XrdpUlalaca *v, int flags, int frame_id);
    int (*mod_suppress_output)(XrdpUlalaca *v, int suppress,
                               int left, int top, int right, int bottom);
    int (*mod_server_monitor_resize)(XrdpUlalaca *v,
                                     int width, int height);
    int (*mod_server_monitor_full_invalidate)(XrdpUlalaca *v,
                                              int width, int height);
    int (*mod_server_version_message)(XrdpUlalaca *v);
    tintptr mod_dumby[100 - 14]; /* align, 100 minus the number of mod
                                  functions above */
    /* server functions */
    int (*server_begin_update)(XrdpUlalaca *v);
    int (*server_end_update)(XrdpUlalaca *v);
    int (*server_fill_rect)(XrdpUlalaca *v, int x, int y, int cx, int cy);
    int (*server_screen_blt)(XrdpUlalaca *v, int x, int y, int cx, int cy,
                             int srcx, int srcy);
    int (*server_paint_rect)(XrdpUlalaca *v, int x, int y, int cx, int cy,
                             char *data, int width, int height, int srcx, int srcy);
    int (*server_set_cursor)(XrdpUlalaca *v, int x, int y, char *data, char *mask);
    int (*server_palette)(XrdpUlalaca *v, int *palette);
    int (*server_msg)(XrdpUlalaca *v, const char *msg, int code);
    int (*server_is_term)(XrdpUlalaca *v);
    int (*server_set_clip)(XrdpUlalaca *v, int x, int y, int cx, int cy);
    int (*server_reset_clip)(XrdpUlalaca *v);
    int (*server_set_fgcolor)(XrdpUlalaca *v, int fgcolor);
    int (*server_set_bgcolor)(XrdpUlalaca *v, int bgcolor);
    int (*server_set_opcode)(XrdpUlalaca *v, int opcode);
    int (*server_set_mixmode)(XrdpUlalaca *v, int mixmode);
    int (*server_set_brush)(XrdpUlalaca *v, int x_origin, int y_origin,
                            int style, char *pattern);
    int (*server_set_pen)(XrdpUlalaca *v, int style,
                          int width);
    int (*server_draw_line)(XrdpUlalaca *v, int x1, int y1, int x2, int y2);
    int (*server_add_char)(XrdpUlalaca *v, int font, int character,
                           int offset, int baseline,
                           int width, int height, char *data);
    int (*server_draw_text)(XrdpUlalaca *v, int font,
                            int flags, int mixmode, int clip_left, int clip_top,
                            int clip_right, int clip_bottom,
                            int box_left, int box_top,
                            int box_right, int box_bottom,
                            int x, int y, char *data, int data_len);
    int (*server_reset)(XrdpUlalaca *v, int width, int height, int bpp);
    int (*server_get_channel_count)(XrdpUlalaca *v);
    int (*server_query_channel)(XrdpUlalaca *v, int index,
                                char *channel_name,
                                int *channel_flags);
    int (*server_get_channel_id)(XrdpUlalaca *v, const char *name);
    int (*server_send_to_channel)(XrdpUlalaca *v, int channel_id,
                                  char *data, int data_len,
                                  int total_data_len, int flags);
    int (*server_bell_trigger)(XrdpUlalaca *v);
    int (*server_chansrv_in_use)(XrdpUlalaca *v);
    tintptr server_dumby[100 - 27]; /* align, 100 minus the number of server
                                     functions above */
    
public:
    constexpr static int ULALACA_VERSION = 1;
    constexpr static int NO_ERROR = 0;
    
    explicit XrdpUlalaca();
    
    static int lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp);
    static int lib_mod_connect(XrdpUlalaca *_this);
    static int lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2,
                             long arg3, long arg4);
    static int lib_mod_signal(XrdpUlalaca *_this);
    static int lib_mod_end(XrdpUlalaca *_this);
    static int lib_mod_set_param(XrdpUlalaca *_this, const char *name, const char *value);
    static int lib_mod_session_change(XrdpUlalaca *_this, int, int);
    static int lib_mod_get_wait_objs(XrdpUlalaca *_this, tbus *read_objs, int *rcount,
                             tbus *write_objs, int *wcount, int *timeout);
    static int lib_mod_check_wait_objs(XrdpUlalaca *_this);
    static int lib_mod_frame_ack(XrdpUlalaca *_this, int flags, int frame_id);
    static int lib_mod_suppress_output(XrdpUlalaca *_this, int suppress,
                               int left, int top, int right, int bottom);
    static int lib_mod_server_monitor_resize(XrdpUlalaca *_this,
                                     int width, int height);
    static int lib_mod_server_monitor_full_invalidate(XrdpUlalaca *_this,
                                              int width, int height);
    static int lib_mod_server_version_message(XrdpUlalaca *_this);
    
    /* server functions */
    static int lib_server_begin_update(XrdpUlalaca *_this);
    static int lib_server_end_update(XrdpUlalaca *_this);
    static int lib_server_fill_rect(XrdpUlalaca *_this, int x, int y, int cx, int cy);
    static int lib_server_screen_blt(XrdpUlalaca *_this, int x, int y, int cx, int cy,
                             int srcx, int srcy);
    static int lib_server_paint_rect(XrdpUlalaca *_this, int x, int y, int cx, int cy,
                             char *data, int width, int height, int srcx, int srcy);
    static int lib_server_set_cursor(XrdpUlalaca *_this, int x, int y, char *data, char *mask);
    static int lib_server_palette(XrdpUlalaca *_this, int *palette);
    static int lib_server_msg(XrdpUlalaca *_this, const char *msg, int code);
    static int lib_server_is_term(XrdpUlalaca *_this);
    static int lib_server_set_clip(XrdpUlalaca *_this, int x, int y, int cx, int cy);
    static int lib_server_reset_clip(XrdpUlalaca *_this);
    static int lib_server_set_fgcolor(XrdpUlalaca *_this, int fgcolor);
    static int lib_server_set_bgcolor(XrdpUlalaca *_this, int bgcolor);
    static int lib_server_set_opcode(XrdpUlalaca *_this, int opcode);
    static int lib_server_set_mixmode(XrdpUlalaca *_this, int mixmode);
    static int lib_server_set_brush(XrdpUlalaca *_this, int x_origin, int y_origin,
                            int style, char *pattern);
    static int lib_server_set_pen(XrdpUlalaca *_this, int style,
                          int width);
    static int lib_server_draw_line(XrdpUlalaca *_this, int x1, int y1, int x2, int y2);
    static int lib_server_add_char(XrdpUlalaca *_this, int font, int character,
                           int offset, int baseline,
                           int width, int height, char *data);
    static int lib_server_draw_text(XrdpUlalaca *_this, int font,
                            int flags, int mixmode, int clip_left, int clip_top,
                            int clip_right, int clip_bottom,
                            int box_left, int box_top,
                            int box_right, int box_bottom,
                            int x, int y, char *data, int data_len);
    static int lib_server_reset(XrdpUlalaca *_this, int width, int height, int bpp);
    static int lib_server_get_channel_count(XrdpUlalaca *_this);
    static int lib_server_query_channel(XrdpUlalaca *_this, int index,
                                char *channel_name,
                                int *channel_flags);
    static int lib_server_get_channel_id(XrdpUlalaca *_this, const char *name);
    static int lib_server_send_to_channel(XrdpUlalaca *_this, int channel_id,
                                  char *data, int data_len,
                                  int total_data_len, int flags);
    static int lib_server_bell_trigger(XrdpUlalaca *_this);
    static int lib_server_chansrv_in_use(XrdpUlalaca *_this);
    
    int handleEvent(XrdpEvent &event);
    int updateDisplay();
    
    void serverMessage(const char *message, int code);
    
    
private:
    std::string _username;
    std::string _password;
    std::string _ip;
    std::string _port;
    int _keyLayout;
    int _delayMs;
    guid _guid;
    int _encodingsMask;
    xrdp_client_info _clientInfo;
};

};

#endif ULALACA_HPP