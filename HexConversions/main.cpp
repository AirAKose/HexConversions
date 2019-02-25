#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <sstream>
#include <chrono>
#include <vector>
#include "Util.h"
#include "Hex.h"

template<typename PAIR_t, typename FUNCTOR_t>
bool check_pair(std::string_view prefix, PAIR_t pair, FUNCTOR_t func) noexcept
{
    auto compare = func(pair.first);
    if (compare != pair.second)
    {
        std::cerr << prefix << ": Value did not match] " << compare << " vs " << pair.second << "\n";
        return false;
    }
    return true;
}

template<std::size_t REPEAT, typename FUNCTOR_t>
void time_me(std::string_view prefix, FUNCTOR_t func)
{
    auto stream = std::stringstream{};
    stream << "Running " << prefix << " for " << REPEAT << " iterations...";
    std::cout << std::setw(64) << std::left << stream.str();
    auto timer = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < REPEAT; ++i)
    {
        func();
    }
    auto diff = std::chrono::high_resolution_clock::now() - timer;
    auto timeMS = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

    std::cout << " " << timeMS << "ms\n";
}

void test_char_count()
{
    constexpr std::pair<uint64_t, std::size_t> values[] = {
        std::pair{0x0000000000000000, 1},
        std::pair{0x000000000000000f, 1},
        std::pair{0x00000000000000f0, 2},
        std::pair{0x0000000000000f00, 3},
        std::pair{0x000000000000f000, 4},
        std::pair{0x000000000000ffff, 4},
        std::pair{0xf000000000000000, 16},
        std::pair{0x0f00000000000000, 15},
        std::pair{0x00f0000000000000, 14},
        std::pair{0x000f000000000000, 13},
        std::pair{0xffff000000000000, 16},
        std::pair{0x0000f00000000000, 12},
        std::pair{0x00000f0000000000, 11},
        std::pair{0x000000f000000000, 10},
        std::pair{0x0000000f00000000, 9},
        std::pair{0x0000ffff00000000, 12},
        std::pair{0x00000000f0000000, 8},
        std::pair{0x000000000f000000, 7},
        std::pair{0x0000000000f00000, 6},
        std::pair{0x00000000000f0000, 5},
        std::pair{0x00000000ffff0000, 8},
        std::pair{0xffffffffffffffff, 16},
        std::pair{0xf00000000000000f, 16},
        std::pair{0x0000000ff0000000, 9},
    };

    for (auto pair : values)
    {
        check_pair("Char Count", pair, [](auto value)
        {
            return Hex::char_count_for(value);
        });
    }
}

void test_to_hex()
{
    using INT_t = uint64_t;
    constexpr std::pair<INT_t, std::string_view> values[] = {
        std::pair{0x0000000000000000, "0"},
        std::pair{0x000000000000000f, "f"},
        std::pair{0x00000000000000f0, "f0"},
        std::pair{0x0000000000000f00, "f00"},
        std::pair{0x000000000000f000, "f000"},
        std::pair{0x000000000000ffff, "ffff"},
        std::pair{0xf000000000000000, "f000000000000000"},
        std::pair{0x0f00000000000000, "f00000000000000"},
        std::pair{0x00f0000000000000, "f0000000000000"},
        std::pair{0x000f000000000000, "f000000000000"},
        std::pair{0xffff000000000000, "ffff000000000000"},
        std::pair{0x0000f00000000000, "f00000000000"},
        std::pair{0x00000f0000000000, "f0000000000"},
        std::pair{0x000000f000000000, "f000000000"},
        std::pair{0x0000000f00000000, "f00000000"},
        std::pair{0x0000ffff00000000, "ffff00000000"},
        std::pair{0x00000000f0000000, "f0000000"},
        std::pair{0x000000000f000000, "f000000"},
        std::pair{0x0000000000f00000, "f00000"},
        std::pair{0x00000000000f0000, "f0000"},
        std::pair{0x00000000ffff0000, "ffff0000"},
        std::pair{0xffffffffffffffff, "ffffffffffffffff"},
        std::pair{0xf00000000000000f, "f00000000000000f"},
        std::pair{0x0000000ff0000000, "ff0000000"},
    };

    char buff[Hex::max_char_count_for<INT_t> + 1] = { 0 };
    for (auto pair : values)
    {
        check_pair("Static Buffer", pair, [&](auto first)
        {
            buff[Hex::to_hex(first, buff)] = '\0';
            return std::string_view{ buff };
        });
    }

    constexpr auto strCount = Hex::max_char_count_for<INT_t> + 1;
    auto* ptrBuff = new char[strCount]{ 0 };
    for (auto pair : values)
    {
        check_pair("Dynamic Buffer", pair, [&](auto first)
        {
            ptrBuff[Hex::try_to_hex(first, ptrBuff, strCount)] = '\0';
            return std::string_view{ptrBuff};
        });
    }
    delete[] ptrBuff;


    for (auto pair : values)
    {
        check_pair("Hex String", pair, [&](auto first)
        {
            return Hex::to_hex(first);
        });
    }
}

