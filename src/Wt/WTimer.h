// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTIMER_H_
#define WTIMER_H_

#include <Wt/WApplication.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WEvent.h>

#include <chrono>

namespace Wt {

class WTimerWidget;
class Time;

/*! \class WTimer Wt/WTimer.h Wt/WTimer.h
 *  \brief A utility class which provides timer signals and single-shot timers.
 *
 * To use a timer, create a %WTimer instance, set the timer
 * interval using setInterval() and connect a slot to the timeout signal.
 * Then, start the timer using start(). An active timer may be cancelled
 * at any time using stop().
 *
 * By default, a timer will continue to generate events until you
 * stop() it. To create a timer that will fire only once, use
 * setSingleShot(). 
 * \if cpp
 * There is also a convience static method singleShot().
 * \endif 
 *
 * When connecting stateless slot implementations to the timeout
 * signal, these stateless slot implementations will be used as for
 * any other signal (when Ajax is available).
 *
 * In clients without (enabled) JavaScript support, the minimum
 * resolution of the timer is one second (1000 milli-seconds), and it
 * is probably wise to use timers sparingly.
 *
 * A WTimer is only usable inside of a %Wt event loop.
 * \if cpp
 * If you want to create a timer outside the %Wt event loop, take a look at asio deadline_timer or steady_timer.
 * \else
 * If you want to create a timer outside the %Wt event loop, take a look at {javadoclink java.util.Timer}.
 * \endif
 * 
 * \if cpp
 * Timers are one way to provide updates of a web page without the
 * user generating an event. Alternatively you may consider
 * server-initiated updates, see WApplication::enableUpdates().
 * \endif
 *
 * \if cpp
 * Usage example:
 * \code
 * // setup a timer which calls MyClass::timeout() every 2 seconds, until timer->stop() is called.
 * auto timer = root()->addChild(std::make_unique<Wt::WTimer>());
 * timer->setInterval(std::chrono::seconds(2));
 * timer->timeout().connect(this, &MyClass::timeout);
 * timer->start();
 * \endcode
 * \endif
 */
class WT_API WTimer : public WObject
{
public:
  /*! \brief Construct a new timer with the given parent.
   */
  WTimer();

  /*! \brief Destuctor.
   */
  ~WTimer();

  /*! \brief Returns the interval
   */
  std::chrono::milliseconds interval() const { return interval_; }

  /*! \brief Sets the interval
   */
  void setInterval(std::chrono::milliseconds interval);

  /*! \brief Returns if the timer is running.
   */
  bool isActive() const { return active_; }

  /*! \brief Is this timer set to fire only once.
   */
  bool isSingleShot() const { return singleShot_; }

  /*! \brief Configures this timer to fire only once.
   *
   * A Timer is by default not single shot, and will fire continuously,
   * until it is stopped.
   *
   * \if cpp
   * \sa singleShot()
   * \endif
   */
  void setSingleShot(bool singleShot);

#ifndef WT_TARGET_JAVA
  /*! \brief This static function calls a slot after a given time interval.
   *
   * For example, the following code will call this->doSome() after 2
   * seconds: 
   * \code
   *   WTimer::singleShot(2000, this, &MyClass::doSome);
   * \endcode
   */
  template <class T, class V>
  static void singleShot(std::chrono::milliseconds interval, T *receiver, void (V::*method)());

  /*! \brief This static function calls a function after a given time interval.
   *
   * This variant of the overloaded singleShot() method supports a
   * template function object (which supports operator ()).
   */
  template <class F>
  static void singleShot(std::chrono::milliseconds interval, const F& f);
#endif // WT_TARGET_JAVA

  /*! \brief Starts the timer.
   *
   * The timer will be isActive(), until either the interval has
   * elapsed, after which the timeout signal is activated,
   * or until stop() is called.
   */
  void start();

  /*! \brief Stops the timer.
   *
   * You may stop the timer during its timeout(), or cancel a running timer
   * at any other time.
   *
   * \sa start()
   */
  void stop();

  /*! \brief %Signal emitted when the timer timeouts.
   *
   * The %WMouseEvent does not provide any meaningful information but is
   * an implementation artefact.
   */
  EventSignal<WMouseEvent>& timeout();

private:
  observing_ptr<WTimerWidget> timerWidget_;
  std::unique_ptr<WTimerWidget> uTimerWidget_;

  std::chrono::milliseconds  interval_;
  bool singleShot_;
  bool active_;

  std::unique_ptr<Time> timeout_;

  void gotTimeout();
  int getRemainingInterval() const;

  friend class WTimerWidget;
};

#ifndef WT_TARGET_JAVA
template <class T, class V>
void WTimer::singleShot(std::chrono::milliseconds interval, T *receiver, void (V::*method)())
{
  auto timer = WApplication::instance()->addChild(cpp14::make_unique<WTimer>());
  timer->setSingleShot(true);
  timer->setInterval(interval);
  timer->start();
  timer->timeout().connect(receiver, method);
  timer->timeout().connect([timer]{ WApplication::instance()->removeChild(timer); });
}

template <class F>
void WTimer::singleShot(std::chrono::milliseconds interval, const F& f)
{
  auto timer = WApplication::instance()->addChild(cpp14::make_unique<WTimer>());
  timer->setSingleShot(true);
  timer->setInterval(interval);
  timer->start();
  timer->timeout().connect(f);
  timer->timeout().connect([timer]{ WApplication::instance()->removeChild(timer); });
}
#endif // WT_TARGET_JAVA

}

#endif // WTIMER_H_
