#pragma once

#include <optional>

#include <cstddef>
#include <cstdint>

#include "libmedia/mpeg4.hh"
#include "libmedia/raw_data.hh"

namespace Mpeg4 {

struct ChunkOffsetBoxView
{
    constexpr static TypeTag stco_tag = TypeTag::from_str("stco");

    ChunkOffsetBoxView(FullBoxView box) : m_box(box)
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

        BoxHeader base_header = full_header->header;
        if (full_header->header.type != stco_tag) {
            return false;
        }

        size_t required_size = 0;
        required_size += sizeof(uint32_t); // entry_count
        if (required_size > data->size()) {
            return false;
        }

        uint32_t entry_count = read_be<uint32_t>(data.value());
        required_size +=
            entry_count * sizeof(uint32_t); // chunk_offset * entry_count
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

    std::optional<uint32_t> get_entry_count() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        auto data = m_box.get_data().value().subspan(0);

        return read_be<uint32_t>(data);
    }

    std::optional<uint32_t> get_chunk_offset(uint32_t entry_index) const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        auto entry_count = get_entry_count();
        if (!entry_count) {
            return std::nullopt;
        }

        if (entry_count.value() <= entry_index) {
            return std::nullopt;
        }

        return get_chunk_offset_unsafe(entry_index);
    }

    uint32_t get_chunk_offset_unsafe(uint32_t entry_index) const
    {
        size_t offset = 0;
        offset += sizeof(uint32_t); // entry_count
        offset += sizeof(uint32_t) * entry_index;
        auto data = m_box.get_data()->subspan(offset);
        return read_be<uint32_t>(data);
    }

  private:
    FullBoxView m_box;
};

} // namespace Mpeg4
