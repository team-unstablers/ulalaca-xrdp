//
// Created by Gyuhwan Park on 2023/06/04.
//

#ifndef XRDP_TUMOD_CLIPBOARDCHANNEL_HPP
#define XRDP_TUMOD_CLIPBOARDCHANNEL_HPP

#include <string>

#include "XrdpStream.hpp"
#include "XrdpChannelEvent.hpp"

class XrdpUlalaca;

/**
 * TODO: split this class into XrdpChannel (base) and ClipboardChannel
 */
class ClipboardChannel {
public:
    static const std::string CHANNEL_NAME;

    explicit ClipboardChannel(XrdpUlalaca *mod);

    bool openChannel();
    bool closeChannel();

    bool canHandle(const XrdpChannelEvent &event) const;
    void handleEvent(const XrdpChannelEvent &event);

    void handlePDU(XrdpStream &stream);

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
    XrdpUlalaca *_mod;

    bool _isChannelOpened;
    int _channelId;

    std::shared_ptr<XrdpStream> _dechunkedStream;
};


#endif //XRDP_TUMOD_CLIPBOARDCHANNEL_HPP
