/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "taskqueue.h"
#include "filesource.h"
#include "LineCountProcessor.h"

namespace boar
{
    bool LineCount3(const std::vector<std::wstring>& args)
    {
        // 1059203070      463179
        // 36,762,348,544 bytes.
        // AMD E2-7110
        for (auto it = args.begin(); it != args.end(); ++it)
        {
            boost::filesystem::path fileName(*it);
            size_t lineCount = 0;
            FileSourceWithFileRead(fileName, [&lineCount](const void *addr, size_t n) {
                const char* p = reinterpret_cast<const char*>(addr);
                size_t c = 0;
                for (size_t i = 0; i < n; i++)
                {
                    if (p[i] == '\n') c++;
                }
                lineCount += c;
            });
            std::wcout << fileName << '\t' << lineCount << std::endl;
        }
        return true;
    }

    bool LineCount2(const std::vector<std::wstring>& args)
    {
        // 1059203072      414019
        // 36,762,348,544 bytes.
        // AMD E2-7110
        for (auto it = args.begin(); it != args.end(); ++it)
        {
            boost::filesystem::path fileName(*it);
            size_t lineCount = 0;
            FileSourceWithOverlapRead(fileName, [&lineCount](const void *addr, size_t n) {
                const char* p = reinterpret_cast<const char*>(addr);
                size_t c = 0;
                for (size_t i = 0; i < n; i++)
                {
                    if (p[i] == '\n') c++;
                }
                lineCount += c;
            });
            std::wcout << fileName << '\t' << lineCount << std::endl;
        }
        return true;
    }

    bool LineCount(const std::vector<std::wstring>& args)
    {
        std::auto_ptr<TextProcessor> proc(new LineCountProcessor());
        proc->ProcessFileList(args);
        return true;
    }

    template <typename charT>
    bool DropLines(const std::vector<std::wstring>& args)
    {
        for (auto it = args.begin(); it != args.end(); ++it)
        {
            boost::filesystem::path fileName(*it);
            size_t lineCount = 0;
            FileSourceWithOverlapRead(fileName, [&lineCount](const void *addr, size_t n) {
                intptr_t addrValue = reinterpret_cast<intptr_t>(addr);
                const charT* first = reinterpret_cast<const charT*>(addrValue);
                const charT* last = reinterpret_cast<const charT*>(addrValue + n);
                size_t c = 0;
                const charT* lineStart = first;
                for (const charT* p = first; p != last; p++)
                {
                    if (*p == '\n')
                    {
                        const std::basic_string<wchar_t> s(lineStart, p);
                        std::wcout << s << std::endl;
                        lineStart = p + 1;
                        c++;
                    }
                }
                lineCount += c;
            });
        }
        return true;
    }

    int DumpProfile(std::function<size_t()> func)
    {
        clock_t startTime = clock();
        size_t lineCount = func();
        clock_t endTime = clock();
        clock_t t = endTime - startTime;
        std::wcout << lineCount << '\t' << t << std::endl;
        return 0;
    }

    int Usage()
    {
        std::wcout <<
            L"usage: boar <command> [<args>]\n"
            "\n"
            "Boar is a toolkit to process text files.\n"
            "\n"
            "List of commands:\n"
            "\n"
            "   lc      Count the number of lines.\n"
            << std::endl;
        return 1;
    }

    int Main(const std::vector<std::wstring>& args)
    {
        int status;
        if (args.size() == 1)
        {
            return Usage();
        }
        else if (args.size() == 2)
        {
            const std::wstring commandName(args[0]);
            const std::vector<std::wstring> args2(args.begin() + 1, args.end());
            if (commandName == L"count2")
            {
                status = DumpProfile([&args2]() { return LineCount2(args2); });
            }
            if (commandName == L"count3")
            {
                status = DumpProfile([&args2]() { return LineCount3(args2); });
            }
            else if (commandName == L"drop")
            {
                status = DumpProfile([&args2]() { return DropLines<char>(args2); });
            }
            else if (commandName == L"count")
            {
                status = DumpProfile([&args2]() { return LineCount(args2); });
            }
        }
        else
        {
            return Usage();
        }
        return 0;
    }
}
