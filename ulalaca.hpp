#ifndef ULALACA_HPP
#define ULALACA_HPP

#include <memory>
#include <functional>
#include <mutex>

#include <thread>

extern "C" {
#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
#include "guid.h"
#include "xrdp_client_info.h"
};

#include "XrdpEvent.hpp"
#include "XrdpTransport.hpp"
#include "XrdpStream.hpp"

#include "UnixSocket.hpp"

#include "UlalacaMessages.hpp"

class ProjectionThread;

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
    int (*server_begin_update)(XrdpUlalaca *v) = nullptr;
    int (*server_end_update)(XrdpUlalaca *v) = nullptr;
    int (*server_fill_rect)(XrdpUlalaca *v, int x, int y, int cx, int cy) = nullptr;
    int (*server_screen_blt)(XrdpUlalaca *v, int x, int y, int cx, int cy,
                             int srcx, int srcy) = nullptr;
    int (*server_paint_rect)(XrdpUlalaca *v, int x, int y, int cx, int cy,
                             char *data, int width, int height, int srcx, int srcy) = nullptr;
    int (*server_set_cursor)(XrdpUlalaca *v, int x, int y, char *data, char *mask) = nullptr;
    int (*server_palette)(XrdpUlalaca *v, int *palette) = nullptr;
    int (*server_msg)(XrdpUlalaca *v, const char *msg, int code) = nullptr;
    int (*server_is_term)(XrdpUlalaca *v) = nullptr;
    int (*server_set_clip)(XrdpUlalaca *v, int x, int y, int cx, int cy) = nullptr;
    int (*server_reset_clip)(XrdpUlalaca *v) = nullptr;
    int (*server_set_fgcolor)(XrdpUlalaca *v, int fgcolor) = nullptr;
    int (*server_set_bgcolor)(XrdpUlalaca *v, int bgcolor) = nullptr;
    int (*server_set_opcode)(XrdpUlalaca *v, int opcode) = nullptr;
    int (*server_set_mixmode)(XrdpUlalaca *v, int mixmode) = nullptr;
    int (*server_set_brush)(XrdpUlalaca *v, int x_origin, int y_origin,
                            int style, char *pattern) = nullptr;
    int (*server_set_pen)(XrdpUlalaca *v, int style,
                          int width) = nullptr;
    int (*server_draw_line)(XrdpUlalaca *v, int x1, int y1, int x2, int y2) = nullptr;
    int (*server_add_char)(XrdpUlalaca *v, int font, int character,
                           int offset, int baseline,
                           int width, int height, char *data) = nullptr;
    int (*server_draw_text)(XrdpUlalaca *v, int font,
                            int flags, int mixmode, int clip_left, int clip_top,
                            int clip_right, int clip_bottom,
                            int box_left, int box_top,
                            int box_right, int box_bottom,
                            int x, int y, char *data, int data_len) = nullptr;
    int (*server_reset)(XrdpUlalaca *v, int width, int height, int bpp) = nullptr;
    int (*server_get_channel_count)(XrdpUlalaca *v) = nullptr;
    int (*server_query_channel)(XrdpUlalaca *v, int index,
                                char *channel_name,
                                int *channel_flags) = nullptr;
    int (*server_get_channel_id)(XrdpUlalaca *v, const char *name) = nullptr;
    int (*server_send_to_channel)(XrdpUlalaca *v, int channel_id,
                                  char *data, int data_len,
                                  int total_data_len, int flags) = nullptr;
    int (*server_bell_trigger)(XrdpUlalaca *v) = nullptr;
    int (*server_chansrv_in_use)(XrdpUlalaca *v) = nullptr;
    /* off screen bitmaps */
    int (*server_create_os_surface)(XrdpUlalaca *v, int rdpindex,
                                    int width, int height) = nullptr;
    int (*server_switch_os_surface)(XrdpUlalaca *v, int rdpindex) = nullptr;
    int (*server_delete_os_surface)(XrdpUlalaca *v, int rdpindex) = nullptr;
    int (*server_paint_rect_os)(XrdpUlalaca *mod, int x, int y,
                                int cx, int cy,
                                int rdpindex, int srcx, int srcy) = nullptr;
    int (*server_set_hints)(XrdpUlalaca *mod, int hints, int mask) = nullptr;
    /* rail */
    int (*server_window_new_update)(XrdpUlalaca *mod, int window_id,
                                    struct rail_window_state_order *window_state,
                                    int flags) = nullptr;
    int (*server_window_delete)(XrdpUlalaca *mod, int window_id) = nullptr;
    int (*server_window_icon)(XrdpUlalaca *mod,
                              int window_id, int cache_entry, int cache_id,
                              struct rail_icon_info *icon_info,
                              int flags) = nullptr;
    int (*server_window_cached_icon)(XrdpUlalaca *mod,
                                     int window_id, int cache_entry,
                                     int cache_id, int flags) = nullptr;
    int (*server_notify_new_update)(XrdpUlalaca *mod,
                                    int window_id, int notify_id,
                                    struct rail_notify_state_order *notify_state,
                                    int flags) = nullptr;
    int (*server_notify_delete)(XrdpUlalaca *mod, int window_id,
                                int notify_id) = nullptr;
    int (*server_monitored_desktop)(XrdpUlalaca *mod,
                                    struct rail_monitored_desktop_order *mdo,
                                    int flags) = nullptr;
    int (*server_set_pointer_ex)(XrdpUlalaca *mod, int x, int y, char *data,
                                 char *mask, int bpp) = nullptr;
    int (*server_add_char_alpha)(XrdpUlalaca *mod, int font, int character,
                                 int offset, int baseline,
                                 int width, int height, char *data) = nullptr;
    int (*server_create_os_surface_bpp)(XrdpUlalaca *v, int rdpindex,
                                        int width, int height, int bpp) = nullptr;
    int (*server_paint_rect_bpp)(XrdpUlalaca *v, int x, int y, int cx, int cy,
                                 char *data, int width, int height,
                                 int srcx, int srcy, int bpp) = nullptr;
    int (*server_composite)(XrdpUlalaca *v, int srcidx, int srcformat,
                            int srcwidth, int srcrepeat, int *srctransform,
                            int mskflags, int mskidx, int mskformat,
                            int mskwidth, int mskrepeat, int op,
                            int srcx, int srcy, int mskx, int msky,
                            int dstx, int dsty, int width, int height,
                            int dstformat) = nullptr;
    int (*server_paint_rects)(XrdpUlalaca *v,
                              int num_drects, short *drects,
                              int num_crects, short *crects,
                              char *data, int width, int height,
                              int flags, int frame_id) = nullptr;
    int (*server_session_info)(XrdpUlalaca *v, const char *data,
                               int data_bytes) = nullptr;
    tintptr server_dumby[100 - 46]; /* align, 100 minus the number of server
                                       functions above */
    /* common */
    tintptr handle; /* pointer to self as int */
    tintptr wm; /* struct xrdp_wm* */
    tintptr painter;
    struct source_info *si;
    