void test_from_hex()
{
    using INT_t = uint64_t;
    constexpr std::pair<std::string_view, INT_t> values[] = {
        std::pair{"0", 0x0000000000000000},
        std::pair{"00000f", 0x000000000000000f},
        std::pair{"00000f0", 0x00000000000000f0},
        std::pair{"0f00", 0x0000000000000f00},
        std::pair{"000f000", 0x000000000000f000},
        std::pair{"00000000ffff", 0x000000000000ffff},
        std::pair{"f000000000000000", 0xf000000000000000},
        std::pair{"f00000000000000", 0x0f00000000000000},
        std::pair{"f0000000000000", 0x00f0000000000000},
        std::pair{"f000000000000", 0x000f000000000000},
        std::pair{"ffff000000000000", 0xffff000000000000},
        std::pair{"000f00000000000", 0x0000f00000000000},
        std::pair{"000f0000000000", 0x00000f0000000000},
        std::pair{"000f000000000", 0x000000f000000000},
        std::pair{"000000f00000000", 0x0000000f00000000},
        std::pair{"0000ffff00000000", 0x0000ffff00000000},
        std::pair{"0f0000000", 0x00000000f0000000},
        std::pair{"00000f000000", 0x000000000f000000},
        std::pair{"00000f00000", 0x0000000000f00000},
        std::pair{"0000000000f0000", 0x00000000000f0000},
        std::pair{"0000ffff0000", 0x00000000ffff0000},
        std::pair{"ffffffffffffffff", 0xffffffffffffffff},
        std::pair{"f00000000000000f", 0xf00000000000000f},
        std::pair{"00000ff0000000", 0x0000000ff0000000},
        std::pair{"0123456789abcdef", 0x0123456789abcdef},
        std::pair{"fedcba98765432101", 0xfedcba9876543210},
        std::pair{"1234567887654321", 0x1234567887654321},
        std::pair{"abcddcbaabcddcba", 0xabcddcbaabcddcba},
        std::pair{"00000000000000001", 0x0},
        std::pair{"gagagagagag", 0x0},
        std::pair{"agagagagag", 0xa}
    };

    for (auto pair : values)
    {
        check_pair("From Hex", pair, [&](auto first)
        {
            return Hex::from_hex<INT_t>(first);
        });
    }
}

void time_hex()
{
    constexpr std::size_t repeat = 200000;
    constexpr auto value = 0x57fcb3c9ff43abcdull;

    auto values = std::vector<std::string>{}; // ensure our loops aren't optimized away + the values are correct
    auto temp = value;

    values.clear();
    values.reserve(repeat);
    temp = value;
    time_me<repeat>("StringStream Hex", [&]() {
        auto stream = std::stringstream{};
        stream << std::hex << ++temp;
        values.emplace_back(stream.str());
    });

    values.clear();
    values.reserve(repeat);
    temp = value;
    time_me<repeat>("Sprintf Hex", [&]() {
        char buff[50];
        std::size_t count = sprintf_s(buff, 50, "%llx", ++temp);
        values.emplace_back(buff, buff + count);
    });

    values.clear();
    values.reserve(repeat);
    temp = value;
    time_me<repeat>("Custom Hex", [&]() {
        values.emplace_back(Hex::to_hex(++temp));
    });

    /* Wide string variations */

    auto wvalues = std::vector<std::wstring>{}; // ensure our loops aren't optimized away + the values are correct
    wvalues.clear();
    wvalues.reserve(repeat);
    temp = value;
    time_me<repeat>("StringStream W Hex", [&]() {
        auto stream = std::wstringstream{};
        stream << std::hex << ++temp;
        wvalues.emplace_back(stream.str());
    });

    wvalues.clear();
    wvalues.reserve(repeat);
    temp = value;
    time_me<repeat>("Sprintf W Hex", [&]() {
        wchar_t buff[50];
        std::size_t count = swprintf_s(buff, 50, L"%llx", ++temp);
        wvalues.emplace_back(buff, buff + count);
    });

    wvalues.clear();
    wvalues.reserve(repeat);
    temp = value;
    time_me<repeat>("Custom W Hex", [&]() {
        wvalues.emplace_back(Hex::to_hex_w(++temp));
    });
}

int main()
{
    std::cout << "STARTING TESTS..." << std::endl;

    test_char_count();

    test_to_hex();

    std::cout << "TESTS COMPLETE\n" << std::endl;

    std::cout << "STARTING TIMERS..." << std::endl;
    time_hex();

    std::cout << "[[DONE]]" << std::endl;
    std::cin.get();
    return 0;
}