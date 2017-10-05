// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_FIELD_H_
#define WT_DBO_FIELD_H_

#include <string>

#include <Wt/Dbo/ptr.h>

namespace Wt {
  namespace Dbo {
    template <class C> class collection;

    namespace Impl {
const int FKNotNull = 0x01;
const int FKOnUpdateCascade = 0x02;
const int FKOnUpdateSetNull = 0x04;
const int FKOnDeleteCascade = 0x08;
const int FKOnDeleteSetNull = 0x10;
    }

/*! \brief Type that indicates one or more foreign key constraints.
 *
 * This type behaves like an <tt>enum</tt> but avoid ambiguous overloading
 * problems.
 *
 * \sa \link Wt::Dbo::NotNull NotNull\endlink
 * \sa \link Wt::Dbo::OnUpdateCascade OnUpdateCascade\endlink,
 *     \link Wt::Dbo::OnUpdateSetNull OnUpdateSetNull\endlink
 * \sa \link Wt::Dbo::OnDeleteCascade OnDeleteCascade\endlink
 *     \link Wt::Dbo::OnDeleteSetNull OnDeleteSetNull\endlink
 *
 * \sa belongsTo(), hasMany()
 *
 * \ingroup dbo
 */
class ForeignKeyConstraint {
public:
  explicit ForeignKeyConstraint(int value) : value_(value) { }

  int value() const { return value_; }

private:
  int value_;
};

/*! \brief Combines two constraints.
 *
 * \ingroup dbo
 */
inline ForeignKeyConstraint operator|
  (ForeignKeyConstraint lhs, ForeignKeyConstraint rhs)
{
  return ForeignKeyConstraint(lhs.value() | rhs.value());
}

/*! \brief A constraint that prevents a \c null ptr.
 *
 * A database constraint which prevents that a ptr references no object
 * and has a value of \c null.
 *
 * \ingroup dbo
 */
#ifdef DOXYGEN_ONLY
const ForeignKeyConstraint NotNull;
#else
const ForeignKeyConstraint NotNull(Impl::FKNotNull);
#endif

/*! \brief A constraint that cascades updates.
 *
 * A database constraint which propagates updates to the natural primary key
 * in the referenced table.
 *
 * \note This constraint only affects the database schema creation. Currently
 *       it is not possible to update a natural Id of an already saved object
 *       through %Dbo itself. 
 *
 * \ingroup dbo
 */
#ifdef DOXYGEN_ONLY
const ForeignKeyConstraint OnUpdateCascade;
#else
const ForeignKeyConstraint OnUpdateCascade(Impl::FKOnUpdateCascade);
#endif

/*! \brief A constraint that cascades updates.
 *
 * A database constraint which sets the value of the ptr to null when the
 * referenced primary key changes.
 *
 * \note This constraint only affects the database schema creation. Currently
 *       it is not possible to update a natural Id of an already saved object
 *       through %Dbo itself. 
 *
 * \ingroup dbo
 */
#ifdef DOXYGEN_ONLY
const ForeignKeyConstraint OnUpdateSetNull;
#else
const ForeignKeyConstraint OnUpdateSetNull(Impl::FKOnUpdateSetNull);
#endif

/*! \brief A constraint that cascades deletes.
 *
 * A database constraint which propagates deletes of the referenced object
 * to also delete the object(s) that reference it.
 *
 * \note This constraint only affects the database schema creation.
 *
 * \ingroup dbo
 */
#ifdef DOXYGEN_ONLY
const ForeignKeyConstraint OnDeleteCascade;
#else
const ForeignKeyConstraint OnDeleteCascade(Impl::FKOnDeleteCascade);
#endif

/*! \brief A constraint that cascades deletes.
 *
 * A database constraint which propagates deletes of the referenced object
 * to also delete the objects that reference.
 *
 * \note This constraint only affects the database schema creation.
 *
 * \ingroup dbo
 */
#ifdef DOXYGEN_ONLY
const ForeignKeyConstraint OnDeleteSetNull;
#else
const ForeignKeyConstraint OnDeleteSetNull(Impl::FKOnDeleteSetNull);
#endif

class Session;
class SqlStatement;

template <typename V>
class FieldRef
{
public:
  enum Flag {
    AuxId = 0x1
  };

