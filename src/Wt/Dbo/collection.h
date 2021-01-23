// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_COLLECTION_H_
#define WT_DBO_COLLECTION_H_

#include <cstddef>
#include <iterator>
#include <set>

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/Session.h>

namespace Wt {
  namespace Dbo {
    struct DirectBinding;
    struct DynamicBinding;
    template <class Result, typename BindStrategy> class Query;
    class SqlStatement;

  /*! \class collection Wt/Dbo/collection.h Wt/Dbo/collection.h
   *  \brief An STL container for iterating query results.
   *
   * This is an STL-compatible container that is backed by an SQL
   * query for fetching data.
   *
   * A %collection has two uses in Wt::Dbo:
   * - to iterate the results of a Query;
   * - to map the many-side of a Many-to-One or Many-to-Many relation.
   *
   * Its iterators implement the InputIterator requirements, meaning
   * you have to traverse the results from begin() to end() solely by
   * alternating between reading an element and incrementing the
   * iterator. When the collection represents the results of a Query,
   * you can only iterate the results just once: i.e. you can have
   * only one begin() call.
   *
   * \code
   * typedef dbo::collection< dbo::ptr<User> > Users;
   * Users allUsers = session.find<User> ();
   * for (Users::const_iterator i = allUsers.begin();
   *     i != allUsers.end(); ++i){
   *   dbo::ptr<User> user = *i;
   * }
   * \endcode
   *
   * The container is read only when it reflects results of a query.
   * Otherwise, when involved in a Many-to-One or Many-to-Many
   * relation, you may also insert() and erase() objects in it.
   *
   * You will typically iterate the container results for local
   * processing, or copy the results into a standard STL container for
   * extended processing. Not only the weak guarantees of the
   * iterators make this recommended, but also in the current
   * implementation of the library, all sql statements are
   * non-reentrant prepared statements (this limitation is likely to
   * be removed in the future): only one %collection, which is backed
   * by the same SQL statement may be used at once per session. Thus,
   * the following will fail:
   *
   * \code
   * void iterateChildren(Wt::Dbo::ptr<Comment> comment)
   * {
   *     typedef Wt::Dbo::collection<Wt::Dbo::ptr<Comment> > Comments;
   *     Comments children = comment->children;
   *
   *     for (Comments::const_iterator i = children.begin(); i != children.end(); ++i) {
   *        std::cerr << "Comment: " << i->text << std::endl;
   *        iterateChildren(*i); // Illegal since will result in nested use of the same query.
   *     }
   * }
   * \endcode
   *
   * If you cannot guarantee that during its iteration the same query
   * will be reused, you should copy the results in a standard
   * container. Note that this is no big overhead since dbo pointers
   * are lightweight.
   *
   * \code
   * void iterateChildren(Wt::Dbo::ptr<Comment> comment)
   * {
   *     typedef std::vector<Wt::Dbo::ptr<Comment> > Comments;
   *
   *     Comments children(comment->children.begin(), comment->children.end()); // copy into an STL container, freeing the underlying query for reuse 
   *
   *     for (Comments::const_iterator i = children.begin(); i != children.end(); ++i) {
   *        std::cerr << "Comment: " << i->text << std::endl;
   *        iterateChildren(*i); // Okay now.
   *     }
   * }
   * \endcode
   *
   * Before iterating a %collection, the session is flushed. In this
   * way, the %collection will reflect any pending dirty changes.
   *
   * \ingroup dbo
   */
  template <class C>
  class collection
  {
    struct Activity {
      std::set<C> inserted, erased;
      std::set<C> transactionInserted, transactionErased;
    };

  public:
    /*! \brief Value type.
     */
    typedef                   C value_type;

    typedef          value_type key_type;
    typedef   const value_type& const_reference;
    typedef         std::size_t size_type;
    typedef      std::ptrdiff_t difference_type;
    typedef         value_type *pointer;
    typedef   const value_type *const_pointer;

    class const_iterator;

    /*! \brief Iterator.
     */
    class iterator
    {
    public:

