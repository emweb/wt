// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WOBJECT_H_
#define WOBJECT_H_

#include <Wt/WDllDefs.h>
#include <Wt/WGlobal.h>
#include <Wt/Core/observable.hpp>
#include <Wt/Http/Request.h>

#include <cassert>
#include <map>
#include <vector>

#ifndef WT_TARGET_JAVA
#include <type_traits>
#endif // WT_TARGET_JAVA

namespace Wt {

class WStatelessSlot;

/*! \class WObject Wt/WObject.h Wt/WObject.h
 *  \brief A base class for objects that participate in the signal/slot system.
 *
 * The main feature offered by %WObject is automated object life-time
 * tracking when involved in signal/slot connections. Connections
 * between signals and slots of %WObject instances implement a
 * type-safe event callback system. For example, one can simply
 * connect() the WInteractWidget::clicked() signal of a WPushButton to
 * the WApplication::quit() method, to exit the application when the
 * button is clicked:
 *
 * \code
 * std::unique_ptr<Wt::WInteractWidget> sender{new Wt::WText("Quit.")};
 * Wt::WApplication *app = Wt::WApplication::instance();
 * sender->clicked().connect(app, &Wt::WApplication::quit);
 * \endcode
 *
 * %Wt's signals may also propagate arguments to slots. For example,
 * the same clicked() signal provides event details in
 * a WMouseEvent details class, and these details may be received in
 * the slot:
 * \code
 * class MyClass : public Wt::WContainerWidget
 * {
 * public:
 *   MyClass()
 *   {
 *     Wt::WText *text = addWidget(std::make_unique<Wt::WText>("Click here"));
 *     text->clicked().connect(this, &MyClass::handleClick);
 *
 *     ...
 *   }
 *
 * private:
 *   void handleClick(const Wt::WMouseEvent& event) {
 *     if (event.modifiers().test(Wt::KeyboardModifier::Shift)) {
 *       ...
 *     }
 *   }
 * };
 * \endcode
 * As the example illustrates, slots are ordinary %WObject methods.
 *
 * In conjunction with EventSignal, %WObject also facilitates learning
 * of client-side event handling (in JavaScript) through invocation of
 * the slot method. This is only possible when the slot behaviour is
 * stateless, i.e. independent of any application state, and can be
 * specified using the implementStateless() methods.
 *
 * \sa Signal, EventSignal
 *
 * \ingroup signalslot
 */
class WT_API WObject : public Wt::Core::observable
{
public:
  /*! \brief Typedef for a %WObject method without arguments.
   */
  typedef void (WObject::*Method)();

  WObject();
  WObject(const WObject&) = delete;
  WObject &operator =(const WObject&) = delete;

  virtual ~WObject();  

#ifndef WT_TARGET_JAVA
  /*! \brief Add a child WObject whose lifetime is determined by this WObject
   */
  void addChild(std::unique_ptr<WObject> child);

  /*! \brief Add a child WObject, returning a raw pointer.
   *
   * This is implemented as:
   *
   * \code
   * Child *result = child.get();
   * addChild(std::unique_ptr<WObject>(std::move(child)));
   * return result;
   * \endcode
   */
  template <typename Child>
    Child *addChild(std::unique_ptr<Child> child)
  {
    Child *result = child.get();
    addChild(std::unique_ptr<WObject>(std::move(child)));
    return result;
  }

  /*! \brief Remove a child WObject, so its lifetime is no longer determined by this WObject
   */
  std::unique_ptr<WObject> removeChild(WObject *child);

  /*! \brief Remove a child WObject, so its lifetime is no longer determined by this WObject
   *
   * This is an overload that automatically casts the returned value to a unique_ptr<Child> for convenience
   *
   * This is implemented as:
   * \code
   * return std::unique_ptr<Child>(static_cast<Child*>(removeChild(static_cast<WObject*>(child)).release()));
   * \endcode
   */
  template <typename Child>
#ifdef DOXYGEN_ONLY
  std::unique_ptr<Child> removeChild(Child *child)
#else // DOXYGEN_ONLY
  typename std::enable_if<std::is_convertible<Child*,WObject*>::value && !std::is_same<Child,WObject>::value, std::unique_ptr<Child>>::type removeChild(Child *child)
#endif // DOXYGEN_ONLY
  {
    std::unique_ptr<WObject> result = removeChild(static_cast<WObject*>(child));
    // The result is null or the given child, guaranteeing that result.release() can be casted to Child*
    assert(result == nullptr || result.get() == child);
    return std::unique_ptr<Child>(static_cast<Child*>(result.release()));
  }
#endif // WT_TARGET_JAVA

