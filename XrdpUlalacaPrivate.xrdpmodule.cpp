#include <sstream>

#include "XrdpUlalacaPrivate.hpp"

#include "ulalaca.hpp"
#include "SessionBrokerClient.hpp"
#include "ProjectorClient.hpp"

#include "messages/broker.h"

int XrdpUlalacaPrivate::libModStart(int width, int height, int bpp) {
    // #517eb9
    constexpr const unsigned int BACKGROUND_COLOR = 0xb97e51;

    setSessionSize(width, height);

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

    try {
        serverMessage("requesting session with given credentials...", 0);

        SessionBrokerClient brokerClient(sessionBrokerIPCPath);
        auto response = brokerClient.requestSession(_username, _password);

        std::fill(_password.begin(), _password.end(), '\0');
        _password.clear();

        if (!response.isSuccessful) {
            std::stringstream sstream;
            sstream << "failed to request session for user " << _username << ": ";

            switch (response.reason) {
                case REJECT_REASON_AUTHENTICATION_FAILED:
                    sstream << "authentication failed (code 1)";
                    break;
                case REJECT_REASON_SESSION_NOT_AVAILABLE:
                    sstream << "session not available; is sessionprojector.app running? (code 2)";
                    break;
                case REJECT_REASON_INCOMPATIBLE_VERSION:
                    sstream << "incompatible IPC protocol version (code 3)";
                    break;
                default:
                    sstream << "unknown error (code " << response.reason << ")";
                    break;
            }

            std::string message = sstream.str();
            serverMessage(message.c_str(), 0);

            return 1;
        }


        serverMessage("attaching to session..", 0);

        // TODO: try-catch
        attachToSession(response.sessionPath);

        return 0;
    } catch (SystemCallException &e) {
        std::stringstream sstream;
        sstream << "caught SystemCallException: " << e.what() << " (errno=" << e.getErrno() << ")";
        std::string message = sstream.str();

        serverMessage(message.c_str(), 0);
    }

    return 1;
}

int XrdpUlalacaPrivate::libModEvent(int type, long arg1, long arg2, long arg3, long arg4) {
    XrdpEvent event {
            (XrdpEvent::Type) type,
            arg1, arg2, arg3, arg4
    };

    if (_projectorClient != nullptr) {
        _projectorClient->handleEvent(event);
    }

    return 0;
}

int XrdpUlalacaPrivate::libModSignal() {
    LOG(LOG_LEVEL_INFO, "lib_mod_signal() called");
    return 0;
}

int XrdpUlalacaPrivate::libModEnd() {
    LOG(LOG_LEVEL_INFO, "lib_mod_end() called");

    if (_projectorClient != nullptr) {
        _projectorClient->stop();
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
    // FIXME
    if (!_isUpdateThreadRunning) {
        return 0;
    }

    if (_error != 0) {
        return 1;
    }

    readObjs[(*rcount)++] = _projectorClient->descriptor();
    writeObjs[(*wcount)++] = _projectorClient->descriptor();

    return 0;
}

int XrdpUlalacaPrivate::libModCheckWaitObjs() {
    // TODO: move ipcConnection.read()/write() to here..?
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
