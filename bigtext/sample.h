/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once
#include <exception>

namespace bigtext
{
    namespace fs = boost::filesystem;
    namespace ios = boost::iostreams;
    namespace rnd = boost::random;

    struct sample_output_spec
    {
    public:
        fs::path file_name;
        uintmax_t number_of_lines;
        double rate;

        sample_output_spec(const fs::path &file_name) : file_name(file_name), rate(1.0), number_of_lines(0) {}
        sample_output_spec(const fs::path &file_name, double rate) : file_name(file_name), rate(rate), number_of_lines(0) {}
        sample_output_spec(const fs::path &file_name, uintmax_t number_of_lines) : file_name(file_name), rate(0.0), number_of_lines(number_of_lines) {}
    };

    template <typename CharT>
    void file_line_sample(const std::vector<fs::path> &input_file_name_list, double rate, fs::path &output_file_name)
    {
        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::bernoulli_distribution<> dist(rate);

        fs::ofstream out;
        out.open(output_file_name, std::ios::out | std::ios::binary);
        if (!out.is_open())
        {
            std::wcerr << __wcserror(output_file_name.native().c_str());
            return;
        }

        for (auto& file_name : input_file_name_list)
        {
            file_line_source_default<CharT>(file_name, [&dist, &gen, &out](const CharT *s, size_t len)
            {
                if (dist(gen))
                {
                    out.write(reinterpret_cast<const char *>(s), sizeof(CharT) * len);
                }
            });
        }
    }

    template <typename CharT>
    void file_line_sample(const std::vector<fs::path> &input_path_list, const std::vector<sample_output_spec> &output_spec_list)
    {
        struct output_progress
        {
            double random_threshold;
            uintmax_t line_count;
            fs::ofstream out;
        };

        size_t num_outputs;
        output_progress *output_progress_list;
        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::uniform_real_distribution<> dist(0, 1);

        num_outputs = output_spec_list.size();
        output_progress_list = new output_progress[num_outputs];

        for (size_t i = 0; i < num_outputs; i++)
        {
            auto& spec = output_spec_list[i];
            if (spec.number_of_lines != 0)
            {
                throw std::logic_error("Taget lines is not allowed.");
            }
            else if (spec.rate >= 0)
            {
                output_progress_list[i].random_threshold = spec.rate;
            }
            else
            {
                throw std::logic_error("None of taget lines or rate is specified.");
            }
            auto &out = output_progress_list[i].out;
            out.open(spec.file_name, std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                std::wcerr << __wcserror(spec.file_name.native().c_str());
                return;
            }
        }

        for (auto &file_name : input_path_list)
        {
            file_line_source_default<CharT>(file_name, [&dist, &gen, output_progress_list, num_outputs](const CharT *s, size_t len)
            {
                double t = dist(gen);
                for (int i = 0; i < num_outputs; i++)
                {
                    auto &prog = output_progress_list[i];
                    if (t < prog.random_threshold)
                    {
                        prog.out.write(reinterpret_cast<const char *>(s), sizeof(CharT) * len);
                        ++prog.line_count;
                        break;
                    }
                    t -= prog.random_threshold;
                }
            });
        }

        delete[] output_progress_list;
    }

    template<typename CharT>
    void file_shuffle_lines(const std::vector<fs::path> &input_file_name_list, const std::vector<sample_output_spec> &output_spec_list)
    {
        std::vector<size_t> line_index_list;
        std::vector<const CharT *> line_position_list;

        std::vector<ios::mapped_file_source> file_list;
        size_t line_index = 0;
        for (auto &input_file_name : input_file_name_list)
        {
            size_t prev_line_index = line_index;
            file_list.emplace_back();
            auto &file = file_list.back();
            file.open(input_file_name);
            if (!file.is_open())
            {
                std::wcerr << __wcserror(input_file_name.native().c_str());
                return;
            }

            const CharT *s = reinterpret_cast<const CharT *>(file.data());
            size_t len = file.size() / sizeof(CharT);
            std::wcout << input_file_name.native() << "\tCharCount\t" << len << std::endl;

            line_position_list.push_back(s);
            for (size_t i = 0; i < len; i++)
            {
                if (s[i] == '\n')
                {
                    line_index_list.push_back(line_index++);
                    line_position_list.push_back(&s[i + 1]);
                }
            }
            if (s[len - 1] != '\n')
            {
                line_index_list.push_back(line_index++);
                line_position_list.push_back(&s[len]);
            }

            std::wcout << input_file_name.native() << "\tLineCount\t" << (line_index - prev_line_index) << std::endl;
            line_index++;
        }

        // Shuffle lines

        size_t num_lines = line_index_list.size();
        std::cout << "\tLineCount\t" << num_lines << std::endl;
        if (line_index - input_file_name_list.size() != num_lines)
        {
            std::wcerr << "something wrong" << std::endl;
            return;
        }

        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::random_number_generator<rnd::mt19937_64, size_t> dist(gen);
        for (size_t i = 0; i < num_lines - 1; i++)
        {
            size_t j = i + dist(num_lines - i);
            std::swap(line_index_list[i], line_index_list[j]);
        }

        // Write lines

        size_t cur_index = 0;
        for (auto &output_spec : output_spec_list)
        {
            uintmax_t line_count;
            if (output_spec.number_of_lines > 0)
            {
                line_count = output_spec.number_of_lines;
            }
            else if (output_spec.rate >= 1.0)
            {
                line_count = num_lines;
            }
            else
            {
                line_count = static_cast<uintmax_t>(num_lines * output_spec.rate + 0.5);
            }

            std::wcerr << output_spec.file_name << "\tLineCount\t" << min(line_count, num_lines - cur_index) << std::endl;

            fs::basic_ofstream<CharT> out;
            out.open(output_spec.file_name, std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                std::wcerr << __wcserror(output_spec.file_name.native().c_str());
                return;
            }

            for (uintmax_t i = 0; i < line_count; i++)
            {
                if (cur_index >= num_lines)
                {
                    break;
                }

                size_t n = line_index_list[cur_index++];
                const CharT *first = line_position_list[n];
                const CharT *last = line_position_list[n + 1];
                out.write(first, last - first);
            }
        }
    }

