#ifndef SPONGE_TESTS_TEST_SHOULD_BE_HH
#define SPONGE_TESTS_TEST_SHOULD_BE_HH

#include "string_conversions.hh"

#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#define test_should_be(act, exp) _test_should_be(act, exp, #act, #exp, __LINE__)

template <typename T>
static void _test_should_be(const T &actual,
                            const T &expected,
                            const char *actual_s,
                            const char *expected_s,
                            const int lineno) {
    if (actual != expected) {
        std::ostringstream ss;
        ss << "`" << actual_s << "` should have been `" << expected_s << "`, but the former is\n\t" << to_string(actual)
           << "\nand the latter is\n\t" << to_string(expected) << "\n"
           << " (at line " << lineno << ")\n";
        throw std::runtime_error(ss.str());
    }
}

template <typename T, typename U,
          typename = std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>>
static void _test_should_be(const T &actual,
                            const U &expected,
                            const char *actual_s,
                            const char *expected_s,
                            const int lineno) {
    if (static_cast<uint64_t>(actual) != static_cast<uint64_t>(expected)) {
        std::ostringstream ss;
        ss << "`" << actual_s << "` should have been `" << expected_s << "`, but the former is\n\t" << to_string(actual)
           << "\nand the latter is\n\t" << to_string(expected) << "\n"
           << " (at line " << lineno << ")\n";
        throw std::runtime_error(ss.str());
    }
}

#endif  // SPONGE_TESTS_TEST_SHOULD_BE_HH
