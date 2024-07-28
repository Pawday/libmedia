#include <algorithm>
#include <cstddef>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <stringapiset.h>
#include <winbase.h>
#include <winnls.h>
#include <winnt.h>

#include "file_view.hh"

namespace {

std::string win32_strerr(DWORD err_code)
{
    LPSTR message_buffer = nullptr;
    size_t message_len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr,
        err_code,
        0,
        reinterpret_cast<LPSTR>(&message_buffer),
        0,
        nullptr
    );

    std::string output(message_buffer, message_len);
    LocalFree(message_buffer);

    std::transform(
        std::begin(output),
        std::end(output),
        std::begin(output),
        [](const char c) {
            if (c == '\n') {
                return ' ';
            }
            if (c == '\r') {
                return ' ';
            }
            return c;
        }
    );

    return output;
}

} // namespace

struct FileView::Impl
{
    static FileView::Impl &cast(FileView &f)
    {
        return *reinterpret_cast<FileView::Impl *>(f.impl);
    }

    static const FileView::Impl &cast(const FileView &f)
    {
        return *reinterpret_cast<const FileView::Impl *>(f.impl);
    }

    static bool assert_impl()
    {
        static_assert(sizeof(FileView::Impl) <= FileView::impl_size);
        static_assert(alignof(FileView) >= alignof(FileView::Impl));
        return true;
    }

    Impl(const char *file_name) : m_file_name(file_name)
    {
        auto convert_size = MultiByteToWideChar(
            CP_UTF8, 0, m_file_name.c_str(), m_file_name.size(), nullptr, 0
        );
        if (convert_size == 0) {
            auto status = GetLastError();
            throw std::runtime_error(std::format(
                "Unicode filename size determine for file \"{}\" failue, "
                "status {} ({})",
                file_name,
                status,
                win32_strerr(status)
            ));
        }

        std::vector<WCHAR> file_name_wstr;
        file_name_wstr.resize(convert_size + 1);

        convert_size = MultiByteToWideChar(
            CP_UTF8,
            0,
            m_file_name.c_str(),
            m_file_name.size(),
            file_name_wstr.data(),
            file_name_wstr.size()
        );
        if (convert_size == 0) {
            auto status = GetLastError();
            throw std::runtime_error(std::format(
                "Unicode filename convert for file \"{}\" failue, "
                "status {} ({})",
                file_name,
                status,
                win32_strerr(status)
            ));
        }

        auto handle = CreateFileW(
            file_name_wstr.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (INVALID_HANDLE_VALUE == handle) {
            auto status = GetLastError();
            throw std::runtime_error(std::format(
                "Open file \"{}\" failue, status {} ({})",
                file_name,
                status,
                win32_strerr(status)
            ));
        }

        auto map_handle =
            CreateFileMappingW(handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (nullptr == map_handle) {
            auto status = GetLastError();
            CloseHandle(handle);
            throw std::runtime_error(std::format(
                "Create Maping object for file \"{}\" failue, status {} ({})",
                file_name,
                status,
                win32_strerr(status)
            ));
        }

        auto data = MapViewOfFile(map_handle, FILE_MAP_READ, 0, 0, 0);
        if (data == nullptr) {
            auto status = GetLastError();
            CloseHandle(map_handle);
            CloseHandle(handle);
            throw std::runtime_error(std::format(
                "Maping file \"{}\" failue, status {} ({})",
                file_name,
                status,
                win32_strerr(status)
            ));
        }

        m_file_handle = handle;
        m_file_map_handle = map_handle;
        m_data = data;
    }

    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;

    Impl &operator=(const Impl &) = delete;
    Impl &operator=(Impl &&) = delete;

    const char *data()
    {
        return static_cast<const char *>(m_data);
    }
    size_t size() const
    {
        return full_map_size();
    }

    ~Impl()
    {
        UnmapViewOfFile(m_data);
        CloseHandle(m_file_map_handle);
        CloseHandle(m_file_handle);
    }

  private:
    size_t full_map_size() const
    {
        MEMORY_BASIC_INFORMATION i;
        std::memset(&i, 0, sizeof(i));
        size_t info_size = VirtualQuery(m_data, &i, sizeof(i));
        if (info_size == 0) {
            auto status = GetLastError();
            throw std::runtime_error(std::format(
                "Aftermap sizeof file \"{}\" retreave failue, status {} "
                "({})",
                m_file_name,
                status,
                win32_strerr(status)
            ));
        }

        return i.RegionSize;
    }

    std::string m_file_name;
    HANDLE m_file_handle = INVALID_HANDLE_VALUE;
    HANDLE m_file_map_handle = nullptr;
    const void *m_data = nullptr;
};


FileView::FileView(const char *name)
{
    new (impl) FileView::Impl(name);
}

const char *FileView::data()
{
    return Impl::cast(*this).data();
}

size_t FileView::size() const
{
    return Impl::cast(*this).size();
}

FileView::~FileView()
{
    Impl::cast(*this).~Impl();
}
