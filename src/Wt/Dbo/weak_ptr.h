// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBO_WEAK_PTR_H_
#define WT_DBO_DBO_WEAK_PTR_H_

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/Session.h>

namespace Wt {
  namespace Dbo {

/*! \class weak_ptr Wt/Dbo/weak_ptr.h Wt/Dbo/weak_ptr.h
 *  \brief A weak pointer for a database object.
 *
 * A weak_ptr has similar API and capabilities as a ptr, but does not
 * actually store a reference to the object itself. Instead, it uses
 * a query to retrieve the object.
 *
 * It is used only to implement a OneToOne relation:
 * - class A hasOne class B object, in a weak_ptr
 * - class B belongsTo a class A object, in a ptr
 *
 * This cannot be implemented using two plain ptr's because otherwise
 * a cycle would be created which interferes with the count-based
 * shared pointer implementation of ptr.
 *
 * Only in a OneToOne relation, the required querying logic for the
 * weak_ptr to function properly is provided by the hasOne() call, and thus
 * a weak_ptr cannot be used outside of a OneToOne relation.
 *
 * The weak_ptr has almost the same capabilities as a ptr, except that
 * it does not provide a dereference operator (*).
 *
 * It's important to realize that reading a weak_ptr results in a
 * query, and thus you may want to first convert the weak_ptr into a
 * plain ptr when needing multiple operations.
 *
 * A Wt::Dbo::ptr<C> can be converted to a Wt::Dbo::ptr<const C>
 * when extracting data from Dbo objects.
 * There are overloads for the copy constructor, copy assignment,
 * and comparison operators to make this work as expected.
 *
 * \ingroup dbo
 */
template <class C>
class weak_ptr
{
private:
  typedef typename std::remove_const<C>::type MutC;
public:
  typedef C pointed;

  class accessor
  {
  public:
    accessor(const ptr<C>& p);

    const C *operator->() const;
    const C& operator*() const;
    operator const C*() const;

  private:
    ptr<C> p_;
  };

  class mutator
  {
  public:
    mutator(const ptr<C>& p);
    ~mutator();

    C *operator->() const;
    C& operator*() const;
    operator C*() const;

  private:
    ptr<C> p_;
  };

  /*! \brief Creates a new weak pointer.
   *
   * The ptr does not point to anything.
   */
  weak_ptr();

  /*! \brief Copy constructor.
   */
  weak_ptr(const weak_ptr<C>& other);

  template <class D, class = typename std::enable_if<std::is_same<MutC,D>::value>::type>
  weak_ptr(const weak_ptr<D>& other);

  /*! \brief Move constructor.
   */
  weak_ptr(weak_ptr<C> &&other) noexcept;

  template <class D, class = typename std::enable_if<std::is_same<MutC,D>::value>::type>
  weak_ptr(weak_ptr<D>&& other) noexcept;

  /*! \brief Sets the value.
   *
   * This is assigns a new value, and:
   * \code
   *  p.reset(obj);
   * \endcode
   * is equivalent to:
   * \code
   *  p = Wt::Dbo::ptr<T>(obj);
   * \endcode
   *
   * Since this needs to query the previous value, you should have an
   * active transaction.
   */
  void reset(C *obj = 0);

  /*! \brief Copy assignment operator.
   *
   * Since this needs to query the previous value, you should have an
   * active transaction.
   */
  weak_ptr<C>& operator= (const weak_ptr<C>& other);

  template <class D, class = typename std::enable_if<std::is_same<MutC,D>::value>::type>
  weak_ptr<C>& operator= (const weak_ptr<D>& other);

  /*! \brief Move assignment operator.
   */
  weak_ptr<C>& operator= (weak_ptr<C>&& other) noexcept;

  template <class D, class = typename std::enable_if<std::is_same<MutC,D>::value>::type>
  weak_ptr<C>& operator= (weak_ptr<D>&& other) noexcept;

  /*! \brief Copy assignment operator.
   *
   * Since this needs to query the previous value, you should have an
   * active transaction.
   */
  weak_ptr<C>& operator= (const ptr<C>& other);

