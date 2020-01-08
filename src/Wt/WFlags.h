// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFLAGS_H_
#define WFLAGS_H_

#include <Wt/WDllDefs.h>
#include <cassert>

namespace Wt {

/*! \class WFlags Wt/WFlags.h Wt/WFlags.h
 *  \brief Utility class for type-safe combinations of enumeration flags.
 *
 * This type is used in %Wt API whenever one or more flag options are
 * expected, instead of an <tt>int</tt>. The class provides type
 * safety, in the sense that it checks that the correct combination of
 * enum values is bound to the argument, and does not cost any
 * run-time overhead (internally it uses only an <tt>int</tt> to
 * represent the combination of flags.
 */

struct NoFlagsType { };
const NoFlagsType None = NoFlagsType();

template<typename EnumType>
class WFlags
{
  typedef const WFlags<EnumType>& MaskType;
public:
  typedef EnumType enum_type;

  /*! \brief Default constructor.
   */
  inline WFlags(NoFlagsType none = None);

  /*! \brief Construct from a single enum value.
   */
  inline WFlags(EnumType flag);

  /*! \brief Copy constructor.
   */
  inline WFlags(const WFlags<EnumType>& other);

  /*! \brief Assignment operator.
   */
  inline WFlags<EnumType>& operator=(const WFlags<EnumType>& other);

  /*! \brief Assignment operator.
   */
  inline WFlags<EnumType>& operator=(const EnumType other);

  /*! \brief Returns whether a flag is set.
   */
  inline bool test(EnumType flag) const;

  /*! \brief Returns true if this WFlags contains no flags.
   */
  inline bool empty() const;

  /*! \brief Clears a flag.
   */
  inline WFlags<EnumType>& clear(EnumType value);

  /*! \brief Cast to the enum type.
   *
   * The internal <tt>int</tt> representation is simply cast to the
   * enum type, without any additional checks.
   */
  inline operator EnumType() const;

  inline int bitCount() const;

  /*! \brief Negation operator.
   *
   * Returns whether different from 0.
   */
  inline bool operator!() const;

  /*! \brief Bitwise AND operator.
   *
   * Returns flags that are the bitwise AND of this and \p mask.
   */
  inline WFlags<EnumType> operator&(EnumType mask) const;

  /*! \brief Bitwise AND operator.
   *
   * Returns flags that are the bitwise AND of this and \p mask.
   */
  inline WFlags<EnumType> operator&(MaskType mask) const;

  /*! \brief Modifying bitwise AND operator.
   *
   * Sets as value the bitwise AND of this and \p mask.
   */
  inline WFlags<EnumType>& operator&=(EnumType mask);

  /*! \brief Modifying bitwise AND operator.
   *
   * Sets as value the bitwise AND of this and \p mask.
   */
  inline WFlags<EnumType>& operator&=(MaskType mask);

  /*! \brief Bitwise XOR operator.
   *
   * Returns flags that are the bitwise XOR of this and \p other.
   */
  inline WFlags<EnumType> operator^(WFlags<EnumType> other) const;

   /*! \brief Bitwise XOR operator.
   *
   * Returns flags that are the bitwise XOR of this and \p other.
   */
  inline WFlags<EnumType> operator^(EnumType other) const;

  /*! \brief Modifying bitwise XOR operator.
   *
   * Sets as value the bitwise XOR of this and \p other.
   */
  inline WFlags<EnumType>& operator^=(WFlags<EnumType> other);

  /*! \brief Modifying bitwise XOR operator.
   *
   * Sets as value the bitwise XOR of this and \p other.
   */
  inline WFlags<EnumType>& operator^=(EnumType other);

  /*! \brief Bitwise OR operator.
   *
   * Returns flags that are the bitwise OR of this and \p other.
   */
  inline WFlags<EnumType> operator|(WFlags<EnumType> other) const;

  /*! \brief Bitwise OR operator.
   *
   * Returns flags that are the bitwise OR of this and \p other.
   */
  inline WFlags<EnumType> operator|(EnumType other) const;

  /*! \brief Modifying bitwise OR operator.
   *
   * Sets as value the bitwise OR of this and \p other.
   */
  inline WFlags<EnumType>& operator|=(WFlags<EnumType> other);

  /*! \brief Modifying bitwise OR operator.
   *
   * Sets as value the bitwise OR of this and \p other.
   */
  inline WFlags<EnumType>& operator|=(EnumType other);

  /*! \brief Inversion operator.
   *
   * Returns flags that are the inverted of this.
   */
  inline WFlags<EnumType> operator~() const;

  inline bool operator==(WFlags<EnumType> other) const;
  inline bool operator==(EnumType other) const;
  inline bool operator!=(WFlags<EnumType> other) const;
  inline bool operator!=(EnumType other) const;
  inline bool operator<(WFlags<EnumType> other) const;

  inline int value() const { return flags_; }

private:
  unsigned int flags_;

