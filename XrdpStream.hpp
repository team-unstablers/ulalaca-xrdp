//
// Created by Gyuhwan Park on 2022/04/30.
//

#ifndef ULALACA_XRDPSTREAM_HPP
#define ULALACA_XRDPSTREAM_HPP

#include <memory>
#include <functional>

extern "C" {
#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
};

/**
 * wrapper class for xrdp stream
 */
class XrdpStream {
public:
    using Stream = stream;
    using StreamDeleter = std::function<void(Stream *)>;
    
    XrdpStream(size_t size);
    
    explicit operator Stream const *() const;
    
    template<typename T>
    XrdpStream &operator<< (T value);

    void write(uint8_t value);
    void write(uint16_t value);
    void write(uint32_t value);
    void write(uint64_t value);
    void write(const uint8_t *data, size_t size);

    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();

    /**
     * @brief calculates size of the entire stream
     */
    ptrdiff_t size() const;

    /**
     * @brief calculates distance between current position and the end of the stream
     */
    ptrdiff_t distance() const;

    bool checkDistance(int n) const;

    const char *channelHeader() const;

    void seekAbsolute(ptrdiff_t offset);
    void seekRelative(ptrdiff_t offset);

    /**
     * equivalent to s_push_layer(s, channel_hdr, n)
     */
    void markChannelHeader(int offset);
    /**
     * equivalent to s_pop_layer(s, channel_hdr)
     */
    void seekToChannelHeader();

    /**
     * @brief returns pointer to the beginning of the stream
     */
    const char *begin() const;
    void seekToBegin();

    const char *end() const;
    void markEnd();

private:
    std::unique_ptr<Stream, StreamDeleter> _stream;
};

#include "XrdpStream.template.cpp"

#endif //ULALACA_XRDPSTREAM_HPP
