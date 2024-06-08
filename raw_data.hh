#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

struct Segment
{
    uint64_t start;
    uint64_t size;
};

struct U64Storage
{
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
    uint8_t b5;
    uint8_t b6;
    uint8_t b7;
};

constexpr bool operator==(const U64Storage &l, const U64Storage &r)
{
    bool output = true;
    output &= l.b0 == r.b0;
    output &= l.b1 == r.b1;
    output &= l.b2 == r.b2;
    output &= l.b3 == r.b3;
    output &= l.b4 == r.b4;
    output &= l.b5 == r.b5;
    output &= l.b6 == r.b6;
    output &= l.b7 == r.b7;
    return output;
}

static_assert(sizeof(U64Storage) == 8);

static constexpr U64Storage incr_byte_sequence{
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

constexpr bool is_le()
{
    uint64_t val = 0x7766554433221100;
    return std::bit_cast<U64Storage>(val) == incr_byte_sequence;
}

constexpr bool is_be()
{
    uint64_t val = 0x0011223344556677;
    return std::bit_cast<U64Storage>(val) == incr_byte_sequence;
}

static_assert(is_be() || is_le(), "Oh god please no");

template <std::integral T>
constexpr T betoh(T val)
{
    if constexpr (is_le()) {
        return std::byteswap(val);
    }
    return val;
}

template <std::integral T>
constexpr T letoh(T val)
{
    if constexpr (is_be()) {
        return std::byteswap(val);
    }
    return val;
}

template <size_t S, size_t EXT>
constexpr std::array<std::byte, S>
    copy_array(std::span<const std::byte, EXT> source)
{
    std::array<std::byte, S> output;
    std::ranges::copy(source | std::views::take(S), std::begin(output));
    return output;
}

template <std::unsigned_integral I, size_t S>
constexpr I from_array_as_le(std::array<std::byte, S> data)
{
    I output = 0;

    for (size_t byte_idx = 0; byte_idx < S; byte_idx++) {
        I part = std::to_integer<uint8_t>(data[byte_idx]);
        part <<= (byte_idx * 8);
        output |= part;
    }

    return output;
}

template <size_t S>
constexpr std::array<std::uint8_t, S>
    to_uint8_arr(const std::array<std::byte, S> &bytes)
{
    std::array<std::uint8_t, S> output;
    auto chars = bytes | std::views::transform(std::to_integer<char>);
    std::ranges::copy(chars, begin(output));
    return output;
}

template <std::unsigned_integral T, size_t EXT>
constexpr T read_be(std::span<const std::byte, EXT> data)
{
    T output = from_array_as_le<uint32_t>(copy_array<sizeof(uint32_t)>(data));
    output = betoh(output);
    return output;
}

template <std::unsigned_integral T, size_t EXT>
T read_le(std::span<const std::byte, EXT> data)
{
    return from_array_as_le<uint32_t>(copy_array<sizeof(uint32_t)>(data));
}