public:
    static const int RECT_SIZE_BYPASS_CREATE = 0;

    constexpr static const int ULALACA_VERSION = 1;
    constexpr static const int NO_ERROR = 0;
    
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
    
    /* session-broker related */
    /** @deprecated */
    inline std::string getSessionSocketPath(std::string &username);

    /* paint related */
    inline int decideCopyRectSize() const;
    inline std::unique_ptr<std::vector<Rect>> createCopyRects(std::vector<Rect> &dirtyRects, int rectSize) const;

    void addDirtyRect(Rect &rect);
    void commitUpdate(const uint8_t *image, int32_t width, int32_t height);

    void calculateSessionSize();

    /* utility methods / lib_server_* wrappers */
    void serverMessage(const char *message, int code);
private:
    int _error = 0;

    Rect _sessionSize;
    std::vector<Rect> _screenLayouts;

    int _bpp;
    
    std::atomic_int64_t _frameId = 0;
    std::atomic_int64_t _ackFrameId = 0;
    
    std::string _username;
    std::string _password;
    std::string _ip;
    std::string _port;
    int _keyLayout;
    int _delayMs;
    guid _guid;
    int _encodingsMask;
    xrdp_client_info _clientInfo;
    
    std::unique_ptr<UnixSocket> _socket;
    std::unique_ptr<ProjectionThread> _projectionThread;
    
    std::mutex _commitUpdateLock;
    
    std::vector<Rect> _dirtyRects;
};

};


#endif