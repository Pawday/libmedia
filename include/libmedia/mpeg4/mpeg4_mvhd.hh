#pragma once

#include <array>
#include <optional>
#include <utility>

#include <cstddef>
#include <cstdint>

#include "mpeg4.hh"
#include "raw_data.hh"

namespace Mpeg4 {

struct BoxViewMovieHeader
{
    constexpr static TypeTag mvhd_tag = TypeTag::from_str("mvhd");

    BoxViewMovieHeader(FullBoxView box) : m_box(box)
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
        required_size += sizeof(uint32_t);     // rate
        required_size += sizeof(uint16_t);     // volume
        required_size += sizeof(uint16_t);     // reserved_0
        required_size += sizeof(uint32_t) * 2; // reserved_1
        required_size += sizeof(uint32_t) * 9; // matrix
        required_size += sizeof(uint32_t) * 6; // pre_defined
        required_size += sizeof(uint32_t);     // next_track_ID
        if (required_size > data->size()) {
            return false;
        }

        BoxHeader base_header = full_header->header;
        if (full_header->header.type != mvhd_tag) {
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
    std::optional<std::array<uint16_t, 2>> get_rate() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        auto data = m_box.get_data().value().subspan(offset);
        return std::array<uint16_t, 2>{
            read_be<uint16_t>(data),
            read_be<uint16_t>(data.subspan(sizeof(uint16_t)))};
    }
    std::optional<std::array<uint8_t, 2>> get_volume() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t); // rate
        auto data = m_box.get_data().value().subspan(offset);
        return std::array<uint8_t, 2>{
            std::to_integer<uint8_t>(data[0]),
            std::to_integer<uint8_t>(data[1])};
    }
    std::optional<uint16_t> get_reserved_0() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t); // rate
        offset += sizeof(uint16_t); // volume
        auto data = m_box.get_data().value().subspan(offset);
        return read_le<uint16_t>(data);
    }
    std::optional<std::array<uint32_t, 2>> get_reserved_1() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t); // rate
        offset += sizeof(uint16_t); // volume
        offset += sizeof(uint16_t); // reserved_0
        auto data = m_box.get_data().value().subspan(offset);

        uint32_t v0 = read_be<uint32_t>(data.subspan(0));
        uint32_t v1 = read_be<uint32_t>(data.subspan(sizeof(uint32_t)));

        return std::array<uint32_t, 2>{v0, v1};
    }
    std::optional<std::array<uint32_t, 9>> get_matrix() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t);     // rate
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_0
        offset += sizeof(uint32_t) * 2; // reserved_1
        auto data = m_box.get_data().value().subspan(offset);

        std::array<uint32_t, 9> output;
        for (auto &out : output) {
            out = read_be<uint32_t>(data);
            data = data.subspan(sizeof(uint32_t));
        }
        return output;
    }
    std::optional<std::array<uint32_t, 6>> get_pre_defined() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t);     // rate
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_0
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint32_t) * 9; // matrix
        auto data = m_box.get_data().value().subspan(offset);

        std::array<uint32_t, 6> output;
        for (auto &out : output) {
            out =
                from_array_as_le<uint32_t>(copy_array<sizeof(uint32_t)>(data));
            data = data.subspan(sizeof(uint32_t));
        }
        return output;
    }
    std::optional<uint32_t> get_next_track_ID() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t);     // rate
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_0
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint32_t) * 9; // matrix
        offset += sizeof(uint32_t) * 6; // pre_defined
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint32_t>(data);
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
