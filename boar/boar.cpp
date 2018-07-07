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

    bool Lexer::IsOperator(CharType ch)
    {
        return
            ch == '!' ||
            ch == '$' ||
            ch == '%' ||
            ch == '&' ||
            ch == '\'' ||
            ch == '*' ||
            ch == '+' ||
            ch == '-' ||
            ch == '.' ||
            ch == '/' ||
            ch == ':' ||
            ch == '<' ||
            ch == '=' ||
            ch == '>' ||
            ch == '?' ||
            ch == '@' ||
            ch == '\\' ||
            ch == '^' ||
            ch == '_' ||
            ch == '`' ||
            ch == '|' ||
            ch == '~';
    }

    int Parser::CompareOperator(StringType op1, StringType op2)
    {
        if (op1 == op2) return 0;
        if (op1 == L"+" && op2 == L"*") return -1;
        if (op1 == L"*" && op2 == L"-") return 1;
        return 0;
    }

    Lexer::TokenType Lexer::Lex()
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
                        _current = p;
                        return TokenType::INVALID;
                    }
                    _value.number = std::stod(std::wstring(_current, p));
                    _current = p;
                    return TokenType::NUMBER;
                }
                _value.integer = std::stoi(std::wstring(_current, p));
                _current = p;
                return TokenType::INTEGER;
            }
        }
        else if (IsPostfix(*p))
        {
            p++;
            _value.string = _current;
            _current = p;
            return TokenType::OPERATOR;
        }
        else if (IsOperator(*p))
        {
            CharType first = *p;
            p++;
            while (p != _end && (*p == first || (IsOperator(*p) && !IsPrefix(*p)))) p++;
            _value.string = _current;
            _current = p;
            return TokenType::OPERATOR;
        }
        _current = p;
        return TokenType::INVALID;
    }

    Lexer::Lexer(const CharType * start, const CharType * end)
    {
        _start = start;
        _end = end;
        _current = start;
    }

    void Parser::Parse()
    {
        std::vector<double> valueStack;
        std::vector<std::wstring> operatorStack;

        while (true)
        {
            auto type = _lexer.Lex();
            if (type == TokenType::END)
            {
                while (true)
                {
                    if (operatorStack.empty())
                    {
                        return;
                    }
                    else
                    {
                        auto op = operatorStack.back();
                        operatorStack.pop_back();
                        auto value2 = valueStack.back();
                        valueStack.pop_back();
                        auto value1 = valueStack.back();
                        int result;
                        if (op == L"*") result = value1 * value2;
                        if (op == L"+") result = value1 + value2;
                        std::wcout << op << "(" << value1 << "," << value2 << ")" << std::endl;
                        valueStack.back() = result;
                        valueStack.push_back(value1);
                    }
                }
            }
            else if (type == TokenType::INVALID)
            {
                std::wcout << "INVALID" << std::endl;
            }
            else if (type == TokenType::NUMBER)
            {
                auto x = _lexer.Number();
                valueStack.push_back(x);
                std::wcout << x << std::endl;
            }
            else if (type == TokenType::INTEGER)
            {
                auto n = _lexer.Integer();
                valueStack.push_back(n);
                std::wcout << n << std::endl;
            }
            else if (type == TokenType::OPERATOR)
            {
                auto op = _lexer.String();
                std::wcout << op << std::endl;
                while (true)
                {
                    if (operatorStack.empty() || CompareOperator(operatorStack.back(), op) < 0)
                    {
                        std::wcout << "push" << std::endl;
                        operatorStack.push_back(op);
                        break;
                    }
                    else
                    {
                        auto op2 = operatorStack.back();
                        operatorStack.pop_back();
                        auto value2 = valueStack.back();
                        valueStack.pop_back();
                        auto value1 = valueStack.back();
                        int result;
                        if (op2 == L"*") result = value1 * value2;
                        if (op2 == L"+") result = value1 + value2;
                        std::wcout << op2 << "(" << value1 << "," << value2 << ")" << std::endl;
                        valueStack.back() = result;
                        valueStack.push_back(value1);
                    }
                }
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
            std::wstring s(L"2.343e-5+1.42e1*13");
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