  FieldRef(V& value, const std::string& name, int size, int flags = 0);

  const std::string& name() const;
  int size() const;
  int flags() const;

  std::string sqlType(Session& session) const;
  const std::type_info *type() const;
  const V& value() const { return value_; }
  void setValue(const V& value) const { value_ = value; }

  void bindValue(SqlStatement *statement, int column) const;
  void setValue(Session& session, SqlStatement *statement, int column) const;

private:
  V& value_;
  std::string name_;
  int size_;
  int flags_;
};

/*! \brief Type of an SQL relation.
 *
 * \ingroup dbo
 */
enum RelationType {
  ManyToOne,  //!< Many-to-One relationship
  ManyToMany  //!< Many-to-Many relationship
};

template <class C>
class CollectionRef
{
public:
  CollectionRef(collection< ptr<C> >& value, RelationType type,
		const std::string& joinName, const std::string& joinId,
		int fkConstraints);

  collection< ptr<C> >& value() const { return value_; }
  const std::string& joinName() const { return joinName_; }
  const std::string& joinId() const { return joinId_; }
  bool literalJoinId() const { return literalJoinId_; }
  RelationType type() const { return type_; }
  int fkConstraints() const { return fkConstraints_; }

private:
  collection< ptr<C> >& value_;
  std::string joinName_, joinId_;
  bool literalJoinId_;
  RelationType type_;
  int fkConstraints_;
};

template <class C>
class PtrRef
{
public:
  enum Flag {
    AuxId = 0x1
  };

  PtrRef(ptr<C>& value, const std::string& name, int fkConstraints, int flags = 0);

  const std::string& name() const { return name_; }
  bool literalForeignKey() const { return literalForeignKey_; }
  int fkConstraints() const { return fkConstraints_; }
  int flags() const { return flags_; }
  ptr<C>& value() const { return value_; }
  typename dbo_traits<C>::IdType id() const { return value_.id(); }

  const std::type_info *type() const;

  /*
   * If session = 0, the visited foreign key fields will not be named
   * correctly (ok when e.g. reading/writing data)
   */
  template <typename A> void visit(A& action, Session *session) const;

private:
  ptr<C>& value_;
  std::string name_;
  bool literalForeignKey_;
  int fkConstraints_;
  int flags_;
};

template <class C>
class WeakPtrRef
{
public:
  WeakPtrRef(weak_ptr<C>& value, const std::string& joinName);

