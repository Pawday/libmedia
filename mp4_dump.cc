#include <algorithm>
#include <array>
#include <deque>
#include <exception>
#include <format>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <span>
#include <stack>
#include <string>
#include <string_view>

#include <vector>

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "libmedia/mpeg4.hh"
#include "libmedia/mpeg4/dump.hh"

#include "libmedia/mpeg4/box/ChunkOffset64BoxView.hh"
#include "libmedia/mpeg4/box/ChunkOffsetBoxView.hh"
#include "libmedia/mpeg4/box/FileTypeBoxView.hh"
#include "libmedia/mpeg4/box/HandlerBoxView.hh"
#include "libmedia/mpeg4/box/MediaHeaderBoxView.hh"
#include "libmedia/mpeg4/box/MovieHeaderBoxView.hh"
#include "libmedia/mpeg4/box/SampleDescriptionBoxView.hh"
#include "libmedia/mpeg4/box/SampleSizeBoxView.hh"
#include "libmedia/mpeg4/box/TrackHeaderBoxView.hh"


#include "file_view.hh"

constexpr bool is_container_box(Mpeg4::TypeTag tag)
{
    using Tag = Mpeg4::TypeTag;

    bool output = true;
    output &= Mpeg4::TypeTag::from_str("mdat") != tag;
    output &= Mpeg4::TypeTag::from_str("stsd") != tag;
    return output;
}

