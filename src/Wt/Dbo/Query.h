// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_H_
#define WT_DBO_QUERY_H_

#include <vector>

#include <Wt/Dbo/SqlTraits.h>
#include <Wt/Dbo/ptr.h>

namespace Wt {
  namespace Dbo {

    template <class C> class collection;

    namespace Impl {

      struct SelectField
      {
	std::size_t begin, end;
      };

      typedef std::vector<SelectField> SelectFieldList;
      typedef std::vector<SelectFieldList> SelectFieldLists;

      template <class Result>
      class QueryBase {
      protected:
	std::vector<FieldInfo> fields() const;
	void fieldsForSelect(const SelectFieldList& list,
			     std::vector<FieldInfo>& result) const;
        std::pair<SqlStatement *, SqlStatement *>
        statements(const std::string& join, const std::string &where,
		   const std::string &groupBy,
                   const std::string &having, const std::string &orderBy,
                   int limit, int offset) const;
        Session &session() const;

        QueryBase();
	QueryBase(Session& session, const std::string& sql);
	QueryBase(Session& session, const std::string& table,
		  const std::string& where);

	QueryBase& operator=(const QueryBase& other);

	Result singleResult(const collection<Result>& results) const;

	Session *session_;
	std::string sql_;
	SelectFieldLists selectFieldLists_;
      };
    }

/*! \class DirectBinding Wt/Dbo/Query.h Wt/Dbo/Query.h
 *
 * Bind strategy indicating that parameters are bound directly to an underlying
 * prepared statement.
 */
struct DirectBinding { };

/*! \class DynamicBinding Wt/Dbo/Query.h Wt/Dbo/Query.h
 *
 * Bind strategy indicating that binding to the underlying prepared statement
 * is deferred and parameter values are temporarily stored first in a dynamic
 * value vector.
 */
struct DynamicBinding { };

class Session;

/*! \class AbstractQuery Wt/Dbo/Query.h Wt/Dbo/Query.h
 *  \brief An abstract dynamic database query.
 *
 * \sa Query
 *
 * \ingroup dbo
 */
class WTDBO_API AbstractQuery
{
public:
  /*! \brief Binds a value to the next positional marker.
   *
   * This binds the \p value to the next positional marker in the
   * query condition.
   */
  template<typename T> AbstractQuery& bind(const T& value);

  /*! \brief Resets bound values.
   *
   * This undoes all previous calls to bind().
   */
  void reset();

  /*! \brief Adds a join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  AbstractQuery& join(const std::string& other);

  /*! \brief Adds a left join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>left join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  AbstractQuery& leftJoin(const std::string& other);

  /*! \brief Adds a right join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>right join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  AbstractQuery& rightJoin(const std::string& other);
  
  /*! \brief Adds a query condition.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>where</i> condition expression to the
   * current query.
   *
   * The condition must be a valid SQL condition expression.
   *
   * Multiple conditions may be provided by successive calls to
   * where(), which must each be fulfilled, and are concatenated
   * together using 'and'.
   *
   * As with any part of the SQL query, a condition may contain
   * positional markers '?' to which values may be bound using bind().
   */
  AbstractQuery& where(const std::string& condition);

  /*! \brief Adds a query condition.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>where</i> condition expression to the
   * current query.
   *
   * The condition must be a valid SQL condition expression.
   *
   * Multiple conditions may be provided by successive calls to
   * orWhere(), and are concatenated
   * together using 'or'.
   * Previous conditions will be surrounded by brackets and the
   * new condition will be concatenated using 'or'. For example:
   * \code
   *  query.where("column_a = ?").bind("A")
   *    .where("column_b = ?").bind("B")
   *    .orWhere("column_c = ?").bind("C");
   * \endcode
   * results in:
   * "where ((column_a = 'A') and (column_b = 'B')) or column_c = 'C'"
   *
   * As with any part of the SQL query, a condition may contain
   * positional markers '?' to which values may be bound using bind().
   */
  AbstractQuery& orWhere(const std::string& condition);

  /*! \brief Sets the result order.
   *
   * This is a convenience method for creating a SQL query, and sets an
   * <i>order by</i> field expression for the current query.
   *
   * Orders the results based on the given field name (or multiple
   * names, comma-separated).
   */
  AbstractQuery& orderBy(const std::string& fieldName);

