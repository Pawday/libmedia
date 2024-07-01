#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>

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

consteval BoxHeader::TypeTag make_tag(std::string_view s)
{
    if (s.size() != 4) {
        throw std::runtime_error("Tag can only be 4 bytes long");
    }

    uint8_t b0 = s[0];
    uint8_t b1 = s[1];
    uint8_t b2 = s[2];
    uint8_t b3 = s[3];
    return BoxHeader::TypeTag{b0, b1, b2, b3};
}

struct FullBoxHeader
{
    BoxHeader header;
    uint8_t version;
    std::bitset<24> flags;
};

constexpr bool
    operator==(const BoxHeader::TypeTag &lhs, const BoxHeader::TypeTag &rhs)
{
    return lhs.data == rhs.data;
};

struct BoxView
{
    constexpr BoxView(std::span<const std::byte> data) : m_data(data)
    {
    }

    constexpr std::optional<BoxHeader> get_header() const
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
            if (size.value() < header_size) {
                if consteval {
                    throw std::logic_error(
                        /*
                         * ISO/IEC 14496-12 4.2 Object Structure
                         * size is an integer that specifies the number of bytes
                         * in this box, **including all its fields**
                         */
                        "size of the box is less than it's header size");
                }
                return std::nullopt;
            }
            size = size.value() - header_size;
        }

        return BoxHeader{header_size, size, type, user_type};
    }

    constexpr std::optional<std::span<const std::byte>> get_data() const
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

    std::optional<FullBoxHeader> get_header() const
    {
        auto header = m_box.get_header();
        auto version = get_version();
        auto flags = get_flags();
        if (!header || !version || !flags) {
            return std::nullopt;
        }

        return FullBoxHeader{header.value(), version.value(), flags.value()};
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
        auto bitset_data = box_data.subspan<1, 3>();
        output |= std::to_integer<uint8_t>(bitset_data[0]) << (8 * 2);
        output |= std::to_integer<uint8_t>(bitset_data[1]) << (8 * 1);
        output |= std::to_integer<uint8_t>(bitset_data[2]) << (8 * 0);
        return output;
    }

  private:
    BoxView m_box;
};

}; // namespace Mpeg4
