//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17

// <format>

// C++23 the formatter is a debug-enabled specialization.
// [format.formatter.spec]:
// Each header that declares the template `formatter` provides the following
// enabled specializations:
// For each `charT`, the string type specializations
//   template<class traits, class Allocator>
//     struct formatter<basic_string<charT, traits, Allocator>, charT>;
//   template<class traits>
//     struct formatter<basic_string_view<charT, traits>, charT>;

#include <format>
#include <cassert>
#include <concepts>
#include <iterator>
#include <memory>
#include <type_traits>

#include "make_string.h"
#include "test_format_context.h"
#include "test_macros.h"

#define STR(S) MAKE_STRING(CharT, S)
#define SV(S) MAKE_STRING_VIEW(CharT, S)
#define CSTR(S) MAKE_CSTRING(CharT, S)

template <class T, class ArgumentT, class StringT, class StringViewT>
void test(StringT expected, StringViewT fmt, StringT a, std::size_t offset) {
  static_assert(
      std::same_as<typename T::value_type,
                   typename std::decay_t<ArgumentT>::value_type> &&
      std::same_as<typename T::value_type, typename StringT::value_type>);
  using CharT = typename T::value_type;

  auto parse_ctx = std::basic_format_parse_context<CharT>(fmt);
  std::formatter<T, CharT> formatter;
  static_assert(std::semiregular<decltype(formatter)>);

  std::same_as<typename StringViewT::iterator> auto it = formatter.parse(parse_ctx);
  // std::to_address works around LWG3989 and MSVC STL's iterator debugging mechanism.
  assert(std::to_address(it) == std::to_address(fmt.end()) - offset);

  StringT result;
  auto out = std::back_inserter(result);
  using FormatCtxT = std::basic_format_context<decltype(out), CharT>;

  ArgumentT arg = a;
  FormatCtxT format_ctx = test_format_context_create<decltype(out), CharT>(out, std::make_format_args<FormatCtxT>(arg));
  formatter.format(arg, format_ctx);
  assert(result == expected);
}

template <class T, class ArgumentT, class StringT>
void test_termination_condition(StringT expected, StringT f, StringT arg) {
  // The format-spec is valid if completely consumed or terminates at a '}'.
  // The valid inputs all end with a '}'. The test is executed twice:
  // - first with the terminating '}',
  // - second consuming the entire input.
  using CharT = typename StringT::value_type;
  std::basic_string_view<CharT> fmt{f};
  assert(fmt.back() == CharT('}') && "Pre-condition failure");

  test<T, ArgumentT>(expected, fmt, arg, 1);
  fmt.remove_suffix(1);
  test<T, ArgumentT>(expected, fmt, arg, 0);
}

#if TEST_STD_VER > 20
template <class ArgumentT, class CharT>
constexpr bool test_set_debug_format() {
  std::formatter<ArgumentT, CharT> formatter;
  LIBCPP_ASSERT(formatter.__parser_.__type_ == std::__format_spec::__type::__default);

  formatter.set_debug_format();
  LIBCPP_ASSERT(formatter.__parser_.__type_ == std::__format_spec::__type::__debug);

  std::basic_string_view fmt = SV("s}");
  std::basic_format_parse_context<CharT> parse_ctx{fmt};
  formatter.parse(parse_ctx);
  LIBCPP_ASSERT(formatter.__parser_.__type_ == std::__format_spec::__type::__string);

  formatter.set_debug_format();
  LIBCPP_ASSERT(formatter.__parser_.__type_ == std::__format_spec::__type::__debug);

  return true;
}
#endif

template <class T, class ArgumentT>
void test_string_type() {
  static_assert(std::same_as<typename T::value_type,
                             typename std::decay_t<ArgumentT>::value_type>);
  using CharT = typename T::value_type;

  test_termination_condition<T, ArgumentT>(STR(" azAZ09,./<>?"), STR("}"),
                                           STR(" azAZ09,./<>?"));

  std::basic_string<CharT> s(CSTR("abc\0abc"), 7);
  test_termination_condition<T, ArgumentT>(s, STR("}"), s);

  test_termination_condition<T, ArgumentT>(STR("world"), STR("}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("world"), STR("_>}"),
                                           STR("world"));

  test_termination_condition<T, ArgumentT>(STR("   world"), STR(">8}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("___world"), STR("_>8}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("_world__"), STR("_^8}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("world___"), STR("_<8}"),
                                           STR("world"));

  test_termination_condition<T, ArgumentT>(STR("world"), STR(".5}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("unive"), STR(".5}"),
                                           STR("universe"));

  test_termination_condition<T, ArgumentT>(STR("%world%"), STR("%^7.7}"),
                                           STR("world"));
  test_termination_condition<T, ArgumentT>(STR("univers"), STR("%^7.7}"),
                                           STR("universe"));

#if TEST_STD_VER > 20
  test_set_debug_format<std::remove_cvref_t<ArgumentT>, CharT>();
  static_assert(test_set_debug_format<std::remove_cvref_t<ArgumentT>, CharT>());
#endif
}

template <class CharT>
void test_all_string_types() {
  test_string_type<std::basic_string<CharT>, const std::basic_string<CharT>&>();
  test_string_type<std::basic_string_view<CharT>,
                   std::basic_string_view<CharT>>();
}

int main(int, char**) {
  test_all_string_types<char>();
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
  test_all_string_types<wchar_t>();
#endif
  return 0;
}
