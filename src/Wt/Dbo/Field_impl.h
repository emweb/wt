// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_FIELD_IMPL_H_
#define WT_DBO_FIELD_IMPL_H_

#include <boost/lexical_cast.hpp>

#include <Wt/Dbo/Exception>
#include <Wt/Dbo/SqlStatement>
#include <Wt/Dbo/SqlTraits>

namespace Wt {
  namespace Dbo {

template <typename V>
FieldRef<V>::FieldRef(V& value, const std::string& name)
  : value_(value),
    name_(name)
{ }

template <typename V>
const std::string& FieldRef<V>::name() const
{
  return name_;
}

template <typename V>
std::string FieldRef<V>::sqlType(Session& session) const
{
  return sql_value_traits<V>::type();
}

template <typename V>
void FieldRef<V>::bindValue(SqlStatement *statement, int column) const
{
  sql_value_traits<V>::bind(value_, statement, column);
}

template <typename V>
void FieldRef<V>::setValue(Session& session, SqlStatement *statement,
			   int column) const
{
  sql_value_traits<V>::read(value_, statement, column);
}

template <typename V>
template <class A>
void FieldRef<V>::descend(A& action) const
{ }

template <class C>
CollectionRef<C>::CollectionRef(collection< ptr<C> >& value,
				RelationType type,
				const std::string& joinName,
				const std::string& joinId)
  : value_(value), joinName_(joinName), joinId_(joinId), type_(type)
{ }

template <class C>
FieldRef< ptr<C> >::FieldRef(ptr<C>& value, const std::string& name)
  : value_(value),
    name_(std::string(name) + "_id")
{ }

template <class C>
const std::string& FieldRef< ptr<C> >::name() const
{
  return name_;
}

template <class C>
std::string FieldRef< ptr<C> >::sqlType(Session& session) const
{
  return std::string("integer references \"")
    + session.tableName<C>() + "\"(\"id\")";
}

template <class C>
void FieldRef< ptr<C> >::bindValue(SqlStatement *statement, int column)
  const
{
  if (value_)
    statement->bind(column, value_.id());
  else
    statement->bindNull(column);
}

template <class C>
void FieldRef< ptr<C> >::setValue(Session& session,
				  SqlStatement *statement, int column)
  const
{
  long long id;
  bool notNull = statement->getResult(column, &id);

  if (notNull)
    value_ = session.load<C>(id);
}

template <class C>
template <class A>
void FieldRef< ptr<C> >::descend(A& action) const
{
  action.descend(value_);
}


template <class A, typename V>
void field(A& action, V& value, const std::string& name)
{
  action.act(FieldRef<V>(value, name));
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name)
{
  action.act(FieldRef<ptr<C> >(value, name));
}

template <class A, class C>
void hasMany(A& action, collection< ptr<C> >& value,
	     RelationType type, const std::string& joinName,
	     const std::string& joinId)
{
  action.actCollection(CollectionRef<C>(value, type, joinName, joinId));
}

  }
}

#endif // WT_DBO_FIELD_IMPL_H_
