#pragma once

#include <array>
#include <optional>
#include <utility>

#include <cstddef>
#include <cstdint>

#include "mpeg4.hh"
#include "raw_data.hh"

namespace Mpeg4 {

struct BoxViewTrackHeader
{
    constexpr static BoxHeader::TypeTag tkhd_tag = {'t', 'k', 'h', 'd'};

    BoxViewTrackHeader(FullBoxView box) : m_box(box)
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
        required_size += sizeof(uint32_t) * 2; // reserved_1
        required_size += sizeof(uint16_t);     // layer
        required_size += sizeof(uint16_t);     // alternate_group
        required_size += sizeof(uint16_t);     // volume
        required_size += sizeof(uint16_t);     // reserved_2
        required_size += sizeof(uint32_t) * 9; // matrix
        required_size += sizeof(uint32_t);     // width
        required_size += sizeof(uint32_t);     // height
        if (required_size > data->size()) {
            return false;
        }

        BoxHeader base_header = full_header->header;
        if (full_header->header.type != tkhd_tag) {
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

    std::optional<uint32_t> get_track_ID() const
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

    std::optional<uint32_t> get_reserved_0() const
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
            offset += sizeof(uint32_t);
        } else {
            offset += sizeof(uint64_t);
            offset += sizeof(uint64_t);
            offset += sizeof(uint32_t);
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

    std::optional<std::array<uint32_t, 2>> get_reserved_1() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        auto data = m_box.get_data().value().subspan(offset);
        return std::array<uint32_t, 2>{
            read_be<uint32_t>(data),
            read_be<uint32_t>(data.subspan(sizeof(uint32_t)))};
    }

    std::optional<uint16_t> get_layer() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint16_t>(data);
    }

    std::optional<uint16_t> get_alternate_group() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint16_t>(data);
    }

    std::optional<uint16_t> get_volume() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        offset += sizeof(uint16_t);     // alternate_group
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint16_t>(data);
    }

    std::optional<uint16_t> get_reserved_2() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        offset += sizeof(uint16_t);     // alternate_group
        offset += sizeof(uint16_t);     // volume
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint16_t>(data);
    }

    std::optional<std::array<uint32_t, 9>> get_matrix() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        offset += sizeof(uint16_t);     // alternate_group
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_2
        auto data = m_box.get_data().value().subspan(offset);

        std::array<uint32_t, 9> output;
        for (auto &o : output) {
            o = read_be<uint32_t>(data);
            data = data.subspan(sizeof(uint32_t));
        }
        return output;
    }

    std::optional<std::array<uint16_t, 2>> get_width() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        offset += sizeof(uint16_t);     // alternate_group
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_2
        offset += sizeof(uint32_t) * 9; // matrix
        auto data = m_box.get_data().value().subspan(offset);
        std::array<uint16_t, 2> output;
        for (auto &o : output) {
            o = read_be<uint16_t>(data);
            data = data.subspan(sizeof(uint16_t));
        }
        return output;
    }

    std::optional<std::array<uint16_t, 2>> get_height() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        size_t offset =
            version_depended_header_size(m_box.get_version().value());
        offset += sizeof(uint32_t) * 2; // reserved_1
        offset += sizeof(uint16_t);     // layer
        offset += sizeof(uint16_t);     // alternate_group
        offset += sizeof(uint16_t);     // volume
        offset += sizeof(uint16_t);     // reserved_2
        offset += sizeof(uint32_t) * 9; // matrix
        offset += sizeof(uint32_t);     // width
        auto data = m_box.get_data().value().subspan(offset);
        std::array<uint16_t, 2> output;
        for (auto &o : output) {
            o = read_be<uint16_t>(data);
            data = data.subspan(sizeof(uint16_t));
        }
        return output;
    }

  private:
    FullBoxView m_box;

    size_t version_depended_header_size(uint8_t version) const
    {
        size_t output = 0;
        if (version == 0) {
            output += sizeof(uint32_t); // creation_time v0
            output += sizeof(uint32_t); // modification_time v0
            output += sizeof(uint32_t); // track_ID v0
            output += sizeof(uint32_t); // reserved_0 v0
            output += sizeof(uint32_t); // duration v0
        }
        if (version == 1) {
            output += sizeof(uint64_t); // creation_time v1
            output += sizeof(uint64_t); // modification_time v1
            output += sizeof(uint32_t); // track_ID v1
            output += sizeof(uint32_t); // reserved_0 v0
            output += sizeof(uint64_t); // duration v1
        }
        return output;
    }
};
} // namespace Mpeg4
