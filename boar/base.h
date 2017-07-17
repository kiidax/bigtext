/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <string>

namespace boar {
    inline std::u16string ToUnicode(const char *s)
    {
        return std::u16string(s, s + strlen(s));
    }

    inline std::string FromUnicode(const char16_t* s)
    {
        std::u16string t(s);
        return std::string(t.begin(), t.end());
    }
}