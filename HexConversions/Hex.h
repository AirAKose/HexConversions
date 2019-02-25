#ifndef _HexConversions_Hex_HEADER_
#define _HexConversions_Hex_HEADER_

#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include "Util.h"

namespace Hex
{
    namespace Impl
    {
        template<typename VALUE_t>
        constexpr VALUE_t make_mask(std::size_t count) noexcept
        {
            if (count == 0) { return 0; }
            VALUE_t mask = 0x0f;
            for (std::size_t i = 1; i < count; ++i)
            {
                mask = (mask << VALUE_t{ 4 }) | VALUE_t{ 0x0f };
            }
            return mask;
        }

        constexpr inline std::size_t bit_offset(std::size_t start, std::size_t count) noexcept
        {
            return (start - count) * 4;
        }

        // binary search through bitmasks to figure out how many hex characters we need
        template<std::size_t START, std::size_t COUNT, typename VALUE_t>
        constexpr inline std::size_t char_count(VALUE_t num) noexcept
        {
            if constexpr (COUNT == 0) { return 1; }
            else if constexpr (COUNT == 1)
            {
                constexpr VALUE_t mask = make_mask<VALUE_t>(COUNT) << bit_offset(START, COUNT);
                if (num & mask) { return START; }
                else { return 1; }
            }
            else
            {
                constexpr std::size_t half = COUNT / std::size_t{ 2 };
                constexpr VALUE_t mask_front = make_mask<VALUE_t>(half) << bit_offset(START + half, COUNT);
                constexpr VALUE_t mask_back = make_mask<VALUE_t>(half) << bit_offset(START, COUNT);
                if (num & mask_front)
                {
                    return char_count<START, half>(num);
                }
                else if (num & mask_back)
                {
                    return char_count<START - half, half>(num);
                }
                else
                {
                    return 1;
                }
            }
        }

        template<typename CHAR_t>
        constexpr inline CHAR_t bad_hex = CHAR_t{'g'}; // obviously wrong for hex, but not destructive

        template<typename T>
        constexpr inline bool in_range_incl(T val, T start, T end) noexcept
        {
            return val >= start && val <= end;
        }

        template<typename CHAR_t, typename NUM_t>
        constexpr CHAR_t value_to_char(NUM_t num) noexcept
        {
            if (in_range_incl<NUM_t>(num, 0, 9))
            {
                return static_cast<CHAR_t>(num + NUM_t{'0'});
            }
            else
            {
                constexpr auto offset = NUM_t{ 'a' - 10 };
                return static_cast<CHAR_t>(num + offset);
            }
        }


        template<typename NUM_t, typename CHAR_t>
        constexpr NUM_t char_to_value(CHAR_t c) noexcept
        {
            if (in_range_incl<CHAR_t>(c, '0', '9'))
            {
                return static_cast<NUM_t>(c - CHAR_t{'0'});
            }
            else if (in_range_incl<CHAR_t>(c, 'a', 'z'))
            {
                return static_cast<NUM_t>(c - CHAR_t{'a'}) + NUM_t{10};
            }
            else if (in_range_incl<CHAR_t>(c, 'A', 'Z'))
            {
                return static_cast<NUM_t>(c - CHAR_t{'A'}) + NUM_t{10};
            }
            return bad_hex<NUM_t>;
        }

        template<typename T>
        constexpr inline std::size_t max_char_count_for = sizeof(T) * 2;

        template<typename T>
        constexpr inline std::size_t char_count_for(T num) noexcept
        {
            constexpr std::size_t valueSize = max_char_count_for<T>;
            return Impl::char_count<valueSize, valueSize>(Util::to_unsigned_s(num));
        }

        
        template<typename NUM_t, typename STR_t>
        constexpr std::size_t to_hex(NUM_t num, STR_t buff, std::size_t hexCount) noexcept
        {
            using CHAR_t = std::remove_pointer_t<std::decay_t<STR_t>>;
            if (num == 0) // special case so we output something regardless
            {
                buff[0] = CHAR_t{'0'};
                buff[1] = CHAR_t{'\0'};
                return 1;
            }

            std::size_t index = hexCount - 1;
            constexpr auto mask = NUM_t{ 0x0f };
            for ( ; num; (num = num >> 4), --index)
            {
                buff[index] = Impl::value_to_char<CHAR_t>(num & mask);
            }
            return hexCount;
        }
    } // Impl





