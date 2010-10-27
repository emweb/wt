// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_FIELD_IMPL_H_
#define WT_DBO_FIELD_IMPL_H_

#include <boost/lexical_cast.hpp>

#include <Wt/Dbo/Session>
#include <Wt/Dbo/Exception>
#include <Wt/Dbo/SqlStatement>
#include <Wt/Dbo/SqlTraits>

namespace Wt {
  namespace Dbo {

template <typename V>
FieldRef<V>::FieldRef(V& value, const std::string& name, int size)
  : value_(value),
    name_(name),
    size_(size)
{ }

template <typename V>
const std::string& FieldRef<V>::name() const
{
  return name_;
}

template <typename V>
int FieldRef<V>::size() const
{
  return size_;
}

template <typename V>
std::string FieldRef<V>::sqlType(Session& session) const
{
  return sql_value_traits<V>::type(session.connection(false), size_);
}

template <typename V>
const std::type_info *FieldRef<V>::type() const
{
  return &typeid(V);
}

template <typename V>
void FieldRef<V>::bindValue(SqlStatement *statement, int column) const
{
  sql_value_traits<V>::bind(value_, statement, column, size_);
}

template <typename V>
void FieldRef<V>::setValue(Session& session, SqlStatement *statement,
			   int column) const
{
  sql_value_traits<V>::read(value_, statement, column, size_);
}

template <class C>
CollectionRef<C>::CollectionRef(collection< ptr<C> >& value,
				RelationType type,
				const std::string& joinName,
				const std::string& joinId)
  : value_(value), joinName_(joinName), joinId_(joinId), type_(type)
{ }

template <class C>
PtrRef<C>::PtrRef(ptr<C>& value, const std::string& name, int size)
  : value_(value),
    name_(name),
    size_(size)
{ }

template <class C>
template <class A>
void PtrRef<C>::visit(A& action, Session *session) const
{
  typename dbo_traits<C>::IdType id;

  if (action.setsValue())
    id = dbo_traits<C>::invalidId();
  else
    id = value_.id();

  std::string idFieldName = "stub";
  int size = size_;

  if (session) {
    Session::MappingInfo *mapping = session->getMapping<C>();
    idFieldName = mapping->naturalIdFieldName;
    size = mapping->naturalIdFieldSize;

    if (idFieldName.empty())
      idFieldName = mapping->surrogateIdFieldName;
  }

  field(action, id, name_ + "_" + idFieldName, size);

  if (action.setsValue()) {
    if (!(id == dbo_traits<C>::invalidId())) {
      if (session)
	value_ = session->loadLazy<C>(id);
      else
	throw std::logic_error("Could not load referenced Dbo::ptr, "
			       "no session?");
    }
  }
}

template <class C>
const std::type_info *PtrRef<C>::type() const
{
  return &typeid(typename dbo_traits<C>::IdType);
}

template <class A, typename V>
void id(A& action, V& value, const std::string& name, int size)
{
  action.actId(value, name, size);
}

template <class A, typename V>
void field(A& action, V& value, const std::string& name, int size)
{
  action.act(FieldRef<V>(value, name, size));
}

template <class A, class C>
void field(A& action, ptr<C>& value, const std::string& name, int size)
{
  action.actPtr(PtrRef<C>(value, name, size));
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name, int size)
{
  action.actPtr(PtrRef<C>(value, name, size));
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