  template <class D, class = typename std::enable_if<std::is_same<MutC,D>::value>::type>
  weak_ptr<C>& operator= (const ptr<D>& other);

#ifdef DOXYGEN_ONLY
  /*! \brief Dereference operator.
   *
   * Note that this operator returns a const copy of the referenced
   * object. Use modify() to get a non-const reference.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
  const C *operator->() const;

  /*! \brief Dereference operator, for writing.
   *
   * Returns the underlying object (or, rather, a proxy for it) with
   * the intention to modify it. The proxy object will mark the object
   * as dirty from its destructor. An involved modification should
   * therefore preferably be implemented as a separate method or
   * function to make sure that the object is marked as dirty after the
   * whole modification:
   * \code
   *   weak_ptr<A> a = ...;
   *   a.modify()->doSomething();
   * \endcode
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
  C *modify() const;
#else
  accessor operator->() const;
  mutator modify() const;
#endif // DOXYGEN_ONLY

  /*! \brief Comparison operator.
   *
   * Two pointers are equal if and only if they reference the same
   * database object.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
#ifdef DOXYGEN_ONLY
  bool operator== (const weak_ptr<C>& other) const;
#else
  bool operator== (const weak_ptr<MutC>& other) const;
  bool operator== (const weak_ptr<const C>& other) const;
#endif // DOXYGEN_ONLY

  /*! \brief Comparison operator.
   *
   * Two pointers are equal if and only if they reference the same
   * database object.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
#ifdef DOXYGEN_ONLY
  bool operator== (const ptr<C>& other) const;
#else
  bool operator== (const ptr<MutC>& other) const;
  bool operator== (const ptr<const C>& other) const;
#endif // DOXYGEN_ONLY

  /*! \brief Comparison operator.
   *
   * Two pointers are equal if and only if they reference the same
   * database object.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
#ifdef DOXYGEN_ONLY
  bool operator!= (const weak_ptr<C>& other) const;
#else
  bool operator!= (const weak_ptr<MutC>& other) const;
  bool operator!= (const weak_ptr<const C>& other) const;
#endif // DOXYGEN_ONLY

  /*! \brief Comparison operator.
   *
   * Two pointers are equal if and only if they reference the same
   * database object.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
#ifdef DOXYGEN_ONLY
  bool operator!= (const ptr<C>& other) const;
#else
  bool operator!= (const ptr<MutC>& other) const;
  bool operator!= (const ptr<const C>& other) const;
#endif // DOXYGEN_ONLY

  /*! \brief Checks for null.
   *
   * Returns true if the pointer is pointing to a non-null object.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
  explicit operator bool() const;

  /*! \brief Casting operator to a ptr.
   *
   * Returns the value as a plain ptr.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
  operator ptr<C>() const;

  template <class D, class = typename std::enable_if<std::is_same<D, std::add_const<C>>::value>>
  operator ptr<D>() const; // for conversion to ptr<const C>

  /*! \brief Promotes to a ptr.
   *
   * Returns the value as a plain ptr.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   *
   * This is equivalent to lock() or the ptr<C> cast operator.
   */
  ptr<C> query() const;

  /*! \brief Promotes to a ptr.
   *
   * Returns the value as a plain ptr.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   *
   * This is equivalent to query() or the ptr<C> cast operator.
   */
  ptr<C> lock() const;

  /*! \brief Returns the object id.
   *
   * Since this needs to query the value, you should have an active
   * transaction.
   */
  typename dbo_traits<C>::IdType id() const;

private:
  collection< ptr<MutC> > collection_;

  void setRelationData(MetaDboBase *dbo, const std::string *sql,
		       Impl::SetInfo *info);

  friend class DboAction;
  friend class SaveBaseAction;
  friend class SetReciproceAction;
  friend class weak_ptr<MutC>;
  friend class weak_ptr<const C>;
};

  }
}

#endif // WT_DBO_DBO_WEAK_PTR_H_
