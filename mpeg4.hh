#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include "raw_data.hh"

namespace Mpeg4 {

struct BoxHeader
{
    struct TypeTag
    {
        std::array<uint8_t, 4> data;
    };

    struct UserType
    {
        std::array<uint8_t, 16> data;
    };

    static constexpr TypeTag uuid{'u', 'u', 'i', 'd'};

    uint8_t header_size;
    std::optional<uint64_t> box_size;
    TypeTag type;
    std::optional<UserType> usertype;
};

constexpr bool
    operator==(const BoxHeader::TypeTag &lhs, const BoxHeader::TypeTag &rhs)
{
    return lhs.data == rhs.data;
};

struct BoxView
{
    BoxView(std::span<const std::byte> data) : m_data(data)
    {
    }

    std::optional<BoxHeader> get_header() const
    {
        auto data = m_data;
        uint8_t header_size = 0;
        std::optional<uint64_t> size = std::nullopt;
        std::optional<BoxHeader::UserType> user_type;

        if (data.size() < sizeof(uint32_t)) {
            return std::nullopt;
        }
        uint32_t size_32 = read_be<uint32_t>(data);
        data = data.subspan(sizeof(uint32_t));
        header_size += sizeof(uint32_t);

        if (data.size() < 4) {
            return std::nullopt;
        }
        std::array<std::uint8_t, 4> type_data =
            to_uint8_arr(copy_array<4>(data));
        data = data.subspan(4);
        header_size += 4;

        BoxHeader::TypeTag type{type_data};

        if (size_32 == 1) {
            if (data.size() < sizeof(uint64_t)) {
                return std::nullopt;
            }

            size = read_be<uint64_t>(data);
            data = data.subspan(sizeof(uint64_t));
            header_size += sizeof(uint64_t);

        } else if (size_32 != 0) {
            size = size_32;
        }

        if (type == BoxHeader::uuid) {
            if (data.size() < 16) {
                return std::nullopt;
            }

            std::array<std::uint8_t, 16> user_type_data =
                to_uint8_arr(copy_array<16>(data));
            data = data.subspan(16);
            header_size += 16;

            user_type = BoxHeader::UserType{user_type_data};
        }

        if (size.has_value()) {

            if (*size < header_size) {
                return std::nullopt;
            }

            *size -= header_size;
        }

        return BoxHeader{header_size, size, type, user_type};
    }

    std::optional<std::span<const std::byte>> get_data() const
    {
        auto header = get_header();
        if (!header) {
            return std::nullopt;
        }

        auto box_data = m_data.subspan(header->header_size);
        if (box_data.size() < header->box_size) {
            return std::nullopt;
        }

        if (header->box_size) {
            box_data = box_data.subspan(0, *header->box_size);
        }

        return box_data;
    }

  private:
    std::span<const std::byte> m_data;
};

struct FullBoxView
{
    FullBoxView(BoxView box) : m_box(box)
    {
    }

    std::optional<BoxHeader> get_header() const
    {
        return m_box.get_header();
    }

    std::optional<std::span<const std::byte>> get_data() const
    {
        auto box_data_opt = m_box.get_data();
        if (!box_data_opt) {
            return std::nullopt;
        }
        auto box_data = box_data_opt.value();
        if (box_data.size() < 4) {
            return std::nullopt;
        }

        return box_data.subspan(4);
    }

    std::optional<uint8_t> get_version() const
    {
        auto box_data_opt = m_box.get_data();
        if (!box_data_opt) {
            return std::nullopt;
        }
        auto box_data = box_data_opt.value();

        if (box_data.size() < 1) {
            return std::nullopt;
        }

        return std::to_integer<uint8_t>(box_data[0]);
    }

    std::optional<std::bitset<24>> get_flags() const
    {
        auto box_data_opt = m_box.get_data();
        if (!box_data_opt) {
            return std::nullopt;
        }
        auto box_data = box_data_opt.value();

        if (box_data.size() < 4) {
            return std::nullopt;
        }

        std::bitset<24> output = 0;
        auto biset_data = box_data.subspan<1, 3>();

        for (auto b : biset_data) {
            output |= std::to_integer<uint8_t>(b);
            output <<= 8;
        }
        return output;
    }

  private:
    BoxView m_box;
};

struct BoxViewFileType
{
    constexpr static BoxHeader::TypeTag ftyp_tag = {'f', 't', 'y', 'p'};
    using Brand_t = std::array<char, 4>;

    bool validate() const
    {
        auto header = m_box.get_header();
        auto data = m_box.get_data();
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

        auto data = m_box.get_data();
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

        auto data_opt = m_box.get_data();
        if (!data_opt) {
            return std::nullopt;
        }
        auto data = data_opt.value();

        return read_be<uint32_t>(m_box.get_data().value().subspan(4));
    }

    std::optional<std::vector<Brand_t>> get_compatible_brands() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        std::vector<Brand_t> output;
        auto brands_data = m_box.get_data().value().subspan(8);

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

    BoxViewFileType(BoxView box) : m_box(box)
    {
    }

  private:
    BoxView m_box;
};

}; // namespace Mpeg4
