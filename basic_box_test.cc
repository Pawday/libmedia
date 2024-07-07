#include <algorithm>
#include <array>

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

#include "mpeg4.hh"

template <size_t SIZE>
consteval auto as_bytes(const uint8_t (&d)[SIZE])
{
    std::array<std::byte, SIZE> output{};
    auto make_byte = [](uint8_t n) { return std::byte(n); };
    std::ranges::copy(d | std::views::transform(make_byte), std::begin(output));
    return output;
}

constexpr auto test_box_data =
    as_bytes({0, 0, 0, 12, 't', 'e', 's', 't', 1, 2, 3, 4});
constexpr Mpeg4::BoxView box(test_box_data);

constexpr auto header = *box.get_header();

static_assert(header.header_size == 8);
static_assert(header.box_size == 4);
static_assert(header.type == Mpeg4::TypeTag::from_str("test"));
static_assert(!header.usertype.has_value());
static_assert(header.box_size.has_value());

constexpr auto data = *box.get_data();
static_assert(data.size() == header.box_size);
static_assert(data[0] == std::byte(1));
static_assert(data[1] == std::byte(2));
static_assert(data[2] == std::byte(3));
static_assert(data[3] == std::byte(4));
