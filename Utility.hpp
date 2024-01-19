//
// Created by Gyuhwan Park on 1/19/24.
//

#ifndef XRDP_TUMOD_UTILITY_HPP
#define XRDP_TUMOD_UTILITY_HPP

#include <string>

namespace ulalaca::utility {
    std::string unix2dos(const std::string &unixText);

    /**
     * @brief converts UTF-8 string to UTF-16LE string
     * @FIXME bad name. actually, it converts UTF-8 to UTF-16LE
     */
    std::string ansi2unicode(const std::string &ansiText);
}


#endif //XRDP_TUMOD_UTILITY_HPP
