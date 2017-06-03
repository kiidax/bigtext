/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"

namespace boar
{
    using namespace std;

    int Main(const vector<u16string>& args)
    {
        Buffer<char8_t> buffer;
        Cursor<char8_t> cursor(buffer);

        ifstream fin;
        fin.open("test.txt");
        if (fin.is_open())
        {
            string line;
            while (getline(fin, line))
            {
                c8string c8line(line.begin(), line.end());
                c8line += '\n';
                cursor.Append(c8line.c_str(), c8line.c_str() + c8line.length());
            }
        }

        cursor.MoveBeginningOfBuffer();
        while (true)
        {
            auto c8line = cursor.GetLineAndMoveNext();
            if (c8line.empty()) break;
            string line(c8line.begin(), c8line.end());
            std::cout << line << endl;
        }

        return 0;
    }

    template<typename charT>
    void Cursor<charT>::Append(const charT* start, const charT* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);

        if (_l1node == nullptr)
        {
            assert(_l2node == nullptr);

            _buffer._children.push_back(L1Node<charT>());
            _l1node = &_buffer._children.back();
        }

        if (_l2node == nullptr)
        {
            assert(_gapStart == nullptr);
            assert(_gapEnd == nullptr);
            assert(_current == nullptr);

            (*_l1node)._children.push_back(L2Node<charT>());
            _l2node = &(*_l1node)._children.back();
            charT* newData = (charT*)malloc(1024 * sizeof(charT));
            if (newData == nullptr) throw bad_alloc();

            _l2node->_data = newData;
            _l2node->_length = 1024;

            _gapStart = _l2node->_data;
            _gapEnd = _l2node->_data + _l2node->_length;
            _current = _l2node->_data;
        }

        assert(_gapStart != nullptr);
        assert(_gapEnd != nullptr);
        assert(_current != nullptr);

        if (_gapEnd - _gapStart < end - start)
        {
            size_t newLength = _l2node->_length + (end - start) - (_gapEnd - _gapStart);
            charT* newData = (charT*)realloc(_l2node->_data, newLength * sizeof(charT));
            if (newData == nullptr) throw bad_alloc();

            // Adjust the pointers.
            _gapStart = newData + (_gapStart - _l2node->_data);
            _gapEnd = newData + (_gapEnd - _l2node->_data);
            _current = newData + (_current - _l2node->_data);

            _l2node->_data = newData;
            _l2node->_length = newLength;
        }

        memcpy(_gapStart, start, (end - start) * sizeof(charT));
        _gapStart += end - start;
        _current += end - start;
    }

    template<typename charT>
    basic_string<charT> Cursor<charT>::GetLineAndMoveNext()
    {
        if (_l1node == nullptr)
        {
            return basic_string<charT>();
        }

        charT *it;
        for (it = _current; it != _l2node->_data + _l2node->_length; ++it)
        {
            if (*it == '\n')
                break;
        }

        basic_string<charT> ret(_current, it - _current);

        _current = it + 1;
        if (_current == _l2node->_data + _l2node->_length)
        {
            _l1node = nullptr;
            _l2node = nullptr;
            _current = nullptr;
            _gapStart = nullptr;
            _gapEnd = nullptr;
        }

        return ret;
    }

    template<typename charT>
    void Cursor<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _l1node = _buffer._children.data();
        _l2node = _l1node->_children.data();
        _gapStart = _l2node->_data + _l2node->_length;
        _gapEnd = _gapStart;
        _current = _l2node->_data;
    }
    template<typename charT>
    void Cursor<charT>::_CloseNode()
    {
        _l2node->_length = _current - _l2node->_data;
    }
}
