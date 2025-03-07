#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "libmedia/mpeg4.hh"

#include "libmedia/mpeg4/box/ChunkOffset64BoxView.hh"
#include "libmedia/mpeg4/box/ChunkOffsetBoxView.hh"
#include "libmedia/mpeg4/box/FileTypeBoxView.hh"
#include "libmedia/mpeg4/box/HandlerBoxView.hh"
#include "libmedia/mpeg4/box/MediaHeaderBoxView.hh"
#include "libmedia/mpeg4/box/MovieHeaderBoxView.hh"
#include "libmedia/mpeg4/box/SampleDescriptionBoxView.hh"
#include "libmedia/mpeg4/box/SampleEntryBoxView.hh"
#include "libmedia/mpeg4/box/SampleSizeBoxView.hh"
#include "libmedia/mpeg4/box/TrackHeaderBoxView.hh"

namespace Mpeg4 {

inline std::string dump_fields_only(const BoxHeader &box)
{
    std::string user_type_string;

    auto is_printable = [](auto a) { return std::isprint(a); };

    if (box.usertype.has_value()) {

        bool user_type_is_printable =
            std::ranges::all_of(box.usertype->data, is_printable);

        std::string user_type_data;

        if (user_type_is_printable) {
            std::ranges::copy(
                box.usertype->data, std::back_inserter(user_type_data));
        } else {
            user_type_data += '[';
            bool first = true;
            for (auto c : box.usertype->data) {
                if (!first) {
                    user_type_data += ", ";
                }
                first = false;
                user_type_data += std::format("0x{:x}", c);
            }
            user_type_data += ']';
        }

        user_type_string = std::format(", user_type: {}", user_type_data);
    }

    std::string size_string;

    if (box.box_content_size.has_value()) {
        size_string = std::format(", size: {}", box.box_content_size.value());
    }

    std::string header_size_string;
    if (box.header_size != 8) {
        header_size_string = std::format(", header_size: {}", box.header_size);
    }

    bool type_printable = std::ranges::all_of(box.type.data, is_printable);

    std::string type_as_chars;

    if (type_printable) {
        std::ranges::copy(box.type.data, std::back_inserter(type_as_chars));
        type_as_chars = std::format("\"{}\"", type_as_chars);
    } else {
        bool first = true;
        type_as_chars += '[';
        for (uint8_t c : box.type.data) {
            if (!first) {
                type_as_chars += ", ";
            }
            first = false;
            type_as_chars += std::format("0x{:x}", c);
        }
        type_as_chars += ']';
    }

    return std::format(
        "type: {}{}{}{}",
        type_as_chars,
        header_size_string,
        size_string,
        user_type_string);
}

inline std::string dump_fields_only(const FullBoxHeader &box)
{
    return std::format(
        "version: {}, flags: 0b{}", box.version, box.flags.to_string());
}

inline std::string dump(const BoxHeader &box)
{
    return std::format("{{{}}}", dump_fields_only(box));
}

inline std::string dump(const FullBoxHeader &box)
{
    return std::format(
        "{{{}, {}}}", dump_fields_only(box.header), dump_fields_only(box));
}

inline std::string dump(const FileTypeBoxView &file_type_box)
{
    auto maj_brand = file_type_box.get_major_brand();
    if (!maj_brand) {
        throw std::runtime_error(
            "Mpeg4::dump(BoxViewFileType): major_brand parse failue");
    }

    auto version = file_type_box.get_minor_version();
    if (!version) {
        throw std::runtime_error(
            "Mpeg4::dump(BoxViewFileType): minor_version parse failue");
    }

    auto compatible_brands = file_type_box.get_compatible_brands();
    if (!compatible_brands) {
        throw std::runtime_error(
            "Mpeg4::dump(BoxViewFileType): compatible_brands parse failue");
    }

    std::string compatible_brands_string;

    if (compatible_brands.value().size() != 0) {
        compatible_brands_string = ", compatible_brands: [";
        bool f = true;
        for (auto &brand : compatible_brands.value()) {
            if (!f) {
                compatible_brands_string += ", ";
            }
            f = false;
            compatible_brands_string += std::format("{:?s}", brand);
        }
        compatible_brands_string += ']';
    }

    return std::format(
        "{{major_brand: {:?s}, minor_version: {}{}}}",
        maj_brand.value(),
        version.value(),
        compatible_brands_string);
}

inline std::string dump(const MovieHeaderBoxView &mvhdr_type_box)
{
    auto creation_time = mvhdr_type_box.get_creation_time();
    auto modification_time = mvhdr_type_box.get_modification_time();
    auto timescale = mvhdr_type_box.get_timescale();
    auto duration = mvhdr_type_box.get_duration();
    auto rate = mvhdr_type_box.get_rate();
    auto volume = mvhdr_type_box.get_volume();
    auto reserved_0 = mvhdr_type_box.get_reserved_0();
    auto reserved_1 = mvhdr_type_box.get_reserved_1();
    auto matrix = mvhdr_type_box.get_matrix();
    auto pre_defined = mvhdr_type_box.get_pre_defined();
    auto next_track_ID = mvhdr_type_box.get_next_track_ID();

    std::string error_message = "Mpeg4::dump(BoxViewFileType): ";

    if (!creation_time) {
        throw std::runtime_error(
            error_message + "creation_time" + " parse failue");
    }
    if (!modification_time) {
        throw std::runtime_error(
            error_message + "modification_time" + " parse failue");
    }
    if (!timescale) {
        throw std::runtime_error(error_message + "timescale" + " parse failue");
    }
    if (!duration) {
        throw std::runtime_error(error_message + "duration" + " parse failue");
    }
    if (!rate) {
        throw std::runtime_error(error_message + "rate" + " parse failue");
    }
    if (!volume) {
        throw std::runtime_error(error_message + "volume" + " parse failue");
    }
    if (!reserved_0) {
        throw std::runtime_error(
            error_message + "reserved_0" + " parse failue");
    }
    if (!reserved_1) {
        throw std::runtime_error(
            error_message + "reserved_1" + " parse failue");
    }
    if (!matrix) {
        throw std::runtime_error(error_message + "matrix" + " parse failue");
    }
    if (!pre_defined) {
        throw std::runtime_error(
            error_message + "pre_defined" + "parse failue");
    }
    if (!next_track_ID) {
        throw std::runtime_error(
            error_message + "next_track_ID" + "parse failue");
    }

    return std::format(
        "{{creation_time: {}, modification_time: {}, timescale: {}, duration: "
        "{}, rate: {}, volume: {}, reserved_0: {}, reserved_1: {}, matrix: {}, "
        "pre_defined: {}, next_track_ID: {}}}",

        creation_time.value(),
        modification_time.value(),
        timescale.value(),
        duration.value(),
        rate.value(),
        volume.value(),
        reserved_0.value(),
        reserved_1.value(),
        matrix.value(),
        pre_defined.value(),
        next_track_ID.value());
}

inline std::string dump(const TrackHeaderBoxView &tkhd_type_box)
{
    auto creation_time = tkhd_type_box.get_creation_time();
    auto modification_time = tkhd_type_box.get_modification_time();
    auto track_ID = tkhd_type_box.get_track_ID();
    auto reserved_0 = tkhd_type_box.get_reserved_0();
    auto duration = tkhd_type_box.get_duration();
    auto reserved_1 = tkhd_type_box.get_reserved_1();
    auto layer = tkhd_type_box.get_layer();
    auto alternate_group = tkhd_type_box.get_alternate_group();
    auto volume = tkhd_type_box.get_volume();
    auto reserved_2 = tkhd_type_box.get_reserved_2();
    auto matrix = tkhd_type_box.get_matrix();
    auto width = tkhd_type_box.get_width();
    auto height = tkhd_type_box.get_height();

    std::string error_message = "Mpeg4::dump(BoxViewTrackHeader): ";

    if (!creation_time) {
        throw std::runtime_error(
            error_message + "creation_time" + " parse failue");
    }
    if (!modification_time) {
        throw std::runtime_error(
            error_message + "modification_time" + " parse failue");
    }
    if (!track_ID) {
        throw std::runtime_error(error_message + "track_ID" + " parse failue");
    }
    if (!reserved_0) {
        throw std::runtime_error(
            error_message + "reserved_0" + " parse failue");
    }
    if (!duration) {
        throw std::runtime_error(error_message + "duration" + " parse failue");
    }
    if (!reserved_1) {
        throw std::runtime_error(
            error_message + "reserved_1" + " parse failue");
    }
    if (!layer) {
        throw std::runtime_error(error_message + "layer" + " parse failue");
    }
    if (!alternate_group) {
        throw std::runtime_error(
            error_message + "alternate_group" + " parse failue");
    }
    if (!volume) {
        throw std::runtime_error(error_message + "volume" + " parse failue");
    }
    if (!reserved_2) {
        throw std::runtime_error(
            error_message + "reserved_2" + " parse failue");
    }
    if (!matrix) {
        throw std::runtime_error(error_message + "matrix" + " parse failue");
    }
    if (!width) {
        throw std::runtime_error(error_message + "width" + " parse failue");
    }
    if (!height) {
        throw std::runtime_error(error_message + "height" + " parse failue");
    }

    return std::format(
        "{{creation_time: {}, modification_time: {}, track_ID: {}, reserved_0: "
        "{}, duration: {}, reserved_1: {}, layer: {}, alternate_group: {}, "
        "volume: {}, reserved_2: {}, matrix: {}, width: {}, height: {}}}",

        creation_time.value(),
        modification_time.value(),
        track_ID.value(),
        reserved_0.value(),
        duration.value(),
        reserved_1.value(),
        layer.value(),
        alternate_group.value(),
        volume.value(),
        reserved_2.value(),
        matrix.value(),
        width.value(),
        height.value());
}

inline std::string dump(const MediaHeaderBoxView &mdia_type_box)
{
    auto creation_time = mdia_type_box.get_creation_time();
    auto modification_time = mdia_type_box.get_modification_time();
    auto timescale = mdia_type_box.get_timescale();
    auto duration = mdia_type_box.get_duration();
    auto pad = mdia_type_box.get_pad();
    auto language = mdia_type_box.get_language();
    auto pre_defined = mdia_type_box.get_pre_defined();

    std::string error_message = "Mpeg4::dump(BoxViewMediaHeader): ";

    if (!creation_time) {
        throw std::runtime_error(
            error_message + "creation_time" + " parse failue");
    }
    if (!modification_time) {
        throw std::runtime_error(
            error_message + "modification_time" + " parse failue");
    }
    if (!timescale) {
        throw std::runtime_error(error_message + "timescale" + " parse failue");
    }
    if (!duration) {
        throw std::runtime_error(error_message + "duration" + " parse failue");
    }

    if (!pad.has_value()) {
        throw std::runtime_error(error_message + "pad" + " parse failue");
    }

    if (!language) {
        throw std::runtime_error(error_message + "language" + " parse failue");
    }

    if (!pre_defined) {
        throw std::runtime_error(
            error_message + "pre_defined" + " parse failue");
    }

    auto language_val = language.value();
    std::array<char, 3> language_decoded;

    std::ranges::copy(
        language_val | std::views::transform(std::to_integer<char>),
        begin(language_decoded));
    std::ranges::for_each(language_decoded, [](char &c) { c += 0x60; });

    std::string language_dump_val;

    bool stringify_language = true;

    stringify_language &= std::ranges::all_of(
        language_val, [](std::byte c) { return c != std::byte(0); });

    stringify_language &= std::ranges::all_of(
        language_decoded, [](char c) { return std::isprint(c); });

    if (stringify_language) {
        language_dump_val = std::format("\"{:s}\"", language_decoded);
    } else {
        language_dump_val = std::format(
            "{}",
            language_val | std::views::transform(std::to_integer<uint8_t>));
    }

    return std::format(
        "{{creation_time: {}, modification_time: {}, timescale: {}, duration: "
        "{}, pad: {}, language: {}, pre_defined: {}}}",

        creation_time.value(),
        modification_time.value(),
        timescale.value(),
        duration.value(),
        pad.value(),
        language_dump_val,
        pre_defined.value());
}

inline std::string dump(const HandlerBoxView &hdlr_type_box)
{
    auto pre_defined = hdlr_type_box.get_pre_defined();
    auto handler_type = hdlr_type_box.get_handler_type();
    auto reserved = hdlr_type_box.get_reserved();
    auto name = hdlr_type_box.get_name_span();

    std::string error_message = "Mpeg4::dump(BoxViewHandler): ";

    if (!pre_defined) {
        throw std::runtime_error(
            error_message + "pre_defined" + " parse failue");
    }

    if (!handler_type) {
        throw std::runtime_error(
            error_message + "handler_type" + " parse failue");
    }

    if (!reserved) {
        throw std::runtime_error(error_message + "reserved" + " parse failue");
    }

    if (!name) {
        throw std::runtime_error(error_message + "name" + " parse failue");
    }

    std::string name_str;
    std::ranges::copy(name.value(), std::back_inserter(name_str));

    return std::format(
        "{{pre_defined: {}, handler_type: {}, reserved: {}, name: {:?}}}",
        pre_defined.value(),
        handler_type.value(),
        reserved.value(),
        name_str);
}

inline std::string dump(const ChunkOffsetBoxView &stco_type_box)
{
    auto entry_count = stco_type_box.get_entry_count();

    std::string error_message = "Mpeg4::dump(BoxViewChunkOffset): ";
    if (!entry_count) {
        throw std::runtime_error(
            error_message + "entry_count" + " parse failue");
    }

    return std::format("{{chunk_offsets_size: {}}}", entry_count.value());
}

inline std::string dump(const ChunkOffset64BoxView &co64_type_box)
{
    auto entry_count = co64_type_box.get_entry_count();

    std::string error_message = "Mpeg4::dump(BoxViewChunkLargeOffset): ";
    if (!entry_count) {
        throw std::runtime_error(
            error_message + "entry_count" + " parse failue");
    }

    return std::format("{{large_chunk_offsets_size: {}}}", entry_count.value());
}

inline std::string dump(const SampleSizeBoxView &stsz_type_box)
{
    auto samples_count = stsz_type_box.get_samples_count();
    auto default_sample_size = stsz_type_box.get_default_sample_size();

    std::string error_message = "Mpeg4::dump(BoxViewSampleSize): ";
    if (!samples_count) {
        throw std::runtime_error(
            error_message + "samples_count" + " parse failue");
    }

    std::string default_sample_size_string;

    if (default_sample_size.has_value()) {
        default_sample_size_string = std::format(
            ", default_sample_size: {}", default_sample_size.value());
    }

    return std::format(
        "{{samples_count: {}{}}}",
        samples_count.value(),
        default_sample_size_string);
}

inline std::string dump(const SampleEntryBoxView sample_entry)
{
    std::string err_prefix = "Mpeg4::dump(SampleEntryBoxView)";

    auto header = sample_entry.get_box_header();
    auto reserved = sample_entry.get_reserved();
    auto data_reference_index = sample_entry.get_data_reference_index();
    if (!header) {
        throw std::runtime_error(std::format("{}: No header", err_prefix));
    }
    if (!reserved) {
        throw std::runtime_error(std::format("{}: No reserved", err_prefix));
    }
    if (!data_reference_index) {
        throw std::runtime_error(
            std::format("{}: No data_reference_index", err_prefix));
    }

    auto reserved_int_view =
        reserved.value() | std::views::transform(std::to_integer<uint16_t>);

    return std::format(
        "{{header: {}, reserved: {}, data_reference_index: {}}}",
        dump(header.value()),
        reserved_int_view,
        data_reference_index.value());
}

inline std::string dump(const SampleDescriptionBoxView &stsd_type_box)
{
    std::string err_prefix = "Mpeg4::dump(BoxViewSampleDescription)";

    auto entries_opt = stsd_type_box.get_entries();
    if (!entries_opt) {
        throw std::runtime_error(
            std::format("{}: get_entries failue", err_prefix));
    }

    std::string output = "{entries: [";

    auto entries = entries_opt.value();

    bool first = true;
    for (auto entry : entries) {
        if (!first) {
            output.append(", ");
        }
        output.append(dump(entry));
        first = false;
    }

    output.append("]}");

    return output;
}

} // namespace Mpeg4
