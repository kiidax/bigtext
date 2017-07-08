/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include "minibuffer.h"
#include "linkedlist.h"

#include <string>
#include <assert.h>

namespace boar {

    template<typename charT>
    class BufferNode : public MiniBuffer<charT>
    {

    };

    template<typename charT>
    class Buffer
    {
    public:
        Buffer() : _lineSeparator('\n')
        {
            _currentNode = _nodeList.begin();
        }
        ~Buffer()
        {
        }
        void Open(const char16_t* fileName);
        void Dump();
        void Insert(const charT* s, size_t n)
        {
            if (n > _currentNode->GetCapacity() - _currentNode->GetSize())
            {
                _currentNode = _currentNode.GetCurrent()->Split();
            }
            size_t pos = _currentNode->GetSize();
            _currentNode->Insert(pos, s, n);
        }
        void Test() {}
        void MoveBeginningOfBuffer() {}
        std::basic_string<charT> GetLineAndMoveNext() { return std::basic_string<charT>(); }
    protected:
        charT _lineSeparator;
        LinkedList<BufferNode<charT>> _nodeList;
        LinkedListIterator<BufferNode<charT>> _currentNode;
    };
}