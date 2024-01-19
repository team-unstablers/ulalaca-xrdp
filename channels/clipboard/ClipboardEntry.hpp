//
// Created by Gyuhwan Park on 2023/06/04.
//

#ifndef XRDP_TUMOD_CLIPBOARDENTRY_HPP
#define XRDP_TUMOD_CLIPBOARDENTRY_HPP

#include <string>
#include <memory>

#include "XrdpStream.hpp"

namespace ulalaca::channel {

    enum ClipboardEntryType {
        TEXT
    };

    class ClipboardEntry {
    public:
        explicit ClipboardEntry();

        virtual ClipboardEntryType type() const = 0;
    };

    class ClipboardTextEntry: public ClipboardEntry {
    public:
        static ClipboardTextEntry fromUnix(const std::string &text);
        static ClipboardTextEntry fromDos(const std::string &text);

        explicit ClipboardTextEntry(const std::string &textAnsi, const std::string &textUnicode);

        virtual ClipboardEntryType type() const override;

        /**
         * Returns the ANSI representation of the text
         * @FIXME bad name. actually, it contains UTF-8 string
         */
        const std::string &ansi() const;

        /**
         * Returns the UTF-16LE representation of the text
         */
        const std::string &unicode() const;

    private:
        std::string _textAnsi;
        std::string _textUnicode;
    };


}



#endif //XRDP_TUMOD_CLIPBOARDENTRY_HPP
