//
// Created by Gyuhwan Park on 2023/06/05.
//

#ifndef XRDP_TUMOD_CHANNELEVENT_HPP
#define XRDP_TUMOD_CHANNELEVENT_HPP

#include <memory>

#include "XrdpEvent.hpp"

namespace ulalaca::channel {
    class XrdpChannelEvent {
    public:
        explicit XrdpChannelEvent(int channelId, int flags, size_t size, size_t totalSize,
                                  std::shared_ptr<uint8_t> data);

        explicit XrdpChannelEvent(XrdpEvent &rawEvent);

        int channelId() const;
        int flags() const;

        size_t size() const;
        size_t totalSize() const;

        std::shared_ptr<uint8_t> data() const;

    private:
        int _channelId;
        int _flags;

        size_t _size;
        size_t _totalSize;

        std::shared_ptr<uint8_t> _data;
    };
}


#endif //XRDP_TUMOD_CHANNELEVENT_HPP
