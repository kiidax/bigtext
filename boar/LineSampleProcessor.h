/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include <random>

namespace boar
{
    namespace fs = boost::filesystem;

    struct OutputSpec
    {
        boost::uintmax_t numberOfLines;
        double rate;
        fs::path fileName;

    public:
        OutputSpec(const fs::path &fileName, double rate) : fileName(fileName), rate(rate), numberOfLines(0) {}
        OutputSpec(const fs::path &fileName, boost::uintmax_t numberOfLines) : fileName(fileName), rate(0.0), numberOfLines(numberOfLines) {}
    };

    template <typename CharT>
    class LineSampleProcessor
    {
    public:
        typedef CharT CharType;

    private:
        std::vector<fs::path> _inputFileNameList;
        fs::path _outputFileName;
        LineFileWriter<CharT> _out;
        int _threshold;

    public:
        LineSampleProcessor(const std::vector<fs::path> &inputFileNameList, double rate, fs::path& outputFileName)
        {
            _threshold = static_cast<int>(rate * RAND_MAX + 0.5);
            _inputFileNameList = inputFileNameList;
            _outputFileName = outputFileName;
        }

        void Run()
        {
            _out.Open(_outputFileName);
            for (auto& fileName : _inputFileNameList)
            {
                FileLineSourceDefault(fileName, *this);
            }
            _out.Close();
        }

        bool ProcessLine(const CharT *s, size_t len)
        {
            if (std::rand() < _threshold)
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
            int randomTreshold;
            boost::uintmax_t lineCount;
            LineFileWriter<CharT> writer;
        };

    protected:
        size_t _numOutputs;
        OutputProgress *_outputProgressList;

    public:
        void Run(const std::vector<fs::path>& inputPathList, const std::vector<OutputSpec>& outputSpecList, bool overwrite)
        {
            _numOutputs = outputSpecList.size();
            _outputProgressList = new OutputProgress[_numOutputs];

            for (size_t i = 0; i < _numOutputs; i++)
            {
                auto& spec = outputSpecList[i];
                if (spec.numberOfLines > 0)
                {
                    // TODO: overflow
                    _outputProgressList[i].randomTreshold = 0;
                }
                else if (spec.rate >= 0)
                {
                    _outputProgressList[i].randomTreshold = static_cast<int>(spec.rate * RAND_MAX + 0.5);
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
            int t = std::rand();
            for (int i = 0; i < _numOutputs; i++)
            {
                auto &prog = _outputProgressList[i];
                if (t < prog.randomTreshold)
                {
                    prog.writer.WriteLine(s, len);
                    ++prog.lineCount;
                }
                t -= prog.randomTreshold;
            }
        }
    };
}