    template<typename CharT>
    void file_shuffle_lines(const std::vector<fs::path> &input_file_name_list, const std::vector<sample_output_spec> &output_spec_list, uintmax_t interleaving_size, size_t max_buffer_size)
    {
        std::wcout << "\tInterleavingSize\t" << interleaving_size << std::endl;
        std::vector<const CharT *> line_position_list;
        std::wcout << "\tMaxBufferSize\t" << max_buffer_size << std::endl;
        heap_vector<CharT> heap(max_buffer_size / sizeof(CharT));

        size_t line_index = 0;
        CharT *p = heap.ptr();
        CharT *last = heap.ptr() + max_buffer_size / sizeof(CharT);
        for (uintmax_t slice_start = interleaving_size; slice_start > 0; --slice_start)
        {
            std::wcout << "\tCurrentSlice\t" << slice_start << std::endl;
            uintmax_t current_slice = slice_start;
            uintmax_t line_count = 0;

            for (auto &input_file_name : input_file_name_list)
            {
                line_position_list.push_back(p);
                std::wcout << input_file_name.native() << "\tReading" << std::endl;
                file_line_source_default<CharT>(input_file_name, [&p, last, &current_slice, &line_position_list, &line_count, interleaving_size](const CharT *s, size_t len)
                {
                    if (len > 0)
                    {
                        if (--current_slice == 0)
                        {
                            std::memcpy(p, s, len * sizeof(CharT));
                            p += len;
                            line_position_list.push_back(p);
                            current_slice = interleaving_size;
                        }
                        line_count++;
                    }
                });
            }

            // Shuffle lines

            std::vector<size_t> line_index_list;
            uintmax_t num_lines = line_position_list.size() - 1;
            std::wcout << "Shuffling " << num_lines << " lines" << std::endl;
            std::wcout.flush();
            line_index_list.resize(num_lines);
            for (size_t i = 0; i != num_lines; i++)
            {
                line_index_list[i] = i;
            }

            rnd::mt19937_64 gen(std::time(nullptr));
            rnd::random_number_generator<rnd::mt19937_64, size_t> dist(gen);
            for (size_t i = 0; i < num_lines - 1; i++)
            {
                size_t j = i + dist(num_lines - i);
                std::swap(line_index_list[i], line_index_list[j]);
            }

            // Write lines

            size_t cur_index = 0;
            for (auto &output_spec : output_spec_list)
            {
                uintmax_t line_count;
                if (output_spec.number_of_lines > 0)
                {
                    line_count = output_spec.number_of_lines;
                }
                else if (output_spec.rate >= 1.0)
                {
                    line_count = num_lines;
                }
                else
                {
                    line_count = static_cast<uintmax_t>(num_lines * output_spec.rate + 0.5);
                }

                std::wcerr << output_spec.file_name.native() << "\tLineCount\t" << min(line_count, num_lines - cur_index) << std::endl;

                fs::basic_ofstream<CharT> out;
                int mode = std::ios::out | std::ios::binary;
                if (slice_start != interleaving_size)
                {
                    mode |= std::ios::app;
                }
                out.open(output_spec.file_name, mode);
                if (!out.is_open())
                {
                    std::wcerr << __wcserror(output_spec.file_name.native().c_str());
                    return;
                }

                for (uintmax_t i = 0; i < line_count; i++)
                {
                    if (cur_index >= num_lines)
                    {
                        break;
                    }

                    size_t n = line_index_list[cur_index++];
                    const CharT *first = line_position_list[n];
                    const CharT *last = line_position_list[n + 1];
                    out.write(first, last - first);
                }
            }

            line_position_list.clear();
        }
    }

    template <typename CharT>
    void file_quick_sample_file_lines(fs::path &input_file_name, const std::vector<sample_output_spec> &output_spec_list, bool shuffle_output)
    {
        static_assert(sizeof(CharT) == sizeof(char), "Only char type is supported.");
        rnd::mt19937_64 gen(std::time(nullptr));
        uintmax_t file_size = fs::file_size(input_file_name);
        rnd::uniform_int_distribution<std::streamoff> dist(0, file_size);
        fs::ifstream fin(input_file_name);
        if (!fin.is_open())
        {
            return;
        }
        for (auto &spec : output_spec_list)
        {
            std::vector<std::basic_string<CharT>> line_list;
            for (int i = 0; i < spec.number_of_lines; i++)
            {
                uintmax_t pos = dist(gen);
                fin.seekg(pos);
                std::basic_string<CharT> line;
                std::getline(fin, line);
                if (fin.eof())
                {
                    fin.clear();
                    fin.seekg(0);
                }
                std::getline(fin, line);
                if (fin.eof())
                {
                    fin.clear();
                }
                line_list.push_back(std::move(line));
            }
            fs::ofstream fout(spec.file_name, std::ios::out | std::ios::binary);
            for (auto &line : line_list)
            {
                fout << line << '\n';
            }
        }
    }
}