//
// Created by Gyuhwan Park on 1/19/24.
//

#include <CoreFoundation/CoreFoundation.h>

#include "Utility.hpp"

namespace ulalaca::utility {
    std::string unix2dos(const std::string &unixText) {
        std::string dosText;
        dosText.reserve(unixText.size());

        for (char it : unixText) {
            if (it == '\n') {
                dosText.push_back('\r');
            }
            dosText.push_back(it);
        }

        return std::move(dosText);
    }

    std::string ansi2unicode(const std::string &ansiText) {
        // sorry, i'm too stupid to implement this function.
        CFStringRef cfAnsiText
            = CFStringCreateWithCString(kCFAllocatorDefault,
                                        ansiText.c_str(),
                                        kCFStringEncodingUTF8);

        CFIndex length = CFStringGetLength(cfAnsiText);

        std::unique_ptr<UniChar> utf16LEText(new UniChar[sizeof(UniChar) * length]);
        CFStringGetCharacters(cfAnsiText,
                              CFRangeMake(0, length),
                              utf16LEText.get());

        CFRelease(cfAnsiText);

        return std::string(reinterpret_cast<char *>(utf16LEText.get()),
                           sizeof(UniChar) * length);
    }
}