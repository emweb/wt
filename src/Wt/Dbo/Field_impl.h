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
#include <Wt/Dbo/DbAction>

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
  : value_(value), joinName_(joinName), joinId_(joinId),
    literalJoinId_(false),
    type_(type), fkConstraints_(fkConstraints)
{
  if (type == ManyToOne && !joinName.empty() && joinName[0] == '>') {
    joinName_ = std::string(joinName.c_str() + 1, joinName.size() - 1);
  }
  if (type == ManyToMany && !joinId.empty() && joinId[0] == '>') {
    joinId_ = std::string(joinId.c_str() + 1, joinId.size() - 1);
    literalJoinId_ = true;
  }
}

template <class C>
PtrRef<C>::PtrRef(ptr<C>& value, const std::string& name,
		  int fkConstraints)
  : value_(value),
    name_(name),
    literalForeignKey_(false),
    fkConstraints_(fkConstraints)
{
  if (!name.empty() && name[0] == '>') {
    name_ = std::string(name.c_str() + 1, name.size() - 1);
    literalForeignKey_ = true;
  }
}

template <class C, class A, class Enable = void>
struct LoadLazyHelper
{
  static void loadLazy(ptr<C>& p, typename dbo_traits<C>::IdType id,
		       Session *session) { }
};

template <class C, class A>
struct LoadLazyHelper<C, A, typename boost::enable_if<action_sets_value<A> >::type>
{
  static void loadLazy(ptr<C>& p, typename dbo_traits<C>::IdType id,
		       Session *session) {
    if (!(id == dbo_traits<C>::invalidId())) {
      if (session)
	p = session->loadLazy<C>(id);
      else
	throw Exception("Could not load referenced Dbo::ptr, no session?");
    }
  }
};

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
  int size = -1;

  if (session) {
    Impl::MappingInfo *mapping = session->getMapping<C>();
    action.actMapping(mapping);
    idFieldName = mapping->naturalIdFieldName;
    size = mapping->naturalIdFieldSize;

    if (idFieldName.empty())
      idFieldName = mapping->surrogateIdFieldName;
  }

  if (literalForeignKey()) {
    field(action, id, name_, size);
  } else {
    field(action, id, name_ + "_" + idFieldName, size);
  }

  LoadLazyHelper<C, A>::loadLazy(value_, id, session);
}

template <class C>
WeakPtrRef<C>::WeakPtrRef(weak_ptr<C>& value, const std::string& joinName)
  : value_(value),
    joinName_(joinName)
{
  if (!joinName.empty() && joinName[0] == '>') {
    joinName_ = std::string(joinName.c_str() + 1, joinName.size() - 1);
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
void field(A& action, ptr<C>& value, const std::string& name, int)
{
  action.actPtr(PtrRef<C>(value, name, 0));
}

template <class A, class C>
void belongsToImpl(A& action, ptr<C>& value, const std::string& name,
		   int fkConstraints)
{
  if (name.empty() && action.session())
    action.actPtr(PtrRef<C>(value, action.session()->template tableName<C>(),
			    fkConstraints));
  else
    action.actPtr(PtrRef<C>(value, name, fkConstraints));
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name)
{
  belongsToImpl(action, value, name, 0);
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value, const std::string& name,
	       ForeignKeyConstraint constraint)
{
  belongsToImpl(action, value, name, constraint.value());
}

template <class A, class C>
void belongsTo(A& action, ptr<C>& value,
	       ForeignKeyConstraint constraint)
{
  belongsToImpl(action, value, std::string(), constraint.value());
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
