// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WANY_H_
#define WT_WANY_H_

#include <Wt/WString.h>
#include <Wt/cpp17/any.hpp>

#include <sstream>

namespace Wt {

/*! \brief A traits class for a type stored in a cpp17::any.
 *
 * The traits are used by %Wt's view classes (such as Wt::WTableView
 * Wt::WTreeView, Wt::Chart::WCartesianChart) to deal with values in a
 * Wt::WAbstractItemModel:
 *  - conversion to a string by Wt::asString()
 *  - conversion to a number by Wt::asNumber()
 *  - sorting by Wt::WSortFilterProxyModel or WStandardItemModel.
 *
 * This traits class provides a default implementation for supporting
 * values <i>value</i> of a custom type <i>Type</i> in %Wt's MVC system,
 * which relies on:
 * - serializing to a stream (std::ostream << <i>value</i>) for converting the
 *   value to a string.
 * - lexical interpretation as a number for converting the value to a number,
 *   using std::stod()
 * - sorting using operator== and operator<
 *
 * You can specialize these traits if you need to customize one or more of these
 * operations.
 *
 * \ingroup modelview
 */
template <typename Type>
struct any_traits {
  /*! \brief Converts a value to a string.
   *
   * The default implementation uses std::ostream<< operator for the value.
   */
  static WString asString(const Type& value, const WString& format);

  /*! \brief Converts a value to a number.
   *
   * The default implementation uses std::stod() on the string value
   */
  static double asNumber(const Type& v);

  /*! \brief Compares two values.
   *
   * The default implementation uses operator== and operator<.
   */
  static int compare(const Type& v1, const Type& v2);
};

/*! \brief Interprets a cpp17::any as a string value.
 *
 * The conversion works as follows:
 *  - A cpp17::any without a value is converted to an empty string
 *  - Number values (integers and doubles) are formatted using the
 *    format string with <i>snprintf()</i>, or with
 *    <i>std::stod()</i> if the format string is empty.
 *  - Data of type WDate is converted with WDate::toString() using the indicated
 *    format string. If the format string is empty, WLocalce::currentLocale().dateFormat() is assumed.
 *  - Data of type WTime is converted with WTime::toString() using the indicated
 *    format string. If the format string is emtpy, WLocale::currentLocale().timeFormat() is assumed.
 *  - Data of type WDateTime is converted with WDateTime::toString() using the
 *    indicated format string. If the format string is empty,
 *    WLocale::currentLocale().dateTimeFormat() is assumed.
 *
 * Other types are converted according to
 * Wt::any_traits<Type>::asString(). For these other types, you need
 * to register their support first using Wt::registerType<Type>() and you may
 * want to specialize Wt::any_traits<Type> for a custom handling of
 * their conversions.
 *
 * \sa asNumber(), any_traits 
 *
 * \ingroup modelview
 */
#ifdef DOXYGEN_ONLY
extern WString asString(const cpp17::any& v,
			const WT_USTRING& formatString = WT_USTRING());
#else
extern WT_API WString asString(const cpp17::any& v,
			       const WT_USTRING& formatString = WT_USTRING());
#endif

/*! \brief Interprets a cpp17::any as a number value.
 *
 * The conversion works as follows:
 * - A cpp17::any without a value, or a string that does not represent a
 * number, is converted to a <i>NaN</i>.
 * - A string is lexically casted to a double
 * - Data of type WDate is converted to an integer number using
 * WDate::toJulianDay().
 * - Data of type WDateTime is converted to an integer number using
 * WDateTime::toTime_t().
 * - Data of type WTime is converted to an integer number as the number of
 * milliseconds since midnight.
 *
 * Other types are converted according to
 * Wt::any_traits<Type>::asNumber(). For these other types, you need to
 * register their support first using Wt::registerType<Type>() and you may
 * want to specialize Wt::any_traits<Type> for a custom handling of
 * their conversions.
 *
 * \sa asString(), any_traits 
 *
 * \ingroup modelview
 */
#ifdef DOXYGEN_ONLY
extern double asNumber(const cpp17::any& v);
#else
extern WT_API double asNumber(const cpp17::any& v);
#endif // DOXYGEN_ONLY

/*
 * Converts a value of one type to a value of another type, using a
 * WString as an intermediate form.
 */
extern WT_API cpp17::any
convertAnyToAny(const cpp17::any& v,
		const std::type_info& type,
		const WT_USTRING& formatString = WT_USTRING());

/*! \brief Registers MVC-support for a type passed in a cpp17::any.
 *
 * By registering a type using this method, the global function
 * Wt::asString(), which converts a cpp17::any to a string, will know
 * how to interpret a cpp17::any holding a value of type \p Type using
 * Wt::any_traits<Type>::asString(). This gives %Wt's built-in
 * standard View classes the ability to display Wt::ItemDataRole::Display data
 * of a Wt::WAbstractItemModel model.
 *
 * Similarly, Wt::asNumber() uses
 * Wt::any_traits<Type>::asNumber() for use by
 * Wt::Chart::WCartesianChart and Wt::Chart::WPieChart as numerical
 * data.
 *
 * The default implementation of Wt::any_traits<Type> converts a
 * value \p t to a Wt::WString by using std::ostream<<, expecting a
 * UTF-8 string. This conversion thus relies on the std::ostream<<
 * operator to be overloaded for the type. You may want to specialize
 * Wt::any_traits to provide a custom conversion for a type, or
 * if you want to take into account the format string.
 *
 * The following types are registered by %Wt itself:
 *  - strings of type WString or std::string
 *  - WDate, WTime, WDateTime
 *  - standard C++ numeric types (int, double, etc...)
 *  - bool
 *
 * This method is thread-safe, and it is not an error to register the
 * same type multiple times.
 *
 * \ingroup modelview
 */
template <typename Type> void registerType();

namespace Impl {
  class WT_API AbstractTypeHandler {
  public:
    AbstractTypeHandler();
    virtual ~AbstractTypeHandler();
    virtual WString asString(const cpp17::any& v, const WString& format) = 0;
    virtual double asNumber(const cpp17::any& v) = 0;
    virtual int compare(const cpp17::any& v1, const cpp17::any& v2) = 0;
  };

