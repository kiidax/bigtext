/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include <random>

#include "Processor.h"

namespace boar
{
    template <typename CharT>
    class LineSampleProcessor : Processor
    {
    public:
        typedef CharT CharType;

    private:
        int _threshold;

    public:
        LineSampleProcessor(double rate)
        {
            _threshold = static_cast<int>(rate * RAND_MAX + 0.5);
        }

        virtual bool ProcessLine(const CharT *s, size_t len)
        {
            if (std::rand() < _threshold)
            {
                OutputText(s, len);
            }
            return true;
        }

        void OutputText(const CharT* s, size_t len)
        {
            const char *p = reinterpret_cast<const char *>(s);
            _outf.write(p, len / sizeof (CharT));
        }
    };
}