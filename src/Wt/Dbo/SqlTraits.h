// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SQL_TRAITS_H_
#define WT_DBO_SQL_TRAITS_H_

#include <string>
#include <vector>

#include <Wt/Dbo/WDboDllDefs.h>
#include <Wt/cpp17/any.hpp>

namespace Wt {
  namespace Dbo {
    namespace Impl {
      // Hiding boost usage
      extern WTDBO_API std::string::const_iterator ifind_last_as(const std::string &name);
    }

class Session;
class SqlStatement;

/*! \class sql_value_traits Wt/Dbo/SqlTraits.h Wt/Dbo/SqlTraits.h
 *  \brief Traits class for value types.
 *
 * This traits class may be specialized for a custom type \p V, to add
 * dbo support for custom types. A value type has a one-to-one mapping
 * to a single database column.
 *
 * The library has built-in support for:
 *  - <tt>std::string</tt>
 *  - <tt>char const *</tt> (read-only: only as a bound parameter for a query)
 *  - <tt>short</tt>, <tt>int</tt>, <tt>long long</tt>
 *  - <tt>long</tt>:  since the size of a <tt>long</tt> is 64bit on
 *    UNIX/Linux 64bit systems and 32bit otherwise, it is mapped to an
 *    <tt>integer</tt> or a <tt>bigint</tt> depending on the environment.
 *  - <tt>float</tt>, <tt>double</tt>
 *  - enum types
 *  - <tt>bool</tt>
 *  - <tt>std::vector<unsigned char></tt> (binary data)
 *  - <tt>boost::optional<T></tt>: to make the type optional
 *    (allowing an SQL <tt>null</tt> value)
 *  - <tt>boost::posix_time::ptime</tt>: time stamp, an invalid value (e.g.
 *    default constructed), maps to <tt>null</tt>
 *  - <tt>boost::posix_time::time_duration</tt>: time interval, an invalid
 *    value (boost::posix_time::not_a_date_time), maps to <tt>null</tt>
 *
 * In <Wt/Dbo/WtSqlTraits>, traits classes are also provided for:
 *  - WDate
 *  - WDateTime
 *  - WTime
 *  - WString
 *  - Json::Object
 *  - Json::Array
 *
 * \sa query_result_traits
 *
 * \ingroup dbo
 */
template <typename V, class Enable = void>
struct sql_value_traits
{
  static const bool not_specialized = true;

#ifdef DOXYGEN_ONLY
  /*! \brief Returns the SQL type name.
   *
   * The \p size (for strings) is a hint and may be ignored by a back-end.
   *
   * This will usually return a type ending with " not null" except
   * for C++ types that support \c null values. For a normal c++ value
   * type \p T, boost::optional<T> has been specialized to allow for
   * \c null values.
   */
  static const char *type(SqlConnection *connection, int size);

  /*! \brief Binds a value to a statement parameter.
   *
   * The value \p v must be bound to parameter with index \p index in the
   * \p statement.
   *
   * \sa SqlStatement::bind()
   */
  static void bind(const V& v, SqlStatement *statement, int index, int size);

  /*! \brief Reads a result from an executed query.
   *
   * The value \p v must be read from result column \p column in the \p
   * statement.
   *
   * Returns \c true if the value was not \c null. This result may be
   * used by the boost::optional<V> specialization to support fields that
   * may have \c null values.
   *
   * \sa SqlStatement::getResult()
   */
  static bool read(V& v, SqlStatement *statement, int column, int size);
#endif // DOXYGEN_ONLY

  static void bind(const char *v, SqlStatement *statement, int column,
		   int size);
};

/*! \brief Flags
 */
enum FieldFlags {
  SurrogateId = 0x1, //!< Field is a surrogate id
  NaturalId = 0x2,   //!< Field is (part of) a natural id
  Version = 0x4,     //!< Field is an optimistic concurrency version field
  Mutable = 0x8,     //!< Field can be edited
  NeedsQuotes = 0x10,//!< Field name needs quotes when using in SQL
  ForeignKey = 0x20, //!< Field is (part of) a foreign key
  FirstDboField = 0x40,
  LiteralJoinId = 0x80,
  AuxId = 0x81,
  AliasedName = 0x100 // there is an AS in the field, so the name is aliased
};

/*! \class FieldInfo Wt/Dbo/SqlTraits.h Wt/Dbo/SqlTraits.h
 *  \brief Description of a field.
 *
 * \sa query_result_traits::getFields(), Query::fields()
 *
 * \ingroup dbo
 */
class WTDBO_API FieldInfo
{
public:
  /*! \brief Creates a field description.
   */
  FieldInfo(const std::string& name, const std::type_info *type,
	    const std::string& sqlType, int flags);

  /*! \brief Creates a field description.
   */
  FieldInfo(const std::string& name, const std::type_info *type,
	    const std::string& sqlType,
	    const std::string& foreignKeyTable,
	    const std::string& foreignKeyName,
	    int flags, int fkConstraints);

  /*! \brief Sets a qualifier for the field.
   */
  void setQualifier(const std::string& qualifier, bool firstQualified = false);

  /*! \brief Returns the field name.
   */
  const std::string& name() const { return name_; }

  /*! \brief Returns the field SQL type.
   */
  const std::string& sqlType() const { return sqlType_; }

