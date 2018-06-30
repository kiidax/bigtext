/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

namespace boar
{
    int Usage()
    {
        std::wcout <<
            L"usage: boar <command> [<args>]\n"
            "\n"
            "Boar is a toolkit to process text files.\n"
            "\n"
            "List of commands:\n"
            "\n"
            "   find       Find a string in lines.\n"
            "   count      Count the number of lines.\n"
            << std::endl;
        return 1;
    }

    Parser::TokenType Parser::Lex()
    {
        const CharType *p = _current;
        while (p != _end && IsSpace(*p)) p++;
        if (p == _end)
        {
            _current = p;
            return TokenType::END;
        }
        else if (IsNumber(*p))
        {
            if (*p == '0')
            {
                _current = p;
                return TokenType::END;
            }
            else
            {
                while (p != _end && IsNumber(*p)) p++;
                if (p != _end && *p == '.')
                {
                    p++;
                    while (p != _end && IsNumber(*p)) p++;
                    if (p != _end && (*p == 'e' || *p == 'E'))
                    {
                        p++;
                        if (p != _end && (*p == '-' || *p == '+'))
                        {
                            p++;
                        }
                        if (p != _end && IsNonZeroNumber(*p))
                        {
                            p++;
                            while (p != _end && IsNumber(*p)) p++;
                            _value.number = std::stod(std::wstring(_current, p));
                            _current = p;
                            return TokenType::NUMBER;
                        }
                    }
                }
                _current = p;
                return TokenType::INVALID;
            }
        }
    }

    Parser::Parser(const CharType * start, const CharType * end)
    {
        _start = start;
        _end = end;
        _current = start;
    }

    void Parser::Parse()
    {
        TokenType type;
        while ((type = Lex()) != TokenType::END)
        {
            if (type == TokenType::INVALID)
            {
                std::wcout << "INVALID" << std::endl;
            }
            else if (type == TokenType::NUMBER)
            {
                std::wcout << _value.number << std::endl;
            }
        }
    }

    int Main(const std::vector<std::wstring>& args)
    {
        if (args.size() == 1)
        {
            return Usage();
        }
        else if (args.size() >= 2)
        {
            std::wstring s(L"2.343e-5 1.42e");
            Parser parser(s.c_str(), s.c_str() + s.length());
            parser.Parse();
        }
        else
        {
            return Usage();
        }
        return 0;
    }
}
