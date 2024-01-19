//
// Created by Gyuhwan Park on 1/19/24.
//

#include "XrdpChannelBase.hpp"

namespace ulalaca::channel {
    XrdpChannelBase::XrdpChannelBase(XrdpUlalaca *mod, const std::string &channelName):
            _mod(mod),
            _channelName(channelName),
            _channelId(-1),
            _isChannelOpen(false)
    {
    }

    const std::string &XrdpChannelBase::channelName() const {
        return _channelName;
    }

    int XrdpChannelBase::channelId() const {
        return _channelId;
    }

    void XrdpChannelBase::setChannelId(int channelId) {
        _channelId = channelId;
    }

    bool XrdpChannelBase::isChannelOpen() const {
        return _isChannelOpen;
    }

    void XrdpChannelBase::setChannelOpen(bool isChannelOpen) {
        _isChannelOpen = isChannelOpen;
    }

    XrdpUlalaca *XrdpChannelBase::mod() const {
        return _mod;
    }

    bool XrdpChannelBase::canHandle(const XrdpChannelEvent &event) const {
        return event.channelId() == channelId();
    }


}