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
// Derived from CC0-licensed `tl::expected` library which can be found
// at https://github.com/TartanLlama/expected.  See expected.LICENSE.txt.

/// @file
///
/// @brief An implementation of the proposed std::expected with extensions.

#ifndef CARGO_EXPECTED_H_INCLUDED
#define CARGO_EXPECTED_H_INCLUDED

#include <cargo/detail/expected.h>
#include <cargo/error.h>
#include <cargo/functional.h>

namespace cargo {
/// @addtogroup cargo
/// @{

template <class T, class E>
class expected;

/// @brief Wrapper for storing an unexpected type.
///
/// @tparam E The unexpected type.
template <class E>
class unexpected {
 public:
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  unexpected() = delete;

  /// @brief Copy construct from unexpected value.
  ///
  /// @param e The unexpected value.
  constexpr explicit unexpected(const E &e) : m_val(e) {}

  /// @brief Move construct from the unexpected value.
  ///
  /// @param e The unexpected value.
  constexpr explicit unexpected(E &&e) : m_val(std::move(e)) {}

  /// @brief Access the unexpected value.
  ///
  /// @return Returns a const reference to the unexpected value.
  constexpr const E &value() const & { return m_val; }

  /// @brief Access the unexpected value.
  ///
  /// @return Returns a reference to the unexpected value.
  CARGO_CXX14_CONSTEXPR E &value() & { return m_val; }

  /// @brief Access the unexpected value.
  ///
  /// @return Returns an r-value reference to the unexpected value.
  CARGO_CXX14_CONSTEXPR E &&value() && { return std::move(m_val); }

  /// @brief Access the unexpected value.
  ///
  /// @return Returns a const r-value reference to the unexpected value.
  constexpr const E &&value() const && { return std::move(m_val); }

 private:
  E m_val;
};

/// @brief Determine if an unexpected value is equal to another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if values are equal, false otherwise.
template <class E>
constexpr bool operator==(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() == rhs.value();
}

/// @brief Determine if an unexpected value is not equal to another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if values are not equal, false otherwise.
template <class E>
constexpr bool operator!=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() != rhs.value();
}

/// @brief Determine if an unexpected value is less than another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if `lhs` is less than `rhs`, false otherwise.
template <class E>
constexpr bool operator<(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() < rhs.value();
}

/// @brief Determine if an unexpected value is less than or equal to another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if `lhs` is less than or equal to` rhs`, false
/// otherwise.
template <class E>
constexpr bool operator<=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() <= rhs.value();
}

/// @brief Determine if an unexpected value is greater than another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if `lhs` is greater than `rhs`, false otherwise.
template <class E>
constexpr bool operator>(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() > rhs.value();
}

/// @brief Determine if an unexpected value is greater than or equal to another.
///
/// @tparam E The unexpected value type.
/// @param lhs Left hand unexpected to compare.
/// @param rhs Right hand unexpected to compare.
///
/// @return Returns true if `lhs` is greater than or equal to` rhs`, false
/// otherwise.
template <class E>
constexpr bool operator>=(const unexpected<E> &lhs, const unexpected<E> &rhs) {
  return lhs.value() >= rhs.value();
}

/// @brief Create an unexpected from `e`, deducing the return type.
///
/// @param[in] e Unexpected value to wrap.
///
/// The following two lines of code have the same semantics.
///
/// ```cpp
/// auto e1 = make_unexpected(42);
/// unexpected<int> e2(42);
/// ```
template <class E>
CARGO_NODISCARD unexpected<decay_t<E>> make_unexpected(E &&e) {
  return unexpected<decay_t<E>>(std::forward<E>(e));
}

/// @brief A tag type to tell expected to construct the unexpected value.
static constexpr unexpect_t unexpect{};

