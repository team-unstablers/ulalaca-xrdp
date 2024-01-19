//
// Created by Gyuhwan Park on 2023/06/04.
//

#include "ClipboardChannel.hpp"

#include "ms-rdpeclip.h"

#include "ulalaca.hpp"

namespace ulalaca::channel {

    const std::string ClipboardChannel::CHANNEL_NAME = "cliprdr";

    ClipboardChannel::ClipboardChannel(XrdpUlalaca *mod):
        XrdpChannelBase(mod, CHANNEL_NAME)
    {

    }

    bool ClipboardChannel::openChannel() {
        if (isChannelOpen()) {
            LOG(LOG_LEVEL_ERROR, "Clipboard channel is already open");
            return true;
        }

        int channelId = mod()->server_get_channel_id(mod(), CHANNEL_NAME.c_str());

        if (channelId < 0) {
            LOG(LOG_LEVEL_ERROR, "Clipboard is not available");
            return false;
        }

        setChannelId(channelId);

        /**
         * Send two PDUs to initialise the channel. The client should
         * respond with a CB_CLIP_CAPS PDU of its own. See [MS-RDPECLIP]
         * 1.3.2.1
         */
        {
            XrdpStream clipCapsPDU(8192);

            // set header
            prepareHeader(clipCapsPDU, CB_CLIP_CAPS, 0);

            clipCapsPDU << (uint16_t) 1; // #cCapabilitiesSets
            clipCapsPDU << (uint16_t) 0; // pad1

            /* CLIPRDR_GENERAL_CAPABILITY */

            clipCapsPDU << (uint16_t) CB_CAPSTYPE_GENERAL; // capabilitySetType
            clipCapsPDU << (uint16_t) 12; // lengthCapability
            clipCapsPDU << (uint32_t) CB_CAPS_VERSION_2;
            clipCapsPDU << (uint32_t) CB_USE_LONG_FORMAT_NAMES;
            clipCapsPDU.markEnd();

            sendStreamToChannel(clipCapsPDU);
        }

        {
            // Send the monitor ready PDU
            XrdpStream monitorReadyPDU(8192);
            prepareHeader(monitorReadyPDU, CB_MONITOR_READY, 0);
            monitorReadyPDU.markEnd();

            sendStreamToChannel(monitorReadyPDU);
        }

        setChannelOpen(true);
        return true;
    }

    void ClipboardChannel::handleEvent(const XrdpChannelEvent &event) {
        if (!canHandle(event)) {
            return;
        }

        bool isFirst = (event.flags() & XR_CHANNEL_FLAG_FIRST) != 0;
        bool isLast = (event.flags() & XR_CHANNEL_FLAG_LAST) != 0;

        if (event.size() > event.totalSize()) {
            LOG(LOG_LEVEL_ERROR, "Ignoring bad PDU chunk (size=%d totalSize=%d)",
                event.size(), event.totalSize());
        }

        if (isFirst && _dechunkedStream != nullptr) {
            // illegal state
            LOG(LOG_LEVEL_ERROR, "Received first chunk while previous chunk is not processed");
            _dechunkedStream = nullptr;
        } else if (!isFirst && _dechunkedStream == nullptr) {
            // illegal state
            LOG(LOG_LEVEL_ERROR, "Received non-first chunk while previous chunk is not processed");
            _dechunkedStream = nullptr;
            return;
        }

        if (isFirst) {
            _dechunkedStream = std::make_unique<XrdpStream>(event.totalSize());

            /* MS-RDPBCGR 3.1.5.2.2.1 states:-
             *
             *     A reassembly buffer MUST be created by the virtual channel
             *     endpoint using the size specified by totalLength when
             *     the first chunk is received.
             *
             * The 'total_size' can be several GB in size, so we really need
             * to check for an allocation failure here */
            if (_dechunkedStream->begin() == nullptr) {
                LOG(LOG_LEVEL_ERROR,
                    "Memory exhausted dechunking a %u byte virtual channel PDU",
                    event.totalSize());
                _dechunkedStream = nullptr;
                return;
            }
        } else {
            if (_dechunkedStream->distance() < event.size()) {
                LOG(LOG_LEVEL_ERROR, "Received chunk is too large");
                _dechunkedStream = nullptr;
                return;
            }
        }

        _dechunkedStream->write(event.data().get(), event.size());

        if (isLast) {
            _dechunkedStream->markEnd();
            _dechunkedStream->seekToBegin();

            handlePDU(*_dechunkedStream);
            _dechunkedStream = nullptr;
        }
    }