  /*
   * Unique id's
   */
  unsigned rawUniqueId() const { return id_; }
  const std::string uniqueId() const;

  /*! \brief Returns the (unique) identifier for this object
   *
   * For a %WWidget, this corresponds to the id of the DOM element
   * that represents the widget. This is not entirely unique, since a
   * \link WCompositeWidget composite widget\endlink shares the same
   * id as its implementation.
   *
   * By default, the id is auto-generated, unless a custom id is set
   * for a widget using WWidget::setId().
   *
   * \sa WWidget::jsRef()
   */
  virtual const std::string id() const;

  /*! \brief Sets an object name.
   *
   * The object name can be used to easily identify a type of object
   * in the DOM, and does not need to be unique. It will usually
   * reflect the widget type or role.
   *
   * The object name is present in the DOM in the 'data-object-name'
   * attribute.
   *
   * The default object name is empty (no object name).
   */
  virtual void setObjectName(const std::string& name);

  /*! \brief Returns the object name.
   *
   * \sa setObjectName()
   */
  virtual std::string objectName() const;

  /*! \brief Resets learned stateless slot implementations.
   *
   * Clears the stateless implementation for all slots declared to be
   * implemented with a stateless implementation.
   *
   * \sa resetLearnedSlot(), implementStateless() 
   */
  void resetLearnedSlots();

  /*! \brief Resets a learned stateless slot implementation.
   *
   * Clears the stateless implementation for the given slot that
   * was declared to be implemented with a stateless implementation.
   *
   * When something has changed that breaks the contract of a
   * stateless slot to always have the same effect, you may call this
   * method to force the application to discard the current
   * implementation.
   *
   * \sa implementStateless()
   */
  template <class T>
    void resetLearnedSlot(void (T::*method)());
   
  /*! \brief Declares a slot to be stateless and learn client-side behaviour
   *         on first invocation.
   *
   * Indicate that the given slot is stateless, and meets the requirement
   * that the slot's code does not depend on any state of the object, but
   * performs the same visual effect regardless of any state, or at
   * least until resetLearnedSlot() is called.
   *
   * When this slot is connected to an EventSignal (such as those exposed
   * by WInteractWidget and WFormWidget), the %Wt library may decide to
   * cache the visual effect of this slot in JavaScript code at client-side:
   * this effect will be learned automatically at the first invocation.
   * This has no consequences for the normal event handling, since the slot
   * implementation is still executed in response to any event notification.
   * Therefore, it is merely an optimization of the latency for the visual
   * effect, but it does not change the behaviour of the application.
   *
   * When for some reason the visual effect does change, one may use
   * resetLearnedSlot() or resetLearnedSlots() to flush the existing cached
   * visual effect, forcing the library to relearn it.
   *
   * It is crucial that this function be applied first to a slot that is 
   * intended to be stateless before any %EventSignal connects to that slot.
   * Otherwise, the connecting %EventSignal cannot find the stateless
   * slot implementation for the intended slot, and the statement will have
   * no effect for that connection.
   *
   * \sa resetLearnedSlot(), EventSignal
   */
#ifndef WT_TARGET_JAVA
  template <class T>
    WStatelessSlot *implementStateless(void (T::*method)());
#else // WT_TARGET_JAVA
  template <class T1>
    WStatelessSlot *implementStateless(T1 method);
#endif // WT_TARGET_JAVA

  /*! \brief Declares a slot to be stateless and learn client-side behaviour
   *         in advance.
   *
   * This method has the same effect as
   *\link implementStateless() implementStateless(void (T::*method)())\endlink,
   * but learns the visual effect of the slot before the first
   * invocation of the event.
   *
   * To learn the visual effect, the library will simulate the event and
   * record the visual effect. To restore the application state, it will
   * call the undoMethod which must restore the effect of method. 
   *
   * \sa \link implementStateless() implementStateless(void (T::*method)())\endlink
   */
#ifndef WT_TARGET_JAVA
  template <class T>
    WStatelessSlot *implementStateless(void (T::*method)(),
				       void (T::*undoMethod)());
#else // WT_TARGET_JAVA
  template <class T1, class T2>
    WStatelessSlot *implementStateless(T1 method, T2 undoMethod);
#endif // WT_TARGET_JAVA

