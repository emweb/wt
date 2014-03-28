// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_FIELD_IMPL_H_
#define WT_DBO_FIELD_IMPL_H_

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
				const std::string& joinId,
				int fkConstraints)
  : value_(value), joinName_(joinName), joinId_(joinId), type_(type),
    fkConstraints_(fkConstraints)
{ }

template <class C>
PtrRef<C>::PtrRef(ptr<C>& value, const std::string& name, int size,
		  int fkConstraints)
  : value_(value),
    name_(name),
    size_(size),
    fkConstraints_(fkConstraints)
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
	throw Exception("Could not load referenced Dbo::ptr, no session?");
    }
  }
}

template <class C>
WeakPtrRef<C>::WeakPtrRef(weak_ptr<C>& value, const std::string& joinName)
  : value_(value),
    joinName_(joinName)
{ }

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

template <class A, class C>
void id(A& action, ptr<C>& value, const std::string& name,
	ForeignKeyConstraint constraint, int size)
{
  action.actId(value, name, size, constraint.value());
}

template <class A, typename V>
void field(A& action, V& value, const std::string& name, int size)
{
  action.act(FieldRef<V>(value, name, size));
}

template <class A, class C>
void field(A& action, ptr<C>& value, const std::string& name, int size)
{
  action.actPtr(PtrRef<C>(value, name, size, 0));
}

template <class A, class C>
void belongsToImpl(A& action, ptr<C>& value, const std::string& name,
		   int fkConstraints, int size)
{
  if (name.empty() && action.session())
    action.actPtr(PtrRef<C>(value, action.session()->template tableName<C>(),
			    size, fkConstraints));
  else
    action.actPtr(PtrRef<C>(value, name, size, fkConstraints));
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name, int size)
{
  belongsToImpl(action, value, name, 0, size);
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name,
	       ForeignKeyConstraint constraint, int size)
{
  belongsToImpl(action, value, name, constraint.value(), size);
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value,
	       ForeignKeyConstraint constraint, int size)
{
  belongsToImpl(action, value, std::string(), constraint.value(), size);
}

template <class A, class C>
void hasOne(A& action, weak_ptr<C>& value, const std::string& joinName)
{
  action.actWeakPtr(WeakPtrRef<C>(value, joinName));
}

template <class A, class C>
void hasMany(A& action, collection< ptr<C> >& value,
	     RelationType type, const std::string& joinName)
{
  action.actCollection(CollectionRef<C>(value, type, joinName, std::string(),
                                        Impl::FKNotNull |
                                        Impl::FKOnDeleteCascade));
}

template <class A, class C>
void hasMany(A& action, collection< ptr<C> >& value,
	     RelationType type, const std::string& joinName,
	     const std::string& joinId, ForeignKeyConstraint constraint)
{
  if (type != ManyToMany)
    throw Exception("hasMany() with named joinId only for a ManyToMany relation");

  action.actCollection(CollectionRef<C>(value, type, joinName, joinId,
					constraint.value()));
}

  }
}

#endif // WT_DBO_FIELD_IMPL_H_
