#pragma once

#include <cstddef>

struct alignas(8) FileView
{
    FileView(const char *name);

    FileView(const FileView &) = delete;
    FileView(FileView &&) = delete;
    FileView &operator=(const FileView &) = delete;
    FileView &operator=(FileView &&) = delete;
    ~FileView();

    const char *data();
    size_t size() const;

  private:
    static constexpr size_t impl_size = 32;
    struct Impl;
    char impl[impl_size];

};
