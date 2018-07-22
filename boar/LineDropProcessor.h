/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "LineProcessor.h"
#include <random>

namespace boar
{
    template <typename charT>
    class LineDropProcessor : public LineProcessor<charT>
    {
    private:
        int _threshold;

    public:
        LineDropProcessor(double rate)
        {
            _threshold = static_cast<int>(rate * RAND_MAX + 0.5);
        }

        virtual bool ProcessLine(const charT* first, const charT* last)
        {
            if (std::rand() < _threshold)
            {
                const std::basic_string<wchar_t> s(first, last);
                std::wcout << s << std::endl;
            }
            return true;
        }
    };
}