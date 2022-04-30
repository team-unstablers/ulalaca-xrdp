//
// Created by Gyuhwan Park on 2022/04/28.
//

#include <string>
#include <functional>

#include <AVFoundation/AVCaptureSession.h>


#include "ulalaca.hpp"
#include "XrdpStream.template.cpp"

bool XrdpEvent::isKeyEvent() const {
    return type == KEY_DOWN || type == KEY_UP;
}

bool XrdpEvent::isMouseEvent() const {
    return type >= MOUSE_MOVE && type <= MOUSE_UNKNOWN_2;
}

XrdpUlalaca::XrdpUlalaca():
    size(sizeof(XrdpUlalaca)),
    version(ULALACA_VERSION),
    
    mod_start(&lib_mod_start),
    mod_connect(&lib_mod_connect),
    mod_event(&lib_mod_event),
    mod_signal(&lib_mod_signal),
    mod_end(&lib_mod_end),
    mod_set_param(&lib_mod_set_param),
    mod_session_change(&lib_mod_session_change),
    mod_get_wait_objs(&lib_mod_get_wait_objs),
    mod_check_wait_objs(&lib_mod_check_wait_objs),
    mod_frame_ack(&lib_mod_frame_ack),
    mod_suppress_output(&lib_mod_suppress_output),
    mod_server_monitor_resize(&lib_mod_server_monitor_resize),
    mod_server_monitor_full_invalidate(&lib_mod_server_monitor_full_invalidate),
    mod_server_version_message(&lib_mod_server_version_message),
    
    server_begin_update(&lib_server_begin_update),
    server_end_update(&lib_server_end_update),
    server_fill_rect(&lib_server_fill_rect),
    server_screen_blt(&lib_server_screen_blt),
    server_paint_rect(&lib_server_paint_rect),
    server_set_cursor(&lib_server_set_cursor),
    server_palette(&lib_server_palette),
    server_msg(&lib_server_msg),
    server_is_term(&lib_server_is_term),
    server_set_clip(&lib_server_set_clip),
    server_reset_clip(&lib_server_reset_clip),
    server_set_fgcolor(&lib_server_set_fgcolor),
    server_set_bgcolor(&lib_server_set_bgcolor),
    server_set_opcode(&lib_server_set_opcode),
    server_set_mixmode(&lib_server_set_mixmode),
    server_set_brush(&lib_server_set_brush),
    server_set_pen(&lib_server_set_pen),
    server_draw_line(&lib_server_draw_line),
    server_add_char(&lib_server_add_char),
    server_draw_text(&lib_server_draw_text),
    server_reset(&lib_server_reset),
    server_get_channel_count(&lib_server_get_channel_count),
    server_query_channel(&lib_server_query_channel),
    server_get_channel_id(&lib_server_get_channel_id),
    server_send_to_channel(&lib_server_send_to_channel),
    server_bell_trigger(&lib_server_bell_trigger),
    server_chansrv_in_use(&lib_server_chansrv_in_use)
{
}

int XrdpUlalaca::handleEvent(XrdpEvent &event) {
    if (event.isKeyEvent()) {
        auto keyCode = event.param2;
        return NO_ERROR;
    } else if (event.type == XrdpEvent::KEY_SYNCHRONIZE_LOCK) {
        auto lockStatus = event.param1;
        return NO_ERROR;
    }
    
    if (event.isMouseEvent()) {
        uint16_t posX = event.param1;
        uint16_t posY = event.param2;
        
        
        return NO_ERROR;
    }
    
    
    if (event.type == XrdpEvent::INVALIDATE_REQUEST) {
        uint16_t x1 = HIWORD(event.param1);
        uint16_t y1 = LOWORD(event.param1);
        uint16_t x2 = HIWORD(event.param2);
        uint16_t y2 = LOWORD(event.param2);
    
        // TODO: redraw(rect)?
        return NO_ERROR;
    }
    
    if (event.type == XrdpEvent::CHANNEL_EVENT) {
        uint16_t channelId = LOWORD(event.param1);
        uint16_t flags = HIWORD(event.param1);
        auto size = (int) event.param2;
        auto data = (char *) event.param3;
        auto total_size = (int) event.param4;
        
        return NO_ERROR;
    }
    
    return NO_ERROR;
}

int XrdpUlalaca::updateDisplay() {
    XrdpStream stream(8192);
    
}

int XrdpUlalaca::lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2, long arg3, long arg4) {
    constexpr static const int NO_ERROR = 0;
    constexpr static const int ERROR = 1;
    
    XrdpEvent event {
        (XrdpEvent::Type) type,
        arg1, arg2, arg3, arg4
    };
    
    return _this->handleEvent(event);
}

int XrdpUlalaca::lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp) {
    constexpr const int BACKGROUND_COLOR = 0xff517eb9;
    
    _this->server_begin_update(_this);
    _this->server_set_fgcolor(_this, BACKGROUND_COLOR);
    _this->server_fill_rect(_this, 0, 0, width, height);
    _this->server_end_update(_this);
    
    // _this->updateBpp(bpp);
}

int XrdpUlalaca::lib_mod_set_param(XrdpUlalaca *_this, const char *_name, const char *_value) {
    std::string name(_name);
    std::string value(_value);
    
    if (name == "username") {
        _this->_username = value;
    } else if (name == "password") {
        _this->_password = value;
    } else if (name == "ip") {
        _this->_ip = value;
    } else if (name == "port") {
        _this->_port = value;
    } else if (name == "keylayout") {
        _this->_keyLayout = std::stoi(value);
    } else if (name == "delay_ms") {
        _this->_delayMs = std::stoi(value);
    } else if (name == "guid") {
        auto *_guid = reinterpret_cast<const guid *>(_value);
        _this->_guid = *_guid;
    } else if (name == "disabled_encodings_mask") {
        _this->_encodingsMask = ~std::stoi(value);
    } else if (name == "client_info") {
        auto *clientInfo = reinterpret_cast<const xrdp_client_info *>(_value);
        _this->_clientInfo = *clientInfo;
    }
    
    return 0;
}

int XrdpUlalaca::lib_mod_connect(XrdpUlalaca *_this) {
    _this->serverMessage("establishing connection to SessionProjector", 0);
    
    
    _this->serverMessage("welcome to the fantasy zone, get ready!", 0);
}

void XrdpUlalaca::serverMessage(const char *message, int code) {
    this->server_msg(this, message, code);
    LOG(LOG_LEVEL_INFO, message);
}

tintptr EXPORT_CC mod_init(void) {
    auto *ulalaca = (XrdpUlalaca *) g_malloc(sizeof(XrdpUlalaca), 1);
    new (ulalaca) XrdpUlalaca();
    
    return (tintptr) ulalaca;
}

int EXPORT_CC mod_exit(tintptr handle) {
    auto *ulalaca = (XrdpUlalaca *) handle;
    LOG(LOG_LEVEL_TRACE, "Ulalaca: mod_exit()");
    
    if (ulalaca == nullptr) {
        LOG(LOG_LEVEL_WARNING, "Ulalaca: mod_exit(): handle is nullptr");
        return 0;
    }
    
    // trans_delete(ulalaca->trans);
    
    // call destructor manually
    ulalaca->~XrdpUlalaca();
    g_free(ulalaca);
    
    // TODO: cleanup
    
    return 0;
}