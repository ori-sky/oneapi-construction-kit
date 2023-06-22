// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions; you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Derived from CC0-licensed `tl::optional` library which can be found
// at https://github.com/TartanLlama/optional.  See optional.LICENSE.txt.

/// @file
///
/// @brief An implementation of std::optional with extensions

#ifndef CARGO_OPTIONAL_H_INCLUDED
#define CARGO_OPTIONAL_H_INCLUDED

#include <cargo/detail/optional.h>
#include <cargo/error.h>

namespace cargo {
/// @addtogroup cargo
/// @{

/// @brief A tag type to represent an empty optional
struct nullopt_t {
  // This is a passkey so that users are forced to use `nullopt` rather than
  // creating their own
  struct do_not_use {};
  constexpr explicit nullopt_t(do_not_use, do_not_use) {}
};

/// @brief Represents an empty optional
///
/// *Examples*:
/// ```
/// cargo::optional<int> a = cargo::nullopt;
/// void foo (cargo::optional<int>);
/// foo(cargo::nullopt); //pass an empty optional
/// ```
static constexpr nullopt_t nullopt{nullopt_t::do_not_use{},
                                   nullopt_t::do_not_use{}};

/// @brief Either contains a value or nothing.
///
/// An optional object is an object that contains the storage for another
/// object and manages the lifetime of this contained object, if any. The
/// contained object may be initialized after the optional object has been
/// initialized, and may be destroyed before the optional object has been
/// destroyed. The initialization state of the contained object is tracked by
/// the optional object.
///
/// Examples:
///
/// ```
/// cargo::optional<int> maybe_get_data() {
///   if (is_data_available()) {
///     return get_data();
///   }
///   return cargo::nullopt;
/// }
///
/// auto data = maybe_get_data();
///
/// data.has_value(); // check if a value is stored
/// if (data) {} // also works
///
/// data.value(); // retrieve value
/// *data; // ditto
/// ```
///
/// This implementation also has support for functional composition. Instead of
/// writing:
///
/// ```
/// cargo::optional<image> get_cute_cat(const image& img) {
///   auto cropped = crop_to_cat(img);
///   if (!cropped) {
///     return cargo::nullopt;
///   }
///
///   auto with_tie = add_bow_tie(*cropped);
///   if (!with_tie) {
///     return cargo::nullopt;
///   }
///
///   auto with_sparkles = make_eyes_sparkle(*with_tie);
///   if (!with_sparkles) {
///     return cargo::nullopt;
///   }
///
///   return add_rainbow(make_smaller(*with_sparkles));
/// }
/// ```
///
/// you can write:
///
/// ```
/// cargo::optional<image> get_cute_cat(const image& img) {
///   return crop_to_cat(img)
///         .and_then(add_bow_tie)
///         .and_then(make_eyes_sparkle)
///         .map(make_smaller)
///         .map(add_rainbow);
/// }
/// ```
///
/// @tparam T Type of contained element.
template <class T>
class optional : private detail::optional_move_assign_base<wrap_reference_t<T>>,
                 private detail::delete_ctor_base<
                     std::is_copy_constructible<wrap_reference_t<T>>::value,
                     std::is_move_constructible<wrap_reference_t<T>>::value>,
                 private detail::delete_assign_base<
                     std::is_copy_assignable<wrap_reference_t<T>>::value,
                     std::is_move_assignable<wrap_reference_t<T>>::value> {
  using base = detail::optional_move_assign_base<wrap_reference_t<T>>;
  using storage_type = wrap_reference_t<T>;

 public:
  /// @brief Type of the contained element.
  using value_type = T;

  /// @brief Default constructor.
  ///
  /// Constructs an optional that does not contain a value.
  constexpr optional() = default;

  /// @brief Empty constructor.
  ///
  /// Constructs an optional that does not contain a value.
  constexpr optional(nullopt_t) {}

  /// @brief Copy constructor.
  ///
  /// If `rhs` contains a value, the stored value is direct-initialized with
  /// it. Otherwise, the constructed optional is empty.
  CARGO_CXX14_CONSTEXPR optional(const optional &rhs) = default;

  /// @brief Move constructor.
  ///
  /// If `rhs` contains a value, the stored value is direct-initialized with
  /// it. Otherwise, the constructed optional is empty.
  CARGO_CXX14_CONSTEXPR optional(optional &&rhs) = default;

  /// @brief In-place construction.
  ///
  /// Constructs the stored value in-place using the given arguments.
  template <class... Args>
  constexpr explicit optional(
      enable_if_t<std::is_constructible<T, Args...>::value, in_place_t>,
      Args &&...args)
      : base(in_place, std::forward<Args>(args)...) {}

  /// @brief In-place construction.
  ///
  /// Constructs the stored value in-place using the given arguments.
  template <class U, class... Args>
  CARGO_CXX14_CONSTEXPR explicit optional(
      enable_if_t<std::is_constructible<T, std::initializer_list<U> &,
                                        Args &&...>::value,
                  in_place_t>,
      std::initializer_list<U> il, Args &&...args) {
    this->construct(il, std::forward<Args>(args)...);
  }

  /// @brief Construction from a value.
  ///
  /// Constructs the stored value with `u`.
  template <class U = T,
            enable_if_t<std::is_convertible<U &&, T>::value> * = nullptr,
            detail::enable_forward_value<T, U> * = nullptr>
  CARGO_CXX14_CONSTEXPR optional(U &&u) : base(in_place, std::forward<U>(u)) {}

  /// @brief Construction from a value.
  ///
  /// Constructs the stored value with `u`.
  template <class U = T,
            enable_if_t<!std::is_convertible<U &&, T>::value> * = nullptr,
            detail::enable_forward_value<T, U> * = nullptr>
  CARGO_CXX14_CONSTEXPR explicit optional(U &&u)
      : base(in_place, std::forward<U>(u)) {}

  /// @brief Converting copy constructor.
  template <class U, detail::enable_from_other<T, U, const U &> * = nullptr,
            enable_if_t<std::is_convertible<const U &, T>::value> * = nullptr>
  optional(const optional<U> &rhs) {
    this->construct(*rhs);
  }

  /// @brief Converting copy constructor.
  template <class U, detail::enable_from_other<T, U, const U &> * = nullptr,
            enable_if_t<!std::is_convertible<const U &, T>::value> * = nullptr>
  explicit optional(const optional<U> &rhs) {
    this->construct(*rhs);
  }

  /// @brief Converting move constructor.
  template <class U, detail::enable_from_other<T, U, U &&> * = nullptr,
            enable_if_t<std::is_convertible<U &&, T>::value> * = nullptr>
  optional(optional<U> &&rhs) {
    this->construct(std::move(*rhs));
  }

  /// @brief Converting move constructor.
  template <class U, detail::enable_from_other<T, U, U &&> * = nullptr,
            enable_if_t<!std::is_convertible<U &&, T>::value> * = nullptr>
  explicit optional(optional<U> &&rhs) {
    this->construct(std::move(*rhs));
  }

  /// @brief Destructor
  ///
  /// Destroys the stored value if there is one.
  ~optional() = default;

  /// @brief Assignment to empty.
  ///
  /// Destroys the current value if there is one.
  optional &operator=(nullopt_t) {
    if (has_value()) {
      this->m_value.~storage_type();
      this->m_has_value = false;
    }
    return *this;
  }

  /// @brief Copy assignment.
  ///
  /// Copies the value from `rhs` if there is one. Otherwise resets the stored
  /// value in `*this`.
  optional &operator=(const optional &rhs) = default;

  /// @brief Move assignment.
  ///
  /// Moves the value from `rhs` if there is one. Otherwise resets the stored
  /// value in `*this`.
  optional &operator=(optional &&rhs) = default;

  /// @brief Assignment from value.
  ///
  /// Assigns the stored value from `u`, destroying the old value if there was
  /// one.
  template <class U = T, detail::enable_assign_forward<T, U> * = nullptr>
  optional &operator=(U &&u) {
    if (has_value()) {
      this->m_value = std::forward<U>(u);
    } else {
      this->construct(std::forward<U>(u));
    }
    return *this;
  }

  /// @brief Converting copy assignment operator.
  ///
  /// Copies the value from `rhs` if there is one. Otherwise resets the stored
  /// value in `*this`.
  template <class U,
            detail::enable_assign_from_other<T, U, const U &> * = nullptr>
  optional &operator=(const optional<U> &rhs) {
    if (has_value()) {
      if (rhs.has_value()) {
        this->m_value = *rhs;
      } else {
        this->hard_reset();
      }
    }
    if (rhs.has_value()) {
      this->construct(*rhs);
    }
    return *this;
  }

  /// @brief Converting move assignment operator.
  ///
  /// Moves the value from `rhs` if there is one. Otherwise resets the stored
  /// value in `*this`.
  template <class U, detail::enable_assign_from_other<T, U, U> * = nullptr>
  optional &operator=(optional<U> &&rhs) {
    if (has_value()) {
      if (rhs.has_value()) {
        this->m_value = std::move(*rhs);
      } else {
        this->hard_reset();
      }
    }
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    }
    return *this;
  }

// The different versions for C++14 and 11 are needed because deduced return
// types are not SFINAE-safe. This provides better support for things like
// generic lambdas. C.f.
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0826r0.html
#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54)
  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), value())`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), value())` is returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) & {
    using result = invoke_result_t<F, T &>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), **this) : result(nullopt);
  }

  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), std::move(value()))`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), std::move(value()))` is
  /// returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) && {
    using result = invoke_result_t<F, T &&>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : result(nullopt);
  }

  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), value())`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), value())` is returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto and_then(F &&f) const & {
    using result = invoke_result_t<F, const T &>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), **this) : result(nullopt);
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), std::move(value()))`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), std::move(value()))` is
  /// returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto and_then(F &&f) const && {
    using result = invoke_result_t<F, const T &&>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : result(nullopt);
  }