      typedef std::input_iterator_tag iterator_category;
      typedef                       C value_type;
      typedef          std::ptrdiff_t difference_type;
      typedef             value_type *pointer;
      typedef             value_type& reference;

      /*! \brief Copy constructor.
       */
      iterator(const iterator& other);

      /*! \brief Destructor.
       */
      ~iterator();

      /*! \brief Assignment operator.
       */
      iterator& operator= (const iterator& other);

      /*! \brief Dereference operator.
       */
      C& operator*();

      /*! \brief Dereference operator.
       */
      C *operator->();

      /*! \brief Comparison operator.
       *
       * Returns true if two iterators point to the same value in the
       * same %collection, or point both to the end of a collection.
       */
      bool operator== (const iterator& other) const;

      /*! \brief Comparison operator.
       */
      bool operator!= (const iterator& other) const;

      /*! \brief Pre increment operator.
       */
      iterator& operator++ ();

      /*! \brief Post increment operator.
       */
      iterator  operator++ (int);

      struct shared_impl {
	const collection<C>& collection_;
	SqlStatement *statement_;
	value_type current_;
	int useCount_;
	bool queryEnded_;
	unsigned posPastQuery_;
	bool ended_;

	shared_impl(const collection<C>& collection, SqlStatement *statement);
	~shared_impl();

	void fetchNextRow();
	typename collection<C>::value_type& current();
      };

    private:
      shared_impl *impl_;

      iterator();
      iterator(const collection<C>& collection, SqlStatement *statement);

      void takeImpl();
      void releaseImpl();

      friend class collection<C>;
      friend class const_iterator;
    };

    /*! \brief Const Iterator.
     */
    class const_iterator
    {
    public:

      typedef std::input_iterator_tag iterator_category;
      typedef                       C value_type;
      typedef          std::ptrdiff_t difference_type;
      typedef             value_type *pointer;
      typedef             value_type& reference;

      /*! \brief Copy constructor.
       */
      const_iterator(const const_iterator& other);

      /*! \brief Copy constructor.
       */
      const_iterator(const typename collection<C>::iterator& other);

      /*! \brief Assignment operator.
       */
      const_iterator& operator= (const const_iterator& other);

      /*! \brief Assignment operator.
       */
      const_iterator& operator= (const iterator& other);

      /*! \brief Dereference operator.
       */
      C operator*();

      /*! \brief Dereference operator.
       */
      const C *operator->();

      /*! \brief Comparison operator.
       *
       * Returns true if two iterators point to the same value in the
       * same %collection.
       */
      bool operator== (const const_iterator& other) const;

      /*! \brief Comparison operator.
       */
      bool operator!= (const const_iterator& other) const;

       /*! \brief Pre increment operator.
       */
      const_iterator& operator++ ();

      /*! \brief Post increment operator.
       */
      const_iterator  operator++ (int);

    private:
      typename collection<C>::iterator impl_;

      const_iterator();
      const_iterator(const collection<C>& collection, SqlStatement *statement);

      friend class collection<C>;
    };

    /*! \brief Default constructor.
     *
     * Constructs an empty %collection that is not bound to a database
     * session or query.
     */
    collection();

    /*! \brief Copy constructor.
     */
    collection(const collection<C>& other);

    /*! \brief Move constructor.
     */
    collection(collection<C>&& other) noexcept;

    /*! \brief Copy assignment operator.
     */
    collection<C>& operator=(const collection<C>& other);

    /*! \brief Move assignment operator.
     */
    collection<C>& operator=(collection<C>&& other) noexcept;

    /*! \brief Destructor.
     */
    ~collection();

    /*! \brief Returns an iterator to the begin of the %collection.
     *
     * \sa end()
     */
    iterator begin();

    /*! \brief Returns an iterator to the end of the %collection.
     *
     * \sa begin()
     */
    iterator end();

    /*! \brief Returns a const iterator to the begin of the %collection.
     *
     * \sa end()
     */
    const_iterator begin() const;

    /*! \brief Returns a const iterator to the end of the %collection.
     *
     * \sa begin()
     */
    const_iterator end() const;

