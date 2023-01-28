//
// Created by Gyuhwan Park on 2023/01/28.
//

#include "XrdpUlalacaPrivate.hpp"

#include "ProjectionThread.hpp"

XrdpUlalacaPrivate::XrdpUlalacaPrivate(XrdpUlalaca *mod):
    _mod(mod),
    _error(0),

    _sessionSize(),
    _screenLayouts(),

    _bpp(0),

    _frameId(0),
    _ackFrameId(0),

    _username(),
    _password(),
    _ip(),
    _port(),

    _keyLayout(0),
    _delayMs(0),
    _guid(),
    _encodingsMask(0),
    _clientInfo(),

    _socket(),
    _projectionThread(),

    _fullInvalidate(false),
    _commitUpdateLock(),

    _dirtyRects()
{

}


