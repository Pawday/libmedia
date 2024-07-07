#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "mpeg4.hh"
#include "raw_data.hh"

namespace Mpeg4 {

struct BoxViewSampleSize
{
    constexpr static TypeTag stsz_tag = make_tag("stsz");

    BoxViewSampleSize(FullBoxView box) : m_box(box)
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
        if (full_header->header.type != stsz_tag) {
            return false;
        }

        size_t required_size = 0;
        required_size += sizeof(uint32_t); // sample_size
        required_size += sizeof(uint32_t); // sample_count
        if (required_size > data->size()) {
            return false;
        }

        uint32_t sample_size = read_be<uint32_t>(data.value());
        if (sample_size != 0) {
            return true;
        }

        uint32_t sample_count =
            read_be<uint32_t>(data.value().subspan(sizeof(sample_size)));
        required_size +=
            sample_count * sizeof(uint32_t); // entry_size * sample_count
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

    std::optional<uint32_t> get_default_sample_size() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }
        auto data = m_box.get_data().value().subspan(0);
        auto default_sample_size = read_be<uint32_t>(data);

        /*
         * If this field (sample_size) is not 0
         * it specifies the constant sample size
         */
        if (default_sample_size == 0) {
            return std::nullopt;
        }

        return default_sample_size;
    }

    std::optional<uint32_t> get_samples_count() const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        size_t offset = 0;
        offset += sizeof(uint32_t); // sample_size

        auto data = m_box.get_data().value().subspan(offset);

        return read_be<uint32_t>(data);
    }

    std::optional<uint32_t> get_sample_size_at(size_t sample_index) const
    {
        if (is_not_valid()) {
            return std::nullopt;
        }

        auto sample_size = get_default_sample_size();
        if (sample_size.has_value()) {
            return sample_size.value();
        }

        auto samples_count = get_samples_count();
        if (!samples_count) {
            return std::nullopt;
        }

        if (samples_count.value() <= sample_index) {
            return std::nullopt;
        }

        return get_sample_size_at_unsafe(sample_index);
    }

    uint32_t get_sample_size_at_unsafe(size_t sample_index) const
    {
        size_t offset = 0;
        offset += sizeof(uint32_t); // sample_size
        offset += sizeof(uint32_t); // sample_count
        offset += sizeof(uint32_t) * sample_index;
        auto data = m_box.get_data()->subspan(offset);
        return read_be<uint32_t>(data);
    }

  private:
    FullBoxView m_box;
};

} // namespace Mpeg4
