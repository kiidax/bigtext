/* bigtext - bigtext is a collection of tools to process large text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

namespace bigtext {

    namespace fs = boost::filesystem;

    extern const wchar_t* PROGRAM_NAME;
    extern const int MAJOR_VERSION;
    extern const int MINOR_VERSION;
    extern const int BUILD_VERSION;
    extern const int REVISION_VERSION;

    int main(int argc, wchar_t *argv[]);
    int count_command(int argc, wchar_t *argv[]);
    int sample_command(int argc, wchar_t *argv[]);
    int vocab_command(int argc, wchar_t *argv[]);
    int version_command(int argc, wchar_t *argv[]);
    std::wstring get_version_string();
    bool check_input_files(const std::vector<fs::path> &input_file_name_list);
    bool check_output_files(const std::vector<fs::path> &output_file_name_list);
    bool try_parse_rate(const std::wstring &s, double &rate);
    bool try_parse_number(const std::wstring &s, uintmax_t &number_of_lines);
    uintmax_t get_physical_memory_size();

    template <typename CharT>
    bool is_new_line(CharT ch)
    {
        return ch == '\n';
    }

    template <typename CharT>
    bool is_white_space(CharT ch)
    {
        return ch >= '\0' && ch <= ' ';
    }

#ifdef WIN32
    template <typename T>
    class heap_vector
    {
    public:
        heap_vector() : ptr_(NULL)
        {
        }

        ~heap_vector()
        {
            if (ptr_ != NULL)
            {
                VirtualFree(ptr_, 0, MEM_RELEASE);
            }
        }

        T *ptr() const { return reinterpret_cast<T *>(ptr_); }
        size_t size() const { return size_ / sizeof T; }

        void alloc(size_t min_size, size_t max_size)
        {
            if (ptr_ != NULL)
            {
                throw std::bad_alloc();
            }
            size_t cur_size = max_size;
            while (cur_size >= min_size)
            {
                ptr_ = VirtualAlloc(NULL, cur_size, MEM_COMMIT, PAGE_READWRITE);
                if (ptr_ != NULL)
                {
                    size_ = cur_size;
                    return;
                }
                cur_size = cur_size * 8 / 10;
            }
            throw std::bad_alloc();
        }

        void clear()
        {
            VirtualAlloc(ptr_, size_, MEM_RESET, PAGE_READWRITE);
        }

    private:
        LPVOID ptr_;
        size_t size_;
    };
#endif
}