  /*! \brief Marks the current function as not stateless.
   *
   * This may be useful if your current function is manipulating the
   * UI in a way that is not stateless (i.e. does depend on some
   * state), but which happens to be called from within a function
   * that is marked stateless (such as WWidget::setHidden()). This
   * will reject stateless slot pre-learning in this case, reverting
   * to plain server-side dynamic UI updates.
   */
  void isNotStateless();

  /*! \brief Provides a JavaScript implementation for a method
   *
   * This method sets the JavaScript implementation for a method. As a
   * result, if JavaScript is available, the JavaScript version will
   * be used on the client side and the visual effect of the C++
   * implementation will be ignored.
   *
   * This is very similar to an auto-learned stateless slot, but here the
   * learning is avoided by directly setting the JavaScript implementation.
   *
   * The \p jsCode should be one or more valid JavaScript statements.
   *
   * \sa \link implementStateless() implementStateless(void (T::*method)())\endlink
   */
  template <class T>
    WStatelessSlot *implementJavaScript(void (T::*method)(),
					const std::string& jsCode);

  static void seedId(unsigned id);

  WStatelessSlot* isStateless(Method method) const;

protected:
  virtual void signalConnectionsChanged();

  struct FormData {
    FormData(const Http::ParameterValues& aValues,
	     const std::vector<Http::UploadedFile>& aFiles)
      : values(aValues), files(aFiles) { }

    const Http::ParameterValues& values;
    std::vector<Http::UploadedFile> files;
  };

  virtual void setFormData(const FormData& formData);
  virtual void setRequestTooLarge(::int64_t size);

  /*! \brief On-demand stateless slot implementation.
   *
   * This method returns a stateless slot implementation for the given
   * \p method. To avoid the cost of declaring methods to be
   * stateless when they are not used, you may reimplement this method
   * to provide a stateless implementation for a method only when the
   * method is involved in a slot connection.
   *
   * Use implementStateless() to provide a stateless implementation of the
   * given \p method, or return the base class implementation otherwise.
   */
  virtual WStatelessSlot *getStateless(Method method);

private:
  WStatelessSlot *implementPrelearn(Method method, Method undoMethod);
  WStatelessSlot *implementPrelearned(Method method, const std::string& jsCode);
  WStatelessSlot *implementAutolearn(Method method);
  void resetLearnedSlot(Method method);

  std::vector<std::unique_ptr<WStatelessSlot> > statelessSlots_;
  std::vector<std::unique_ptr<WObject> > children_;

  unsigned id_;
  std::string name_;

  static unsigned nextObjId_;

  friend class EventSignalBase;
  friend class WebSession;
};

template <class T>
void WObject::resetLearnedSlot(void (T::*method)())
{
  assert(dynamic_cast<T *>(this));
  resetLearnedSlot(static_cast<Method>(method));
}

#ifndef WT_TARGET_JAVA
template <class T>
WStatelessSlot *WObject::implementStateless(void (T::*method)())
{
  assert(dynamic_cast<T *>(this));
  return implementAutolearn(static_cast<Method>(method));
}

template <class T>
WStatelessSlot *WObject::implementStateless(void (T::*method)(),
					    void (T::*undoMethod)())
{
  assert(dynamic_cast<T *>(this));
  return implementPrelearn(static_cast<Method>(method),
			   static_cast<Method>(undoMethod));
}
#endif // WT_TARGET_JAVA

template <class T>
WStatelessSlot *WObject::implementJavaScript(void (T::*method)(),
					     const std::string& jsCode)
{
  assert(dynamic_cast<T *>(this));
  return implementPrelearned(static_cast<Method>(method), jsCode);
}

}

#ifdef USING_NAMESPACE_WT
using namespace Wt;
#endif // USING_NAMESPACE_WT

#endif // WOBJECT_H_
