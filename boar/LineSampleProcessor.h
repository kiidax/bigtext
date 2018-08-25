/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "LineProcessor.h"
#include <random>

namespace boar
{
    namespace fs = boost::filesystem;

    template <typename charT>
    class LineSampleProcessor : public LineProcessor<charT>
    {
    private:
        int _threshold;

    public:
        LineSampleProcessor(double rate)
        {
            _threshold = static_cast<int>(rate * RAND_MAX + 0.5);
        }

        virtual bool ProcessLine(const charT* first, const charT* last)
        {
            if (std::rand() < _threshold)
            {
                OutputText(first, last);
            }
            return true;
        }
    };

    template <typename charT>
    class LineSampleProcessor2
    {
    public:
        struct OutputSpec
        {
            boost::uintmax_t numberOfLines;
            double rate;
            fs::path path;
        };

    protected:
        struct OutputProgress
        {
            int randomTreshold;
            boost::uintmax_t lineCount;
            LineFileWriter<charT> writer;
        };

    protected:
        size_t _numOutputs;
        OutputProgress *_outputProgressList;

    public:
        void Run(const std::vector<fs::path>& inputPathList, const std::vector<OutputSpec>& OutputSpecList, bool overwrite)
        {
            if (CheckInputFile(inputPathList))
            {
                std::cerr << "can't read." << std::endl;
            }

            if (!overwrite && CheckOutputFile(OutputSpecList))
            {
                std::cerr << "can't write." << std::endl;
            }

            _numOutputs = OutputSpecList.size();
            _outputProgressList = new OutputProgress[_numProgress];

            for (size_t i = 0; i < _numProgress; i++)
            {
                auto& spec = OutputSpecList[i];
                if (spec.numberOfLines > 0)
                {
                    // TODO: overflow
                    _outputProgressList[i].rateOrNumberOfLines = -static_cast<double>(spec.numberOfLines);
                }
                else if (spec.rate >= 0)
                {
                    _outputProgressList[i].rateOrNumberOfLines = spec.rate;
                }
                else
                {
                    return;
                }
                _outputProgressList[i].writer.Open(spec.path);
            }

            for (auto &fileName : inputPathList)
            {
                FileLineSourceDefault(fileName, this);
            });
        }

        void ProcessList(const CharT *s, size_t len)
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
                t -= prog.randomThresold;
            }
        }

    private:
        bool CheckInputFile(const std::vector<fs::path>& inputPathList) const
        {
            return std::all_of(inputPathList.cbegin(), inputPathList.cend(), [](auto &path)
            {
                return fs::is_regular_file(path);
            });
        }

        bool CheckOutputFile(const std::vector<OutputSpec>& OutputSpecList) const
        {
            return std::all_of(OutputSpecList.cbegin(), OutputSpecList.cend(), [](auto &spec)
            {
                return !fs::exists(spec.path);
            });
        }
    };
}