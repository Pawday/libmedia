#include <algorithm>
#include <cctype>
#include <format>
#include <iterator>
#include <optional>
#include <span>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "libmedia/mpeg4.hh"
#include "libmedia/mpeg4/dump.hh"

#include "libmedia/mpeg4/box/views/ftype.hh"
#include "libmedia/mpeg4/box/views/hdlr.hh"
#include "libmedia/mpeg4/box/views/mdia.hh"
#include "libmedia/mpeg4/box/views/mvhd.hh"
#include "libmedia/mpeg4/box/views/sample_entry.hh"
#include "libmedia/mpeg4/box/views/stsd.hh"
#include "libmedia/mpeg4/box/views/stsz.hh"
#include "libmedia/mpeg4/box/views/tkhd.hh"

static bool check(Mpeg4::FileTypeBoxView box)
{
    auto brands_opt = box.get_compatible_brands();
    auto m_brand_opt = box.get_major_brand();
    if (!brands_opt || !m_brand_opt) {
        return false;
    }

    auto brands = brands_opt.value();
    auto m_brand = m_brand_opt.value();

    auto brand_is_printable = [](const Mpeg4::FileTypeBoxView::Brand_t &b) {
        if (!std::ranges::all_of(b, [](auto a) { return std::isprint(a); })) {
            return false;
        }
        return true;
    };

    if (!brand_is_printable(m_brand)) {
        return false;
    }

    if (!std::ranges::all_of(brands, brand_is_printable)) {
        return false;
    }

    return true;
}

static bool check(Mpeg4::HandlerBoxView box)
{
    auto name_opt = box.get_name_span();
    if (!name_opt) {
        return false;
    }

    auto name = name_opt.value();

    if (!std::ranges::all_of(name, [](auto a) { return std::isprint(a); })) {
        return false;
    }
    return true;
}

static bool check(Mpeg4::SampleDescriptionBoxView box)
try {
    auto entries = box.get_entries().value();

    auto entry_is_printable = [](const Mpeg4::SampleEntryBoxView &b) {
        auto header = b.get_box_header().value();
        if (!std::ranges::all_of(
                header.type.data, [](auto a) { return std::isprint(a); })) {
            return false;
        }
        return true;
    };

    if (!std::ranges::any_of(entries, entry_is_printable)) {
        return false;
    }

    return true;
} catch (std::bad_optional_access &e) {
    return false;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    std::byte const *data = reinterpret_cast<const std::byte *>(Data);
    std::span<const std::byte> box_span(data, Size);
    Mpeg4::BoxView box(box_span);

    auto header = box.get_header();
    if (!header) {
        return -1;
    }

    auto box_data = box.get_data();
    if (!box_data) {
        return -1;
    }

    std::vector<char> output;

    auto ft_box = Mpeg4::FileTypeBoxView(box);
    if (ft_box.is_valid()) {
        std::format_to(std::back_inserter(output), ",{}", Mpeg4::dump(ft_box));
        if (!check(ft_box)) {
            return -1;
        }
        return 0;
    }

    auto mvhd_box = Mpeg4::MovieHeaderBoxView(box);
    if (mvhd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(mvhd_box));
    }

    auto tkhd_box = Mpeg4::TrackHeaderBoxView(box);
    if (tkhd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(tkhd_box));
    }

    auto mdia_box = Mpeg4::MediaHeaderBoxView(box);
    if (mdia_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(mdia_box));
    }

    auto hdlr_box = Mpeg4::HandlerBoxView(box);
    if (hdlr_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(hdlr_box));

        if (!check(hdlr_box)) {
            return -1;
        }
        return 0;
    }

    auto stco_box = Mpeg4::ChunkOffsetBoxView(box);
    if (stco_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stco_box));
    }

    auto co64_box = Mpeg4::ChunkOffset64BoxView(box);
    if (co64_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(co64_box));
    }

    auto stsz_box = Mpeg4::SampleSizeBoxView(box);
    if (stsz_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stsz_box));
    }

    auto stsd_box = Mpeg4::SampleDescriptionBoxView(box);
    if (stsd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stsd_box));
        if (!check(stsd_box)) {
            return -1;
        }
        return 0;
    }

    return 0;
}
