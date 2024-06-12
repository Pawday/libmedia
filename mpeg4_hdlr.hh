#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>

#include <cstddef>
#include <cstdint>
#include <span>

#include "mpeg4.hh"
#include "raw_data.hh"

namespace Mpeg4 {

struct BoxViewHandler
{
    constexpr static BoxHeader::TypeTag hdlr_tag = {'h', 'd', 'l', 'r'};

    BoxViewHandler(FullBoxView box) : m_box(box)
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
        if (full_header->header.type != hdlr_tag) {
            return false;
        }

        size_t required_size = 0;
        required_size += sizeof(uint32_t);     // pre_defined
        required_size += sizeof(uint32_t);     // handler_type;
        required_size += sizeof(uint32_t) * 3; // reserved;
        if (required_size > data->size()) {
            return false;
        }

        auto utf8_zeroterm_name_subspan = data->subspan(required_size);

        return std::end(utf8_zeroterm_name_subspan) !=
            std::ranges::find(utf8_zeroterm_name_subspan, std::byte(0));
    }

    bool is_valid() const
    {
        return validate();
    }

    bool is_not_valid() const
    {
        return !is_valid();
    }

    std::optional<uint32_t> get_pre_defined() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t offset = 0;
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint32_t>(data);
    }

    std::optional<uint32_t> get_handler_type() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t offset = 0;
        offset += sizeof(uint32_t); // pre_defined
        auto data = m_box.get_data().value().subspan(offset);
        return read_be<uint32_t>(data);
    }

    std::optional<std::array<uint32_t, 3>> get_reserved() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t offset = 0;
        offset += sizeof(uint32_t); // pre_defined
        offset += sizeof(uint32_t); // handler_type
        auto data = m_box.get_data().value().subspan(offset);

        std::array<uint32_t, 3> output;
        for (auto &o : output) {
            o = read_be<uint32_t>(data);
            data = data.subspan(sizeof(uint32_t));
        }
        return output;
    }

    std::optional<std::span<const char>> get_name_span() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t offset = 0;
        offset += sizeof(uint32_t);     // pre_defined
        offset += sizeof(uint32_t);     // handler_type;
        offset += sizeof(uint32_t) * 3; // reserved
        auto data = m_box.get_data().value().subspan(offset);

        auto zero_term_pos = std::ranges::find(data, std::byte(0));
        if (std::end(data) == zero_term_pos) {
            return std::nullopt;
        }
        auto name_span_size = std::distance(std::begin(data), zero_term_pos);

        auto output_span = std::span<const char>(
            reinterpret_cast<const char *>(data.data()), data.size());

        return output_span.subspan(0, name_span_size);
    }

  private:
    FullBoxView m_box;
};

} // namespace Mpeg4
