#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

#include "mpeg4.hh"
#include "raw_data.hh"

namespace Mpeg4 {

struct BoxViewMediaHeader
{
    constexpr static TypeTag mdia_tag = TypeTag::from_str("mdhd");

    BoxViewMediaHeader(FullBoxView box) : m_box(box)
    {
    }

    bool validate() const
    {
        std::optional<FullBoxHeader> full_header = m_box.get_header();
        auto data = m_box.get_data();
        auto version = m_box.get_version();
        if (!full_header || !data || !version) {
            return false;
        }

        switch (version.value()) {
        case 0:
        case 1:
            break;
        default:
            return false;
        }

        size_t required_size = version_depended_header_size(version.value());
        required_size +=
            2; // 1 (pad_bit) + 5 * 3 (language bits) = 16 bits -> 2 bytes
        required_size += sizeof(uint16_t); // pre_defined
        if (required_size > data->size()) {
            return false;
        }

        BoxHeader base_header = full_header->header;
        if (full_header->header.type != mdia_tag) {
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

    std::optional<uint64_t> get_creation_time() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        uint8_t version = m_box.get_version().value();
        auto data = m_box.get_data().value().subspan(0);

        switch (version) {
        case 0:
            return read_be<uint32_t>(data);
        case 1:
            return read_be<uint64_t>(data);
        }
        std::unreachable();
    }

    std::optional<uint64_t> get_modification_time() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        uint8_t version = m_box.get_version().value();

        size_t offset = 0;
        if (version == 0) {
            offset += sizeof(uint32_t);
        } else {
            offset += sizeof(uint64_t);
        };
        auto data = m_box.get_data().value().subspan(offset);

        switch (version) {
        case 0:
            return read_be<uint32_t>(data);
        case 1:
            return read_be<uint64_t>(data);
        }
        std::unreachable();
    }
    std::optional<uint32_t> get_timescale() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        uint8_t version = m_box.get_version().value();
        size_t offset = 0;
        if (version == 0) {
            offset += sizeof(uint32_t);
            offset += sizeof(uint32_t);
        } else {
            offset += sizeof(uint64_t);
            offset += sizeof(uint64_t);
        };
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint32_t>(data);
    }
    std::optional<uint64_t> get_duration() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        uint8_t version = m_box.get_version().value();
        size_t offset = 0;
        if (version == 0) {
            offset += sizeof(uint32_t);
            offset += sizeof(uint32_t);
            offset += sizeof(uint32_t);
        } else {
            offset += sizeof(uint64_t);
            offset += sizeof(uint64_t);
            offset += sizeof(uint32_t);
        };
        auto data = m_box.get_data().value().subspan(offset);
        switch (version) {
        case 0:
            return read_be<uint32_t>(data);
        case 1:
            return read_be<uint64_t>(data);
        }
        std::unreachable();
    }

    std::optional<bool> get_pad() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        auto data = m_box.get_data().value().subspan(offset);

        uint8_t byte_with_pad_bit = std::to_integer<uint8_t>(data[0]);

        return (byte_with_pad_bit & 0b10000000) != 0;
    }

    // Each character is packed as the difference between its ASCII value and
    // 0x60
    std::optional<std::array<std::byte, 3>> get_language() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        auto data = m_box.get_data().value().subspan(offset);

        std::array<std::byte, 3> output;

        /*
         * 0b0111112222233333
         * 0b0000000011111111
         *   |      ||      |
         *   01111100|      |   output[0] mask
         *           |      |
         *           00011111   output[2] mask
         *
         */
        std::array<std::byte, 2> compressed_output = copy_array<2>(data);

        output[0] = compressed_output[0];
        output[0] >>= 2;
        output[0] &= std::byte(0b00011111);

        std::byte output_1_msb = compressed_output[0];
        output_1_msb <<= 3;
        output_1_msb &= std::byte(0b00011000);

        std::byte output_1_lsb = compressed_output[1];
        output_1_lsb >>= 5;
        output_1_lsb &= std::byte(0b00000111);

        output[1] = output_1_msb | output_1_lsb;

        output[2] = compressed_output[1];
        output[2] &= std::byte(0b00011111);

        return output;
    }

    std::optional<uint16_t> get_pre_defined() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint16_t); // pad + language

        auto data = m_box.get_data().value().subspan(offset);

        return read_be<uint16_t>(data);
    }

  private:
    FullBoxView m_box;

    size_t version_depended_header_size(uint8_t version) const
    {
        size_t output = 0;
        if (version == 0) {
            output += sizeof(uint32_t); // creation_time v0
            output += sizeof(uint32_t); // modification_time v0
            output += sizeof(uint32_t); // timescale v0
            output += sizeof(uint32_t); // duration v0
        }
        if (version == 1) {
            output += sizeof(uint64_t); // creation_time v1
            output += sizeof(uint64_t); // modification_time v1
            output += sizeof(uint32_t); // timescale v1
            output += sizeof(uint64_t); // duration v1
        }
        return output;
    }
};
} // namespace Mpeg4
