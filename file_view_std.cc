#include "file_view.hh"

#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace {
std::optional<std::vector<uint8_t>> load_file(std::string_view file_name)
{
    std::vector<uint8_t> output;

    std::ifstream f;
    f.open(file_name);

    if (!f.is_open()) {
        return std::nullopt;
    }

    f.tellg();

    std::array<char, 8196> read_buff;
    size_t readen = 0;

    while (!f.eof()) {
        f.read(read_buff.data(), read_buff.size());
        readen = f.gcount();

        std::ranges::copy(
            read_buff | std::views::take(readen), std::back_inserter(output));
    }

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

    Impl(const char *name)
    {
        auto data_mb = load_file(name);
        if (!data_mb.has_value()) {
            throw std::runtime_error(
                std::format("File \"{}\" open failue", name));
        }

        m_data = *data_mb;
    }

    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;
    Impl &operator=(const Impl &) = delete;
    Impl &operator=(Impl &&) = delete;
    ~Impl() = default;

    const char *data()
    {
        return reinterpret_cast<const char *>(m_data.data());
    }

    size_t size() const
    {
        return m_data.size();
    }

  private:
    std::vector<uint8_t> m_data;
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