  /*! \brief Returns the field qualifier.
   */
  const std::string& qualifier() const { return qualifier_; }

  /*! \brief Returns the field type.
   */
  const std::type_info *type() const { return type_; }

  /*! \brief Returns whether the field is an Id field.
   */
  bool isIdField() const { return (flags_ & (SurrogateId | NaturalId)) != 0; }

  /*! \brief Returns whether the field is a Natural Id field.
   */
  bool isNaturalIdField() const { return (flags_ & NaturalId) != 0; }

  /*! \brief Returns whether the field is a Surroaget Id field.
   */
  bool isSurrogateIdField() const { return flags_ & SurrogateId; }

  /*! \brief Returns whether the field is a Version field.
   */
  bool isVersionField() const { return (flags_ & Version) != 0; }

  /*! \brief Returns whether the field is mutable.
   */
  bool isMutable() const { return (flags_ & Mutable) != 0; }

  /*! \brief Returns whether the field name needs to be quoted.
   */
  bool needsQuotes() const { return (flags_ & NeedsQuotes) != 0; }

  /*! \brief Returns whether the field is part of a foreign key.
   */
  bool isForeignKey() const { return (flags_ & ForeignKey) != 0; }

  bool isFirstDboField() const { return (flags_ & FirstDboField) != 0; }
  bool literalJoinId() const { return (flags_ & LiteralJoinId) != 0; }
  bool isAuxIdField() const { return (flags_ & AuxId) != 0; }
  bool isAliasedName() const { return (flags_ & AliasedName) != 0; }
  std::string foreignKeyName() const { return foreignKeyName_; }
  std::string foreignKeyTable() const { return foreignKeyTable_; }
  int fkConstraints() const { return fkConstraints_; }

  std::string sql() const;

private:
  std::string name_, sqlType_, qualifier_;
  std::string foreignKeyName_, foreignKeyTable_;
  const std::type_info *type_;
  int flags_;
  int fkConstraints_;
};

/*! \class query_result_traits Wt/Dbo/SqlTraits.h Wt/Dbo/SqlTraits.h
 *  \brief Traits class for result types.
 *
 * This traits class may be used to add support for using classes or structs
 * as a result for a Session::query().
 *
 * The library provides by default support for primitive types, using
 * sql_value_traits, mapped objects held by ptr types, and
 * boost::tuple<> of any combination of these.
 *
 * \sa sql_value_traits, ptr
 *
 * \ingroup dbo
 */
template <typename Result>
struct query_result_traits
{
  /*! \brief Obtains the list of fields in this result.
   *
   * This is used to build the <i>select</i> clause of an Sql query.
   *
   * The given \p aliases may be used to qualify fields that correspond to
   * entire tables (popping values from the front of this vector). An
   * exception is thrown if not enough aliases were provided.
   *
   * This method is needed when you want to use Result as the result
   * of query.
   */
  static void getFields(Session& session,
			std::vector<std::string> *aliases,
			std::vector<FieldInfo>& result);

  /*! \brief Reads a result from an executed query.
   *
   * This reads the value from the \p statement, starting at column \p
   * column, and advancing the column pointer for as many columns as
   * needed (and according to the number of fields returned by
   * getFields()).
   *
   * This method is needed when you want to use Result as the result
   * of query.
   */
  static Result load(Session& session, SqlStatement& statement,
		     int& column);

  /*! \brief Returns result values.
   *
   * This returns the individual field values in the given \p result.
   *
   * This method needs to be implemented only if you want to display
   * the result in a QueryModel (which implements Wt's MVC item
   * model).
   */
  static void getValues(const Result& result, std::vector<cpp17::any>& values);

  /*! \brief Sets a result value.
   *
   * Sets the value at \p index, where index indicates the field whose
   * value needs to be updated.
   *
   * When \p index is out-of-bounds, it should be decremented with as
   * many fields as there are in this result type. Otherwise, index
   * should be set to -1 after the value has been set.
   *
   * This method needs to be implemented only if you want to modify
   * the result from a QueryModel (which implements Wt's MVC item
   * model).
   */
  static void setValue(Result& result, int& index, const cpp17::any& value);

  /*! \brief Creates a new result.
   *
   * Creates a new result. This should initialize a result so that its values
   * can be set using setValue() or read using getValues().
   *
   * The result should not yet be associated with a session.
   *
   * This method needs to be implemented only if you want to create
   * new results from a QueryModel (which implements Wt's MVC item
   * model).
   */
  static Result create();

  /*! \brief Adds a new result to the session.
   *
   * Adds a (newly created) result to a session.
   *
   * This method needs to be implemented only if you want to create
   * new results from a QueryModel (which implements Wt's MVC item
   * model).
   */
  static void add(Session& session, Result& result);

  /*! \brief Removes a result from the session.
   *
   * This method needs to be implemented only if you want to remove
   * results from a QueryModel (which implements Wt's MVC item model).
   */
  static void remove(Result& result);

  /*! \brief Returns a unique id for a result.
   *
   * This method needs to be implemented to return a unique id for a result
   * which can later be used to find the result.
   *
   * If not supported, return -1.
   *
   * \sa findbyId()
   */
  static long long id(const Result& result);

  /*! \brief Find a result by id.
   *
   * This needs to be inverse of id()
   */
  static Result findById(Session& session, long long id);
};

  }
}

#endif // WT_DBO_SQL_TRAITS
