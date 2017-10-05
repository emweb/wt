// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SESSION_H_
#define WT_DBO_SESSION_H_

#include <map>
#include <set>
#include <string>
#include <typeinfo>

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/Query.h>
#include <Wt/Dbo/Transaction.h>
#include <Wt/Dbo/SqlConnection.h>

namespace Wt {
  namespace Dbo {
    namespace Impl {
      struct MetaDboBaseSet;

      extern WTDBO_API std::string quoteSchemaDot(const std::string& table);
      template <class C, typename T> struct LoadHelper;

      struct WTDBO_API SetInfo {
	enum SetInfoFlags {
	  // Normally, if there is no surrogate key, the field name of the natural id
	  // is appended to joinSelfId/joinOtherId. These flags prevent that.
	  LiteralSelfId = 0x1, // joinSelfId is literal, don't append primary key name
	  LiteralOtherId = 0x2 // joinOtherId is literal, don't append primary key name
	};

	const char *tableName;
	std::string joinName;
	std::string joinSelfId, joinOtherId;
	int flags;
	RelationType type;
	int fkConstraints, otherFkConstraints;

	SetInfo(const char *aTableName, RelationType type,
		const std::string& aJoinName,
		const std::string& aJoinSelfId,
		int someFkConstraints);
      };

      struct WTDBO_API MappingInfo {
	bool initialized_;
	const char *tableName;
	const char *versionFieldName;
	const char *surrogateIdFieldName;

	std::string naturalIdFieldName; // for non-auto generated id
	int naturalIdFieldSize;         // for non-auto generated id

	std::string idCondition;

	std::vector<FieldInfo> fields;
	std::vector<SetInfo> sets;

	std::vector<std::string> statements;

	MappingInfo();
	virtual ~MappingInfo();
	virtual void init(Session& session);
	virtual void dropTable(Session& session,
			       std::set<std::string>& tablesDropped);
	virtual void rereadAll();
	virtual MetaDboBase *create(Session& session);
	virtual void load(Session& session, MetaDboBase *obj);
	virtual MetaDboBase *load(Session& session, SqlStatement *statement,
				  int& column);

	std::string primaryKeys() const;
      };
    }

struct Null {
  static Null null_;
};

/*! \brief Enumeration that indicates the flush mode.
 *
 * \sa setFlushMode(), discardUnflushed()
 */
enum class FlushMode {
  Auto,    //!< Dbo decides when to flush changes to a transaction
  Manual   //!< Changes are never automatically flushed
};

class Call;
//class SqlConnection;
class SqlConnectionPool;
class SqlStatement;
template <typename Result, typename BindStrategy> class Query;
struct DirectBinding;
struct DynamicBinding;

/*! \class Session Wt/Dbo/Session.h Wt/Dbo/Session.h
 *  \brief A database session.
 *
 * A database session manages meta data about the mapping of C++
 * classes to database tables, and keeps track of a working set of
 * in-memory objects (objects which are referenced from your code or
 * from within a transaction).
 *
 * It also manages an active transaction, which you need to access
 * database objects.
 *
 * You can provide the session with a dedicated database connection
 * using setConnection(), or with a connection pool (from which it
 * will take a connection while processing a transaction) using
 * setConnectionPool().
 *
 * A session will typically be a long-lived object in your
 * application.
 *
 * \ingroup dbo
 */
class WTDBO_API Session
{
public:
  /*! \brief Creates a database session.
   */
  Session();

  /*! \brief Destructor.
   *
   * A session must survive all database objects that have been loaded
   * through it, and will warning during this destructor if there are
   * still database objects that are being referenced from a ptr.
   */
  virtual ~Session();

  // Sessions are not copyable
  Session(const Session &) = delete;
  Session& operator=(const Session &) = delete;

  // Sessions are not movable
  Session(Session &&) = delete;
  Session& operator=(Session &&) = delete;

  /*! \brief Sets a dedicated connection.
   *
   * The connection will be used exclusively by this session.
   *
   * \sa setConnectionPool()
   */
  void setConnection(std::unique_ptr<SqlConnection> connection);

  /*! \brief Sets a connection pool.
   *
   * The connection pool is typically shared with other sessions.
   *
   * \sa setConnection()
   */
  void setConnectionPool(SqlConnectionPool& pool);

  /*! \brief Maps a class to a database table.
   *
   * The class \p C is mapped to table with name \p tableName. You
   * need to map classes to tables.
   *
   * You may provide a schema-qualified table name, if the underlying
   * database supports this, eg. <tt>"myschema.users"</tt>.
   */
  template <class C> void mapClass(const char *tableName);

