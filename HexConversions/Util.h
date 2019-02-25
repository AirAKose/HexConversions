#ifndef _HexConversions_Util_HEADER_
#define _HexConversions_Util_HEADER_

#include <type_traits>

namespace Util
{
    namespace Impl
    {
        template<typename T>
        constexpr inline auto toggle_sign_s(T num) noexcept
        {
            using SIGNED_t = std::make_signed_t<T>;
            using TARGET_t = std::conditional_t<std::is_unsigned_v<T>, SIGNED_t, std::make_unsigned_t<T>>;
            constexpr T min = T{ 0x01 } << (sizeof(T) * T { 8 } -T{ 1 });
            constexpr T max = std::numeric_limits<SIGNED_t>::max();

            // Take the negative bit and shift it into positive so it doesn't get overflow checked, then shift back
            // Should ideally work even on non-two's compliment systems (C++20 can't come soon enough) and regardless of endianess
            return static_cast<TARGET_t>(num & max) | (static_cast<TARGET_t>(num & min >> T{ 1 }) << TARGET_t{ 1 });
        }
    }

    /**
       A consistent, safe unsigned cast that isn't implementation-defined + is constexpr compatible
       Will be slower, but is much safer until we get guaranteed cast behavior
    **/
    template<typename T, typename SFINAE = std::enable_if_t<std::is_integral_v<T>>>
    constexpr inline auto to_unsigned_s(T num) noexcept
    {
        if constexpr (std::is_unsigned_v<T>)
        {
            return num;
        }
        else
        {
            return Impl::toggle_sign_s(num);
        }
    }

}// Util

#endif