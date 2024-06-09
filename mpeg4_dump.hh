#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <stdexcept>
#include <string>

#include "mpeg4.hh"

namespace Mpeg4 {

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

inline std::string dump(const BoxHeader &box)
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
        "{{type: {:?s}{}{}{}}}",
        type_as_chars,
        header_size_string,
        size_string,
        user_type_string);
}
} // namespace Mpeg4
