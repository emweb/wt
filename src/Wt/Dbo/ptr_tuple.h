// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_TUPLE_PTR_H_
#define WT_DBO_TUPLE_PTR_H_

#include <tuple>
#include <Wt/Dbo/Query.h>

namespace Wt {
  namespace Dbo {

namespace Impl {
  template <int I, typename... Ts>
  struct helper
  {
    typedef typename std::tuple<Ts...> TupleType;
    typedef typename std::tuple_element<I, std::tuple<Ts...>>::type EIType;

    static void getFields(Session& session,
			  std::vector<std::string> *aliases,
			  std::vector<FieldInfo>& result)
    {
      helper<I-1, Ts...>::getFields(session, aliases, result);
      query_result_traits<EIType>::getFields(session, aliases, result);
    }

    static void load(Session& session, SqlStatement& statement,
		     int& column, TupleType& result)
    {
      helper<I-1, Ts...>::load(session, statement, column, result);

      std::get<I>(result)
	= query_result_traits<EIType>::load(session, statement, column);
    }

    static void getValues(const TupleType& result, std::vector<cpp17::any>& values)
    {
      helper<I-1, Ts...>::getValues(result, values);

      query_result_traits<EIType>::getValues(std::get<I>(result), values);
    }

    static void setValue(TupleType& result, int& index, const cpp17::any& value)
    {
      if (index >= 0) {
	helper<I-1, Ts...>::setValue(result, index, value);

	if (index >= 0)
	  query_result_traits<EIType>::setValue(std::get<I>(result),
						index, value);
      }
    }

    static void create(TupleType& result)
    {
      helper<I-1, Ts...>::create(result);
      std::get<I>(result) = query_result_traits<EIType>::create();
    }

    static void add(Session& session, TupleType& result)
    {
      helper<I-1, Ts...>::add(session, result);

      query_result_traits<EIType>::add(session, std::get<I>(result));
    }

    static void remove(TupleType& result)
    {
      helper<I-1, Ts...>::remove(result);

      query_result_traits<EIType>::remove(std::get<I>(result));
    }
  };

  template <typename... Ts>
  struct helper<-1, Ts...>
  {
    typedef typename std::tuple<Ts...> TupleType;
    static void getFields(Session& session,
			  std::vector<std::string> *aliases,
			  std::vector<FieldInfo>& result)
    { }

    static void load(Session& session, SqlStatement& statement,
		     int& column, TupleType& result)
    { }

    static void getValues(const TupleType& result, std::vector<cpp17::any>& values)
    { }

    static void setValue(TupleType& result, int& index, const cpp17::any& value)
    { }

    static void create(TupleType& result)
    { }

    static void add(Session& session, TupleType& result)
    { }

    static void remove(TupleType& result)
    { }
  };

}

template <typename... T>
struct query_result_traits<std::tuple<T...>>
{
  typedef Impl::helper<sizeof...(T) - 1, T...> helper;

  static void getFields(Session& session,
			std::vector<std::string> *aliases,
			std::vector<FieldInfo>& result);

  static std::tuple<T...> load(Session& session, SqlStatement& statement,
			 int& column);

  static void getValues(const std::tuple<T...>& t, std::vector<cpp17::any>& values);

  static void setValue(std::tuple<T...>& t, int& index, const cpp17::any& value);

  static std::tuple<T...> create();
  static void add(Session& session, std::tuple<T...>& t);
  static void remove(std::tuple<T...>& t);

  static long long id(const std::tuple<T...>& ptr);
  static std::tuple<T...> findById(Session& session, long long id);
};

template <typename... T>
void query_result_traits<std::tuple<T...>>
::getFields(Session& session, std::vector<std::string> *aliases,
	    std::vector<FieldInfo>& result)
{
  helper::getFields(session, aliases, result);
}

template <typename... T>
std::tuple<T...> query_result_traits<std::tuple<T...>>
::load(Session& session, SqlStatement& statement, int& column)
{
  std::tuple<T...> result;
  helper::load(session, statement, column, result);
  return result;
}

template <typename... T>
void query_result_traits<std::tuple<T...>>
::getValues(const std::tuple<T...>& t, std::vector<cpp17::any>& values)
{
  helper::getValues(t, values);
}

template <typename... T>
void query_result_traits<std::tuple<T...>>::setValue(std::tuple<T...>& t,
						     int& index,
						     const cpp17::any& value)
{
  helper::setValue(t, index, value);
}

template <typename... T>
std::tuple<T...> query_result_traits<std::tuple<T...>>::create()
{
  std::tuple<T...> result;
  helper::create(result);
  return result;
}

template <typename... T>
void query_result_traits<std::tuple<T...>>::add(Session& session,
						std::tuple<T...>& t)
{
  helper::add(session, t);
}

template <typename... T>
void query_result_traits<std::tuple<T...>>::remove(std::tuple<T...>& t)
{
  helper::remove(t);
}

template <typename... T>
long long query_result_traits<std::tuple<T...>>::id(const std::tuple<T...>& t)
{
  return -1;
}

template <typename... T>
std::tuple<T...> query_result_traits<std::tuple<T...>>
::findById(Session& session, long long id)
{
  return std::tuple<T...>();
}

  }
}

#endif // WT_DBO_DBO_PTR_H_