  /*! \brief Sets the grouping field(s).
   *
   * This is a convenience method for creating a SQL query, and sets a
   * <i>group by</i> field expression for the current query.
   *
   * Groups results based on unique values of the indicated field(s),
   * which is a comma separated list of fields. Only fields on which
   * you group and aggregate functions can be selected by a query.
   *
   * A field that refers to a database object that is selected by the
   * query is expanded to all the corresponding fields of that
   * database object (as in the select statement).
   */
  AbstractQuery& groupBy(const std::string& fields);

  /*! \brief Sets the grouping filter(s).
   *
   * It's like where(), but for aggregate fields.
   *
   * For example you can't go:
   *
   *   select department.name, count(employees) from department
   *    where count(employees) > 5
   *    group by count(employees);
   *          
   * Because you can't have aggregate fields in a where clause, but you can go:
   *
   *   select department.name, count(employees) from department
   *    group by count(employees)
   *   having count(employees) > 5;
   *          
   * This will of course return all the departments with more than 5 employees
   * (and their employee count).
   *
   * \note You must have a group by clause, in order to have a 'having' clause
   */  
  AbstractQuery& having(const std::string& fields);

  /*! \brief Sets a result offset.
   *
   * Sets a result offset. This has the effect that the next
   * resultList() call will skip as many results as the offset
   * indicates. Use -1 to indicate no offset.
   *
   * This provides the (non standard) <i>offset</i> part of an SQL query.
   *
   * \sa limit()
   */
  AbstractQuery& offset(int count);

  /*! \brief Returns an offset set for this query.
   *
   * \sa offset(int)
   */
  int offset() const;

  /*! \brief Sets a result limit.
   *
   * Sets a result limit. This has the effect that the next
   * resultList() call will return up to \p count results. Use -1 to
   * indicate no limit.
   *
   * This provides the (non standard) <i>limit</i> part of an SQL query.
   *
   * \sa offset()
   */
  AbstractQuery& limit(int count);

  /*! \brief Returns a limit set for this query.
   *
   * \sa limit(int)
   */  
  int limit() const;

protected:
  std::string join_, where_, groupBy_, having_, orderBy_;
  int limit_, offset_;

  AbstractQuery();
  ~AbstractQuery();
  AbstractQuery(const AbstractQuery& other);
  AbstractQuery& operator= (const AbstractQuery& other);
  void bindParameters(Session *session, SqlStatement *statement) const;

