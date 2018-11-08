#pragma once

#include <string>
#include <sys/stat.h>
#include <vector>

template <class ElementType, std::ptrdiff_t Extent>
struct PtrCompat 
{
    using element_type = ElementType;
    using index_type = std::ptrdiff_t;
    using pointer = element_type*;
    using reference = element_type&;

    constexpr PtrCompat(pointer ptr, index_type count) : data_(ptr), count_(count) {}

    constexpr index_type size() const noexcept { return count_; }

    constexpr reference operator[](index_type idx) const
    {
        return data_[idx];
    }

private:
    pointer data_;
    index_type count_;
};

class Utils {
public:
    static bool createParentDir(const std::string& filePath, mode_t mode = 0755);

private:
    static void splitString(const std::string& path, const std::string& delim, std::vector<std::string>& out);
};
