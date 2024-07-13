#pragma once

#include <array>
#include <optional>

#include <cstddef>
#include <cstdint>

#include "libmedia/mpeg4.hh"
#include "libmedia/raw_data.hh"

namespace Mpeg4 {

struct SampleEntryBoxView
{
    SampleEntryBoxView(BoxView box) : m_box(box)
    {
    }

    bool validate() const
    {
        auto box_header = m_box.get_header();
        auto data = m_box.get_data();
        if (!box_header || !data) {
            return false;
        }

        size_t required_size = 0;
        required_size += sizeof(uint8_t) * 6; // reserved
        required_size += sizeof(uint16_t);    // data_reference_index;
        if (required_size > data->size()) {
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

    std::optional<BoxHeader> get_box_header() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        auto header = m_box.get_header();
        if (!header.has_value()) {
            return std::nullopt;
        }
        return header.value();
    }

    std::optional<std::array<std::byte, 6>> get_reserved() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        auto data = m_box.get_data();
        if (!data || data->size() < 6) {
            return std::nullopt;
        }

        return copy_array<6>(data.value());
    }

    std::optional<uint16_t> get_data_reference_index() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t read_offset = 6; // sizeof(reserved)

        auto data = m_box.get_data();
        if (!data || data->size() < sizeof(uint16_t) + read_offset) {
            return std::nullopt;
        }

        return read_be<uint16_t>(data->subspan(read_offset));
    }

  private:
    BoxView m_box;
};

} // namespace Mpeg4