  std::vector<Impl::ParameterBase *> parameters_;
};
  
/*! \class Query Wt/Dbo/Query.h Wt/Dbo/Query.h
 *  \brief A database query.
 *
 * The query fetches results of type \p Result from the database. This
 * can be any type for which query_result_traits are properly
 * implemented. The library provides these implementations for
 * primitive values (see sql_value_traits), database objects (ptr) and
 * <tt>std::tuple</tt>.
 *
 * Simple queries can be done using Session::find(), while more elaborate
 * queries (with arbitrary result types) using Session::query().
 *
 * You may insert positional holders anywhere in the query for
 * parameters using '?', and bind these to actual values using bind().
 *
 * The query result may be fetched using resultValue() or resultList().
 *
 * Usage example:
 * \code
 * typedef Wt::Dbo::ptr<Account> AccountPtr;
 * typedef Wt::Dbo::collection<AccountPtr> Accounts;
 *
 * Wt::Dbo::Query<AccountPtr> query = session.find<Account>().where("balance > ?").bind(100000);
 * Accounts accounts = query.resultList();
 *
 * for (Accounts::const_iterator i = accounts.begin(); i != accounts.end(); ++i)
 *   std::cerr << "Name: " << (*i)->name << std::end;
 * \endcode
 *
 * The \p BindStrategy specifies how you want to bind parameters to
 * your query (if any).
 *
 * When using DynamicBinding (which is the default), parameter binding
 * to an actual sql statement is deferred until the query is run. This
 * has the advantage that you can compose the query definition using
 * helper methods provided in the query object (where(), orWhere(), groupBy(),
 * having() and orderBy()), possibly intermixing this with parameter
 * binding, and you can keep the query around and run the query
 * multiple times, perhaps with different parameter values or to scroll
 * through the query results. The where(), orWhere(), groupBy(), having(), and
 * orderBy() are merely convenience methods which you may use to
 * compose the querys incrementally, but you may just as well 
 * specify the entire SQL as a single string.
 *
 * When using DirectBinding, parameters are directly bound to an
 * underlying sql statement. Therefore, the query must be specified
 * entirely when created. Because of this reliance on an sql
 * statement, it can be run only once (one call to resultValue() or
 * resultList()) after which it should be discarded. You should not
 * try to keep a query object around when using this parameter binding
 * strategy (that will amost always not do what you would hope it to
 * do).
 *
 * \ingroup dbo
 */
template <class Result, typename BindStrategy = DynamicBinding>
class Query
#ifdef DOXYGEN_ONLY
  : public AbstractQuery
#endif
{
#ifdef DOXYGEN_ONLY
public:
  /*! \brief Default constructor.
   */
  Query();

  /*! \brief Destructor.
   */
  ~Query();

  /*! \brief Copy constructor.
   */
  Query(const Query& other);

  /*! \brief Assignment operator.
   */
  Query& operator= (const Query& other);

  /*! \brief Returns the result fields.
   */
  std::vector<FieldInfo> fields() const;

  /*! \brief Returns the session.
   */
  Session& session() const;

  /*! \brief Binds a value to the next positional marker.
   *
   * This binds the \p value to the next positional marker in the
   * query condition.
   */
  template<typename T>
  Query<Result, BindStrategy>& bind(const T& value);

  /*! \brief Resets bound values.
   *
   * This undoes all previous calls to bind().
   */
  void reset();

  /*! \brief Returns a unique result value.
   *
   * You can use this method if you are expecting the query to return
   * at most one result. If the query returns more than one result a
   * NoUniqueResultException is thrown.
   *
   * When using a DynamicBinding bind strategy, after a result has
   * been fetched, the query can no longer be used.
   */
  Result resultValue() const;

  /*! \brief Returns a result list.
   *
   * This returns a collection which is backed by the underlying query.
   * The query is not actually run until this collection is traversed
   * or its size is asked.
   *
   * When using a DynamicBinding bind strategy, after a result has
   * been fetched, the query can no longer be used.
   */
  collection< Result > resultList() const;

  /*! \brief Returns a unique result value.
   *
   * This is a convenience conversion operator that calls resultValue().
   */
  operator Result () const;

  /*! \brief Returns a result list.
   *
   * This is a convenience conversion operator that calls resultList().
   */
  operator collection< Result > () const;

  /** @name Methods for composing a query (DynamicBinding only)
   */
  //!@{
  /*! \brief Adds a join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& join(const std::string& other);

  /*! \brief Adds a left join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>left join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& leftJoin(const std::string& other);

  /*! \brief Adds a right join.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>right join</i> to the current query.
   *
   * The join should be a valid SQL join expression, e.g. 
   * "customer c on o.customer_id = c.id"
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& rightJoin(const std::string& other);
  
  /*! \brief Adds a query condition.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>where</i> condition expression to the
   * current query.
   *
   * The condition must be a valid SQL condition expression.
   *
   * Multiple conditions may be provided by successive calls to
   * where(), which must each be fulfilled, and are concatenated
   * together using 'and'.
   *
   * As with any part of the SQL query, a condition may contain
   * positional markers '?' to which values may be bound using bind().
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& where(const std::string& condition);

  /*! \brief Adds a query condition.
   *
   * This is a convenience method for creating a SQL query, and
   * concatenates a new <i>where</i> condition expression to the
   * current query.
   *
   * The condition must be a valid SQL condition expression.
   *
   * Multiple conditions may be provided by successive calls to
   * orWhere(), and are concatenated
   * together using 'or'.
   * Previous conditions will be surrounded by brackets and the
   * new condition will be concatenated using 'or'. For example:
   * \code
   *  query.where("column_a = ?").bind("A")
   *    .where("column_b = ?").bind("B")
   *    .orWhere("column_c = ?").bind("C");
   * \endcode
   * results in:
   * "where ((column_a = 'A') and (column_b = 'B')) or column_c = 'C'"
   *
   * As with any part of the SQL query, a condition may contain
   * positional markers '?' to which values may be bound using bind().
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& orWhere(const std::string& condition);

  /*! \brief Sets the result order.
   *
   * This is a convenience method for creating a SQL query, and sets an
   * <i>order by</i> field expression for the current query.
   *
   * Orders the results based on the given field name (or multiple
   * names, comma-separated).
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& orderBy(const std::string& fieldName);

  /*! \brief Sets the grouping field(s).
   *
   * This is a convenience method for creating a SQL query, and sets a
   * <i>group by</i> field expression for the current query.
   *
   * Groups results based on unique values of the indicated field(s),
   * which is a comma separated list of fields. Only fields on which
   * you group and aggregate functions can be selected by a query.
   *
   * A field that refers to a database object that is selected by the
   * query is expanded to all the corresponding fields of that
   * database object (as in the select statement).
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& groupBy(const std::string& fields);

  /*! \brief Sets the grouping filter(s).
   *
   * It's like where(), but for aggregate fields.
   *
   * For example you can't go:
   *
   *   select department.name, count(employees) from department
   *    where count(employees) > 5
   *    group by count(employees);
   *          
   * Because you can't have aggregate fields in a where clause, but you can go:
   *
   *   select department.name, count(employees) from department
   *    group by count(employees)
   *   having count(employees) > 5;
   *          
   * This will of course return all the departments with more than 5 employees
   * (and their employee count).
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   * \note You must have a group by clause, in order to have a 'having' clause
   */
  Query<Result, BindStrategy>& having(const std::string& fields);

  /*! \brief Sets a result offset.
   *
   * Sets a result offset. This has the effect that the next
   * resultList() call will skip as many results as the offset
   * indicates. Use -1 to indicate no offset.
   *
   * This provides the (non standard) <i>offset</i> part of an SQL query.
   *
   * \sa limit()
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& offset(int count);

  /*! \brief Returns an offset set for this query.
   *
   * \sa offset(int)
   */
  int offset() const;

  /*! \brief Sets a result limit.
   *
   * Sets a result limit. This has the effect that the next
   * resultList() call will return up to \p count results. Use -1 to
   * indicate no limit.
   *
   * This provides the (non standard) <i>limit</i> part of an SQL query.
   *
   * \sa offset()
   *
   * \note This method is not available when using a DirectBinding binding
   *       strategy.
   */
  Query<Result, BindStrategy>& limit(int count);

  /*! \brief Returns a limit set for this query.
   *
   * \sa limit(int)
   */
  int limit() const;

  //!@}

#endif // DOXYGEN_ONLY
 };

template <class Result>
class Query<Result, DirectBinding> : private Impl::QueryBase<Result>
{
public:
  using Impl::QueryBase<Result>::fields;
  using Impl::QueryBase<Result>::session;

