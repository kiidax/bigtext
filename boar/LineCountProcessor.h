#pragma once
#include "TextProcessor.h"

namespace boar
{
    class LineCountProcessor :
        public TextProcessor
    {
    private:
        size_t _lineCount;

    public:
        LineCountProcessor();
        ~LineCountProcessor();

        virtual void ProcessBuffer(const void *data, size_t n);
        virtual void EndContent();
    };
}