constexpr bool is_full_box(const Mpeg4::BoxHeader &box)
{

#define CHECK_TAG(tag_to_check_str)                                            \
    if (Mpeg4::TypeTag::from_str(tag_to_check_str) == box.type) {              \
        return true;                                                           \
    }

    CHECK_TAG("assp");
    CHECK_TAG("bxml");
    CHECK_TAG("chnl");
    CHECK_TAG("co64");
    CHECK_TAG("cprt");
    CHECK_TAG("cslg");
    CHECK_TAG("ctts");
    CHECK_TAG("dmix");
    CHECK_TAG("dref");
    CHECK_TAG("elng");
    CHECK_TAG("elst");
    CHECK_TAG("fecr");
    CHECK_TAG("fiin");
    CHECK_TAG("fire");
    CHECK_TAG("fpar");
    CHECK_TAG("gitn");
    CHECK_TAG("hdlr");
    CHECK_TAG("hmhd");
    CHECK_TAG("iinf");
    CHECK_TAG("iloc");
    CHECK_TAG("infe");
    CHECK_TAG("ipro");
    CHECK_TAG("iref");
    CHECK_TAG("kind");
    CHECK_TAG("leva");
    CHECK_TAG("mdhd");
    CHECK_TAG("mehd");
    CHECK_TAG("mere");
    CHECK_TAG("meta");
    CHECK_TAG("mfhd");
    CHECK_TAG("mfro");
    CHECK_TAG("mvhd");
    CHECK_TAG("nmhd");
    CHECK_TAG("padb");
    CHECK_TAG("pdin");
    CHECK_TAG("pitm");
    CHECK_TAG("prft");
    CHECK_TAG("rack");
    CHECK_TAG("saio");
    CHECK_TAG("saiz");
    CHECK_TAG("sbgp");
    CHECK_TAG("schm");
    CHECK_TAG("sdtp");
    CHECK_TAG("sgpd");
    CHECK_TAG("sidx");
    CHECK_TAG("smhd");
    CHECK_TAG("srat");
    CHECK_TAG("srpp");
    CHECK_TAG("ssix");
    CHECK_TAG("stco");
    CHECK_TAG("stdp");
    CHECK_TAG("sthd");
    CHECK_TAG("stri");
    CHECK_TAG("stsc");
    CHECK_TAG("stsd");
    CHECK_TAG("stsg");
    CHECK_TAG("stsh");
    CHECK_TAG("stss");
    CHECK_TAG("stsz");
    CHECK_TAG("stts");
    CHECK_TAG("stvi");
    CHECK_TAG("stz2");
    CHECK_TAG("subs");
    CHECK_TAG("tfdt");
    CHECK_TAG("tfhd");
    CHECK_TAG("tfra");
    CHECK_TAG("tkhd");
    CHECK_TAG("trep");
    CHECK_TAG("trex");
    CHECK_TAG("trun");
    CHECK_TAG("tsel");
    CHECK_TAG("uri ");
    CHECK_TAG("uriI");
    CHECK_TAG("url ");
    CHECK_TAG("urn ");
    CHECK_TAG("vmhd");
    CHECK_TAG("xml ");

#undef CHECK_TAG

    return false;
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

    std::stack<Frame> boxes_stack;

    boxes_stack.push({data});

    while (!boxes_stack.empty()) {

        auto &active_box_data = boxes_stack.top().data;

        if (active_box_data.size() == 0) {
            boxes_stack.pop();
            continue;
        }

        while (active_box_data.size() > 0) {

            size_t offset = active_box_data.data() - data.data();

            auto box_view = Mpeg4::BoxView(active_box_data);

            auto header_opt = box_view.get_header();
            auto box_data_opt = box_view.get_content_data();
            if (!header_opt || !box_data_opt) {
                boxes_stack.pop();
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

            if (!header.box_content_size.has_value()) {
                boxes_stack.pop();
                break;
            }

            active_box_data = active_box_data.subspan(header.box_content_size.value());

            bool go_deeper = is_container_box(header.type) && type_is_printable;
            if (go_deeper) {
                boxes_stack.push({box_data});
                break;
            }
        }
    }
}

struct BoxToDumpData
{
    size_t offset;
    size_t size;
    size_t depth_level;
    Mpeg4::BoxView box;
};

void cb(void *data, Mpeg4::BoxView box, size_t offset, size_t level)
{
    std::vector<BoxToDumpData> &vec =
        *reinterpret_cast<std::vector<BoxToDumpData> *>(data);

    vec.emplace_back(
        offset,
        box.get_header()->header_size + box.get_content_data()->size(),
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
        indent.resize(dump_d.depth_level);
        std::ranges::fill(indent, '-');

        std::string addr_span_str = std::format(
            "0x{:x}-0x{:x}", dump_d.offset, dump_d.offset + dump_d.size);

        auto header = dump_d.box.get_header();
        if (!header) {
            std::cerr << "Header retreave failue\n";
            continue;
        }

        std::format_to(
            std::back_inserter(output),
            "{:{}} |{}",
            addr_span_str,
            max_addr_fmtlen,
            indent);

        if (is_full_box(header.value())) {
            auto full_header = Mpeg4::FullBoxView(dump_d.box).get_header();
            if (full_header) {
                output.append(Mpeg4::dump(full_header.value()));
            }

        } else {
            output.append(Mpeg4::dump(header.value()));
        }

        auto ft_box = Mpeg4::FileTypeBoxView(dump_d.box);
        if (ft_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(ft_box));
        }

        auto mvhd_box = Mpeg4::MovieHeaderBoxView(dump_d.box);
        if (mvhd_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(mvhd_box));
        }

        auto tkhd_box = Mpeg4::TrackHeaderBoxView(dump_d.box);
        if (tkhd_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(tkhd_box));
        }

        auto mdia_box = Mpeg4::MediaHeaderBoxView(dump_d.box);
        if (mdia_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(mdia_box));
        }

        auto hdlr_box = Mpeg4::HandlerBoxView(dump_d.box);
        if (hdlr_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(hdlr_box));
        }

        auto stco_box = Mpeg4::ChunkOffsetBoxView(dump_d.box);
        if (stco_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(stco_box));
        }

        auto co64_box = Mpeg4::ChunkOffset64BoxView(dump_d.box);
        if (co64_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(co64_box));
        }

        auto stsz_box = Mpeg4::SampleSizeBoxView(dump_d.box);
        if (stsz_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(stsz_box));
        }

        auto stsd_box = Mpeg4::SampleDescriptionBoxView(dump_d.box);
        if (stsd_box.is_valid()) {
            std::format_to(
                std::back_inserter(output), ",{}", Mpeg4::dump(stsd_box));
        }

        output.append("\n");
    }

    std::ranges::copy(output, std::ostream_iterator<char>(std::cout));

} catch (std::exception &e) {
    std::cout << std::format("Exception \"{}\"\n", e.what());
    return EXIT_FAILURE;
}
