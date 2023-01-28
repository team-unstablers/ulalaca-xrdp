#include "XrdpUlalacaPrivate.hpp"

#include "ulalaca.hpp"
#include "SessionBrokerClient.hpp"
#include "ProjectorClient.hpp"

int XrdpUlalacaPrivate::libModStart(int width, int height, int bpp) {
    // #517eb9
    constexpr const unsigned int BACKGROUND_COLOR = 0xb97e51;

    _screenLayouts.clear();
    _screenLayouts.emplace_back(ULIPCRect {
            0, 0, (short) width, (short) height
    });
    calculateSessionSize();

    _mod->server_begin_update(_mod);
    _mod->server_set_fgcolor(_mod, (int) BACKGROUND_COLOR);
    _mod->server_fill_rect(_mod, 0, 0, width, height);
    _mod->server_end_update(_mod);

    /*
    if (!isBPPSupported(bpp)) {
        return 1;
    }
     */

    _bpp = bpp;
    // _this->updateBpp(bpp);

    return 0;
}

int XrdpUlalacaPrivate::libModConnect() {
    const std::string sessionBrokerIPCPath("/var/run/ulalaca_broker.sock");

    SessionBrokerClient brokerClient(sessionBrokerIPCPath);
    auto response = brokerClient.requestSession(_username, _password);

    std::fill(_password.begin(), _password.end(), '\0');
    _password.clear();

    if (!response.isSuccessful) {
        serverMessage("failed to request session", response.reason);
        LOG(LOG_LEVEL_ERROR, "failed to request session: code %d (user %s)", response.reason, _username.c_str());
        return 1;
    }

    // TODO: try-catch
    attachToSession(response.sessionPath);
    return 0;
}

int XrdpUlalacaPrivate::libModEvent(int type, long arg1, long arg2, long arg3, long arg4) {
    XrdpEvent event {
            (XrdpEvent::Type) type,
            arg1, arg2, arg3, arg4
    };

    if (_projectionThread != nullptr) {
        _projectionThread->handleEvent(event);
    }

    return 0;
}

int XrdpUlalacaPrivate::libModSignal() {
    LOG(LOG_LEVEL_INFO, "lib_mod_signal() called");
    return 0;
}

int XrdpUlalacaPrivate::libModEnd() {
    LOG(LOG_LEVEL_INFO, "lib_mod_end() called");

    if (_projectionThread != nullptr) {
        _projectionThread->stop();
    }

    return 0;
}

int XrdpUlalacaPrivate::libModSetParam(const char *cstrName, const char *cstrValue) {
    std::string name(cstrName);
    std::string value(cstrValue);

    if (name == "username") {
        _username = value;
    } else if (name == "password") {
        _password = value;
    } else if (name == "ip") {
        _ip = value;
    } else if (name == "port") {
        _port = value;
    } else if (name == "keylayout") {
        _keyLayout = std::stoi(value);
    } else if (name == "delay_ms") {
        _delayMs = std::stoi(value);
    } else if (name == "guid") {
        auto *guid = reinterpret_cast<const struct guid *>(cstrValue);
        _guid = *guid;
    } else if (name == "disabled_encodings_mask") {
        _encodingsMask = ~std::stoi(value);
    } else if (name == "client_info") {
        auto *clientInfo = reinterpret_cast<const struct xrdp_client_info *>(cstrValue);
        _clientInfo = *clientInfo;
    }

    return 0;
}

int XrdpUlalacaPrivate::libModSessionChange(int, int) {
    return 0;
}

int XrdpUlalacaPrivate::libModGetWaitObjs(tbus *readObjs, int *rcount, tbus *writeObjs, int *wcount, int *timeout) {
    return 0;
}

int XrdpUlalacaPrivate::libModCheckWaitObjs() {
    return 0;
}

int XrdpUlalacaPrivate::libModFrameAck(int flags, int frameId) {
    LOG(LOG_LEVEL_TRACE, "lib_mod_frame_ack() called: %d", frameId);
    _ackFrameId = frameId;

    return 0;
}

int XrdpUlalacaPrivate::libModSuppressOutput(int suppress, int left, int top, int right, int bottom) {
    return 0;
}

int XrdpUlalacaPrivate::libModServerMonitorResize(int width, int height) {
    return 0;
}

int XrdpUlalacaPrivate::libModServerMonitorFullInvalidate(int width, int height) {
    _fullInvalidate = true;
    return 0;
}

int XrdpUlalacaPrivate::libModServerVersionMessage() {
    return 0;
}