  template <typename T>
  class TypeHandler final : public AbstractTypeHandler {
    virtual WString asString(const cpp17::any& v, const WString& format) override {
      return any_traits<T>::asString(cpp17::any_cast<T>(v), format);
    }

    virtual double asNumber(const cpp17::any& v) override {
      return any_traits<T>::asNumber(cpp17::any_cast<T>(v));
    }

    virtual int compare(const cpp17::any& v1, const cpp17::any& v2) override {
      return any_traits<T>::compare(cpp17::any_cast<T>(v1),
					  cpp17::any_cast<T>(v2));
    }
  };

  extern WT_API AbstractTypeHandler *getRegisteredType
    (const std::type_info &type, bool takeLock);
  extern WT_API void registerType(const std::type_info &type,
				  AbstractTypeHandler *handler);

  extern WT_API void lockTypeRegistry();
  extern WT_API void unlockTypeRegistry();

  extern WT_API bool matchValue(const cpp17::any& value, const cpp17::any& query,
				WFlags<MatchFlag> flags);
  extern WT_API int compare(const cpp17::any& d1, const cpp17::any& d2);

  extern WT_API cpp17::any updateFromJS(const cpp17::any& v, std::string s);

  extern WT_API std::string asJSLiteral(const cpp17::any& v, 
					TextFormat textFormatbool);
}

template <typename Type> void registerType()
{
  Impl::lockTypeRegistry();
  try {
    if (!Impl::getRegisteredType(typeid(Type), false))
      Impl::registerType(typeid(Type), new Impl::TypeHandler<Type>());

    Impl::unlockTypeRegistry();
  } catch (...) {
    Impl::unlockTypeRegistry();
    throw;
  }
}

template <typename Type>
WString any_traits<Type>::asString(const Type& value,
					 const WString& format)
{
  std::stringstream ss;
  ss << value;
  return WString::fromUTF8(ss.str());
}

template <typename Type>
double any_traits<Type>::asNumber(const Type& v)
{
  return std::stod(asString(v, WString::Empty).toUTF8());
}

template <typename Type>
int any_traits<Type>::compare(const Type& v1, const Type& v2)
{
  return v1 == v2 ? 0 : (v1 < v2 ? -1 : 1);
}

}

#endif // WT_WANY_H_
