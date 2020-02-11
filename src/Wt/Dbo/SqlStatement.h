// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SQL_STATEMENT_H_
#define WT_DBO_SQL_STATEMENT_H_

#include <string>
#include <vector>
#include <chrono>

#include <Wt/Dbo/SqlConnection.h>

namespace Wt {
  namespace Dbo {

/*! \brief Abstract base class for a prepared SQL statement.
 *
 * The statement may be used multiple times, but cannot be used
 * concurrently. It also cannot be copied.
 *
 * This class is part of Wt::Dbo's backend API, and should not be used
 * directly. Its interface must be reimplemented for each backend
 * corresponding to a supported database.
 *
 * \sa SqlConnection
 *
 * \ingroup dbo
 */
class WTDBO_API SqlStatement
{
public:
  /*! \brief Destructor.
   */
  virtual ~SqlStatement();

  /*! \brief Uses the statement.
   *
   * Marks the statement as in-use. If the statement is already in
   * use, return false. In that case, we will later provision that a
   * statement can be cloned and that a list of equivalent statement
   * is kept in the statement cache of a connectin.
   */
  bool use();

  /*! \brief Finish statement use.
   *
   * Marks the statement as no longer used and resets the statement.
   *
   * \sa use()
   */
  void done();

  /*! \brief Resets the statement.
   */
  virtual void reset() = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, const std::string& value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, short value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, int value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, long long value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, float value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, double value) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, const std::chrono::system_clock::time_point& value,
		    SqlDateTimeType type) = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, const std::chrono::duration<int, std::milli>& value)
    = 0;

  /*! \brief Binds a value to a column.
   */
  virtual void bind(int column, const std::vector<unsigned char>& value) = 0;

  /*! \brief Binds \c null to a column.
   */
  virtual void bindNull(int column) = 0;

  /*! \brief Executes the statement.
   */
  virtual void execute() = 0;

  /*! \brief Returns the id if the statement was an SQL <tt>insert</tt>.
   */
  virtual long long insertedId() = 0;

  /*! \brief Returns the affected number of rows.
   *
   * This is only useful for an SQL <tt>update</tt> or <tt>delete</tt>
   * statement.
   */
  virtual int affectedRowCount() = 0;

  /*! \brief Fetches the next result row.
   *
   * Returns \c true if there was one more row to be fetched.
   */
  virtual bool nextRow() = 0;

  /*! \brief Returns the number of columns in the result.
   *
   * \note The column count may only be available after the
   *       query was executed.
   */
  virtual int columnCount() const = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   * The size is the expected size of sql string type and can be used to
   * dimension buffers but the return string may be bigger.
   */
  virtual bool getResult(int column, std::string *value, int size) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, short *value) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, int *value) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, long long *value) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, float *value) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, double *value) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, std::chrono::system_clock::time_point *value,
			 SqlDateTimeType type) = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, std::chrono::duration<int, std::milli> *value)
    = 0;

  /*! \brief Fetches a result value.
   *
   * Returns \c true when the value was not \c null.
   */
  virtual bool getResult(int column, std::vector<unsigned char> *value,
			 int size) = 0;

  /*! \brief Returns the prepared SQL string.
   */
  virtual std::string sql() const = 0;

protected:
  SqlStatement();

private:
  SqlStatement(const SqlStatement&); // non-copyable

  bool inuse_;
};

class WTDBO_API ScopedStatementUse
{
public:
  ScopedStatementUse(SqlStatement *statement = nullptr);
  void operator()(SqlStatement *statement);
  ~ScopedStatementUse();

private:
  SqlStatement *s_;
};

  }
}

#endif // WT_DBO_SQL_STATEMENT_H_
