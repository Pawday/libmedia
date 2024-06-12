#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <stdexcept>
#include <string>

#include "mpeg4.hh"
#include "mpeg4_ftype.hh"
#include "mpeg4_mdia.hh"
#include "mpeg4_mvhd.hh"
#include "mpeg4_tkhd.hh"

namespace Mpeg4 {

inline std::string dump_fields_only(const BoxHeader &box)
{
    std::array<char, 4> type_as_chars{};
    std::ranges::fill(type_as_chars, 0);
    std::ranges::copy(box.type.data, type_as_chars.begin());

    std::string user_type_string;

    if (box.usertype.has_value()) {
        std::array<char, 8> user_type_chars;
        std::ranges::fill(user_type_chars, 0);
        std::ranges::copy(box.usertype->data, user_type_chars.begin());
        user_type_string = std::format(", user_type: {}", user_type_chars);
    }

    std::string size_string;

    if (box.box_size.has_value()) {
        size_string = std::format(", size: {}", box.box_size.value());
    }

    std::string header_size_string;
    if (box.header_size != 8) {
        header_size_string = std::format(", header_size: {}", box.header_size);
    }

    return std::format(
        "type: {:?s}{}{}{}",
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

inline std::string dump(const BoxViewFileType &file_type_box)
{
    auto maj_brand = file_type_box.get_major_brand();
    if (!maj_brand) {
        throw std::runtime_error(
            "Mpeg4::dump(BoxViewFileType): major_brand parse failue");
    }

    auto version = file_type_box.get_major_brand();
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

inline std::string dump(const BoxViewMovieHeader &mvhdr_type_box)
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

inline std::string dump(const BoxViewTrackHeader &tkhd_type_box)
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

inline std::string dump(const BoxViewMediaHeader &mdia_type_box)
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

    return std::format(
        "{{creation_time: {}, modification_time: {}, timescale: {}, duration: "
        "{}, pad: {}, language: {}, pre_defined: {}}}",

        creation_time.value(),
        modification_time.value(),
        timescale.value(),
        duration.value(),
        pad.value(),
        language.value(),
        pre_defined.value());
}

} // namespace Mpeg4