    int ClipboardChannel::handlePDU(XrdpStream &stream) {
        // Ignore PDUs with no complete header
        if (_dechunkedStream->size() < 8) {
            LOG(LOG_LEVEL_ERROR, "Incomplete header");
            return 1;
        }

        int type = stream.readUInt16();
        int flags = stream.readUInt16();
        int dataLength = (int) stream.readUInt32();

        LOG(LOG_LEVEL_DEBUG, "got clip data type %d msg_flags %d length %d",
            type, flags, dataLength);

        // Check the PDU is contained in the stream
        if (stream.distance() < dataLength) {
            LOG(LOG_LEVEL_WARNING, "Incomplete PDU");

            dataLength = (int) stream.distance();

            // ??? i can't understand this part; is it ok to truncate the 'incomplete' PDU?
            // return;
        } else {
            /* Truncate the PDU to the data length so we can use the
             * normal functions to parse the PDU */

            // s->end = s->p + datalen;
            stream.seekAbsolute(dataLength);
            stream.markEnd();
            stream.seekToBegin();
        }

        switch (type) {
            case CB_FORMAT_LIST:
                return handleFormatList(flags, stream);
                break;
            case CB_FORMAT_LIST_RESPONSE:
                // We don't need to do anything with this
                break;
            case CB_FORMAT_DATA_REQUEST:
                return handleFormatDataRequest(stream);
                break;
            case CB_FORMAT_DATA_RESPONSE:
                if (flags == CB_RESPONSE_OK) {
                    return handleFormatDataResponse(stream);
                }
                break;
            case CB_CLIP_CAPS:
                return handleCbCaps(stream);
                break;
            default:
                break;
        }

        return 0;
    }

    int ClipboardChannel::handleFormatList(int msgFlags, XrdpStream &stream) {

        /* This is the last stage of the startup sequence in MS-RDPECLIP 1.3.2.1,
         * although it does occur at other times */
        _isStartupComplete = true;

        XrdpStream outStream(64);
        prepareHeader(outStream, CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK);
        outStream.markEnd();

        sendStreamToChannel(stream);

        /* Send a CB_DATA_REQUEST message to the cliprdr channel,
         * if a suitable text format is available */
        int format = findPreferredTextFormat(msgFlags, stream);

        if (format != 0) {
            /*
            LOG_DEVEL(LOG_LEVEL_INFO,
                      "Asking RDP client for clip data format=%s",
                      cf2text(format, scratch, sizeof(scratch)));
             */

            _requestClipFormat = format;
            _activeDataRequests++;

            XrdpStream dataRequestStream(64);
            prepareHeader(dataRequestStream, CB_FORMAT_DATA_REQUEST, 0);
            dataRequestStream << (uint32_t) format;
            dataRequestStream.markEnd();

            sendStreamToChannel(dataRequestStream);
        }

        return 0;
    }

    int ClipboardChannel::findPreferredTextFormat(int msgFlags, XrdpStream &stream) {
        bool seenCfUnicodeText = false;
        bool seenCfText = false;

        int formatId;

        while (stream.checkDistance(4)) {
            formatId = stream.readUInt32();

            if ((_capabilityFlags & CB_USE_LONG_FORMAT_NAMES) == 0) {
                /* Short format name */
                int skip = std::min((int) stream.distance(), 32);
                stream.seekRelative(skip);
            } else {
                /* Skip a wsz string */
                int wc = 1;
                while (stream.checkDistance(2) && wc != 0) {
                    wc = stream.readUInt16();
                }
            }

            /*
            LOG_DEVEL(LOG_LEVEL_INFO, "VNC: Format id %s available"
                                      " from RDP client",
                      cf2text(format_id, scratch, sizeof(scratch)));
             */

            switch (formatId) {
                case CF_UNICODETEXT:
                    seenCfUnicodeText = true;
                    break;
                case CF_TEXT:
                    seenCfText = true;
                    break;
            }
        }

        if (seenCfUnicodeText) {
            return CF_UNICODETEXT;
        } else if (seenCfText) {
            return CF_TEXT;
        } else {
            return 0;
        }
    }