#endif
#else
  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), value())`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), value())` is returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR invoke_result_t<F, T &> and_then(F &&f) & {
    using result = invoke_result_t<F, T &>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), **this) : result(nullopt);
  }

  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), std::move(value()))`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), std::move(value()))` is
  /// returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR invoke_result_t<F, T &&> and_then(F &&f) && {
    using result = invoke_result_t<F, T &&>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : result(nullopt);
  }

  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), value())`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), value())` is returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  constexpr invoke_result_t<F, const T &> and_then(F &&f) const & {
    using result = invoke_result_t<F, const T &>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), **this) : result(nullopt);
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Carries out some operation which returns an optional on the stored
  /// object if there is one.
  ///
  /// @note `std::invoke(std::forward<F>(f), std::move(value()))`
  /// must return a `std::optional<U>` for some `U`.
  ///
  /// @retval empty An empty optional is returned if `*this` is empty.
  /// @retval value `std::invoke(std::forward<F>(f), std::move(value()))` is
  /// returned if
  /// `*this` has a value.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  constexpr invoke_result_t<F, const T &&> and_then(F &&f) const && {
    using result = invoke_result_t<F, const T &&>;
    static_assert(detail::is_optional<result>::value,
                  "F must return an optional");
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : result(nullopt);
  }