  Query();
  ~Query();
  template<typename T> Query<Result, DirectBinding>& bind(const T& value);
  void reset();
  Result resultValue() const;
  collection< Result > resultList() const;
  operator Result () const;
  operator collection< Result > () const;

private:
  Query(Session& session, const std::string& sql);
  Query(Session& session, const std::string& table, const std::string& where);

  mutable int column_;
  mutable SqlStatement *statement_, *countStatement_;

  void prepareStatements() const;

  friend class Session;
};

template <class Result>
class Query<Result, DynamicBinding> : public AbstractQuery, private Impl::QueryBase<Result>
{
public:
  using Impl::QueryBase<Result>::fields;
  using Impl::QueryBase<Result>::session;
  using AbstractQuery::limit;
  using AbstractQuery::offset;

  Query();
  ~Query();
  Query(const Query& other);
  Query& operator= (const Query& other);
  template<typename T> Query<Result, DynamicBinding>& bind(const T& value);
  Query<Result, DynamicBinding>& join(const std::string& other);
  Query<Result, DynamicBinding>& leftJoin(const std::string& other);
  Query<Result, DynamicBinding>& rightJoin(const std::string& other);
  Query<Result, DynamicBinding>& where(const std::string& condition);
  Query<Result, DynamicBinding>& orWhere(const std::string& condition);
  Query<Result, DynamicBinding>& orderBy(const std::string& fieldName);
  Query<Result, DynamicBinding>& groupBy(const std::string& fields);
  Query<Result, DynamicBinding>& having(const std::string& fields);
  Query<Result, DynamicBinding>& offset(int count);
  Query<Result, DynamicBinding>& limit(int count);
  Result resultValue() const;
  collection< Result > resultList() const;
  operator Result () const;
  operator collection< Result > () const;

private:
  Query(Session& session, const std::string& sql);
  Query(Session& session, const std::string& table, const std::string& where);

  friend class Session;
  template <class C> friend class collection;
};

template <typename T>
AbstractQuery& AbstractQuery::bind(const T& value)
{
  parameters_.push_back(new Impl::Parameter<T>(value));

  return *this;
}

template <class Result>
template <typename T>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::bind(const T& value)
{
  AbstractQuery::bind(value);

  return *this;
}

  }
}

#endif // WT_DBO_QUERY
