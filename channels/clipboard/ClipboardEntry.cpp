//
// Created by Gyuhwan Park on 2023/06/04.
//

#include "../../Utility.hpp"

#include "ClipboardEntry.hpp"

namespace ulalaca::channel {
    ClipboardEntry::ClipboardEntry() {

    }

    ClipboardTextEntry ClipboardTextEntry::fromUnix(const std::string &text) {
        std::string ansi(std::move(utility::unix2dos(text)));
        std::string unicode(std::move(utility::ansi2unicode(ansi)));

        return std::move(ClipboardTextEntry(ansi, unicode));
    }

    ClipboardTextEntry ClipboardTextEntry::fromDos(const std::string &text) {
        std::string unicode(std::move(utility::ansi2unicode(text)));
        return std::move(ClipboardTextEntry(text, unicode));
    }

    ClipboardTextEntry::ClipboardTextEntry(const std::string &textAnsi, const std::string &textUnicode):
        ClipboardEntry(),

        _textAnsi(textAnsi),
        _textUnicode(textUnicode)
    {

    }

    ClipboardEntryType ClipboardTextEntry::type() const {
        return TEXT;
    }

    const std::string &ClipboardTextEntry::ansi() const {
        return _textAnsi;
    }

    const std::string &ClipboardTextEntry::unicode() const {
        return _textUnicode;
    }
}