    int ClipboardChannel::handleFormatDataRequest(XrdpStream &stream) {
        int format = 0;

        if (stream.checkDistance(4)) {
            format = stream.readUInt32();
        }

        if (format == CF_LOCALE) {
            return sendLocaleResponse();
        }

        if (_current == nullptr) {
            LOG(LOG_LEVEL_ERROR, "No clipboard data available");
            return sendFailureResponse();
        }

        auto currentTextEntry = std::dynamic_pointer_cast<ClipboardTextEntry>(_current);

        if (currentTextEntry == nullptr) {
            LOG(LOG_LEVEL_ERROR, "Clipboard data is not text");
            return sendFailureResponse();
        }

        if (format == CF_UNICODETEXT) {
            return sendUnicodeTextResponse(currentTextEntry);
        } else if (format == CF_TEXT) {
            return sendAnsiTextResponse(currentTextEntry);
        } else {
            LOG(LOG_LEVEL_ERROR, "Unsupported clipboard format %d", format);
            return sendFailureResponse();
        }
    }

    int ClipboardChannel::sendLocaleResponse() {
        XrdpStream outStream(64);
        prepareHeader(outStream, CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK);

        outStream.write((uint32_t) 0x0409); // en-US locale
        outStream.markEnd();

        sendStreamToChannel(outStream);

        return 0;
    }

    int ClipboardChannel::sendAnsiTextResponse(std::shared_ptr<ClipboardTextEntry> entry) {
        XrdpStream outStream(64 + entry->ansi().size() + 1);
        prepareHeader(outStream, CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK);

        for (char c: entry->ansi()) {
            outStream.write((uint8_t) c);
        }

        outStream.seekRelative(1);
        outStream.markEnd();

        sendStreamToChannel(outStream);
    }

    int ClipboardChannel::sendUnicodeTextResponse(std::shared_ptr<ClipboardTextEntry> entry) {
        // ???: why adding 2? (see vnc_clip.c:438)
        XrdpStream outStream(64 + entry->unicode().size() + 2);
        prepareHeader(outStream, CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK);

        for (char c: entry->unicode()) {
            outStream.write((uint8_t) c);
        }

        outStream.seekRelative(2);
        outStream.markEnd();

        sendStreamToChannel(outStream);
    }

    int ClipboardChannel::sendFailureResponse() {
        XrdpStream outStream(64);
        prepareHeader(outStream, CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_FAIL);
        outStream.markEnd();

        sendStreamToChannel(outStream);

        return 0;
    }

    void ClipboardChannel::prepareHeader(XrdpStream &stream, int msgType, int msgFlags) {
        stream << (uint16_t) msgType;
        stream << (uint16_t) msgFlags;

        /* Save the datalen location so we can update it later */
        stream.markChannelHeader(4);
    }

    void ClipboardChannel::sendStreamToChannel(XrdpStream &stream) {
        size_t pos = 0;

        stream.seekToChannelHeader();

        size_t streamLength = stream.size();

        // update PDU length
        size_t dataLength = stream.distance() - 4;
        stream.write((uint32_t) dataLength);

        stream.seekToBegin();
        int msgType = stream.readUInt16();
        int msgFlags = stream.readUInt16();

        LOG(LOG_LEVEL_DEBUG, "Sending cliprdr PDU type: %s flags: %d datalen: %d",
            CB_PDUTYPE_TO_STR(msgType), msgFlags, dataLength);


        while (pos < streamLength) {
            size_t pduLength = std::min((size_t) CHANNEL_CHUNK_LENGTH, (streamLength - pos));
            int flags = 0;

            // Determine chunking flags for this PDU (MS-RDPBCGR 3.1.5.2.1)
            if (pos == 0) {
                if ((pos + pduLength) == streamLength) {
                    // only one chunk
                    flags = (XR_CHANNEL_FLAG_FIRST | XR_CHANNEL_FLAG_LAST);
                } else {
                    // first chunk of several
                    flags = (XR_CHANNEL_FLAG_FIRST | XR_CHANNEL_FLAG_SHOW_PROTOCOL);
                }
            } else if ((pos + pduLength) == streamLength) {
                // last chunk of several
                flags = (XR_CHANNEL_FLAG_LAST | XR_CHANNEL_FLAG_SHOW_PROTOCOL);
            } else {
                // intermediate chunk of several
                flags = XR_CHANNEL_FLAG_SHOW_PROTOCOL;
            }

            int retval = mod()->server_send_to_channel(
                    mod(), channelId(),
                    const_cast<char *>(stream.begin() + pos), (int) pduLength,
                    (int) streamLength, flags
            );

            if (retval != 0) {
                LOG(LOG_LEVEL_ERROR, "Failed to send cliprdr PDU");
                // TODO: throw exception
                return;
            }

            pos += pduLength;
        }
    }


}