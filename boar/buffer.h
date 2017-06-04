/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT> class Cursor;

    const int BufferLevel = 2;

    class BufferNodeBase {
    public:
        int numLines;
        BufferNodeBase() : numLines() {}
        ~BufferNodeBase() {}
    };

    template<typename charT, int level>
    class BufferNode : BufferNodeBase
    {
        typedef BufferNode<charT, level - 1> ChildNodeType;
        friend class Cursor<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class BufferNode<charT, 0> : BufferNodeBase
    {
        typedef charT ChildNodeType;
        friend class Cursor<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class Buffer
    {
        typedef BufferNode<charT, BufferLevel> ChildNodeType;
        friend class Cursor<charT>;
    private:
        ChildNodeType _root;
    public:
        Buffer() {}
        ~Buffer() {}
        void Open(const char16_t * fileName);
    };
}