/// @brief A type which may contain an expected or unexpected value.
///
/// An `expected<T,E>` object is an object that contains the storage for
/// another object and manages the lifetime of this contained object `T`.
/// Alternatively it could contain the storage for another unexpected object
/// `E`. The contained object may not be initialized after the expected object
/// has been initialized, and may not be destroyed before the expected object
/// has been destroyed. The initialization state of the contained object is
/// tracked by the expected object.
///
/// Example usage:
///
/// ```cpp
/// enum class error_t {
///   failure,
///   permission_denied,
///   not_a_directory,
///   insufficient_storage,
/// };
///
/// auto open_file =
///     [](const char *filename) -> cargo::expected<FILE *, error_t> {
///   FILE *file = std::fopen(filename, "w");
///   if (nullptr == file) {
///     switch (errno) {
///       case EACCES:
///         return cargo::make_unexpected(error_t::permission_denied);
///       case ENOTDIR:
///         return cargo::make_unexpected(error_t::not_a_directory);
///       default:
///         return cargo::make_unexpected(error_t::failure);
///     }
///   }
///   return file;
/// };
///
/// auto write_message = [](FILE *file) -> cargo::expected<FILE *, error_t> {
///   const char *message = "hello, expected!\n";
///   std::fwrite(message, 1, std::strlen(message), file);
///   if (errno == ENOMEM) {
///     return cargo::make_unexpected(error_t::insufficient_storage);
///   }
///   return file;
/// };
///
/// auto close_file = [](FILE *file) {
///   std::fclose(file);
/// };
///
/// auto result = open_file("hello_expected.txt")
///                   .and_then(write_message)
///                   .map(close_file);
/// if (!result) {
///   switch (result.error()) {
///     case error_t::failure:
///       printf("failed to open file\n");
///       break;
///     case error_t::permission_denied:
///       printf("permission denied\n");
///       break;
///     case error_t::not_a_directory:
///       printf("not a directory\n");
///       break;
///     case error_t::insufficient_storage:
///       printf("insufficient storage\n");
///       break;
///   }
/// }
/// ```
///
/// @tparam T Type of the expected value.
/// @tparam E Type of the unexpected value.
template <class T, class E>
class expected
    : private detail::expected_move_assign_base<wrap_reference_t<T>, E>,
      private detail::delete_ctor_base<
          detail::is_copy_constructible_or_void<wrap_reference_t<T>>::value &&
              detail::is_copy_constructible_or_void<E>::value,
          std::is_move_constructible<wrap_reference_t<T>>::value &&
              std::is_move_constructible<E>::value>,
      private detail::delete_assign_base<
          std::is_copy_assignable<wrap_reference_t<T>>::value &&
              std::is_copy_assignable<E>::value,
          std::is_move_constructible<wrap_reference_t<T>>::value &&
              std::is_move_constructible<E>::value>,
      private detail::expected_default_ctor_base<wrap_reference_t<T>, E> {
  static_assert(!std::is_same<T, std::remove_cv<in_place_t>>::value,
                "T must not be in_place_t");
  static_assert(!std::is_same<T, std::remove_cv<unexpect_t>>::value,
                "T must not be unexpect_t");
  static_assert(!std::is_same<T, std::remove_cv<unexpected<E>>>::value,
                "T must not be unexpected<E>");
  static_assert(!std::is_reference<E>::value, "E must not be a reference");

  using impl_base = detail::expected_move_assign_base<wrap_reference_t<T>, E>;
  using ctor_base = detail::expected_default_ctor_base<wrap_reference_t<T>, E>;
  using storage_type = wrap_reference_t<T>;

 public:
  using value_type = T;
  using pointer = remove_reference_t<T> *;
  using const_pointer = const remove_reference_t<T> *;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  /// @brief Default constructor.
  constexpr expected() = default;

  /// @brief Default copy constructor.
  ///
  /// @param rhs Object to copy.
  constexpr expected(const expected &rhs) = default;

  /// @brief Default move constructor.
  ///
  /// @param rhs Object to move.
  constexpr expected(expected &&rhs) = default;

  /// @brief In place value constructor.
  ///
  /// @tparam Args Types of `T`'s constructor arguments.
  /// @param args Forwarded constructor arguments for `T`.
  template <
      class... Args,
      enable_if_t<std::is_constructible<T, Args &&...>::value> * = nullptr>
  constexpr expected(in_place_t, Args &&... args)
      : impl_base(in_place, std::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief In place value initializer list value constructor.
  ///
  /// @tparam U Type of initializer list elements.
  /// @tparam Args Types of `T`'s constructor arguments.
  /// @param il Forwarded constructor initializer list for `T`.
  /// @param args Forwarded constructor arguments for `T`.
  template <class U, class... Args,
            enable_if_t<std::is_constructible<T, std::initializer_list<U> &,
                                              Args &&...>::value> * = nullptr>
  constexpr expected(in_place_t, std::initializer_list<U> il, Args &&... args)
      : impl_base(in_place, il, std::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief Unexpected copy constructor.
  ///
  /// @tparam G Type of the unexpected value.
  /// @param e Unexpected value to copy.
  template <class G = E,
            enable_if_t<std::is_constructible<E, const G &>::value> * = nullptr,
            enable_if_t<!std::is_convertible<const G &, E>::value> * = nullptr>
  explicit constexpr expected(const unexpected<G> &e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief Unexpected copy constructor.
  ///
  /// @tparam G Type of the unexpected value.
  /// @param e Unexpected value to copy.
  template <class G = E,
            enable_if_t<std::is_constructible<E, const G &>::value> * = nullptr,
            enable_if_t<std::is_convertible<const G &, E>::value> * = nullptr>
  constexpr expected(unexpected<G> const &e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief Unexpected move constructor.
  ///
  /// @tparam G Type of the unexpected value.
  /// @param e Unexpected value to copy.
  template <class G = E,
            enable_if_t<std::is_constructible<E, G &&>::value> * = nullptr,
            enable_if_t<!std::is_convertible<G &&, E>::value> * = nullptr>
  explicit constexpr expected(unexpected<G> &&e) noexcept(
      std::is_nothrow_constructible<E, G &&>::value)
      : impl_base(unexpect, std::move(e.value())),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief Unexpected move constructor.
  ///
  /// @tparam G Type of the unexpected value.
  /// @param e Unexpected value to copy.
  template <class G = E,
            enable_if_t<std::is_constructible<E, G &&>::value> * = nullptr,
            enable_if_t<std::is_convertible<G &&, E>::value> * = nullptr>
  constexpr expected(unexpected<G> &&e) noexcept(
      std::is_nothrow_constructible<E, G &&>::value)
      : impl_base(unexpect, std::move(e.value())),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief In place unexpected constructor.
  ///
  /// @tparam Args Types of `E`'s constructor arguments.
  /// @param args Forwarded constructor arguments for `E`.
  template <
      class... Args,
      enable_if_t<std::is_constructible<E, Args &&...>::value> * = nullptr>
  constexpr explicit expected(unexpect_t, Args &&... args)
      : impl_base(unexpect, std::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief In place value initializer list unexpected constructor.
  ///
  /// @tparam U Type of initializer list elements.
  /// @tparam Args Types of `E`'s constructor arguments.
  /// @param il Forwarded constructor initializer list for `E`.
  /// @param args Forwarded constructor arguments for `E`.
  template <class U, class... Args,
            enable_if_t<std::is_constructible<E, std::initializer_list<U> &,
                                              Args &&...>::value> * = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il,
                              Args &&... args)
      : impl_base(unexpect, il, std::forward<Args>(args)...),
        ctor_base(detail::default_constructor_tag{}) {}

  /// @brief Copy constructor.
  ///
  /// @tparam U Type of `rhs`'s expected value.
  /// @tparam G Type of `rhs'`s unexpected value.
  /// @param rhs Expected object to copy.
  template <
      class U, class G,
      enable_if_t<!(std::is_convertible<U const &, T>::value &&
                    std::is_convertible<G const &, E>::value)> * = nullptr,
      detail::expected_enable_from_other<T, E, U, G, const U &, const G &> * =
          nullptr>
  explicit CARGO_CXX14_CONSTEXPR expected(const expected<U, G> &rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  /// @brief Copy constructor.
  ///
  /// @tparam U Type of `rhs`'s expected value.
  /// @tparam G Type of `rhs'`s unexpected value.
  /// @param rhs Expected object to copy.
  template <class U, class G,
            enable_if_t<(std::is_convertible<U const &, T>::value &&
                         std::is_convertible<G const &, E>::value)> * = nullptr,
            detail::expected_enable_from_other<T, E, U, G, const U &, const G &>
                * = nullptr>
  CARGO_CXX14_CONSTEXPR expected(const expected<U, G> &rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  /// @brief Move constructor.
  ///
  /// @tparam U Type of `rhs`'s expected value.
  /// @tparam G Type of `rhs'`s unexpected value.
  /// @param rhs Expected object to move.
  template <
      class U, class G,
      enable_if_t<!(std::is_convertible<U &&, T>::value &&
                    std::is_convertible<G &&, E>::value)> * = nullptr,
      detail::expected_enable_from_other<T, E, U, G, U &&, G &&> * = nullptr>
  explicit CARGO_CXX14_CONSTEXPR expected(expected<U, G> &&rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    } else {
      this->construct_error(std::move(rhs.error()));
    }
  }

  /// @brief Move constructor.
  ///
  /// @tparam U Type of `rhs`'s expected value.
  /// @tparam G Type of `rhs'`s unexpected value.
  /// @param rhs Expected object to move.
  template <
      class U, class G,
      enable_if_t<(std::is_convertible<U &&, T>::value &&
                   std::is_convertible<G &&, E>::value)> * = nullptr,
      detail::expected_enable_from_other<T, E, U, G, U &&, G &&> * = nullptr>
  CARGO_CXX14_CONSTEXPR expected(expected<U, G> &&rhs)
      : ctor_base(detail::default_constructor_tag{}) {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    } else {
      this->construct_error(std::move(rhs.error()));
    }
  }

  /// @brief Expected value move constructor.
  ///
  /// @tparam U Type of expected value.
  /// @param v Expected value to move.
  template <class U = T,
            enable_if_t<!std::is_convertible<U &&, T>::value> * = nullptr,
            detail::expected_enable_forward_value<T, E, U> * = nullptr>
  explicit CARGO_CXX14_CONSTEXPR expected(U &&v)
      : expected(in_place, std::forward<U>(v)) {}

  /// @brief Expected value move constructor.
  ///
  /// @tparam U Type of expected value.
  /// @param v Expected value to move.
  template <class U = T,
            enable_if_t<std::is_convertible<U &&, T>::value> * = nullptr,
            detail::expected_enable_forward_value<T, E, U> * = nullptr>
  CARGO_CXX14_CONSTEXPR expected(U &&v)
      : expected(in_place, std::forward<U>(v)) {}

  /// @brief Default copy assignment operator.
  ///
  /// @param rhs Object to copy.
  expected &operator=(const expected &rhs) = default;

  /// @brief Default move assignment operator.
  ///
  /// @param rhs Object to move.
  expected &operator=(expected &&rhs) = default;

  /// @brief Expected value move assignment operator.
  ///
  /// @tparam U Type of expected value.
  /// @param v Expected value to move.
  template <
      class U = T, class G = T,
      enable_if_t<std::is_nothrow_constructible<T, U &&>::value> * = nullptr,
      enable_if_t<!std::is_void<G>::value> * = nullptr,
      enable_if_t<(
          !std::is_same<expected<T, E>, decay_t<U>>::value &&
          !conjunction<std::is_scalar<T>, std::is_same<T, decay_t<U>>>::value &&
          std::is_constructible<T, U>::value &&
          std::is_assignable<G &, U>::value &&
          std::is_nothrow_move_constructible<E>::value)> * = nullptr>
  expected &operator=(U &&v) {
    if (has_value()) {
      val() = std::forward<U>(v);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(std::forward<U>(v));
      this->m_has_val = true;
    }
    return *this;
  }

  /// @brief Expected value move assignment operator.
  ///
  /// @tparam U Type of expected value.
  /// @param v Expected value to move.
  template <
      class U = T, class G = T,
      enable_if_t<!std::is_nothrow_constructible<T, U &&>::value> * = nullptr,
      enable_if_t<!std::is_void<U>::value> * = nullptr,
      enable_if_t<(
          !std::is_same<expected<T, E>, decay_t<U>>::value &&
          !conjunction<std::is_scalar<T>, std::is_same<T, decay_t<U>>>::value &&
          std::is_constructible<T, U>::value &&
          std::is_assignable<G &, U>::value &&
          std::is_nothrow_move_constructible<E>::value)> * = nullptr>
  expected &operator=(U &&v) {
    if (has_value()) {
      val() = std::forward<U>(v);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(std::forward<U>(v));
      this->m_has_val = true;
    }
    return *this;
  }

  /// @brief Unexpected value copy assignment operator.
  ///
  /// @tparam G Type of unexpected value.
  /// @param rhs Expected value to copy.
  template <class G = E,
            enable_if_t<std::is_nothrow_copy_constructible<G>::value &&
                        std::is_assignable<G &, G>::value> * = nullptr>
  expected &operator=(const unexpected<G> &rhs) {
    if (!has_value()) {
      err() = rhs;
    } else {
      val().~storage_type();
      new (errptr()) unexpected<E>(rhs);
      this->m_has_val = false;
    }
    return *this;
  }

  /// @brief Unexpected value move assignment operator.
  ///
  /// @tparam G Type of unexpected value.
  /// @param rhs Expected value to move.
  template <class G = E,
            enable_if_t<std::is_nothrow_move_constructible<G>::value &&
                        std::is_move_assignable<G>::value> * = nullptr>
  expected &operator=(unexpected<G> &&rhs) noexcept {
    if (!has_value()) {
      err() = std::move(rhs);
    } else {
      val().~storage_type();
      new (errptr()) unexpected<E>(std::move(rhs));
      this->m_has_val = false;
    }
    return *this;
  }

#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54) && \
    !defined(CARGO_GCC55)

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) & {
    return and_then_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) && {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  constexpr auto and_then(F &&f) const & {
    return and_then_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  constexpr auto and_then(F &&f) const && {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }
#endif

#else

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) & -> decltype(
      and_then_impl(*this, std::forward<F>(f))) {
    return and_then_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto and_then(F &&f) && -> decltype(
      and_then_impl(std::move(*this), std::forward<F>(f))) {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  constexpr auto and_then(F &&f) const & -> decltype(
      and_then_impl(*this, std::forward<F>(f))) {
    return and_then_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable returning an expected on the stored object, if
  /// there is one.
  ///
  /// @note Requires that `invoke(std::forward<F>(f), value())` returns a
  /// `expected<U,E>` for some type `U`.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the stored object.
  ///
  /// @return Returns the result of the callable.
  /// @retval If `has_value()` is true, the value returned from
  /// `invoke(std::forward<F>(f), value())` is returned.
  /// @retval If `has_value()` is false, the returned value is empty.
  template <class F>
  constexpr auto and_then(F &&f) const && -> decltype(
      and_then_impl(std::move(*this), std::forward<F>(f))) {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }
#endif

#endif

#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54) && \
    !defined(CARGO_GCC55)

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) & {
    return expected_map_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) && {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  constexpr auto map(F &&f) const & {
    return expected_map_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is false, the result is `*this`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  template <class F>
  constexpr auto map(F &&f) const && {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }
#endif

#else

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) & -> decltype(
      expected_map_impl(std::declval<expected &>(), std::declval<F &&>())) {
    return expected_map_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map(F &&f) && -> decltype(
      expected_map_impl(std::declval<expected &>(), std::declval<F &&>())) {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  /// @retval If `has_value()` is false, the result is `*this`.
  template <class F>
  constexpr auto map(F &&f) const & -> decltype(expected_map_impl(
      std::declval<const expected &>(), std::declval<F &&>())) {
    return expected_map_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable on the stored object, if there is one.
  ///
  /// @tparam F Type of the callable object.
  /// @param[in] f Callable to invoke on the value.
  ///
  /// @return Returns the result of the callable wrapped in a
  /// `expected<U,E>` where `U` is the type returned from the callable.
  /// @retval If `U` is not `void`, returns an `expected<U,E>`.
  /// @retval If `U` is `void`, returns an `expected<monostate,E>`.
  /// @retval If `has_value()` is false, the result is `*this`.
  /// @retval If `has_value()` is true, an `expected<U,E>` is constructed from
  /// the return value of `invoke(std::forward<F>(f), value())` and
  /// returned.
  template <class F>
  constexpr auto map(F &&f) const && -> decltype(expected_map_impl(
      std::declval<const expected &&>(), std::declval<F &&>())) {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }
#endif

#endif

#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54) && \
    !defined(CARGO_GCC55)

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map_error(F &&f) & {
    return map_error_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map_error(F &&f) && {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  constexpr auto map_error(F &&f) const & {
    return map_error_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  constexpr auto map_error(F &&f) const && {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }

#else

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map_error(F &&f) & -> decltype(
      map_error_impl(std::declval<expected &>(), std::declval<F &&>())) {
    return map_error_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  CARGO_CXX14_CONSTEXPR auto map_error(F &&f) && -> decltype(
      map_error_impl(std::declval<expected &&>(), std::declval<F &&>())) {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  constexpr auto map_error(F &&f) const & -> decltype(
      map_error_impl(std::declval<const expected &>(), std::declval<F &&>())) {
    return map_error_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable on the stored unexpected object, if there is
  /// one.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke on the unexpected value.
  ///
  /// @return Returns an expected, where `U` is the result type of
  /// `invoke(std::forward<F>(f), value())`.
  /// @retval If `U` is `void`, return an `expected<T,monostate>`.
  /// @retval If `U` is not `void`, returns `expected<T,U>`.
  /// @retval If `has_value()` is true, return `*this`.
  /// @retval If `has_value()` is false, return a newly constructed
  /// `expected<T,U>` from
  /// `make_unexpected(invoke(std::forward<F>(f), value()))`.
  template <class F>
  constexpr auto map_error(F &&f) const && -> decltype(
      map_error_impl(std::declval<const expected &&>(), std::declval<F &&>())) {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }
#endif

#endif

  /// @brief Invoke a callable when in an unexpected state.
  ///
  /// @note Requires that callable `F` is invokable with type `E` and
  /// `invoke_result_t<F>` must be convertible to `expected<T,E>`.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke when in the unexpected state.
  ///
  /// @return Returns an expected, invoking callable on the unexpected object.
  /// @retval If `has_value()` is true, returns `*this`.
  /// @retval If `has_value()` is false and `f` returns `void`, invoke `f` and
  /// return `expected<T,monostate>`.
  /// @retval If `has_value()` is false and `f` does not return `void`, return
  /// `std::forward<F>(f)(error())`.
  template <class F>
  expected CARGO_CXX14_CONSTEXPR or_else(F &&f) & {
    return or_else_impl(*this, std::forward<F>(f));
  }

  /// @brief Invoke a callable when in an unexpected state.
  ///
  /// @note Requires that callable `F` is invokable with type `E` and
  /// `invoke_result_t<F>` must be convertible to `expected<T,E>`.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke when in the unexpected state.
  ///
  /// @return Returns an expected, invoking callable on the unexpected object.
  /// @retval If `has_value()` is true, returns `*this`.
  /// @retval If `has_value()` is false and `f` returns `void`, invoke `f` and
  /// return `expected<T,monostate>`.
  /// @retval If `has_value()` is false and `f` does not return `void`, return
  /// `std::forward<F>(f)(error())`.
  template <class F>
  expected CARGO_CXX14_CONSTEXPR or_else(F &&f) && {
    return or_else_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Invoke a callable when in an unexpected state.
  ///
  /// @note Requires that callable `F` is invokable with type `E` and
  /// `invoke_result_t<F>` must be convertible to `expected<T,E>`.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke when in the unexpected state.
  ///
  /// @return Returns an expected, invoking callable on the unexpected object.
  /// @retval If `has_value()` is true, returns `*this`.
  /// @retval If `has_value()` is false and `f` returns `void`, invoke `f` and
  /// return `expected<T,monostate>`.
  /// @retval If `has_value()` is false and `f` does not return `void`, return
  /// `std::forward<F>(f)(error())`.
  template <class F>
  expected constexpr or_else(F &&f) const & {
    return or_else_impl(*this, std::forward<F>(f));
  }

#ifndef CARGO_NO_CONSTRR
  /// @brief Invoke a callable when in an unexpected state.
  ///
  /// @note Requires that callable `F` is invokable with type `E` and
  /// `invoke_result_t<F>` must be convertible to `expected<T,E>`.
  ///
  /// @tparam F Type of the callable object.
  /// @param f Callable to invoke when in the unexpected state.
  ///
  /// @return Returns an expected, invoking callable on the unexpected object.
  /// @retval If `has_value()` is true, returns `*this`.
  /// @retval If `has_value()` is false and `f` returns `void`, invoke `f` and
  /// return `expected<T,monostate>`.
  /// @retval If `has_value()` is false and `f` does not return `void`, return
  /// `std::forward<F>(f)(error())`.
  template <class F>
  expected constexpr or_else(F &&f) const && {
    return or_else_impl(std::move(*this), std::forward<F>(f));
  }
#endif

  /// @brief Emplace construct an expected value.
  ///
  /// @tparam Args Type of `T`'s constructor arguments.
  /// @param args Forwarded constructor arguments for `T`.
  template <class... Args,
            enable_if_t<std::is_nothrow_constructible<T, Args &&...>::value> * =
                nullptr>
  void emplace(Args &&... args) {
    if (has_value()) {
      val() = T(std::forward<Args>(args)...);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(std::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

  /// @brief Emplace construct an expected value.
  ///
  /// @tparam Args Type of `T`'s constructor arguments.
  /// @param args Forwarded constructor arguments for `T`.
  template <class... Args,
            enable_if_t<!std::is_nothrow_constructible<T, Args &&...>::value>
                * = nullptr>
  void emplace(Args &&... args) {
    if (has_value()) {
      val() = T(std::forward<Args>(args)...);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(std::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

  /// @brief Emplace initializer list construct an expected value.
  ///
  /// @tparam U Type of initializer list elements.
  /// @tparam Args Types of `E`'s constructor arguments.
  /// @param il Forwarded constructor initializer list for `E`.
  /// @param args Forwarded constructor arguments for `E`.
  template <class U, class... Args,
            enable_if_t<std::is_nothrow_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  void emplace(std::initializer_list<U> il, Args &&... args) {
    if (has_value()) {
      T t(il, std::forward<Args>(args)...);
      val() = std::move(t);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(il, std::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

  /// @brief Emplace initializer list construct an expected value.
  ///
  /// @tparam U Type of initializer list elements.
  /// @tparam Args Types of `E`'s constructor arguments.
  /// @param il Forwarded constructor initializer list for `E`.
  /// @param args Forwarded constructor arguments for `E`.
  template <class U, class... Args,
            enable_if_t<!std::is_nothrow_constructible<
                T, std::initializer_list<U> &, Args &&...>::value> * = nullptr>
  void emplace(std::initializer_list<U> il, Args &&... args) {
    if (has_value()) {
      T t(il, std::forward<Args>(args)...);
      val() = std::move(t);
    } else {
      err().~unexpected<E>();
      new (valptr()) T(il, std::forward<Args>(args)...);
      this->m_has_val = true;
    }
  }

  /// @brief Swap this expected with another.
  ///
  /// @param rhs The expected to swap with this.
  void swap(expected &rhs) noexcept(
      std::is_nothrow_move_constructible<T>::value &&noexcept(
          swap(std::declval<T &>(), std::declval<T &>())) &&
      std::is_nothrow_move_constructible<E>::value &&noexcept(
          swap(std::declval<E &>(), std::declval<E &>()))) {
    if (has_value() && rhs.has_value()) {
      using std::swap;
      swap(val(), rhs.val());
    } else if (!has_value() && rhs.has_value()) {
      using std::swap;
      swap(err(), rhs.err());
    } else if (has_value()) {
      auto temp = std::move(rhs.err());
      new (rhs.valptr()) T(val());
      new (errptr()) unexpected_type(std::move(temp));
      std::swap(this->m_has_val, rhs.m_has_val);
    } else {
      auto temp = std::move(this->err());
      new (valptr()) T(rhs.val());
      new (errptr()) unexpected_type(std::move(temp));
      std::swap(this->m_has_val, rhs.m_has_val);
    }
  }

  /// @brief Access expected value members.
  ///
  /// @note Requires a value is stored.
  ///
  /// @return Returns a pointer to the stored value.
  constexpr const_pointer operator->() const { return valptr(); }

  CARGO_CXX14_CONSTEXPR pointer operator->() { return valptr(); }

  /// @brief Access the expected value.
  ///
  /// @note Requires a value is stored.
  ///
  /// @return Returns a const reference to the stored value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  constexpr const U &operator*() const & {
    return val();
  }

  /// @brief Access the expected value.
  ///
  /// @note Requires a value is stored.
  ///
  /// @return Returns a reference to the stored value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR U &operator*() & {
    return val();
  }

  /// @brief Access the expected value.
  ///
  /// @note Requires a value is stored.
  ///
  /// @return Returns a const r-value reference to the stored value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  constexpr const U &&operator*() const && {
    return std::move(val());
  }

  /// @brief Access the expected value.
  ///
  /// @note Requires a value is stored.
  ///
  /// @return Returns an r-value reference to the stored value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR U &&operator*() && {
    return std::move(val());
  }

  /// @brief Determine whether or not the optional has a value.
  ///
  /// @return Returns true is the expected has a value, false otherwise.
  constexpr bool has_value() const noexcept { return this->m_has_val; }

  constexpr explicit operator bool() const noexcept { return this->m_has_val; }

  /// @brief Access the expected value.
  ///
  /// @return Returns a const reference to the contained value if there is one,
  /// assert otherwise.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR const U &value() const & {
    CARGO_ASSERT(has_value(), "bad expected access");
    return val();
  }

  /// @brief Access the expected value.
  ///
  /// @return Returns a reference to the contained value if there is one,
  /// assert otherwise.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR U &value() & {
    CARGO_ASSERT(has_value(), "bad expected access");
    return val();
  }

  /// @brief Access the expected value.
  ///
  /// @return Returns a const r-value reference to the contained value if there
  /// is one, assert otherwise.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR const U &&value() const && {
    CARGO_ASSERT(has_value(), "bad expected access");
    return std::move(val());
  }

  /// @brief Access the expected value.
  ///
  /// @return Returns an r-value reference to the contained value if there is
  /// one, assert otherwise.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  CARGO_CXX14_CONSTEXPR U &&value() && {
    CARGO_ASSERT(has_value(), "bad expected access");
    return std::move(val());
  }

  /// @brief Access the unexpected value.
  ///
  /// @note Requires there is an unexpected value.
  ///
  /// @return Returns a const reference to the unexpected value.
  constexpr const E &error() const & { return err().value(); }

  /// @brief Access the unexpected value.
  ///
  /// @note Requires there is an unexpected value.
  ///
  /// @return Returns a reference to the unexpected value.
  CARGO_CXX14_CONSTEXPR E &error() & { return err().value(); }

  /// @brief Access the unexpected value.
  ///
  /// @note Requires there is an unexpected value.
  ///
  /// @return Returns a const r-value reference to the unexpected value.
  constexpr const E &&error() const && { return std::move(err().value()); }

  /// @brief Access the unexpected value.
  ///
  /// @note Requires there is an unexpected value.
  ///
  /// @return Returns an r-value reference to the unexpected value.
  CARGO_CXX14_CONSTEXPR E &&error() && { return std::move(err().value()); }

  /// @brief Access the expected value or the provided default value.
  ///
  /// @param v The default value to fallback to.
  ///
  /// @returns Returns a const reference to the stored value if there is one,
  /// otherwise returns the default value.
  template <class U>
  constexpr T value_or(U &&v) const & {
    static_assert(std::is_copy_constructible<T>::value &&
                      std::is_convertible<U &&, T>::value,
                  "T must be copy-constructible and convertible to from U&&");
    return bool(*this) ? **this : static_cast<T>(std::forward<U>(v));
  }

  /// @brief Access the expected value or the provided default value.
  ///
  /// @param v The default value to fallback to.
  ///
  /// @returns Returns an r-value reference to the stored value if there is
  /// one, otherwise returns the default value.
  template <class U>
  CARGO_CXX14_CONSTEXPR T value_or(U &&v) && {
    static_assert(std::is_move_constructible<T>::value &&
                      std::is_convertible<U &&, T>::value,
                  "T must be move-constructible and convertible to from U&&");
    return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(v));
  }

 private:
  /// @brief Get a pointer to expected value storage.
  ///
  /// @return Returns a pointer to the value storage.
  remove_reference_t<T> *valptr() {
    // If T is a reference we need to unwrap it from std::reference_wrapper by
    // assigning if to a reference.
    remove_reference_t<T> &ref = this->m_val;
    return std::addressof(ref);
  }

  /// @brief Get a pointer to unexpected value storage.
  ///
  /// @return Returns a pointer to the unexpected value storage.
  unexpected<E> *errptr() { return std::addressof(this->m_unexpect); }

  /// @brief Get the expected value.
  ///
  /// @tparam U Type of the expected value.
  ///
  /// @return Returns a reference to the expected value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  U &val() {
    return this->m_val;
  }

  /// @brief Get the unexpected value.
  ///
  /// @return Returns a reference to the unexpected value.
  unexpected<E> &err() { return this->m_unexpect; }

  /// @brief Get the expected value.
  ///
  /// @tparam U Type of the expected value.
  ///
  /// @return Returns a const reference to the expected value.
  template <class U = T, enable_if_t<!std::is_void<U>::value> * = nullptr>
  const U &val() const {
    return this->m_val;
  }

  /// @brief Get the unexpected value.
  ///
  /// @return Returns a const reference to the unexpected value.
  const unexpected<E> &err() const { return this->m_unexpect; }
};

/// @}

namespace detail {
template <class Exp>
using exp_t = typename decay_t<Exp>::value_type;

template <class Exp>
using err_t = typename decay_t<Exp>::error_type;

template <class Exp, class Ret>
using ret_t = expected<Ret, err_t<Exp>>;

#ifdef CARGO_CXX14

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<!std::is_void<exp_t<Exp>>::value> * = nullptr>
constexpr auto and_then_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value()
             ? cargo::invoke(std::forward<F>(f), *std::forward<Exp>(exp))
             : Ret(unexpect, exp.error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<std::is_void<exp_t<Exp>>::value> * = nullptr>
constexpr auto and_then_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value() ? cargo::invoke(std::forward<F>(f))
                         : Ret(unexpect, exp.error());
}

#else

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<!std::is_void<exp_t<Exp>>::value> * = nullptr>
auto and_then_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value()
             ? cargo::invoke(std::forward<F>(f), *std::forward<Exp>(exp))
             : Ret(unexpect, exp.error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<std::is_void<exp_t<Exp>>::value> * = nullptr>
constexpr auto and_then_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value() ? cargo::invoke(std::forward<F>(f))
                         : Ret(unexpect, exp.error());
}

#endif

#ifdef CARGO_CXX14

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto expected_map_impl(Exp &&exp, F &&f) {
  using result = ret_t<Exp, decay_t<Ret>>;
  return exp.has_value() ? result(cargo::invoke(std::forward<F>(f),
                                                *std::forward<Exp>(exp)))
                         : result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
auto expected_map_impl(Exp &&exp, F &&f) {
  using result = expected<void, err_t<Exp>>;
  if (exp.has_value()) {
    cargo::invoke(std::forward<F>(f), *std::forward<Exp>(exp));
    return result();
  }

  return result(unexpect, std::forward<Exp>(exp).error());
}

#else

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto expected_map_impl(Exp &&exp, F &&f) -> ret_t<Exp, decay_t<Ret>> {
  using result = ret_t<Exp, decay_t<Ret>>;
  return exp.has_value() ? result(cargo::invoke(std::forward<F>(f),
                                                *std::forward<Exp>(exp)))
                         : result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             *std::declval<Exp>())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
auto expected_map_impl(Exp &&exp, F &&f) -> expected<void, err_t<Exp>> {
  if (exp.has_value()) {
    cargo::invoke(std::forward<F>(f), *std::forward<Exp>(exp));
    return {};
  }

  return unexpected<err_t<Exp>>(std::forward<Exp>(exp).error());
}

#endif

#if defined(CARGO_CXX14) && !defined(CARGO_GCC49) && !defined(CARGO_GCC54) && \
    !defined(CARGO_GCC55)

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, decay_t<Ret>>;
  return exp.has_value()
             ? result(*std::forward<Exp>(exp))
             : result(unexpect, cargo::invoke(std::forward<F>(f),
                                              std::forward<Exp>(exp).error()));
}
template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result(*std::forward<Exp>(exp));
  }
  cargo::invoke(std::forward<F>(f), std::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}

#else

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto map_error_impl(Exp &&exp, F &&f)
    -> expected<exp_t<Exp>, decay_t<Ret>> {
  using result = ret_t<Exp, decay_t<Ret>>;
  return exp.has_value()
             ? result(*std::forward<Exp>(exp))
             : result(unexpect, cargo::invoke(std::forward<F>(f),
                                              std::forward<Exp>(exp).error()));
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
auto map_error_impl(Exp &&exp, F &&f) -> expected<exp_t<Exp>, monostate> {
  using result = expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result(*std::forward<Exp>(exp));
  }
  cargo::invoke(std::forward<F>(f), std::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}

#endif

#ifdef CARGO_CXX14

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
constexpr auto or_else_impl(Exp &&exp, F &&f) {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value() ? std::forward<Exp>(exp)
                         : cargo::invoke(std::forward<F>(f),
                                         std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
decay_t<Exp> or_else_impl(Exp &&exp, F &&f) {
  return exp.has_value() ? std::forward<Exp>(exp)
                         : (cargo::invoke(std::forward<F>(f),
                                          std::forward<Exp>(exp).error()),
                            std::forward<Exp>(exp));
}

#else

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<!std::is_void<Ret>::value> * = nullptr>
auto or_else_impl(Exp &&exp, F &&f) -> Ret {
  static_assert(detail::is_expected<Ret>::value, "F must return an expected");
  return exp.has_value() ? std::forward<Exp>(exp)
                         : cargo::invoke(std::forward<F>(f),
                                         std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(cargo::invoke(std::declval<F>(),
                                             std::declval<Exp>().error())),
          enable_if_t<std::is_void<Ret>::value> * = nullptr>
decay_t<Exp> or_else_impl(Exp &&exp, F &&f) {
  return exp.has_value() ? std::forward<Exp>(exp)
                         : (cargo::invoke(std::forward<F>(f),
                                          std::forward<Exp>(exp).error()),
                            std::forward<Exp>(exp));
}

#endif
}  // namespace detail

/// @addtogroup cargo
/// @{

/// @brief Determine if an expected is equal to another.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of `rhs` expected value.
/// @tparam F Type of `rhs` unexpected value.
/// @param lhs Left hand side expected to compare.
/// @param rhs Right hand side expected to compare.
///
/// @return Returns true if the objects are equal, false otherwise.
template <class T, class E, class U, class F>
constexpr bool operator==(const expected<T, E> &lhs,
                          const expected<U, F> &rhs) {
  return (lhs.has_value() != rhs.has_value())
             ? false
             : (!lhs.has_value() ? lhs.error() == rhs.error() : *lhs == *rhs);
}

/// @brief Determine if an expected is not equal to another.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of `rhs` expected value.
/// @tparam F Type of `rhs` unexpected value.
/// @param lhs Left hand side expected to compare.
/// @param rhs Right hand side expected to compare.
///
/// @return Returns true if the objects are not equal, false otherwise.
template <class T, class E, class U, class F>
constexpr bool operator!=(const expected<T, E> &lhs,
                          const expected<U, F> &rhs) {
  return (lhs.has_value() != rhs.has_value())
             ? true
             : (!lhs.has_value() ? lhs.error() != rhs.error() : *lhs != *rhs);
}

/// @brief Determine if an expected is equal to a value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of value to compare.
/// @param x The expected to compare.
/// @param v The value to compare.
///
/// @return Returns true if the values are equal, false otherwise.
template <class T, class E, class U>
constexpr bool operator==(const expected<T, E> &x, const U &v) {
  return x.has_value() ? *x == v : false;
}

/// @brief Determine if an expected is equal to a value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of value to compare.
/// @param v The value to compare.
/// @param x The expected to compare.
///
/// @return Returns true if the values are equal, false otherwise.
template <class T, class E, class U>
constexpr bool operator==(const U &v, const expected<T, E> &x) {
  return x.has_value() ? *x == v : false;
}

/// @brief Determine if an expected is not equal to a value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of value to compare.
/// @param x The expected to compare.
/// @param v The value to compare.
///
/// @return Returns true if the values are not equal, false otherwise.
template <class T, class E, class U>
constexpr bool operator!=(const expected<T, E> &x, const U &v) {
  return x.has_value() ? *x != v : true;
}

/// @brief Determine if an expected is not equal to a value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @tparam U Type of value to compare.
/// @param v The value to compare.
/// @param x The expected to compare.
///
/// @return Returns true if the values are not equal, false otherwise.
template <class T, class E, class U>
constexpr bool operator!=(const U &v, const expected<T, E> &x) {
  return x.has_value() ? *x != v : true;
}

/// @brief Determine if an expected is equal to an unexpected value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @param x The expected to compare.
/// @param e The value to compare.
///
/// @return Returns true if the unexpected values are equal, false otherwise.
template <class T, class E>
constexpr bool operator==(const expected<T, E> &x, const unexpected<E> &e) {
  return x.has_value() ? false : x.error() == e.value();
}

/// @brief Determine if an expected is equal to an unexpected value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @param e The value to compare.
/// @param x The expected to compare.
///
/// @return Returns true if the unexpected values are equal, false otherwise.
template <class T, class E>
constexpr bool operator==(const unexpected<E> &e, const expected<T, E> &x) {
  return x.has_value() ? false : x.error() == e.value();
}

/// @brief Determine if an expected is not equal to an unexpected value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @param x The expected to compare.
/// @param e The value to compare.
///
/// @return Returns true if the unexpected values are not equal, false
/// otherwise.
template <class T, class E>
constexpr bool operator!=(const expected<T, E> &x, const unexpected<E> &e) {
  return x.has_value() ? true : x.error() != e.value();
}

/// @brief Determine if an expected is not equal to an unexpected value.
///
/// @tparam T Type of `lhs` expected value.
/// @tparam E Type of `lhs` unexpected value.
/// @param e The value to compare.
/// @param x The expected to compare.
///
/// @return Returns true if the unexpected values are not equal, false
/// otherwise.
template <class T, class E>
constexpr bool operator!=(const unexpected<E> &e, const expected<T, E> &x) {
  return x.has_value() ? true : x.error() != e.value();
}

/// @brief Swap an expected with another.
///
/// @tparam T Type of the expected value.
/// @tparam E Type of the unexpected value.
///
/// @param lhs Left hand expected to swap.
/// @param rhs Right hand expected to swap.
template <class T, class E,
          enable_if_t<std::is_move_constructible<T>::value &&
                      std::is_move_constructible<E>::value> * = nullptr>
void swap(expected<T, E> &lhs,
          expected<T, E> &rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}

/// @}
}  // namespace cargo

#endif  // CARGO_EXPECTED_H_INCLUDED