  /*! \brief Returns the mapped table name for a class.
   *
   * \sa mapClass(), tableNameQuoted()
   */
  template <class C> const char *tableName() const;

  /*! \brief Returns the mapped quoted table name for a class.
   *
   * This will quote schemas, as necessary.
   *
   * \sa mapClass(), tableName()
   */
  template <class C> const std::string tableNameQuoted() const;

  /*! \brief Persists a transient object.
   *
   * The transient object pointed to by \p ptr is added to the
   * session, and will be persisted when the session is flushed.
   *
   * A transient object is usually a newly created object which want
   * to add to the database.
   *
   * The method returns \p ptr.
   */
  template <class C> ptr<C> add(ptr<C>& ptr);

  /*! \brief Persists a transient object.
   *
   * This is an overloaded method for convenience, and is implemented as:
   * \code
   * return add(ptr<C>(std::move(obj)));
   * \endcode
   *
   * The method returns a database pointer to the object.
   */
  template <class C> ptr<C> add(std::unique_ptr<C> obj);

  /*! \brief Persists a transient object.
   *
   * This an overloaded method for convenience, and is implemented as:
   * \code
   * return add(ptr<C>(std::unique_ptr<T>(new T(...))))
   * \endcode
   *
   * \sa Wt::Dbo::make_ptr()
   */
  template<typename T, typename ...Args>
  ptr<T> addNew( Args&& ...args )
  {
    return add(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
  }

  /*! \brief Loads a persisted object.
   *
   * This method returns a database object with the given object
   * id. If the object was already loaded in the session, the loaded
   * object is returned, otherwise the object is loaded from the
   * database.
   *
   * If \p forceReread is set to \c true, then a fresh copy is loaded
   * from the database. This is almost equivalent to calling \link
   * ptr<C>::reread() reread()\endlink on the returned object, except
   * that it will not result in two database reads in case the object was
   * in fact not yet loaded in the session.
   *
   * Throws an ObjectNotFoundException when the object was not found.
   *
   * \sa ptr::id(), loadLazy()
   */
  template <class C> ptr<C> load(const typename dbo_traits<C>::IdType& id,
				 bool forceReread = false);

  /*! \brief Lazy loads a persisted object.
   *
   * This method returns a database object with the given object id
   * without directly accessing the database.
   *
   * If the object data is already available in the session, then it will
   * be available upon return. Otherwise, the object data will be retrieved
   * from the database on first access. Note: This will result in an
   * ObjectNotFoundException if the object id is not valid.
   *
   * lazyLoad can be used to obtain a ptr<C> from a known id when a ptr<C>
   * is required, but access to the C object is not anticipated. For
   * instance, a ptr<C> may be required to add an object of class X that
   * is in a belongsTo relationship with C.
   *
   * \sa ptr::id(), load()
   */
  template <class C> ptr<C> loadLazy(const typename dbo_traits<C>::IdType& id);

#ifndef DOXYGEN_ONLY
  template <class C>
    Query< ptr<C> > find(const std::string& condition = std::string()) {
    // implemented in-line because otherwise it crashes gcc 4.0.1
    return find<C, DynamicBinding>(condition);
  }
#endif // DOXYGEN_ONLY

  /*! \brief Finds database objects.
   *
   * This method creates a query for finding objects of type \p C.
   *
   * When passing an empty \p condition parameter, it will return all
   * objects of type \p C. Otherwise, it will add the condition, by
   * generating an SQL <i>where</i> clause.
   *
   * The \p BindStrategy specifies how you want to bind parameters to
   * your query (if any).
   *
   * When using \p DynamicBinding (which is the default), you will
   * defer the binding until the query is run. This has the advantage
   * that you can compose the query definition using helper methods
   * provided in the query object, you can keep the query around and
   * run the query multiple times, perhaps with different parameter
   * values or to scroll through the query results.
   *
   * When using \p DirectBinding, the query must be specified entirely
   * using the \p condition, and can be run only once. This method
   * does have the benefit of binding parameters directly to the
   * underlying prepared statement.
   *
   * This method is convenient when you are querying only results from a
   * single table. For more generic query support, see query().
   *
   * Usage example:
   * \code
   * // Bart is missing, let's find him.
   * Wt::Dbo::ptr<User> bart = session.find<User>().where("name = ?").bind("Bart");
   *
   * // Find all users, order by name
   * typedef Wt::Dbo::collection< Wt::Dbo::ptr<User> > Users;
   * Users users = session.find<User>().orderBy("name");
   * \endcode
   *
   * In the \p condition, parameters can be bound using '?' as a
   * positional placeholder: each occurence of '?' (as a lexical
   * token) is replaced by a bound parameter. This is actually done by
   * most of the backends themselves using prepared statements and
   * parameter binding. Parameter binding is possible for all types
   * for which sql_value_traits is specialized.
   *
   * \sa query()
   */
#ifdef DOXYGEN_ONLY
  template <class C, typename BindStrategy = DynamicBinding>
#else
  template <class C, typename BindStrategy>
#endif
    Query< ptr<C>, BindStrategy>
    find(const std::string& condition = std::string());

#ifndef DOXYGEN_ONLY
  template <class Result> Query<Result> query(const std::string& sql);
#endif // DOXYGEN_ONLY

  /*! \brief Creates a query.
   *
   * The sql statement should be a complete SQL statement, starting
   * with a "select ". The items listed in the "select" must match the
   * \p Result type. An item that corresponds to a database object
   * (ptr) is substituted with the selection of all the fields in the
   * dbo.
   *
   * For example, the following query (class User is mapped onto table 'user'):
   * \code
   * session.query< ptr<User> >("select u from user u").where("u.name = ?").bind("Bart");
   * \endcode
   * is the more general version of:
   * \code
   * session.find<User>().where("name = ?").bind("Bart");
   * \endcode
   *
   * Note that "u" in this query will be expanded to select the fields of the
   * user table (u.id, u.version, u.name, ...). The same expansion happens when
   * using an alias in Query::groupBy().
   *
   * The additional flexibility offered by %query() over find() is
   * however that it may support other result types.
   *
   * Thus, it may return plain values:
   * \code
   * session.query<int>("select count(1) from ...");
   * \endcode
   *
   * Or std::tuple for an arbitrary combination of result values:
   *
   * \code
   * session.query< std::tuple<int, int> >("select A.id, B.id from table_a A, table_b B").where("...");
   * \endcode
   *
   * A tuple may combine any kind of object that is supported as a result,
   * including database objects (see also ptr_tuple):
   * \code
   * session.query< std::tuple<ptr<A>, ptr<B> > >("select A, B from table_a A, table_b B").where("...");
   * \endcode
   *
   * The \p BindStrategy specifies how you want to bind parameters to
   * your query (if any).
   *
   * When using \p DynamicBinding (which is the default), you will
   * defer the binding until the query is run. This has the advantage
   * that you can compose the query using helper methods provided in
   * the Query object, you can keep the query around and run the query
   * multiple times, perhaps with different parameter values or to
   * scroll through the query results.
   *
   * When using \p DirectBinding, the query must be specified entirely
   * using the \p sql, and can be run only once. This method does have
   * the benefit of binding parameters directly to the underlying
   * prepared statement.
   *
   * This method uses query_result_traits to unmarshal the query result
   * into the \p Result type.
   *
   * In the \p sql query, parameters can be bound using '?' as the
   * positional placeholder: each occurence of '?' (as a lexical
   * token) is replaced by a bound parameter. This is actually done by
   * most of the backends themselves using prepared statements and
   * parameter binding. Parameter binding is possible for all types
   * for which sql_value_traits is specialized.
   *
   * \note The query must be a ASCII-7 string: UTF-8 is not supported by
   * the underlying query parser. To add a non-English string to the query
   * use parameter binding instead (which prevents against SQL injection
   * attacks at the same time) instead of string concatenation.
   */
#ifdef DOXYGEN_ONLY
  template <class Result, typename BindStrategy = DynamicBinding>
#else
  template <class Result, typename BindStrategy>
#endif
    Query<Result, BindStrategy> query(const std::string& sql);

  /*! \brief Executs an Sql command.
   *
   * This executs an Sql command. It differs from query() in that no
   * result is expected from the call.
   *
   * Usage example:
   * \code
   * session.execute("update user set name = ? where name = ?").bind("Bart").bind("Sarah");
   * \endcode
   */
  Call execute(const std::string& sql);

  /*! \brief Creates the database schema.
   *
   * This will create the database schema of the mapped tables. Schema
   * creation will fail if one or more tables already existed. The creation
   * of the tables is executed in a transaction that is rolled back when
   * an error occurs.
   *
   * This method throws an Wt::Dbo::Exception if the table creation failed.
   *
   * \sa mapClass(), dropTables()
   */
  void createTables();

  /*! \brief Returns database creation SQL.
   */
  std::string tableCreationSql();

  /*! \brief Drops the database schema.
   *
   * This will drop the database schema. Dropping the schema will fail
   * if one or more tables did not exist.
   *
   * \sa createTables()
   */
  void dropTables();

  /*! \brief Flushes the session.
   *
   * This flushes all modified objects to the database. This does not
   * commit the transaction.
   *
   * Normally, you need not to call this method as the session is
   * flushed automatically before committing a transaction, or before
   * running a query (to be sure to take into account pending
   * modifications).
   */
  void flush();

  /*! \brief Rereads all objects.
   *
   * This rereads all objects from the database, possibly discarding
   * unflushed modifications. This is a catch-all solution for a
   * StaleObjectException.
   *
   * If a \p tableName is given, then only objects of that table are
   * reread.
   *
   * \sa ptr::reread()
   */
  void rereadAll(const char *tableName = nullptr);

  /*! \brief Discards all unflushed changes.
   *
   * This method is useful when the flushMode() is set to Manual. It discards
   * all Dbo-objects which were added to the session and rereads all existing
   * Dbo-objects.
   *
   * \sa setFlushMode()
   */
  void discardUnflushed();

  void getFields(const char *tableName, std::vector<FieldInfo>& result);

  /*! \brief Returns the flushMode.
   *
   * \sa setFlushMode()
   */
  FlushMode flushMode() { return flushMode_; }

  /*! \brief Sets the flushMode.
   *
   * The default flushMode is Auto. LabelOption::Inside a transaction this means that
   * changes are flushed when a query is affected by them. When flushMode is
   * set to Manual, changes are only flushed when the user manually calls
   * flush(), or resets the mode to Auto. Query's wil possibly return an
   * inconsistent result, but collections will still keep track of changes.
   * This also makes it possible to operate on Dbo-objects and collections
   * outside of a transaction. When the moment comes to flush the changes, a
   * transaction must of course be active.
   *
   * <em>Note:</em> only operations on a collecion are tracked in Manual mode,
   * reciproke operations are not yet taken into account.
   *
   * \sa flushMode(), discardUnflushed()
   */
  void setFlushMode(FlushMode mode) { flush(); flushMode_ = mode; }

private:
  mutable std::string longlongType_;
  mutable std::string intType_;
  mutable bool haveSupportUpdateCascade_;

  enum { SqlInsert = 0,
	 SqlUpdate = 1,
	 SqlDelete = 2,
	 SqlDeleteVersioned = 3,
	 SqlSelectById = 4,
	 FirstSqlSelectSet = 5 };

  struct JoinId {
    std::string joinIdName;
    std::string tableIdName;
    std::string sqlType;

    JoinId(const std::string& aJoinIdName,
	   const std::string& aTableIdName,
	   const std::string& aSqlType);
  };

  template <class C>
  struct Mapping : public Impl::MappingInfo
  {
    typedef std::map<typename dbo_traits<C>::IdType, MetaDbo<C> *> Registry;
    Registry registry_;

    virtual ~Mapping();
    virtual void init(Session& session) override;
    virtual void dropTable(Session& session,
			   std::set<std::string>& tablesDropped) override;
    virtual void rereadAll() override;
    virtual MetaDbo<C> *create(Session& session) override;
    virtual void load(Session& session, MetaDboBase *obj) override;
    virtual MetaDbo<C> *load(Session& session, SqlStatement *statement,
			     int& column) override;
  };
  
  typedef const std::type_info * const_typeinfo_ptr;
  struct typecomp {
    bool operator() (const const_typeinfo_ptr& lhs, const const_typeinfo_ptr& rhs) const { return lhs->before(*rhs) != 0;
	}
  };
  
  typedef std::map<const_typeinfo_ptr,
		   Impl::MappingInfo *, typecomp> ClassRegistry;
  typedef std::map<std::string, Impl::MappingInfo *> TableRegistry;

  ClassRegistry classRegistry_;
  TableRegistry tableRegistry_;
  bool schemaInitialized_;
  mutable LimitQuery limitQueryMethod_;
  mutable bool requireSubqueryAlias_;

  Impl::MetaDboBaseSet *dirtyObjects_;
  std::vector<MetaDboBase*> objectsToAdd_;
  std::unique_ptr<SqlConnection> connection_;
  SqlConnectionPool *connectionPool_;
  Transaction::Impl *transaction_;
  FlushMode flushMode_;

  void initSchema() const;
  void resolveJoinIds(Impl::MappingInfo *mapping);
  void prepareStatements(Impl::MappingInfo *mapping);

  void executeSql(std::vector<std::string> &sql, std::ostream *sout);
  void executeSql(std::stringstream &sql, std::ostream *sout);
  std::string constraintName(const char *tableName,
                             std::string foreignKeyName);

  std::vector<JoinId> getJoinIds(Impl::MappingInfo *mapping, 
				 const std::string& joinId,
				 bool literalJoinId);

  void createTable(Impl::MappingInfo *mapping,
		   std::set<std::string>& tablesCreated,
                   std::ostream *sout,
                   bool createConstraints);
  void createRelations(Impl::MappingInfo *mapping,
		       std::set<std::string>& tablesCreated,
		       std::ostream *sout);
  std::string constraintString(Impl::MappingInfo *mapping,
                               const FieldInfo& field,
                               unsigned fromIndex,
                               unsigned toIndex);
  unsigned findLastForeignKeyField(Impl::MappingInfo *mapping,
                                   const FieldInfo& field,
                                   unsigned index);
  void createJoinTable(const std::string& joinName,
		       Impl::MappingInfo *mapping1, Impl::MappingInfo *mapping2,
		       const std::string& joinId1,
		       const std::string& joinId2,
		       int fkConstraints1, int fkConstraints2,
		       bool literalJoinId1, bool literalJoinId2,
		       std::set<std::string>& tablesCreated,
		       std::ostream *sout);
  void addJoinTableFields(Impl::MappingInfo& joinTableMapping,
			  Impl::MappingInfo *mapping, const std::string& joinId,
			  const std::string& foreignKeyName, int fkConstraints,
			  bool literalJoinId);
  void createJoinIndex(Impl::MappingInfo& joinTableMapping,
		       Impl::MappingInfo *mapping,
		       const std::string& joinId,
		       const std::string& foreignKeyName,
		       std::ostream *sout);

  void needsFlush(MetaDboBase *dbo);

  template <class C> Mapping<C> *getMapping() const;
  Impl::MappingInfo *getMapping(const char *tableName) const;

  void load(MetaDboBase *obj);
  template <class C> ptr<C> load(SqlStatement *statement, int& column);

  template <class C>
    MetaDbo<C> *loadWithNaturalId(SqlStatement *statement, int& column);
  template <class C>
    MetaDbo<C> *loadWithLongLongId(SqlStatement *statement, int& column);

  void discardChanges(MetaDboBase *obj);
  template <class C> void prune(MetaDbo<C> *obj);

  template<class C> void implSave(MetaDbo<C>& dbo);
  template<class C> void implDelete(MetaDbo<C>& dbo);
  template<class C> void implTransactionDone(MetaDbo<C>& dbo, bool success);
  template<class C> void implLoad(MetaDbo<C>& dbo, SqlStatement *statement,
				  int& column);

  static std::string statementId(const char *table, int statementIdx);

  template <class C> SqlStatement *getStatement(int statementIdx);
  SqlStatement *getStatement(const std::string& id);
  SqlStatement *getStatement(const char *tableName, int statementIdx);
  const std::string& getStatementSql(const char *tableName, int statementIdx);

  SqlStatement *prepareStatement(const std::string& id,
				 const std::string& sql);
  SqlStatement *getOrPrepareStatement(const std::string& sql);

  template <class C> void prepareStatements();
  template <class C> std::string manyToManyJoinId(const std::string& joinName,
						  const std::string& notId);

  std::unique_ptr<SqlConnection> useConnection();
  void returnConnection(std::unique_ptr<SqlConnection> connection);
  SqlConnection *connection(bool openTransaction);

  MetaDboBase *createDbo(Impl::MappingInfo *mapping);

  template <class C> friend class MetaDbo;
  template <class C> friend class collection;
  template <class C> friend class weak_ptr;
  template <class C, typename S> friend class Query;
  template <class C> friend class Impl::QueryBase;
  template <class C, typename T> friend struct Impl::LoadHelper;
  template <typename V> friend class FieldRef;
  template <class C> friend struct query_result_traits;
  template <class C> friend class SaveDbAction;
  template <class C> friend class LoadDbAction;
  template <class C> friend class PtrRef;
  friend class SetReciproceAction;
  friend class ToAnysAction;
  friend class FromAnysAction;

  friend class Call;
  friend class CollectionHelper;
  friend class DboAction;
  friend class DropSchema;
  friend class FromAnyAction;
  friend class InitSchema;
  friend class LoadBaseAction;
  friend class MetaDboBase;
  friend class SaveBaseAction;
  friend class SessionAddAction;
  friend class Transaction;
  friend class TransactionDoneAction;

  friend struct Transaction::Impl;
};

  }
}

#endif // WT_SESSION_H_
