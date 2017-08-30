/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    template <typename charT>
    class LineProcessorBase : public ProcessorBase
    {
    protected:
        std::vector<charT> _previousPartialLine;

    public:
        LineProcessorBase() {}
        virtual ~LineProcessorBase() {}

        virtual void ProcessBuffer(const void* data_, size_t n_)
        {
            const charT* data = reinterpret_cast<const charT*>(data_);
            const size_t n = n_ / sizeof(charT);
            ProcessLines(data, n);
        }

        virtual void ProcessLines(const charT *data, size_t n)
        {
        }
    };
}
