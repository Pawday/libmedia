#include <algorithm>
#include <array>
#include <deque>
#include <exception>
#include <format>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <strings.h>
#include <utility>
#include <vector>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "file_view.hh"
#include "mpeg4.hh"
#include "mpeg4_dump.hh"

constexpr bool is_container_box(Mpeg4::BoxHeader::TypeTag tag)
{
    using Tag = Mpeg4::BoxHeader::TypeTag;

    bool output = true;
    output &= Tag{'m', 'd', 'a', 't'} != tag;
    return output;
}

constexpr bool is_full_box(Mpeg4::BoxHeader::TypeTag tag)
{
    using Tag = Mpeg4::BoxHeader::TypeTag;

    bool output = false;
    output |= Tag{'a', 's', 's', 'p'} == tag;
    output |= Tag{'b', 'x', 'm', 'l'} == tag;
    output |= Tag{'c', 'h', 'n', 'l'} == tag;
    output |= Tag{'c', 'o', '6', '4'} == tag;
    output |= Tag{'c', 'p', 'r', 't'} == tag;
    output |= Tag{'c', 's', 'l', 'g'} == tag;
    output |= Tag{'c', 't', 't', 's'} == tag;
    output |= Tag{'d', 'm', 'i', 'x'} == tag;
    output |= Tag{'d', 'r', 'e', 'f'} == tag;
    output |= Tag{'e', 'l', 'n', 'g'} == tag;
    output |= Tag{'e', 'l', 's', 't'} == tag;
    output |= Tag{'f', 'e', 'c', 'r'} == tag;
    output |= Tag{'f', 'i', 'i', 'n'} == tag;
    output |= Tag{'f', 'i', 'r', 'e'} == tag;
    output |= Tag{'f', 'p', 'a', 'r'} == tag;
    output |= Tag{'g', 'i', 't', 'n'} == tag;
    output |= Tag{'h', 'd', 'l', 'r'} == tag;
    output |= Tag{'h', 'm', 'h', 'd'} == tag;
    output |= Tag{'i', 'i', 'n', 'f'} == tag;
    output |= Tag{'i', 'l', 'o', 'c'} == tag;
    output |= Tag{'i', 'n', 'f', 'e'} == tag;
    output |= Tag{'i', 'p', 'r', 'o'} == tag;
    output |= Tag{'i', 'r', 'e', 'f'} == tag;
    output |= Tag{'k', 'i', 'n', 'd'} == tag;
    output |= Tag{'l', 'e', 'v', 'a'} == tag;
    output |= Tag{'m', 'd', 'h', 'd'} == tag;
    output |= Tag{'m', 'e', 'h', 'd'} == tag;
    output |= Tag{'m', 'e', 'r', 'e'} == tag;
    output |= Tag{'m', 'e', 't', 'a'} == tag;
    output |= Tag{'m', 'f', 'h', 'd'} == tag;
    output |= Tag{'m', 'f', 'r', 'o'} == tag;
    output |= Tag{'m', 'v', 'h', 'd'} == tag;
    output |= Tag{'n', 'm', 'h', 'd'} == tag;
    output |= Tag{'p', 'a', 'd', 'b'} == tag;
    output |= Tag{'p', 'd', 'i', 'n'} == tag;
    output |= Tag{'p', 'i', 't', 'm'} == tag;
    output |= Tag{'p', 'r', 'f', 't'} == tag;
    output |= Tag{'r', 'a', 'c', 'k'} == tag;
    output |= Tag{'s', 'a', 'i', 'o'} == tag;
    output |= Tag{'s', 'a', 'i', 'z'} == tag;
    output |= Tag{'s', 'b', 'g', 'p'} == tag;
    output |= Tag{'s', 'c', 'h', 'm'} == tag;
    output |= Tag{'s', 'd', 't', 'p'} == tag;
    output |= Tag{'s', 'g', 'p', 'd'} == tag;
    output |= Tag{'s', 'i', 'd', 'x'} == tag;
    output |= Tag{'s', 'm', 'h', 'd'} == tag;
    output |= Tag{'s', 'r', 'a', 't'} == tag;
    output |= Tag{'s', 'r', 'p', 'p'} == tag;
    output |= Tag{'s', 's', 'i', 'x'} == tag;
    output |= Tag{'s', 't', 'c', 'o'} == tag;
    output |= Tag{'s', 't', 'd', 'p'} == tag;
    output |= Tag{'s', 't', 'h', 'd'} == tag;
    output |= Tag{'s', 't', 'r', 'i'} == tag;
    output |= Tag{'s', 't', 's', 'c'} == tag;
    output |= Tag{'s', 't', 's', 'd'} == tag;
    output |= Tag{'s', 't', 's', 'g'} == tag;
    output |= Tag{'s', 't', 's', 'h'} == tag;
    output |= Tag{'s', 't', 's', 's'} == tag;
    output |= Tag{'s', 't', 's', 'z'} == tag;
    output |= Tag{'s', 't', 't', 's'} == tag;
    output |= Tag{'s', 't', 'v', 'i'} == tag;
    output |= Tag{'s', 't', 'z', '2'} == tag;
    output |= Tag{'s', 'u', 'b', 's'} == tag;
    output |= Tag{'t', 'f', 'd', 't'} == tag;
    output |= Tag{'t', 'f', 'h', 'd'} == tag;
    output |= Tag{'t', 'f', 'r', 'a'} == tag;
    output |= Tag{'t', 'k', 'h', 'd'} == tag;
    output |= Tag{'t', 'r', 'e', 'p'} == tag;
    output |= Tag{'t', 'r', 'e', 'x'} == tag;
    output |= Tag{'t', 'r', 'u', 'n'} == tag;
    output |= Tag{'t', 's', 'e', 'l'} == tag;
    output |= Tag{'u', 'r', 'i', ' '} == tag;
    output |= Tag{'u', 'r', 'i', 'I'} == tag;
    output |= Tag{'u', 'r', 'l', ' '} == tag;
    output |= Tag{'u', 'r', 'n', ' '} == tag;
    output |= Tag{'v', 'm', 'h', 'd'} == tag;
    output |= Tag{'x', 'm', 'l', ' '} == tag;
    return output;
}

