#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <ranges>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "libmedia/mpeg4.hh"
#include "libmedia/raw_data.hh"

namespace Mpeg4 {

struct FileTypeBoxView
{
    constexpr static TypeTag ftyp_tag = TypeTag::from_str("ftyp");
    using Brand_t = std::array<char, 4>;

    bool validate() const
    {
        auto header = m_box.get_header();
        auto data = m_box.get_content_data();
        if (!header || !data) {
            return false;
        }

        if (header->type != ftyp_tag) {
            return false;
        }

        if (data->size() < 8) {
            return false;
        }

        size_t brands_data_size = data->size() - 8;
        if (brands_data_size % 4 != 0) {
            return false;
        }

        return true;
    }

    bool is_valid() const
    {
        return validate();
    }

    bool is_not_valid() const
    {
        return !is_valid();
    }

    std::optional<Brand_t> get_major_brand() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        auto data = m_box.get_content_data();
        if (!data) {
            return std::nullopt;
        }

        if (data->size() < 4) {
            return std::nullopt;
        }

        Brand_t output;
        auto arr = copy_array<4>(data.value());
        std::ranges::copy(
            arr | std::views::transform(std::to_integer<char>), output.begin());
        return output;
    }

    std::optional<uint32_t> get_minor_version() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        return read_be<uint32_t>(m_box.get_content_data().value().subspan(4));
    }

    std::optional<std::vector<Brand_t>> get_compatible_brands() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        std::vector<Brand_t> output;
        auto brands_data = m_box.get_content_data().value().subspan(8);

        if (brands_data.size() % 4 != 0) {
            return std::nullopt;
        }

        size_t nb_brands = brands_data.size() / 4;
        output.reserve(nb_brands);

        for (size_t brand_idx = 0; brand_idx < nb_brands; brand_idx++) {
            Brand_t next_brand;
            auto arr = copy_array<4>(brands_data);
            std::ranges::copy(
                arr | std::views::transform(std::to_integer<char>),
                next_brand.begin());
            output.emplace_back(next_brand);
            brands_data = brands_data.subspan(4);
        }

        return output;
    }

    FileTypeBoxView(BoxView box) : m_box(box)
    {
    }

  private:
    BoxView m_box;
};

} // namespace Mpeg4
