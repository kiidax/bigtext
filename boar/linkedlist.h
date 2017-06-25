/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include <assert.h>

namespace boar {

    namespace {
        template<typename elemT>
        class LinkedListNode
        {
        public:
            LinkedListNode() : _prev(this), _next(this) {}
            ~LinkedListNode() {}
            LinkedListNode* Next() { return _next; }
            void InsertAfter(LinkedListNode* other)
            {
                other->_prev = this;
                other->_next = this->_next;
                _next = other;
                other->_next->_prev = other;
            }

        protected:
            LinkedListNode* _prev;
            LinkedListNode* _next;
        };

        template<typename elemT>
        class LinkedListNodeElem : public LinkedListNode<elemT>
        {
        public:
            elemT* Get() { return &_elem; }
            LinkedListNodeElem* Split()
            {
                auto newNode = new LinkedListNodeElem();
                InsertAfter(newNode);
                return newNode;
            }

        protected:
            elemT _elem;
        };
    }

    template<typename elemT>
    class LinkedListIterator
    {
    public:
        LinkedListIterator(LinkedListNode<elemT> *elem) : _current(elem) {}
        LinkedListIterator() : LinkedListIterator(nullptr) {}
        LinkedListNodeElem<elemT>* Get() { return reinterpret_cast<LinkedListNodeElem<elemT>*>(_current); }
        bool operator ==(const LinkedListIterator& other) const { return _current == other._current; }
        bool operator !=(const LinkedListIterator& other) const { return _current != other._current; }
        elemT* operator ->()
        {
            return Get()->Get();
        }
        LinkedListIterator& operator ++() { _current = _current->Next(); return *this; }

    protected:
        LinkedListNode<elemT>* _current;
    };

    template<typename elemT>
    class LinkedList : LinkedListNode<elemT>
    {
    public:
        LinkedList()
        {
            _prev = _next = new LinkedListNodeElem<elemT>();
        }
        LinkedListIterator<elemT> Begin() { return _next; }
        LinkedListIterator<elemT> End() { return this; }
    };
}