using BoxCallback_t =
    void (*)(void *data, Mpeg4::BoxView box, size_t offset, size_t level);

void walk_boxes(
    void *user_data, BoxCallback_t cb, std::span<const std::byte> data)
{
    struct Frame
    {
        std::span<const std::byte> data;
    };

    std::vector<Frame> boxes_stack;

    boxes_stack.push_back({data});

    while (!boxes_stack.empty()) {

        auto &active_box_data = boxes_stack.back().data;

        if (active_box_data.size() == 0) {
            boxes_stack.pop_back();
            continue;
        }

        while (active_box_data.size() > 0) {

            size_t offset =
                std::distance(std::begin(data), std::begin(active_box_data));

            auto box_view = Mpeg4::BoxView(active_box_data);

            auto header_opt = box_view.get_header();
            auto box_data_opt = box_view.get_data();
            if (!header_opt || !box_data_opt) {
                boxes_stack.pop_back();
                break;
            }
            auto header = header_opt.value();

            auto box_data = box_data_opt.value();

            active_box_data = active_box_data.subspan(header.header_size);

            bool type_is_printable =
                std::ranges::all_of(header.type.data, [](const uint8_t &n) {
                    return std::isprint(n);
                });

            if (type_is_printable) {
                cb(user_data, box_view, offset, boxes_stack.size() - 1);
            }

            bool go_deeper = is_container_box(header.type) && type_is_printable;
            if (go_deeper) {
                if (!header.box_size.has_value()) {
                    boxes_stack.pop_back();
                    break;
                }
                active_box_data =
                    active_box_data.subspan(header.box_size.value());

                boxes_stack.push_back({box_data});
                break;
            }
        }
    }
}

std::string to_string_bytes(std::span<const std::byte> data)
{
    std::string data_copy;
    std::ranges::copy(
        data | std::views::transform(std::to_integer<char>),
        std::back_inserter(data_copy));

    for (auto &c : data_copy) {
        if (!std::isprint(c)) {
            c = '_';
        }
    }

    return data_copy;
}

std::vector<std::string_view> split_lines(std::string_view source)
{
    std::vector<std::string_view> output;

    size_t pos = 0;
    size_t next = 0;

    do {
        next = source.find('\n', pos);
        if (next == std::string_view::npos) {
            return output;
        }

        output.emplace_back(source.substr(pos, next - pos));
        pos = next + 1;
    } while (1);

    return output;
}

struct BoxToDumpData
{
    size_t offset;
    size_t size;
    size_t level;
    Mpeg4::BoxView box;
};

void cb(void *data, Mpeg4::BoxView box, size_t offset, size_t level)
{
    std::vector<BoxToDumpData> &vec =
        *reinterpret_cast<std::vector<BoxToDumpData> *>(data);

    vec.emplace_back(
        offset,
        box.get_header()->header_size + box.get_data()->size(),
        level,
        box);
}

int main(int argc, char **argv)
try {
    if (argc < 2) {
        std::cerr << "No args!\n";
        return EXIT_FAILURE;
    }

    FileView f{argv[1]};
    auto boxes_data =
        std::span(reinterpret_cast<const std::byte *>(f.data()), f.size());

    std::vector<BoxToDumpData> boxes;
    walk_boxes(&boxes, cb, boxes_data);

    size_t max_addr_fmtlen = 0;
    for (auto &dump_d : boxes) {
        size_t next_size =
            std::format(
                "0x{:x}-0x{:x}", dump_d.offset, dump_d.offset + dump_d.size)
                .size();
        if (next_size > max_addr_fmtlen) {
            max_addr_fmtlen = next_size;
        }
    }

    std::string output;

    for (auto &dump_d : boxes) {

        std::string indent;
        indent.resize(dump_d.level);
        std::ranges::fill(indent, '-');

        std::string addr_span_str = std::format(
            "0x{:x}-0x{:x}", dump_d.offset, dump_d.offset + dump_d.size);

        std::format_to(
            std::back_inserter(output),
            "{:{}} |{}{}",
            addr_span_str,
            max_addr_fmtlen,
            indent,
            Mpeg4::dump(dump_d.box.get_header().value()));

        auto ft_box = Mpeg4::BoxViewFileType(dump_d.box);
        if (ft_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(ft_box));
        }

        output.append("\n");
    }

    std::ranges::copy(output, std::ostream_iterator<char>(std::cout));

} catch (std::exception &e) {
    std::cout << std::format("Exception \"{}\"\n", e.what());
    return EXIT_FAILURE;
}
