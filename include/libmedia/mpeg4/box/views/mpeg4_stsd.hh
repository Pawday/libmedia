#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "libmedia/mpeg4.hh"
#include "libmedia/mpeg4/box/views/mpeg4_sample_entry.hh"
#include "libmedia/raw_data.hh"

namespace Mpeg4 {

struct SampleDescriptionBoxView
{
    constexpr static TypeTag stbl_tag = TypeTag::from_str("stsd");

    SampleDescriptionBoxView(FullBoxView box) : m_box(box)
    {
    }

    enum class ValidateStatus
    {
        VALID,
        INVALID_TYPE,
        INVALID_BOX_VIEW,
        NO_DATA,
        NO_NEXT_SAMPLE_DATA,
        NO_NEXT_SAMPLE_BOX,
        UNSIZED_SAMPLE_BOX,
        INVALID_SAMPLE_BOX,
    };

    ValidateStatus validate() const
    {
        std::optional<FullBoxHeader> full_header = m_box.get_header();
        auto data = m_box.get_data();
        auto version = m_box.get_version();
        if (!full_header || !data || !version) {
            return ValidateStatus::INVALID_BOX_VIEW;
        }

        if (full_header->header.type != stbl_tag) {
            return ValidateStatus::INVALID_TYPE;
        }

        size_t read_offset = 0;
        if (sizeof(uint32_t) > data->size()) { // entry_count
            return ValidateStatus::NO_DATA;
        }
        uint32_t nb_samples_entries =
            read_be<uint32_t>(data->subspan(read_offset, sizeof(uint32_t)));
        read_offset += sizeof(uint32_t);

        for (size_t sample_entry_box_idx = 0;
             sample_entry_box_idx < nb_samples_entries;
             sample_entry_box_idx++) {
            if (read_offset > data->size()) {
                return ValidateStatus::NO_NEXT_SAMPLE_DATA;
            }

            BoxView next_sample_as_box(data->subspan(read_offset));
            auto header = next_sample_as_box.get_header();
            if (!header) {
                return ValidateStatus::NO_NEXT_SAMPLE_BOX;
            }

            auto entry_box_size = header->box_size;
            if (!entry_box_size.has_value()) {
                // Unsized SampleEntry box
                // make it impossible to index sequence of boxes
                return ValidateStatus::UNSIZED_SAMPLE_BOX;
            }

            SampleEntryBoxView next_sample(next_sample_as_box);
            if (next_sample.is_not_valid()) {
                return ValidateStatus::INVALID_SAMPLE_BOX;
            }

            read_offset += header->header_size + entry_box_size.value();
        }

        return ValidateStatus::VALID;
    }

    bool is_valid() const
    {
        return validate() == ValidateStatus::VALID;
    }

    bool is_not_valid() const
    {
        return !is_valid();
    }

    std::optional<uint32_t> get_entry_count() const
    {
        if (this->is_not_valid()) {
            return std::nullopt;
        }

        auto data = m_box.get_data();
        if (!data) {
            return std::nullopt;
        }

        if (sizeof(uint32_t) > data->size()) { // entry_count
            return std::nullopt;
        }

        return read_be<uint32_t>(data->subspan(0, sizeof(uint32_t)));
    }

    std::optional<std::vector<SampleEntryBoxView>> get_entries() const
    {
        if (this->is_not_valid()) {
            return std::nullopt;
        }

        auto data = m_box.get_data();
        if (!data) {
            return std::nullopt;
        }

        auto nb_samples_entries_opt = get_entry_count();
        if (!nb_samples_entries_opt) {
            return std::nullopt;
        }

        uint32_t nb_samples_entries = nb_samples_entries_opt.value();
        uint32_t read_offset = sizeof(uint32_t); // entry_count

        std::vector<SampleEntryBoxView> output;
        output.reserve(nb_samples_entries);

        for (size_t sample_entry_box_idx = 0;
             sample_entry_box_idx < nb_samples_entries;
             sample_entry_box_idx++) {
            if (read_offset > data->size()) {
                return std::nullopt;
            }

            BoxView next_sample_as_box(data->subspan(read_offset));
            auto header = next_sample_as_box.get_header();
            if (!header) {
                return std::nullopt;
            }

            auto entry_box_size = header->box_size;
            if (!entry_box_size.has_value()) {
                // Unsized SampleEntry box
                // make it impossible to index sequence of boxes
                return std::nullopt;
            }

            SampleEntryBoxView next_sample(next_sample_as_box);
            if (next_sample.is_not_valid()) {
                return std::nullopt;
            }

            read_offset += header->header_size + entry_box_size.value();
            output.push_back(next_sample);
        }

        return output;
    }

  private:
    FullBoxView m_box;
};

} // namespace Mpeg4