  WFlags(int flags, bool): flags_(flags) {}
  inline static WFlags<EnumType> createFromInt(int flags) {
    return WFlags(flags, false);
  }

};

template<typename EnumType>
WFlags<EnumType>::WFlags(const WFlags<EnumType>& other):
  flags_(other.flags_)
{
}

template<typename EnumType>
WFlags<EnumType>::WFlags(enum_type flag):
  flags_(static_cast<unsigned int>(flag))
{
}

template<typename EnumType>
WFlags<EnumType>::WFlags(NoFlagsType /* noFlags */)
  : flags_(0)
{ }

template<typename EnumType>
bool WFlags<EnumType>::test(enum_type flag) const
{
  return flags_ & static_cast<unsigned int>(flag);
}

template<typename EnumType>
bool WFlags<EnumType>::empty() const
{
  return flags_ == 0;
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::clear(enum_type flag)
{
  flags_ &= ~static_cast<unsigned int>(flag);
  return *this;
}

template<typename EnumType>
WFlags<EnumType>::operator EnumType() const
{
  return static_cast<EnumType>(flags_);
}

template<typename EnumType>
int WFlags<EnumType>::bitCount() const
{
  unsigned n = flags_;
  int retval = 0;
  while (n) {
    retval ++;
    n &= n - 1;
  }
  return retval;
}

template<typename EnumType>
bool WFlags<EnumType>::operator!() const
{
  return !flags_;
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator&(EnumType mask) const
{
  return WFlags<EnumType>::createFromInt(flags_ & (unsigned)mask);
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator&(MaskType mask) const
{
  return WFlags<EnumType>::createFromInt(flags_ & mask.value());
}

template<typename EnumType>
WFlags<EnumType> &WFlags<EnumType>::operator&=(MaskType mask)
{
  flags_ &= mask.value();
  return *this;
}

template<typename EnumType>
WFlags<EnumType> &WFlags<EnumType>::operator&=(EnumType mask)
{
  flags_ &= (unsigned)mask;
  return *this;
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator=(const WFlags<EnumType>& other)
{
  flags_ = other.flags_;
  return *this;
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator=(const EnumType other)
{
  flags_ = static_cast<unsigned int>(other);
  return *this;
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator^(WFlags<EnumType> other) const
{
  return WFlags<EnumType>::createFromInt(flags_ ^ other.flags_);
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator^(EnumType other) const
{
  return WFlags<EnumType>::createFromInt(flags_ ^ other);
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator^=(WFlags<EnumType> other)
{
  flags_ ^= other.flags_;
  return *this;
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator^=(EnumType other)
{
  flags_ ^= other;
  return *this;
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator|(WFlags<EnumType> other) const
{
  return WFlags<EnumType>::createFromInt(flags_ | other.flags_);
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator|(EnumType other) const
{
  return WFlags<EnumType>::createFromInt
    (flags_ | static_cast<unsigned int>(other));
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator|=(WFlags<EnumType> other)
{
  flags_ |= other.flags_;
  return *this;
}

template<typename EnumType>
WFlags<EnumType>& WFlags<EnumType>::operator|=(EnumType other)
{
  flags_ |= static_cast<unsigned int>(other);
  return *this;
}

template<typename EnumType>
WFlags<EnumType> WFlags<EnumType>::operator~() const
{
  return WFlags<EnumType>::createFromInt(~flags_);
}

template<typename EnumType>
bool WFlags<EnumType>::operator==(WFlags<EnumType> other) const
{
  return flags_ == other.flags_;
}

template<typename EnumType>
bool WFlags<EnumType>::operator==(EnumType other) const
{
  return flags_ == static_cast<unsigned int>(other);
}

template<typename EnumType>
bool WFlags<EnumType>::operator!=(WFlags<EnumType> other) const
{
  return flags_ != other.flags_;
}

template<typename EnumType>
bool WFlags<EnumType>::operator!=(EnumType other) const
{
  return flags_ != static_cast<unsigned int>(other);
}

template<typename EnumType>
bool WFlags<EnumType>::operator<(WFlags<EnumType> other) const
{
  return flags_ < other.flags_;
}

#ifndef WT_TARGET_JAVA
#define W_DECLARE_OPERATORS_FOR_FLAGS(EnumType)				\
inline Wt::WFlags<EnumType> operator|(EnumType l, EnumType r) {         \
  Wt::WFlags<EnumType> retval(l);					\
  retval |= r;								\
  return retval;							\
}									\
inline Wt::WFlags<EnumType> operator|(EnumType l,		        \
                                      Wt::WFlags<EnumType> r) {		\
  return r | l;								\
}
#else
#define W_DECLARE_OPERATORS_FOR_FLAGS(EnumType)				\
Wt::WFlags<EnumType> operator|(EnumType l, EnumType r);                 \
Wt::WFlags<EnumType> operator|(EnumType l, Wt::WFlags<EnumType> r);     \
bool operator==(EnumType l, Wt::WFlags<EnumType> r);                    \
bool operator==(EnumType l, int zero);
#endif
}

#endif // WFLAGS_H_

