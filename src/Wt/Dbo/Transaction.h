// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_TRANSACTION_H_
#define WT_DBO_TRANSACTION_H_

#include <memory>
#include <vector>
#include <Wt/Dbo/WDboDllDefs.h>

namespace Wt {
  namespace Dbo {

class Session;
class SqlConnection;

class ptr_base;

/*! \class Transaction Wt/Dbo/Transaction.h Wt/Dbo/Transaction.h
 *  \brief A database transaction.
 * 
 * This class implements a RAII transaction. Most dbo manipulations
 * require that a transaction is active, and database changes will not
 * be committed to the database until the active transaction is
 * committed. A transaction can be committed using commit(), but also
 * commits automatically when it is deleted while no exception is
 * being thrown (using std::uncaught_exception()). This means that it
 * is possible that the transaction destructor throws an exception in
 * case the transaction still needs to be committed, and the commit
 * fails. If the transaction is deleted because of stack unwinding while
 * an exception is thrown, then the transaction rolls back.
 *
 * A transaction is active until it is either committed or rolled
 * back. When a transaction is rolled back or fails, the modified
 * database objects are not successfully synchronized with the
 * database. A roll-back does not affect the value of the in memory
 * database objects, so they may possibly be synchronized later in a
 * new transaction or discarded using Session::rereadAll().
 *
 * In most occasions you will want to guard any method that touches
 * the database using a transaction object on the stack.
 *
 * But you may create multiple (nested) transaction objects at the
 * same time: in this way you can guard a method with a transaction
 * object even if it is called from another method which also defines
 * a transaction at a wider scope. Nested transactions act in
 * concert and reference the same logical transaction: the logical
 * transaction will fail if at least one transaction fails, and will
 * be committed only if all transactions are committed.
 *
 * Usage example:
 * \code
 * void doSomething(Wt::Dbo::Session& session)
 * {
 *   Wt::Dbo::Transaction transaction(session);
 *
 *   Wt::Dbo::ptr<Account> a = session.load<Account>(42);
 *   ...
 *
 *   // the transaction will roll back if an exception is thrown, or
 *   // commit otherwise
 * }
 * \endcode
 *
 * \ingroup dbo
 */
class WTDBO_API Transaction
{
public:
  /*! \brief Constructor.
   *
   * Opens a transaction for the given \p session. If a transaction is
   * already open for the session, this transaction is added. All open
   * transactions must commit successfully for the entire transaction to
   * succeed.
   */
  explicit Transaction(Session& session);

  /*! \brief Destructor.
   *
   * If the transaction is still active, it is rolled back.
   */
  virtual ~Transaction() noexcept(false);

  // Transactions are not copyable
  Transaction(const Transaction&) = delete;
  Transaction& operator=(const Transaction&) = delete;

  // Transactions are not movable
  Transaction(Transaction&&) = delete;
  Transaction& operator=(Transaction&&) = delete;

  /*! \brief Returns whether the transaction is still active.
   *
   * A transaction is active unless it has been committed or rolled
   * back.
   *
   * While a transaction is active, new transactions for the same
   * session are treated as nested transactions.
   */
  bool isActive() const;

  /*! \brief Commits the transaction.
   *
   * If this is the last open transaction for the session, the session
   * is flushed and pending changes are committed to the database.
   *
   * Returns whether the transaction was flushed to the database
   * (i.e. whether this was indeed the last open transaction).
   *
   * \sa rollback()
   */
  bool commit();

  /*! \brief Rolls back the transaction.
   *
   * The transaction is rolled back (if it was still active), and is no
   * longer active.
   *
   * \sa commit()
   */
  void rollback();

  /*! \brief Returns the session associated with this transaction.
   *
   * \sa Transaction()
   */
  Session& session() const;

  /*! \brief Returns the connection used by this transaction
   */
  SqlConnection *connection() const;

private:
  struct Impl {
    Session& session_;
    bool active_;
    bool needsRollback_;
    bool open_;

    int transactionCount_;
    std::vector<ptr_base *> objects_;

    std::unique_ptr<SqlConnection> connection_;

    void open();
    void commit();
    void rollback();

    Impl(Session& session_);
    ~Impl();
  };

  bool committed_;
  Session& session_;
  Impl *impl_;

  friend class Session;

  void release();
};

  }
}

#endif // WT_DBO_TRANSACTION_H_