    template<typename T, typename SFINAE = std::enable_if_t<std::is_integral_v<T>>>
    constexpr inline auto max_char_count_for = Impl::max_char_count_for<T>;

    template<typename T, typename SFINAE = std::enable_if_t<std::is_integral_v<T>>>
    constexpr inline auto char_count_for(T num) noexcept
    {
        return Impl::char_count_for(num);
    }

    template<typename NUM_t, typename CHAR_t, std::size_t BUFF_SZ>
    constexpr std::size_t try_to_hex(NUM_t num, CHAR_t (&buff)[BUFF_SZ]) noexcept
    {
        auto hexCount = char_count_for(num);
        if (BUFF_SZ < hexCount + 1)
        {
            return 0;
        }
        return Impl::to_hex(num, buff, hexCount);
    }

    template<typename NUM_t, typename STR_t, typename SFINAE = std::enable_if_t<std::is_pointer_v<STR_t>>>
    constexpr std::size_t try_to_hex(NUM_t num, STR_t buff, std::size_t count) noexcept
    {
        using CHAR_t = std::remove_pointer_t<std::decay_t<STR_t>>;
        auto hexCount = char_count_for(num);
        if (!buff || count < hexCount + 1)
        {
            return 0;
        }
        return Impl::to_hex(num, buff, hexCount);
    }

    template<typename NUM_t, typename CHAR_t, std::size_t BUFF_SZ>
    constexpr inline auto to_hex(NUM_t num, CHAR_t(&buff)[BUFF_SZ]) noexcept
    {
        static_assert(BUFF_SZ >= max_char_count_for<NUM_t> + 1, "to_hex buffer is too small, must be able to fit at least max_char_count_for<type> + 1");
        auto hexCount = char_count_for(num);
        return Impl::to_hex(num, buff, char_count_for(num));
    }

    template<typename NUM_t>
    /*constexpr*/ inline std::string to_hex(NUM_t num) noexcept // TODO: Make constexpr as soon as standard string is constexpr
    {
        auto hexCount = char_count_for(num);
        auto str = std::string{};
        str.resize(hexCount);
        Impl::to_hex(num, str.data(), hexCount);
        return std::move(str);
    }

    template<typename NUM_t>
    /*constexpr*/ inline std::wstring to_hex_w(NUM_t num) noexcept // TODO: Make constexpr as soon as standard string is constexpr
    {
        auto hexCount = char_count_for(num);
        auto str = std::wstring{};
        str.resize(hexCount);
        Impl::to_hex(num, str.data(), hexCount);
        return std::move(str);
    }

    template<typename NUM_t>
    constexpr inline NUM_t from_hex(std::string_view str, std::size_t& out_readCount) noexcept
    {
        auto value = NUM_t{0};
        out_readCount = 0;
        for (auto c : str)
        {
            auto temp = Impl::char_to_value<NUM_t>(c);
            if (temp == Impl::bad_hex<NUM_t>) { break; }

            value = (value << 4) | temp;
            ++out_readCount;
            if (out_readCount >= sizeof(NUM_t) * 2) { break; }
        }
        return value;
    }

    template<typename NUM_t>
    constexpr inline NUM_t from_hex(std::wstring_view str, std::size_t& out_readCount) noexcept
    {
        auto value = NUM_t{0};
        out_readCount = 0;
        for (auto c : str)
        {
            auto temp = Impl::char_to_value<NUM_t>(c);
            if (temp == Impl::bad_hex<NUM_t>) { break; }

            value = (value << 4) | temp;
            ++out_readCount;
            if (out_readCount >= sizeof(NUM_t) * 2) { break; }
        }
        return value;
    }

    template<typename NUM_t>
    constexpr inline NUM_t from_hex(std::string_view str) noexcept
    {
        auto read = std::size_t{0};
        return from_hex<NUM_t>(str, read);
    }

    template<typename NUM_t>
    constexpr inline NUM_t from_hex(std::wstring_view str) noexcept
    {
        auto read = std::size_t{0};
        return from_hex<NUM_t>(str, read);
    }
}// Hex

#endif