// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CPP17_ANY_HPP
#define WT_CPP17_ANY_HPP

#include "Wt/WConfig.h"

#if defined(WT_ANY_IS_EXPERIMENTAL_ANY)

#include <experimental/any>

namespace Wt {
  namespace cpp17 {
    using any = std::experimental::any;
    using bad_any_cast = std::experimental::bad_any_cast;

    inline bool any_has_value(const any &v) {
      return !v.empty();
    }

    template<class ValueType>
    ValueType any_cast(const any& operand) {
      return std::experimental::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any& operand) {
      return std::experimental::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any&& operand) {
      return std::experimental::any_cast<ValueType>(std::move(operand));
    }
    template<class ValueType>
    const ValueType* any_cast(const any* operand) noexcept {
      return std::experimental::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType* any_cast(any* operand) noexcept {
      return std::experimental::any_cast<ValueType>(operand);
    }
  }
}

#elif defined(WT_ANY_IS_STD_ANY)

#include <any>

namespace Wt {
  namespace cpp17 {
    using any = std::any;
    using bad_any_cast = std::bad_any_cast;

    inline bool any_has_value(const any &v) {
      return v.has_value();
    }

    template<class ValueType>
    ValueType any_cast(const any& operand) {
      return std::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any& operand) {
      return std::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any&& operand) {
      return std::any_cast<ValueType>(std::move(operand));
    }
    template<class ValueType>
    const ValueType* any_cast(const any* operand) noexcept {
      return std::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType* any_cast(any* operand) noexcept {
      return std::any_cast<ValueType>(operand);
    }
  }
}

#else // defined(WT_ANY_IS_THELINK2012_ANY)

#include "any/any.hpp"

namespace Wt {
  namespace cpp17 {
    using any = linb::any;
    using bad_any_cast = linb::bad_any_cast;

    inline bool any_has_value(const any &v) {
      return !v.empty();
    }

    template<class ValueType>
    ValueType any_cast(const any& operand) {
      return linb::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any& operand) {
      return linb::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType any_cast(any&& operand) {
      return linb::any_cast<ValueType>(std::move(operand));
    }
    template<class ValueType>
    const ValueType* any_cast(const any* operand) noexcept {
      return linb::any_cast<ValueType>(operand);
    }
    template<class ValueType>
    ValueType* any_cast(any* operand) noexcept {
      return linb::any_cast<ValueType>(operand);
    }
  }
}

#endif

#endif // WT_CPP17_ANY_HPP
