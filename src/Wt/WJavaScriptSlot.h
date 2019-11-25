// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WJAVASCRIPTSLOT_H_
#define WJAVASCRIPTSLOT_H_

#include "Wt/WObject.h"

namespace Wt {

class WWidget;

/*! \class JSlot Wt/WJavaScript Wt/WJavaScript
 *  \brief A slot that is only implemented in client side JavaScript code.
 *
 * This class provides a hook for adding your own JavaScript to respond to
 * events.
 *
 * Carefully consider the use of this. Not only is writing cross-browser
 * JavaScript hard and tedious, but one must also be aware of possible
 * security problems (see further), and of course, the event handling will
 * not be available when JavaScript is disabled or not present at all.
 *
 * \if cpp
 * If you wish to add client side event handling, with automatic
 * fall-back to server-side event handling and without writing
 * JavaScript code with the associated risks and problems, consider
 * using stateless slot implementations instead (see
 * WObject::implementStateless())
 * \endif
 *
 * For some purposes, stateless slot implementations are not
 * sufficient, since they do not allow state inspection. At the same
 * time, the non-availability in case of disabled JavaScript may also
 * be fine for some non-essential functionality (see for example the
 * WSuggestionPopup widget), or when you simply do not care. For these
 * situations a %JSlot can be used to add client-side event handling.
 *
 * The JavaScript code may be set (or changed) using the
 * setJavaScript() method which takes a string that implements a
 * JavaScript function with the following signature:
 *
 * \code
 * function(sender, event) {
 *   // handle the event, and sender is a reference to the DOM element
 *   // which captured the event (and holds the signal). Therefore it
 *   // equivalent to the sender for a normal Wt slot.
 *
 *   // You can prevent the default action using:
 *   var fixed = jQuery.event.fix(event);
 *   fixed.preventDefault();
 *   fixed.stopPropagation();
 * }
 * \endcode
 *
 * In the JavaScript code, you may use WWidget::jsRef() to obtain the
 * DOM element corresponding to any WWidget, or WWidget::id() to
 * obtain the DOM id. In addition you may trigger server-side events
 * using the JavaScript WtSignalEmit function (see JSignal
 * documentation).
 *
 * A %JSlot can take up to six extra arguments. This is so that a
 * JSignal can pass its arguments directly on to a %JSlot, without
 * communicating with the server.
 *
 * That's how far we can help you. For the rest you
 * are left to yourself, buggy browsers and quirky JavaScript
 * (http://www.quirksmode.org/ was a reliable companion to me) -- good
 * luck.
 *
 * Note that the slot object needs to live as long as you want the
 * JavaScript to be executed by connected signals: when the slot is
 * destroyed, the connection is destroyed just as with other
 * signal/slot connections where the target object is deleted. This
 * means that it is (almost?) always a bad idea to declare a JSlot on the
 * stack.
 *
 * \ingroup signalslot
 */
class WT_API JSlot
{
public:
  /*! \brief Constructs a JavaScript-only slot within the parent scope.
   *
   * The JavaScript code block will reside within the scope of the
   * given widget.  By picking a long-lived parent, one may reuse a
   * single block of JavaScript code for multiple widgets.
   *
   * When \p parent = \c 0, then the JavaScript will be inlined in
   * each caller (possibly replicating the same JavaScript).
   *
   * The slot will have no extra arguments.
   */
  JSlot(WWidget *parent = nullptr);

  /*! \brief Constructs a JavaScript-only slot and sets the JavaScript code.
   *
   * The slot will have no extra arguments.
   *
   * \sa JSlot(WWidget *), setJavaScript()
   */
  JSlot(const std::string& javaScript, WWidget *parent = nullptr);

  /*! \brief Constructs a JavaScript-only slot and set the number of arguments.
   *
   * \sa JSlot(WWidget *), setJavaScript()
   */
  JSlot(int nbArgs, WWidget *parent);

  /*! \brief Constructs a JavaScript-only slot and sets the JavaScript code
   * and a number of arguments.
   *
   * \sa JSlot(WWidget *), setJavaScript()
   */
  JSlot(const std::string& javaScript, int nbArgs, WWidget *parent = nullptr);

  /*! \brief Destructor.
   */
  ~JSlot();

  /*! \brief Set or modify the JavaScript code associated with the slot.
   *
   * When the slot is triggered, the corresponding function defined by
   * \p javaScript is executed.
   *
   * The JavaScript function takes at least two parameters and thus should look like:
   * \code
     function(obj, event) {
       // ...
     }
     \endcode
   * The first parameter \p obj is a reference to the DOM element
   * that generates the event. The \p event refers to the
   * JavaScript event object.
   *
   * The JavaScript function can take up to six extra arguments, which is to be
   * configured using the nbArgs parameter.
   * \code
     function(obj, event, a1, a2, a3, a4, a5, a6) {
       // ...
     }
     \endcode
   * If this JSlot is connected to a JSignal, that JSignal's arguments will
   * be passed on to the JSlot.
   *
   * \sa WWidget::jsRef()
   */
  void setJavaScript(const std::string& javaScript, int nbArgs = 0);

  /*! \brief Executes the JavaScript code.
   *
   * This executes the JavaScript code in the same way as when
   * triggered by a EventSignal. This function returns immediately,
   * and execution of the JavaScript code is deferred until after the
   * event handling.
   *
   * The first two arguments are the <tt>"object, event"</tt> arguments of the
   * JavaScript event callback function.
   *
   * \sa setJavaScript()
   */
  void exec(const std::string& object = "null",
	    const std::string& event = "null",
	    const std::string& arg1 = "null",
	    const std::string& arg2 = "null",
	    const std::string& arg3 = "null",
	    const std::string& arg4 = "null",
	    const std::string& arg5 = "null",
	    const std::string& arg6 = "null");

  /*! \brief Returns a JavaScript statement that executes the slot.
   *
   * This returns the JavaScript code to execute the slot.
   *
   * The arguments are the <tt>"object, event"</tt> arguments of the
   * JavaScript event callback function.
   *
   * \sa exec()
   */
   std::string execJs(const std::string& object = "null",
		      const std::string& event = "null",
		      const std::string& arg1 = "null",
		      const std::string& arg2 = "null",
		      const std::string& arg3 = "null",
		      const std::string& arg4 = "null",
		      const std::string& arg5 = "null",
		      const std::string& arg6 = "null");

   /*! \brief Returns the number of extra arguments this %JSlot takes.
    */
   int getNbArgs() { return nbArgs_; }

#ifndef WT_TARGET_JAVA
   void disconnectFrom(EventSignalBase *signal);
#endif

private:
  WWidget *widget_;
  WStatelessSlot* imp_;

  std::string jsFunctionName() const;
  WStatelessSlot* slotimp();
  void create();

  int fid_;
  static int nextFid_;

  int nbArgs_;

  friend class EventSignalBase;
};

typedef JSlot WJavaScriptSlot;

}

#endif
