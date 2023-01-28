#include "XrdpUlalacaPrivate.hpp"
#include "ulalaca.hpp"

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

    if (!isBPPSupported(bpp)) {
        return 1;
    }

    _bpp = bpp;
    // _this->updateBpp(bpp);

    return 0;
}

int XrdpUlalacaPrivate::libModConnect() {
    try {
        std::string socketPath = _this->getSessionSocketPathUsingCredential(
                _this->_username, _this->_password
        );
        _this->_password.clear();

        if (socketPath.empty()) {
            return 1;
        }

        _this->_projectionThread = std::make_unique<ProjectionThread>(
                *_this, socketPath
        );

        _this->_projectionThread->start();

        LOG(LOG_LEVEL_TRACE, "sessionSize: %d, %d", _this->_sessionSize.width, _this->_sessionSize.height);
        _this->_projectionThread->setViewport(_this->_sessionSize);
        _this->_projectionThread->setOutputSuppression(false);
    } catch (SystemCallException &e) {
        _this->serverMessage(e.what(), 0);
        return 1;
    }

    _this->serverMessage("welcome to the fantasy zone, get ready!", 0);

    return 0;
}

int XrdpUlalacaPrivate::libModEvent(int type, long arg1, long arg2, long arg3, long arg4) {
    XrdpEvent event {
            (XrdpEvent::Type) type,
            arg1, arg2, arg3, arg4
    };

    if (_projectionThread != nullptr) {
        _projectionThread->handleEvent(std::move(event));
    }

    return 0;
}

int XrdpUlalacaPrivate::libModSignal() {
    LOG(LOG_LEVEL_INFO, "lib_mod_signal() called");
    return 0;
}

int XrdpUlalacaPrivate::libModEnd() {
    LOG(LOG_LEVEL_INFO, "lib_mod_end() called");

    if (_this->_projectionThread != nullptr) {
        _this->_projectionThread->stop();
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
        auto *_guid = reinterpret_cast<const guid *>(value);
        _guid = *_guid;
    } else if (name == "disabled_encodings_mask") {
        _encodingsMask = ~std::stoi(value);
    } else if (name == "client_info") {
        auto *clientInfo = reinterpret_cast<const xrdp_client_info *>(value);
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
    LOG(LOG_LEVEL_TRACE, "lib_mod_frame_ack() called: %d", frame_id);
    _this->_ackFrameId = frame_id;

    return 0;
}

int XrdpUlalacaPrivate::libModSuppressOutput(int suppress, int left, int top, int right, int bottom) {
    return 0;
}

int XrdpUlalacaPrivate::libModServerMonitorResize(int width, int height) {
    return 0;
}

int XrdpUlalacaPrivate::libModServerMonitorFullInvalidate(int width, int height) {
    _this->_fullInvalidate = true;
    return 0;
}

int XrdpUlalacaPrivate::libModServerVersionMessage() {
    return 0;
}
