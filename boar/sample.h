/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"

namespace boar
{
    namespace fs = boost::filesystem;
    namespace ios = boost::iostreams;

    struct OutputSpec
    {
        uintmax_t numberOfLines;
        double rate;
        fs::path fileName;

    public:
        OutputSpec(const fs::path &fileName, double rate) : fileName(fileName), rate(rate), numberOfLines(0) {}
        OutputSpec(const fs::path &fileName, uintmax_t numberOfLines) : fileName(fileName), rate(0.0), numberOfLines(numberOfLines) {}
    };

    template <typename CharT>
    class LineSampleProcessor
    {
    public:
        typedef CharT CharType;

    private:
        std::default_random_engine _gen;
        std::bernoulli_distribution _dist;

    public:
        LineSampleProcessor()
        {
            std::random_device rd;
            _gen = std::default_random_engine(rd());
            _dist = std::bernoulli_distribution(rate);
        }

        void Run(const std::vector<fs::path> &inputFileNameList, double rate, fs::path& outputFileName)
        {
            fs::ofstream out;
            out.open(_outputFileName, std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                std::cwerr << "cannot open" << std::endl;
            }
            for (auto& fileName : _inputFileNameList)
            {
                FileLineSourceDefault(fileName, *this);
            }
        }

        bool ProcessLine(const CharT *s, size_t len)
        {
            if (_dist(_gen))
            {
                _out.WriteLine(s, len);
            }
            return true;
        }
    };

    template <typename CharT>
    class LineSampleProcessor2
    {
    public:
        typedef CharT CharType;

    protected:
        struct OutputProgress
        {
            double randomThreshold;
            uintmax_t lineCount;
            LineFileWriter<CharT> writer;
        };

    public:
        LineSampleProcessor2()
        {
            std::random_device rd;
            _gen = std::default_random_engine(rd());
        }

    protected:
        size_t _numOutputs;
        OutputProgress *_outputProgressList;
        std::default_random_engine _gen;

    public:
        void Run(const std::vector<fs::path>& inputPathList, const std::vector<OutputSpec>& outputSpecList)
        {
            _numOutputs = outputSpecList.size();
            _outputProgressList = new OutputProgress[_numOutputs];

            for (size_t i = 0; i < _numOutputs; i++)
            {
                auto& spec = outputSpecList[i];
                if (spec.numberOfLines > 0)
                {
                    // TODO: overflow
                    _outputProgressList[i].randomThreshold = 0.0;
                }
                else if (spec.rate >= 0)
                {
                    _outputProgressList[i].randomThreshold = spec.rate;
                }
                else
                {
                    return;
                }
                _outputProgressList[i].writer.Open(spec.fileName);
            }

            for (auto &fileName : inputPathList)
            {
                boar::FileLineSourceDefault<LineSampleProcessor2<CharT>, CharT>(fileName, *this);
            }
        }

        void ProcessLine(const CharT *s, size_t len)
        {
            constexpr std::size_t bits = std::numeric_limits<float>::digits;
            double t = std::generate_canonical<double, bits>(_gen);
            for (int i = 0; i < _numOutputs; i++)
            {
                auto &prog = _outputProgressList[i];
                if (t < prog.randomThreshold)
                {
                    prog.writer.WriteLine(s, len);
                    ++prog.lineCount;
                }
                t -= prog.randomThreshold;
            }
        }
    };

    template<typename CharT>
    void FileShuffleLines(const std::vector<fs::path> &inputFileNameList, const std::vector<OutputSpec> &outputSpecList)
    {
        std::vector<size_t> lineIndexList;
        std::vector<const CharT *> linePositionList;

        std::vector<ios::mapped_file_source> fileList;
        size_t lineIndex = 0;
        for (auto &inputFileName : inputFileNameList)
        {
            size_t prevLineIndex = lineIndex;
            fileList.emplace_back();
            auto &file = fileList.back();
            file.open(inputFileName);
            if (!file.is_open())
            {
                std::wcerr << "error" << std::endl;
                return;
            }

            const CharT *s = reinterpret_cast<const CharT *>(file.data());
            size_t len = file.size() / sizeof(CharT);

            linePositionList.push_back(s);
            std::wcout << inputFileName.native() << "\tCharCount\t" << len << std::endl;
            for (size_t i = 0; i < len; i++)
            {
                if (s[i] == '\n')
                {
                    lineIndexList.push_back(lineIndex++);
                    linePositionList.push_back(&s[i + 1]);
                }
            }
            if (s[len - 1] != '\n')
            {
                lineIndexList.push_back(lineIndex++);
                linePositionList.push_back(&s[len]);
            }

            std::wcout << inputFileName.native() << "\tLineCount\t" << (lineIndex - prevLineIndex) << std::endl;
            lineIndex++;
        }

        // Shuffle lines

        size_t numLines = lineIndexList.size();
        if (lineIndex - inputFileNameList.size() != numLines)
        {
            std::wcerr << "something wrong" << std::endl;
        }
        std::cout << "\tLineCount\t" << numLines << std::endl;
        for (size_t i = 0; i < numLines - 1; i++)
        {
            size_t j = std::rand() % (numLines - i);
            std::swap(lineIndexList[i], lineIndexList[j]);
        }

        // Write lines

        size_t curIndex = 0;
        for (auto &outputSpec : outputSpecList)
        {
            uintmax_t lineCount;
            if (outputSpec.numberOfLines > 0)
            {
                lineCount = outputSpec.numberOfLines;
            }
            else if (outputSpec.rate >= 1.0)
            {
                lineCount = numLines;
            }
            else
            {
                lineCount = static_cast<uintmax_t>(numLines * outputSpec.rate + 0.5);
            }

            std::wcerr << outputSpec.fileName << "\tLineCount\t" << min(lineCount, numLines - curIndex) << std::endl;

            fs::basic_ofstream<CharT> out;
            out.open(outputSpec.fileName, std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                std::wcerr << "write error" << std::endl;
                return;
            }

            for (uintmax_t i = 0; i < lineCount; i++)
            {
                if (curIndex >= numLines)
                {
                    break;
                }

                size_t n = lineIndexList[curIndex++];
                const CharT *first = linePositionList[curIndex];
                const CharT *last = linePositionList[n + 1];
                out.write(first, last - first);
            }
        }
    }
}