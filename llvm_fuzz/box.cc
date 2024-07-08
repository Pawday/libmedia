#include <algorithm>
#include <cctype>
#include <ctype.h>
#include <format>
#include <iterator>
#include <ranges>
#include <span>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "mpeg4.hh"
#include "mpeg4_dump.hh"
#include "mpeg4_ftype.hh"
#include "mpeg4_hdlr.hh"
#include "mpeg4_mdia.hh"
#include "mpeg4_mvhd.hh"
#include "mpeg4_stsd.hh"
#include "mpeg4_stsz.hh"
#include "mpeg4_tkhd.hh"

static bool check(Mpeg4::BoxViewFileType box)
{
    auto brands_opt = box.get_compatible_brands();
    auto m_brand_opt = box.get_major_brand();
    if (!brands_opt || !m_brand_opt) {
        return false;
    }

    auto brands = brands_opt.value();
    auto m_brand = m_brand_opt.value();

    auto brand_is_printable = [](const Mpeg4::BoxViewFileType::Brand_t &b) {
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

    auto ft_box = Mpeg4::BoxViewFileType(box);
    if (ft_box.is_valid()) {
        std::format_to(std::back_inserter(output), ",{}", Mpeg4::dump(ft_box));
        if (!check(ft_box)) {
            return -1;
        }
        return 0;
    }

    auto mvhd_box = Mpeg4::BoxViewMovieHeader(box);
    if (mvhd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(mvhd_box));
    }

    auto tkhd_box = Mpeg4::BoxViewTrackHeader(box);
    if (tkhd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(tkhd_box));
    }

    auto mdia_box = Mpeg4::BoxViewMediaHeader(box);
    if (mdia_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(mdia_box));
    }

    auto hdlr_box = Mpeg4::BoxViewHandler(box);
    if (hdlr_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(hdlr_box));
    }

    auto stco_box = Mpeg4::BoxViewChunkOffset(box);
    if (stco_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stco_box));
    }

    auto co64_box = Mpeg4::BoxViewChunkLargeOffset(box);
    if (co64_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(co64_box));
    }

    auto stsz_box = Mpeg4::BoxViewSampleSize(box);
    if (stsz_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stsz_box));
    }

    auto stsd_box = Mpeg4::BoxViewSampleDescription(box);
    if (stsd_box.is_valid()) {
        std::format_to(
            std::back_inserter(output), ",{}", Mpeg4::dump(stsd_box));
    }

    return 0;
}
