#include "file_view.hh"

#include <cerrno>
#include <cstring>
#include <format>
#include <stdexcept>

#include <cstddef>
#include <cstdint>

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

    Impl(const char *name)
    {
        int new_fd = open(name, O_RDONLY, 0);
        auto status = errno;
        if (new_fd < 0) {
            std::string status_string = strerror(status);
            throw std::invalid_argument(std::format(
                "File \"{}\" open failue: status {} ({})",
                name,
                status,
                status_string));
        }
        fd = new_fd;

        struct stat st;
        if (fstat(new_fd, &st) != 0) {
            auto status = errno;
            std::string status_string = strerror(status);
            close(fd);
            throw std::invalid_argument(std::format(
                "File \"{}\" stat failue: status {} ({})",
                name,
                status,
                status_string));
        }

        m_size = st.st_size;

        void *data = mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if (data == MAP_FAILED) {
            auto status = errno;
            std::string status_string = strerror(status);
            close(fd);
            throw std::invalid_argument(std::format(
                "File \"{}\" mmap failue: status {} ({})",
                name,
                status,
                status_string));
        }

        file_data = static_cast<uint8_t *>(data);
    }

    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;
    Impl &operator=(const Impl &) = delete;
    Impl &operator=(Impl &&) = delete;
    ~Impl()
    {
        munmap(file_data, m_size);
        close(fd);
    }

    const char *data()
    {
        return static_cast<const char *>(file_data);
    }

    size_t size() const
    {
        return m_size;
    }

  private:
    int fd = -1;
    size_t m_size = 0;
    void *file_data = nullptr;
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

