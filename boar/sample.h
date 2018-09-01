/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"

namespace boar
{
    namespace fs = boost::filesystem;

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
        std::vector<fs::path> _inputFileNameList;
        fs::path _outputFileName;
        LineFileWriter<CharT> _out;
        std::default_random_engine _gen;
        std::bernoulli_distribution _dist;

    public:
        LineSampleProcessor(const std::vector<fs::path> &inputFileNameList, double rate, fs::path& outputFileName)
        {
            _inputFileNameList = inputFileNameList;
            _outputFileName = outputFileName;
            std::random_device rd;
            _gen = std::default_random_engine(rd());
            _dist = std::bernoulli_distribution(rate);
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
}