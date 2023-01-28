//
// Created by Gyuhwan Park on 2022/04/28.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "messages/broker.h"
#include "ulalaca.hpp"

#include "XrdpUlalacaPrivate.hpp"
#include "SocketStream.hpp"

#include "ProjectionThread.hpp"



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
    
    si(nullptr),

    _impl(std::make_unique<XrdpUlalacaPrivate>(this))
{
}

int XrdpUlalaca::lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2, long arg3, long arg4) {
    return _this->_impl->libModEvent(type, arg1, arg2, arg3, arg4);
}

int XrdpUlalaca::lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp) {
    return _this->_impl->libModStart(width, height, bpp);
}

int XrdpUlalaca::lib_mod_set_param(XrdpUlalaca *_this, const char *name, const char *value) {
    return _this->_impl->libModSetParam(name, value);
}

int XrdpUlalaca::lib_mod_connect(XrdpUlalaca *_this) {
    return _this->_impl->libModConnect();
}

int XrdpUlalaca::lib_mod_signal(XrdpUlalaca *_this) {
    return _this->_impl->libModSignal();
}

int XrdpUlalaca::lib_mod_end(XrdpUlalaca *_this) {
    return _this->_impl->libModEnd();
}

int XrdpUlalaca::lib_mod_session_change(XrdpUlalaca *_this, int arg1, int arg2) {
    return _this->_impl->libModSessionChange(arg1, arg2);
}

int XrdpUlalaca::lib_mod_get_wait_objs(XrdpUlalaca *_this, tbus *read_objs, int *rcount, tbus *write_objs, int *wcount,
                                       int *timeout) {
    return _this->_impl->libModGetWaitObjs(read_objs, rcount, write_objs, wcount, timeout);
}

int XrdpUlalaca::lib_mod_check_wait_objs(XrdpUlalaca *_this) {
    return _this->_impl->libModCheckWaitObjs();
}

int XrdpUlalaca::lib_mod_frame_ack(XrdpUlalaca *_this, int flags, int frame_id) {
    return _this->_impl->libModFrameAck(flags, frame_id);
}

int XrdpUlalaca::lib_mod_suppress_output(XrdpUlalaca *_this, int suppress, int left, int top, int right, int bottom) {
    return _this->_impl->libModSuppressOutput(suppress, left, top, right, bottom);
}

int XrdpUlalaca::lib_mod_server_monitor_resize(XrdpUlalaca *_this, int width, int height) {
    return _this->_impl->libModServerMonitorResize(width, height);
}

int XrdpUlalaca::lib_mod_server_monitor_full_invalidate(XrdpUlalaca *_this, int width, int height) {
    return _this->_impl->libModServerMonitorFullInvalidate(width, height);
}

int XrdpUlalaca::lib_mod_server_version_message(XrdpUlalaca *_this) {
    return _this->_impl->libModServerVersionMessage();
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
