//
// Created by Gyuhwan Park on 2023/06/04.
//

#ifndef XRDP_TUMOD_CLIPBOARDCHANNEL_HPP
#define XRDP_TUMOD_CLIPBOARDCHANNEL_HPP

#include <string>

#include "../../XrdpStream.hpp"

#include "../XrdpChannelEvent.hpp"
#include "../XrdpChannelBase.hpp"

namespace ulalaca::channel {
    class ClipboardChannel: public XrdpChannelBase {
    public:
        static const std::string CHANNEL_NAME;

        explicit ClipboardChannel(XrdpUlalaca *mod);

        virtual bool openChannel() override;
        virtual bool closeChannel() override;

        virtual void handleEvent(const XrdpChannelEvent &event) override;

        virtual void handlePDU(XrdpStream &stream) override;

    protected:
        /**
         * Adds a CLIPRDR_HEADER ([MS-RDPECLIP] 2.2.1) to the data stream
         *
         * The location of the length is stored in the unused 'channel_hdr' field
         * of the data stream. When send_stream_to_clip_channel() is called,
         * we can use update the data length.
         *
         * @param s Output stream
         * @param msg_type Message Type
         * @param msg_flags Message flags
         */
        void prepareHeader(XrdpStream &stream, int msgType, int msgFlags);

        void sendStreamToChannel(XrdpStream &stream);

    private:
        std::shared_ptr<XrdpStream> _dechunkedStream;
    };
}


#endif //XRDP_TUMOD_CLIPBOARDCHANNEL_HPP
