//
// Created by Gyuhwan Park on 1/19/24.
//

#ifndef XRDP_TUMOD_XRDPCHANNELBASE_HPP
#define XRDP_TUMOD_XRDPCHANNELBASE_HPP

#include <string>

#include "../XrdpStream.hpp"
#include "XrdpChannelEvent.hpp"

// Forward declaration
class XrdpUlalaca;

namespace ulalaca::channel {
    class XrdpChannelBase {
    public:
        explicit XrdpChannelBase(XrdpUlalaca *mod,
                                 const std::string &channelName);

        const std::string &channelName() const;
        int channelId() const;
        bool isChannelOpen() const;

        virtual bool openChannel() = 0;
        virtual bool closeChannel() = 0;

        virtual bool canHandle(const XrdpChannelEvent &event) const;
        virtual void handleEvent(const XrdpChannelEvent &event) = 0;

    protected:
        XrdpUlalaca *mod() const;

        void setChannelId(int channelId);
        void setChannelOpen(bool isChannelOpen);

    private:
        XrdpUlalaca *_mod;

        std::string _channelName;
        int _channelId;
        bool _isChannelOpen;
    };
}

#endif //XRDP_TUMOD_XRDPCHANNELBASE_HPP