    /*! \brief Returns a reference to the first object.
     *
     * This is equivalent to:
     * \code
     * *(collection.begin())
     * \endcode
     */
    C front() const;

    /*! \brief Returns the size.
     *
     * This will execute an SQL <tt>count(*)</tt> statement to fetch the
     * size of the %collection without fetching all results.
     *
     * If the %collection represents the result of a Query, the underlying
     * query is run only once, and its result is cached so that size() always
     * returns the same value.
     */
    size_type size() const;

    /*! \brief Returns whether the collection is empty.
     *
     * Returns whether size() == 0
     */
    bool empty() const;

    /*! \brief Inserts an object.
     *
     * \note This is only implemented for %collections that are
     *       involved in a ManyToOne or ManyToMany relation, and not for
     *       collections that are used to iterated the result of a query.
     *
     * \sa erase()
     */
    void insert(C c);

    /*! \brief Removes an object.
     *
     * \note This is only implemented for %collections that are
     *       involved in a ManyToOne or ManyToMany relation, and not for
     *       collections that are used to iterated the result of a query.
     *
     * \sa insert()
     */
    void erase(C c);

    /*! \brief Clears the collection.
     *
     * \note This is only implemented for %collections that are
     *       involved in a ManyToOne or ManyToMany relation, and not for
     *       collections that are used to iterated the result of a query.
     *
     * \sa erase()
     */
    void clear();

    /*! \brief Returns the whether the collection contains an object.
     *
     * This creates a suitable query that checks whether the collection
     * contains ptr<X> objects (without getting the entire collection from
     * the database). The returned value is the number of times the collection
     * contains the object (0 or 1).
     *
     * \sa find()
     *
     * \note This is (currently) only implemented for %collections that are
     *       involved in a ManyToOne or ManyToMany relation, and not for
     *       collections that are used to iterate the result of a query.
     */
    int count(C c) const;

    /*! \brief Returns the session to which this %collection is bound.
     */
    Session *session() const { return session_; }

    /*! \brief Returns the query that backs the collection.
     *
     * Returns the query that backs the collection. This can be used to
     * search for a subset or to browse the results in a particular order.
     *
     * \sa count()
     *
     * \note This is (currently) only implemented for %collections that are
     *       involved in a ManyToOne or ManyToMany relation, and not for
     *       collections that are used to iterate the result of a query.
     */
    Query<C, DynamicBinding> find() const;

    const std::vector<C>& manualModeInsertions() const { return manualModeInsertions_; }
    std::vector<C>& manualModeInsertions() { return manualModeInsertions_; }
    const std::vector<C>& manualModeRemovals() const { return manualModeRemovals_; }
    std::vector<C>& manualModeRemovals() { return manualModeRemovals_; }

  private:
    Session *session_;
    enum { QueryCollection, RelationCollection } type_;

    // Structure for a relation query
    struct RelationData {
      const std::string *sql;
      MetaDboBase *dbo;
      Impl::SetInfo *setInfo;
      Activity *activity; // only for ManyToMany collections
    };

    struct QueryData {
      SqlStatement *statement, *countStatement;
      int size;
      int useCount;
    };

    union {
      QueryData *query;
      RelationData relation;
    } data_;

    std::vector<C> manualModeInsertions_;
    std::vector<C> manualModeRemovals_;

    friend class DboAction;
    friend class SessionAddAction;
    friend class LoadBaseAction;
    friend class SaveBaseAction;
    friend class TransactionDoneAction;
    template <class D> friend class weak_ptr;
    template <class Result, typename BindStrategy> friend class Query;

    collection(Session *session, SqlStatement *selectStatement,
	       SqlStatement *countStatement);

    void setRelationData(MetaDboBase *dbo, const std::string *sql,
			 Impl::SetInfo *info);
    Activity *activity() const { return data_.relation.activity; }
    void resetActivity();
    void releaseQuery();

    SqlStatement *executeStatement() const;

    void iterateDone() const;
  };

  }
}

#endif // WT_DBO_COLLECTION_H_