#endif
#endif

#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54)

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), value())` and is
  /// returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) & {
    return optional_map_impl(*this, std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), std::move(value()))`
  /// and is returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) && {
    return optional_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), value())` and is
  /// returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto map(F &&f) const & {
    return optional_map_impl(*this, std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), std::move(value()))`
  /// and is returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto map(F &&f) const && {
    return optional_map_impl(std::move(*this), std::forward<F>(f));
  }
#else
  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), value())` and is
  /// returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) & -> decltype(
      optional_map_impl(std::declval<optional &>(), std::declval<F &&>())) {
    return optional_map_impl(*this, std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), std::move(value()))`
  /// and is returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) && -> decltype(
      optional_map_impl(std::declval<optional &&>(), std::declval<F &&>())) {
    return optional_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), value())` and is
  /// returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto map(F &&f) const & -> decltype(optional_map_impl(
      std::declval<const optional &>(), std::declval<F &&>())) {
    return optional_map_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// @retval empty The return value is empty if `*this` is empty.
  /// @retval value If `*this` has a value, an `optional<U>` is constructed from
  /// the return value of `std::invoke(std::forward<F>(f), std::move(value()))`
  /// and is returned.
  ///
  /// @return Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// std::move(value()))`. Returns a `std::optional<U>`.
  template <class F>
  constexpr auto map(F &&f) const && -> decltype(optional_map_impl(
      std::declval<const optional &&>(), std::declval<F &&>())) {
    return optional_map_impl(std::move(*this), std::forward<F>(f));
  }
