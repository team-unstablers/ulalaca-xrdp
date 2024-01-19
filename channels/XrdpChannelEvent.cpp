//
// Created by Gyuhwan Park on 2023/06/05.
//

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif

extern "C" {
#include "defines.h"
};


#include "XrdpChannelEvent.hpp"

namespace ulalaca::channel {

    XrdpChannelEvent::XrdpChannelEvent(int channelId, int flags, size_t size, size_t totalSize,
                                       std::shared_ptr<uint8_t> data) :
            _channelId(channelId),
            _flags(flags),
            _size(size),
            _totalSize(totalSize),
            _data(data) {

    }

    XrdpChannelEvent::XrdpChannelEvent(XrdpEvent &rawEvent) :
            _channelId(LOWORD(rawEvent.param1)),
            _flags(HIWORD(rawEvent.param1)),
            _size((size_t) rawEvent.param2),
            _totalSize((size_t) rawEvent.param4),
            _data() {
        _data = std::shared_ptr<uint8_t>(new uint8_t[_size]);
        memcpy(_data.get(), reinterpret_cast<uint8_t *>(rawEvent.param3), _size);
    }

    int XrdpChannelEvent::channelId() const {
        return _channelId;
    }

    int XrdpChannelEvent::flags() const {
        return _flags;
    }

    size_t XrdpChannelEvent::size() const {
        return _size;
    }

    size_t XrdpChannelEvent::totalSize() const {
        return _totalSize;
    }

    std::shared_ptr<uint8_t> XrdpChannelEvent::data() const {
        return _data;
    }
}