  const std::string& joinName() const { return joinName_; }
  weak_ptr<C>& value() const { return value_; }

private:
  weak_ptr<C>& value_;
  std::string joinName_;
};

/*! \brief Maps a natural primary key (id) field.
 *
 * A natural primary key field is optional. If you define one and its
 * type is not <tt>long long</tt>, you must specialize
 * Wt::Dbo::dbo_traits to match the type \p V as the IdType for this
 * class. When not specified for a class, an auto-generated surrogate
 * key field is used with the name specified by
 * Wt::Dbo::dbo_traits::surrogateIdField(), which defaults to "id".
 *
 * Unlike the default surrogate key, a natural id is not
 * auto-generated and thus you need to give each object a unique value
 * when creating a new object.
 *
 * The id may be a composite type. In that case, you need to
 * specialize Wt::Dbo::field().
 *
 * \ingroup dbo
 */
template <class Action, typename V>
void id(Action& action, V& value, const std::string& name = "id",
	int size = -1);

/*! \brief Maps a natural primary key (id) field that is a foreign key.
 *
 * This overloaded method allows to specify constraints for the
 * foreign key.
 *
 * \sa hasMany()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void id(Action& action, ptr<C>& value, const std::string& name,
	ForeignKeyConstraint constraints, int size = -1);

template <class Action, typename V>
void auxId(Action& action, V& value, const std::string& name,
	   int size = -1);

template <class Action, class C>
void auxId(Action& action, ptr<C>& value, const std::string& name,
	   ForeignKeyConstraint constraint = ForeignKeyConstraint(0), int size = -1);

  
/*! \brief Maps a database object field.
 *
 * This function binds the field \p value to the database field \p name.
 *
 * The optional \p size may be used as a hint for the needed
 * storage. It is only useful for <i>std::string</i> or
 * <i>Wt::WString</i> fields, and causes the schema to use a
 * <tt>varchar(</tt><i><tt>size</tt></i><tt>)</tt> for storing the
 * field instead of an unlimited length string type.
 *
 * You may want to specialize this method for a particular composite
 * type which should be persisted in multiple database fields but not as
 * a separate table (e.g. for natural composite primary keys, see id()).
 *
 * For example:
 * \code
 * struct Coordinate {
 *   int x, y;
 * };
 *
 * namespace Wt {
 *   namespace Dbo {
 *
 *     template <class Action>
 *     void field(Action& action, Coordinate& coordinate, const std::string& name, int size = -1)
 *     {
 *       field(action, coordinate.x, name + "_x");
 *       field(action, coordinate.y, name + "_y");
 *     }
 *
 *   } // namespace Dbo
 * } // namespace Wt
 * \endcode
 *
 * To support a custom type that needs to be persisted as a single
 * field, you should specialize sql_value_traits instead.
 *
 * \ingroup dbo
 */
template <class Action, typename V>
void field(Action& action, V& value, const std::string& name, int size = -1);

/*
 * This is synonym for belongsTo(), and used by id(). We should overload
 * this method also to allow foreign key constraints.
 */
template <class Action, class C>
void field(Action& action, ptr<C>& value, const std::string& name, int size = -1);

/*! \brief Maps the "One"-side (foreign key) of a ManyToOne or OneToOne relation.
 *
 * This function binds the pointer field \p value to the database
 * foreign key field(s) \p name <tt>+ "_" +</tt> (C's primary key(s)).
 *
 * If the name starts with a <tt>&gt;</tt>, the <tt>&gt;</tt> is omitted and
 * the name is used literally, instead of \p name <tt>+ "_" +</tt> (C's primary key(s)),
 * e.g. if the primary key of C is <tt>id</tt>, then the name <tt>foo</tt> translates to a
 * column with the name <tt>foo_id</tt>, and the name <tt>&gt;foo</tt> translates to
 * a column with the name <tt>foo</tt>.
 *
 * If the name is omitted or empty, then C's mapped table name is used.
 *
 * A belongsTo() will usually have a counter-part hasMany() or
 * hasOne() declaration in the referenced class \p C.
 *
 * \sa hasMany()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void belongsTo(Action& action, ptr<C>& value,
	       const std::string& name = std::string());

/*! \brief Maps the "One"-side (foreign key) of a ManyToOne or OneToOne relation.
 *
 * This overloaded method allows to specify constraints for the
 * foreign key.
 *
 * \sa hasMany()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void belongsTo(Action& action, ptr<C>& value, const std::string& name,
	       ForeignKeyConstraint constraints);

/*! \brief Maps the "One"-side (foreign key) of a ManyToOne or OneToOne relation.
 *
 * This overloaded method allows to specify constraints for the
 * foreign key.
 *
 * \sa hasMany()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void belongsTo(Action& action, ptr<C>& value,
	       ForeignKeyConstraint constraints);

/*! \brief Maps the "One"-side of a OneToOne relation.
 *
 * This function binds the weak_ptr field \p value to the associated
 * object (of type \p C).
 *
 * A weak_ptr is required here to break the cycle that would otherwise be
 * created by the hasOne() and belongsTo() associations. The value is not
 * actually stored but defined in terms of a SQL query (not that different
 * from how a hasMany() call backs a collection by a SQL query).
 *
 * The query is defined by the database field(s) \p name <tt>+ "_" +
 * </tt> (C's primary key(s)), in the mapped table for C. This
 * should be the same <i>name</i> as passed to the matching
 * belongsTo() method for the other side of the relation. If the \p
 * name is omitted or empty, then the mapped table name of the current
 * class is used. If the name starts with <tt>&gt;</tt>, the <tt>&gt;</tt>
 * is discarded, and the name is used literally, instead of \p name <tt>+ "_" +
 * </tt> (C's primary key(s)).
 *
 * A hasOne() must have a counter-part belongsTo() declaration in the
 * referenced class \p C.
 *
 * \sa belongsTo()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void hasOne(Action& action, weak_ptr<C>& value,
	    const std::string& name = std::string());

/*! \brief Maps the "Many"-side of a ManyToOne or ManyToMany relation.
 *
 * This function binds the collection field \p value to contain
 * objects (of type \p C) that holds the associated objects: reading
 * from the collection and adding or removing objects from the
 * collection results in SQL statements.
 *
 * For a \link Wt::Dbo::ManyToOne ManyToOne\endlink relation, the
 * query is defined by the database field(s) \p name <tt>+ "_" + </tt>
 * (C's primary key(s))</tt>, in the mapped table for C. This should
 * be the same <i>name</i> as passed to the matching belongsTo()
 * method for the other side of the relation. If the \p name is
 * omitted or empty, then the mapped table name of the current class
 * is used.
 *
 * For a \link Wt::Dbo::ManyToMany ManyToMany\endlink relation, the \p
 * name is the name of a linker table (this linker table may be schema
 * qualified, e.g. <tt>"myschema.posts_tags"</tt>. Thus, also for a
 * ManyToMany relation, both sides of the relationship will have the
 * same \p name passed to them. In the join table, this side of the
 * the relation will be referenced using the table name <tt>+ "_" +
 * </tt> (primary key(s)) of the current class.
 *
 * A hasMany() must have a counter-part belongsTo() or hasMany()
 * declaration in the referenced class \p C.
 *
 * \sa belongsTo()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void hasMany(Action& action, collection< ptr<C> >& value,
	     RelationType type, const std::string& name = std::string());

/*! \brief Maps the "Many"-side of a ManyToMany relation.
 *
 * This function binds the collection field \p value to contain
 * objects (of type \p C).
 *
 * This overloaded method allows to customize the field name of the
 * foreign id in the join table, and specify constraints for this
 * foreign key. The only allowed value for \p type is \link
 * Wt::Dbo::ManyToMany ManyToMany\endlink.
 *
 * The \p joinId is used to reference this side of the relationship in
 * the join table, e.g. if \p joinId is <tt>foo</tt> and the primary key
 * is <tt>id</tt>, then this side of the join table will be named <tt>foo_id</tt>.
 * If \p joinId is left blank, the value will be table
 * name <tt>+ "_" +</tt> (primary key(s)) of the current class. If \p joinId
 * starts with <tt>&gt;</tt>, the <tt>&gt;</tt> is omitted, and the joinId
 * is used literally (the primary key of the current class is not appended),
 * e.g. <tt>&gt;foo</tt> maps to the column name <tt>foo</tt>.
 *
 * A hasMany() must have a counter-part belongsTo() or hasMany()
 * declaration in the referenced class \p C.
 *
 * \sa belongsTo()
 *
 * \ingroup dbo
 */
template <class Action, class C>
void hasMany(Action& action, collection< ptr<C> >& value,
	     RelationType type, const std::string& name,
	     const std::string& joinId,
             ForeignKeyConstraint constraints = (NotNull | OnDeleteCascade));

  }
}

#endif // WT_DBO_FIELD