#endif
#endif

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `*this`. Otherwise, if `f` returns `void`,
  /// calls `std::forward<F>(f)` and returns `std::nullopt`. Otherwise, returns
  /// `std::forward<F>(f)()`.
  template <class F, detail::enable_if_ret_void<F> * = nullptr>
  optional<T> CARGO_CXX14_CONSTEXPR or_else(F &&f) & {
    if (has_value()) {
      return *this;
    }
    std::forward<F>(f)();
    return nullopt;
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `*this`. Otherwise, if `f` returns `void`,
  /// calls `std::forward<F>(f)` and returns `std::nullopt`. Otherwise, returns
  /// `std::forward<F>(f)()`.
  template <class F, detail::disable_if_ret_void<F> * = nullptr>
  optional<T> CARGO_CXX14_CONSTEXPR or_else(F &&f) & {
    return has_value() ? *this : std::forward<F>(f)();
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `std::move(*this)`. Otherwise, if `f`
  /// returns `void`, calls `std::forward<F>(f)` and returns `std::nullopt`.
  /// Otherwise, returns `std::forward<F>(f)()`.
  template <class F, detail::enable_if_ret_void<F> * = nullptr>
  optional<T> or_else(F &&f) && {
    if (has_value()) {
      return std::move(*this);
    }
    std::forward<F>(f)();
    return nullopt;
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `std::move(*this)`. Otherwise, if `f`
  /// returns `void`, calls `std::forward<F>(f)` and returns `std::nullopt`.
  /// Otherwise, returns `std::forward<F>(f)()`.
  template <class F, detail::disable_if_ret_void<F> * = nullptr>
  optional<T> CARGO_CXX14_CONSTEXPR or_else(F &&f) && {
    return has_value() ? std::move(*this) : std::forward<F>(f)();
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `std::move(*this)`. Otherwise, if `f`
  /// returns `void`, calls `std::forward<F>(f)` and returns `std::nullopt`.
  /// Otherwise, returns `std::forward<F>(f)()`.
  template <class F, detail::enable_if_ret_void<F> * = nullptr>
  optional<T> or_else(F &&f) const & {
    if (has_value()) {
      return *this;
    }
    std::forward<F>(f)();
    return nullopt;
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `*this`. Otherwise, if `f` returns `void`,
  /// calls `std::forward<F>(f)` and returns `std::nullopt`. Otherwise, returns
  /// `std::forward<F>(f)()`.
  template <class F, detail::disable_if_ret_void<F> * = nullptr>
  optional<T> CARGO_CXX14_CONSTEXPR or_else(F &&f) const & {
    return has_value() ? *this : std::forward<F>(f)();
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `std::move(*this)`. Otherwise, if `f`
  /// returns `void`, calls `std::forward<F>(f)` and returns `std::nullopt`.
  /// Otherwise, returns `std::forward<F>(f)()`.
  template <class F, detail::enable_if_ret_void<F> * = nullptr>
  optional<T> or_else(F &&f) const && {
    if (has_value()) {
      return std::move(*this);
    }
    std::forward<F>(f)();
    return nullopt;
  }

  /// @brief Calls `f` if the optional is empty
  ///
  /// @note `std::invoke_result_t<F>` must be void or convertible to
  /// `optional<T>`.
  ///
  /// If `*this` has a value, returns `std::move(*this)`. Otherwise, if `f`
  /// returns `void`, calls `std::forward<F>(f)` and returns `std::nullopt`.
  /// Otherwise, returns `std::forward<F>(f)()`.
  template <class F, detail::disable_if_ret_void<F> * = nullptr>
  optional<T> or_else(F &&f) const && {
    return has_value() ? std::move(*this) : std::forward<F>(f)();
  }
#endif

  /// @brief Maps the stored value with `f` if there is one, otherwise returns
  /// `u`.
  ///
  /// If there is a value stored, then `f` is called with `**this`
  /// and the value is returned. Otherwise `u` is returned.
  template <class F, class U>
  U map_or(F &&f, U &&u) & {
    return has_value() ? invoke(std::forward<F>(f), **this)
                       : std::forward<U>(u);
  }

  /// @brief Maps the stored value with `f` if there is one, otherwise returns
  /// `u`.
  ///
  /// If there is a value stored, then `f` is called with `std::move(**this)`
  /// and the value is returned. Otherwise `u` is returned.
  template <class F, class U>
  U map_or(F &&f, U &&u) && {
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : std::forward<U>(u);
  }

  /// @brief Maps the stored value with `f` if there is one, otherwise returns
  /// `u`.
  ///
  /// If there is a value stored, then `f` is called with `**this`
  /// and the value is returned. Otherwise `u` is returned.
  template <class F, class U>
  U map_or(F &&f, U &&u) const & {
    return has_value() ? invoke(std::forward<F>(f), **this)
                       : std::forward<U>(u);
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Maps the stored value with `f` if there is one, otherwise returns
  /// `u`.
  ///
  /// If there is a value stored, then `f` is called with `std::move(**this)`
  /// and the value is returned. Otherwise `u` is returned.
  template <class F, class U>
  U map_or(F &&f, U &&u) const && {
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : std::forward<U>(u);
  }
#endif

  /// @brief Maps the stored value with `f` if there is one, otherwise calls
  /// `u` and returns the result.
  ///
  /// If there is a value stored, then `f` is
  /// called with `**this` and the value is returned. Otherwise
  /// `std::forward<U>(u)()` is returned.
  template <class F, class U>
  invoke_result_t<U> map_or_else(F &&f, U &&u) & {
    return has_value() ? invoke(std::forward<F>(f), **this)
                       : std::forward<U>(u)();
  }

  /// @brief Maps the stored value with `f` if there is one, otherwise calls
  /// `u` and returns the result.
  ///
  /// If there is a value stored, then `f` is
  /// called with `std::move(**this)` and the value is returned. Otherwise
  /// `std::forward<U>(u)()` is returned.
  template <class F, class U>
  invoke_result_t<U> map_or_else(F &&f, U &&u) && {
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : std::forward<U>(u)();
  }

  /// @brief Maps the stored value with `f` if there is one, otherwise calls
  /// `u` and returns the result.
  ///
  /// If there is a value stored, then `f` is
  /// called with `**this` and the value is returned. Otherwise
  /// `std::forward<U>(u)()` is returned.
  template <class F, class U>
  invoke_result_t<U> map_or_else(F &&f, U &&u) const & {
    return has_value() ? invoke(std::forward<F>(f), **this)
                       : std::forward<U>(u)();
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Maps the stored value with `f` if there is one, otherwise calls
  /// `u` and returns the result.
  ///
  /// If there is a value stored, then `f` is
  /// called with `std::move(**this)` and the value is returned. Otherwise
  /// `std::forward<U>(u)()` is returned.
  template <class F, class U>
  invoke_result_t<U> map_or_else(F &&f, U &&u) const && {
    return has_value() ? invoke(std::forward<F>(f), std::move(**this))
                       : std::forward<U>(u)();
  }
#endif

  /// @brief An AND operation on the stored value.
  ///
  /// @return `u` if `*this` has a value, otherwise an empty optional.
  template <class U>
  constexpr optional<typename std::decay<U>::type> conjunction(U &&u) const {
    using result = optional<decay_t<U>>;
    return has_value() ? result{u} : result{nullopt};
  }

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  CARGO_CXX14_CONSTEXPR optional disjunction(const optional &rhs) & {
    return has_value() ? *this : rhs;
  }

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  constexpr optional disjunction(const optional &rhs) const & {
    return has_value() ? *this : rhs;
  }

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  CARGO_CXX14_CONSTEXPR optional disjunction(const optional &rhs) && {
    return has_value() ? std::move(*this) : rhs;
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  constexpr optional disjunction(const optional &rhs) const && {
    return has_value() ? std::move(*this) : rhs;
  }
#endif

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  CARGO_CXX14_CONSTEXPR optional disjunction(optional &&rhs) & {
    return has_value() ? *this : std::move(rhs);
  }

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  constexpr optional disjunction(optional &&rhs) const & {
    return has_value() ? *this : std::move(rhs);
  }

  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  CARGO_CXX14_CONSTEXPR optional disjunction(optional &&rhs) && {
    return has_value() ? std::move(*this) : std::move(rhs);
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief An OR operation on the stored value.
  ///
  /// @return `rhs` if `*this` is empty, otherwise the current value.
  constexpr optional disjunction(optional &&rhs) const && {
    return has_value() ? std::move(*this) : std::move(rhs);
  }
#endif

  /// @brief Takes the value out of the optional, leaving it empty.
  ///
  /// @return An optional equal to `*this`.
  optional take() & {
    optional ret = *this;
    reset();
    return ret;
  }

  /// @brief Takes the value out of the optional, leaving it empty.
  ///
  /// @return An optional equal to `*this`.
  optional take() const & {
    optional ret = *this;
    reset();
    return ret;
  }

  /// @brief Takes the value out of the optional, leaving it empty.
  ///
  /// @return An optional equal to `*this`.
  optional take() && {
    optional ret = std::move(*this);
    reset();
    return ret;
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Takes the value out of the optional, leaving it empty.
  ///
  /// @return An optional equal to `*this`.
  optional take() const && {
    optional ret = std::move(*this);
    reset();
    return ret;
  }
#endif

  /// @brief Constructs the value in-place, destroying the current one if there
  /// is one.
  ///
  /// @param args Arguments to construct the value with.
  ///
  /// @return The constructed value.
  template <class... Args>
  T &emplace(Args &&...args) {
    static_assert(std::is_constructible<T, Args &&...>::value,
                  "T must be constructible with Args");
    *this = nullopt;
    this->construct(std::forward<Args>(args)...);
    return this->m_value;
  }

  /// @brief Constructs the value in-place, destroying the current one if there
  /// is one.
  ///
  /// @param il Arguments to construct the value with.
  /// @param args More arguments to construct the value with.
  ///
  /// @return The constructed value.
  template <class U, class... Args>
  enable_if_t<
      std::is_constructible<T, std::initializer_list<U> &, Args &&...>::value,
      T &>
  emplace(std::initializer_list<U> il, Args &&...args) {
    *this = nullopt;
    this->construct(il, std::forward<Args>(args)...);
    return this->m_value;
  }

  /// @brief Swaps this optional with the other.
  ///
  /// If neither optionals have a value, nothing happens.
  /// If both have a value, the values are swapped.
  /// If one has a value, it is moved to the other and the movee is left
  /// valueless.
  void swap(optional &rhs) {
    if (has_value()) {
      if (rhs.has_value()) {
        using std::swap;
        swap(**this, *rhs);
      } else {
        new (std::addressof(rhs.m_value)) T(std::move(this->m_value));
        this->m_value.T::~storage_type();
      }
    } else if (rhs.has_value()) {
      new (std::addressof(this->m_value)) T(std::move(rhs.m_value));
      rhs.m_value.T::~storage_type();
    }
  }

  /// @brief Retrieve a pointer to the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return A pointer to the contained value  .
  CARGO_CXX14_CONSTEXPR remove_reference_t<T> *operator->() {
    value_type &ref = this->m_value;
    return std::addressof(ref);
  }

  /// @brief Retrieve a pointer to the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return A pointer to the contained value.
  CARGO_CXX14_CONSTEXPR const remove_reference_t<T> *operator->() const {
    const value_type &ref = this->m_value;
    return std::addressof(ref);
  }

  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR T &operator*() & { return this->m_value; }

  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  constexpr const T &operator*() const & { return this->m_value; }

  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR T &&operator*() && { return std::move(this->m_value); }

#ifndef CARGO_NO_CONSTRR
  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  constexpr const T &&operator*() const && { return std::move(this->m_value); }
#endif

  /// @brief Determine if the optional has a value.
  ///
  /// @return Returns true if a value is stored, false otherwise.
  constexpr bool has_value() const { return this->m_has_value; }

  /// @brief Determine if the optional has a value.
  ///
  /// @return Returns true if a value is stored, false otherwise.
  constexpr explicit operator bool() const { return this->m_has_value; }

  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR T &value() & {
    CARGO_ASSERT(has_value(), "optional does not have a value");
    return this->m_value;
  }
  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR const T &value() const & {
    CARGO_ASSERT(has_value(), "optional does not have a value");
    return this->m_value;
  }
  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR T &&value() && {
    CARGO_ASSERT(has_value(), "optional does not have a value");
    return std::move(this->m_value);
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Access the contained value.
  ///
  /// @note The optional must have a value.
  ///
  /// @return The contained value.
  CARGO_CXX14_CONSTEXPR const T &&value() const && {
    CARGO_ASSERT(has_value(), "optional does not have a value");
    return std::move(this->m_value);
  }
#endif

  /// @return The stored value if there is one, otherwise `u`.
  template <class U>
  constexpr T value_or(U &&u) const & {
    static_assert(std::is_copy_constructible<T>::value &&
                      std::is_convertible<U &&, T>::value,
                  "T must be copy constructible and convertible from U");
    return has_value() ? **this : static_cast<T>(std::forward<U>(u));
  }

  /// @return The stored value if there is one, otherwise `u`.
  template <class U>
  CARGO_CXX14_CONSTEXPR T value_or(U &&u) && {
    static_assert(std::is_move_constructible<T>::value &&
                      std::is_convertible<U &&, T>::value,
                  "T must be move constructible and convertible from U");
    return has_value() ? **this : static_cast<T>(std::forward<U>(u));
  }

  /// @brief Destroys the stored value if one exists, making the optional empty
  void reset() {
    if (has_value()) {
      this->m_value.~storage_type();
      this->m_has_value = false;
    }
  }
};  // namespace cargo

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` and `rhs` are equal only if they are
/// both empty.
template <class T, class U>
inline constexpr bool operator==(const optional<T> &lhs,
                                 const optional<U> &rhs) {
  return lhs.has_value() == rhs.has_value() &&
         (!lhs.has_value() || *lhs == *rhs);
}

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` and `rhs` are equal only if they are
/// both empty.
template <class T, class U>
inline constexpr bool operator!=(const optional<T> &lhs,
                                 const optional<U> &rhs) {
  return lhs.has_value() != rhs.has_value() ||
         (lhs.has_value() && *lhs != *rhs);
}

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` is less than `rhs` only if `rhs` is
/// empty and `lhs` is not.
template <class T, class U>
inline constexpr bool operator<(const optional<T> &lhs,
                                const optional<U> &rhs) {
  return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` is less than `rhs` only if `rhs` is
/// empty and `lhs` is not.
template <class T, class U>
inline constexpr bool operator>(const optional<T> &lhs,
                                const optional<U> &rhs) {
  return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` is less than `rhs` only if `rhs` is
/// empty and `lhs` is not.
template <class T, class U>
inline constexpr bool operator<=(const optional<T> &lhs,
                                 const optional<U> &rhs) {
  return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

/// @brief Compares two optional objects
///
/// @return If both optionals contain a value, they are compared with `T`'s
/// relational operators. Otherwise `lhs` is less than `rhs` only if `rhs` is
/// empty and `lhs` is not.
template <class T, class U>
inline constexpr bool operator>=(const optional<T> &lhs,
                                 const optional<U> &rhs) {
  return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is equal to `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator==(const optional<T> &lhs, nullopt_t) {
  return !lhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is equal to `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator==(nullopt_t, const optional<T> &rhs) {
  return !rhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is equal to `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator!=(const optional<T> &lhs, nullopt_t) {
  return lhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is equal to `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator!=(nullopt_t, const optional<T> &rhs) {
  return rhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator<(const optional<T> &, nullopt_t) {
  return false;
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator<(nullopt_t, const optional<T> &rhs) {
  return rhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator<=(const optional<T> &lhs, nullopt_t) {
  return !lhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator<=(nullopt_t, const optional<T> &) {
  return true;
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator>(const optional<T> &lhs, nullopt_t) {
  return lhs.has_value();
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator>(nullopt_t, const optional<T> &) {
  return false;
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator>=(const optional<T> &, nullopt_t) {
  return true;
}

/// @brief Compares the optional with a `nullopt`.
///
/// @return The optional is less than `nullopt` only if it has no value.
template <class T>
inline constexpr bool operator>=(nullopt_t, const optional<T> &rhs) {
  return !rhs.has_value();
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// not equal to the value.
template <class T, class U>
inline constexpr bool operator==(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs == rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// not equal to the value.
template <class T, class U>
inline constexpr bool operator==(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs == *rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// not equal to the value.
template <class T, class U>
inline constexpr bool operator!=(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs != rhs : true;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// not equal to the value.
template <class T, class U>
inline constexpr bool operator!=(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs != *rhs : true;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator<(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs < rhs : true;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator<(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs < *rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator<=(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs <= rhs : true;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator<=(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs <= *rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator>(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs > rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator>(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs > *rhs : true;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator>=(const optional<T> &lhs, const U &rhs) {
  return lhs.has_value() ? *lhs >= rhs : false;
}

/// @brief Compares the optional with a value.
///
/// @return If the optional has a value, it is compared with the other value
/// using `T`s relational operators. Otherwise, the optional is considered
/// less than the value.
template <class T, class U>
inline constexpr bool operator>=(const U &lhs, const optional<T> &rhs) {
  return rhs.has_value() ? lhs >= *rhs : true;
}

/// @brief Swaps two optionals.
///
/// Equivalent to `lhs.swap(rhs)`.
template <class T,
          enable_if_t<std::is_move_constructible<T>::value> * = nullptr>
void swap(optional<T> &lhs, optional<T> &rhs) {
  return lhs.swap(rhs);
}

/// @brief Creates an optional from `v`.
template <class T>
inline constexpr optional<decay_t<T>> make_optional(T &&v) {
  return optional<decay_t<T>>(std::forward<T>(v));
}

/// @brief Creates an optional from `args`.
template <class T, class... Args>
inline constexpr optional<T> make_optional(Args &&...args) {
  return optional<T>(in_place, std::forward<Args>(args)...);
}

/// @brief Creates an optional from `il` and `args`.
template <class T, class U, class... Args>
inline constexpr optional<T> make_optional(std::initializer_list<U> il,
                                           Args &&...args) {
  return optional<T>(in_place, il, std::forward<Args>(args)...);
}

#ifdef CARGO_CXX17
template <class T>
optional(T) -> optional<T>;
#endif

/// @}

namespace detail {
template <class Opt, class F,
          class Ret = decltype(invoke(std::declval<F>(), *std::declval<Opt>())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto optional_map_impl(Opt &&opt, F &&f) -> optional<Ret> {
  return opt.has_value() ? invoke(std::forward<F>(f), *std::forward<Opt>(opt))
                         : optional<Ret>(nullopt);
}

template <class Opt, class F,
          class Ret = decltype(invoke(std::declval<F>(), *std::declval<Opt>())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
auto optional_map_impl(Opt &&opt, F &&f) -> optional<monostate> {
  if (opt.has_value()) {
    invoke(std::forward<F>(f), *std::forward<Opt>(opt));
    return monostate{};
  }
  return nullopt;
}
}  // namespace detail

}  // namespace cargo

namespace std {
template <class T>
struct hash<cargo::optional<T>> {
  ::std::size_t operator()(const cargo::optional<T> &o) const {
    if (!o.has_value()) return 0;
    return std::hash<cargo::remove_const_t<T>>()(*o);
  }
};
}  // namespace std

#endif  // CARGO_OPTIONAL_H_INCLUDED
