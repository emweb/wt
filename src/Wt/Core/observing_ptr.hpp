// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CORE_OBSERVING_PTR_H_
#define WT_CORE_OBSERVING_PTR_H_

#include <Wt/WDllDefs.h>

namespace Wt { namespace Core {

class observable;

namespace Impl {
  struct WT_API observing_ptr_base
  {
    observing_ptr_base();
    ~observing_ptr_base();
    void clear();
    void set(observable *observable);

    observable *observed_;
    bool cleared_;
  };
}

/*! \class observing_ptr Wt/Core/observing_ptr Wt/Core/observing_ptr
 *  \brief A safe smart pointer for an observable
 * 
 * This smart pointer can point only to objects that are
 * observable. They are safe in the sense that they are aware of the
 * life-time of the observed object, and thus cannot dangle. When
 * dereferencing a pointer to an already destroyed object, an
 * exception is thrown.
 *
 * \sa observable
 */
template <typename T /* is an observable */>
class observing_ptr
{
public:
  /*! \brief Constructor.
   *
   * \sa reset()
   */
  observing_ptr(T *t = nullptr);

  /*! \brief Copy constructor.
   */
  observing_ptr(const observing_ptr<T>& other);

  /*! \brief Assignment operator.
   */
  observing_ptr<T>& operator=(const observing_ptr<T>& other);

  /*! \brief Copy constructor with type conversion.
   *
   * This uses a dynamic_cast to perform a type-safe conversion.
   */
  template <typename S>
  observing_ptr(const observing_ptr<S>& other);

  /*! \brief Assignment operator with type conversion.
   *
   * This uses a dynamic_cast to perform a type-safe conversion.
   */
  template <typename S>
  observing_ptr<T>& operator=(const observing_ptr<S>& other);

  /*! \brief Returns the pointer value.
   *
   * Returns the value set to it, or \c null if the object was deleted.
   *
   * \sa reset()
   */
  T *get() const;

  /*! \brief Resets the value.
   */
  void reset(T *v = nullptr);

  /*! \brief Dereferences the pointer.
   *
   * This throws a std::runtime_error if the pointer cannot be
   * dereferenced.
   */
  T *operator->() const;

  /*! \brief Dereferences the pointer.
   *
   * This throws a std::runtime_error if the pointer cannot be
   * dereferenced.
   */
  T& operator*() const;

  /*! \brief Returns whether the pointer is still valid.
   *
   * Returns if the pointer does not point to \c null and the pointed
   * object isn't deleted.
   */
  operator bool() const;

  /*! \brief Returns whether the observed object has been deleted.
   *
   * Returns if the pointed object has been deleted.
   */
  bool observedDeleted() const;

private:
  Impl::observing_ptr_base impl_;
};

}}

#include "observing_ptr_impl.hpp"

#endif // WT_CORE_OBSERVING_PTR_H_
