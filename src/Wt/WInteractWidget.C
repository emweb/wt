/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WInteractWidget.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WFormWidget.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WServer.h"
#include "Wt/WTheme.h"

#include "Configuration.h"
#include "DomElement.h"

/*
 * FIXME: provide a cross-browser mechanism to "cancel" the default
 * action for a keyboard event (and other events!).
 * Note that for key-up/key-down events, you will need to do something with
 * the key-press event, as per http://unixpapa.com/js/key.html
 */

namespace Wt {

const char *WInteractWidget::KEYDOWN_SIGNAL = "M_keydown";
const char *WInteractWidget::KEYPRESS_SIGNAL = "keypress";
const char *WInteractWidget::KEYUP_SIGNAL = "keyup";
const char *WInteractWidget::ENTER_PRESS_SIGNAL = "M_enterpress";
const char *WInteractWidget::ESCAPE_PRESS_SIGNAL = "M_escapepress";
const char *WInteractWidget::CLICK_SIGNAL = "click";
const char *WInteractWidget::M_CLICK_SIGNAL = "M_click";
const char *WInteractWidget::DBL_CLICK_SIGNAL = "M_dblclick";
const char *WInteractWidget::MOUSE_DOWN_SIGNAL = "M_mousedown";
const char *WInteractWidget::MOUSE_UP_SIGNAL = "M_mouseup";
const char *WInteractWidget::MOUSE_OUT_SIGNAL = "M_mouseout";
const char *WInteractWidget::MOUSE_OVER_SIGNAL = "M_mouseover";
const char *WInteractWidget::MOUSE_MOVE_SIGNAL = "M_mousemove";
const char *WInteractWidget::MOUSE_DRAG_SIGNAL = "M_mousedrag";
const char *WInteractWidget::MOUSE_WHEEL_SIGNAL = "mousewheel";
const char *WInteractWidget::WHEEL_SIGNAL = "wheel";
const char *WInteractWidget::TOUCH_START_SIGNAL = "touchstart";
const char *WInteractWidget::TOUCH_MOVE_SIGNAL = "touchmove";
const char *WInteractWidget::TOUCH_END_SIGNAL = "touchend";
const char *WInteractWidget::GESTURE_START_SIGNAL = "gesturestart";
const char *WInteractWidget::GESTURE_CHANGE_SIGNAL = "gesturechange";
const char *WInteractWidget::GESTURE_END_SIGNAL = "gestureend";
const char *WInteractWidget::DRAGSTART_SIGNAL = "dragstart";

WInteractWidget::WInteractWidget()
  : mouseOverDelay_(0)
{ }

WInteractWidget::~WInteractWidget()
{ }

void WInteractWidget::setPopup(bool popup)
{
  if (popup && wApp->environment().ajax()) {
    clicked().connect
      ("function(o,e) { "
       " if (" WT_CLASS ".WPopupWidget && o.wtPopup) {"
           WT_CLASS ".WPopupWidget.popupClicked = o;"
           "$(document).trigger('click', e);"
           WT_CLASS ".WPopupWidget.popupClicked = null;"
       " }"
       "}");
    clicked().preventPropagation();
  }

  WWebWidget::setPopup(popup);
}

EventSignal<WKeyEvent>& WInteractWidget::keyWentDown()
{
  return *keyEventSignal(KEYDOWN_SIGNAL, true);
}

EventSignal<WKeyEvent>& WInteractWidget::keyPressed()
{
  return *keyEventSignal(KEYPRESS_SIGNAL, true);
}

EventSignal<WKeyEvent>& WInteractWidget::keyWentUp()
{
  return *keyEventSignal(KEYUP_SIGNAL, true);
}

EventSignal<>& WInteractWidget::enterPressed()
{
  return *voidEventSignal(ENTER_PRESS_SIGNAL, true);
}

EventSignal<>& WInteractWidget::escapePressed()
{
  return *voidEventSignal(ESCAPE_PRESS_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::clicked()
{
  return *mouseEventSignal(M_CLICK_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::doubleClicked()
{
  return *mouseEventSignal(DBL_CLICK_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseWentDown()
{
  return *mouseEventSignal(MOUSE_DOWN_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseWentUp()
{
  return *mouseEventSignal(MOUSE_UP_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseWentOut()
{
  return *mouseEventSignal(MOUSE_OUT_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseWentOver()
{
  return *mouseEventSignal(MOUSE_OVER_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseMoved()
{
  return *mouseEventSignal(MOUSE_MOVE_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseDragged()
{
  return *mouseEventSignal(MOUSE_DRAG_SIGNAL, true);
}

EventSignal<WMouseEvent>& WInteractWidget::mouseWheel()
{
  if (WApplication::instance()->environment().agentIsIElt(9) ||
      WApplication::instance()->environment().agent() 
      == UserAgent::Edge) {
    return *mouseEventSignal(MOUSE_WHEEL_SIGNAL, true);
  } else {
    return *mouseEventSignal(WHEEL_SIGNAL, true);
  }
}

EventSignal<WTouchEvent>& WInteractWidget::touchStarted()
{
  return *touchEventSignal(TOUCH_START_SIGNAL, true);
}

EventSignal<WTouchEvent>& WInteractWidget::touchMoved()
{
  return *touchEventSignal(TOUCH_MOVE_SIGNAL, true);
}

EventSignal<WTouchEvent>& WInteractWidget::touchEnded()
{
  return *touchEventSignal(TOUCH_END_SIGNAL, true);
}

EventSignal<WGestureEvent>& WInteractWidget::gestureStarted()
{
  return *gestureEventSignal(GESTURE_START_SIGNAL, true);
}

EventSignal<WGestureEvent>& WInteractWidget::gestureChanged()
{
  return *gestureEventSignal(GESTURE_CHANGE_SIGNAL, true);
}

EventSignal<WGestureEvent>& WInteractWidget::gestureEnded()
{
  return *gestureEventSignal(GESTURE_END_SIGNAL, true);
}

void WInteractWidget::updateDom(DomElement& element, bool all)
{
  bool updateKeyDown = false;

  WApplication *app = WApplication::instance();

  /*
   * -- combine enterPress, escapePress and keyDown signals
   */
  EventSignal<> *enterPress = voidEventSignal(ENTER_PRESS_SIGNAL, false);
  EventSignal<> *escapePress = voidEventSignal(ESCAPE_PRESS_SIGNAL, false);
  EventSignal<WKeyEvent> *keyDown = keyEventSignal(KEYDOWN_SIGNAL, false);

  updateKeyDown = (enterPress && enterPress->needsUpdate(all))
    || (escapePress && escapePress->needsUpdate(all))
    || (keyDown && keyDown->needsUpdate(all));

  if (updateKeyDown) {
    std::vector<DomElement::EventAction> actions;

    if (enterPress) {
      if (enterPress->needsUpdate(true)) {
	/*
	 * prevent enterPressed from triggering a changed event on all
	 * browsers except for Opera and IE
	 */
	std::string extraJS;

	const WEnvironment& env = app->environment();

	if (dynamic_cast<WFormWidget *>(this)
	    && !env.agentIsOpera() && !env.agentIsIE())
	  extraJS = "var g=this.onchange;"
	    ""      "this.onchange=function(){this.onchange=g;};";

	actions.push_back
	  (DomElement::EventAction("e.keyCode && (e.keyCode == 13)",
				   enterPress->javaScript() + extraJS,
				   enterPress->encodeCmd(),
				   enterPress->isExposedSignal()));
      }
      enterPress->updateOk();
    }

    if (escapePress) {
      if (escapePress->needsUpdate(true)) {
	actions.push_back
	  (DomElement::EventAction("e.keyCode && (e.keyCode == 27)",
				   escapePress->javaScript(),
				   escapePress->encodeCmd(),
				   escapePress->isExposedSignal()));
      }
      escapePress->updateOk();
    }

    if (keyDown) {
      if (keyDown->needsUpdate(true)) {
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   keyDown->javaScript(),
				   keyDown->encodeCmd(),
				   keyDown->isExposedSignal()));
      }
      keyDown->updateOk();
    }

    if (!actions.empty())
      element.setEvent("keydown", actions);
    else if (!all)
      element.setEvent("keydown", std::string(), std::string());
  }

  /*
   * -- allow computation of dragged mouse distance
   */
  EventSignal<WMouseEvent> *mouseDown
    = mouseEventSignal(MOUSE_DOWN_SIGNAL, false);
  EventSignal<WMouseEvent> *mouseUp
    = mouseEventSignal(MOUSE_UP_SIGNAL, false);
  EventSignal<WMouseEvent> *mouseMove
    = mouseEventSignal(MOUSE_MOVE_SIGNAL, false);
  EventSignal<WMouseEvent> *mouseDrag
    = mouseEventSignal(MOUSE_DRAG_SIGNAL, false);

  bool updateMouseMove
    = (mouseMove && mouseMove->needsUpdate(all))
    || (mouseDrag && mouseDrag->needsUpdate(all));

  bool updateMouseDown
    = (mouseDown && mouseDown->needsUpdate(all))
    || updateMouseMove;

  bool updateMouseUp
    = (mouseUp && mouseUp->needsUpdate(all))
    || updateMouseMove;

  std::string CheckDisabled = "if($(o).hasClass('" +
    app->theme()->disabledClass() +
    "')){" WT_CLASS ".cancelEvent(e);return;}";

  if (updateMouseDown) {
    /*
     * when we have a mouseUp event, we also need a mouseDown event
     * to be able to compute dragDX/Y.
     *
     * When we have:
     *  - a mouseDrag
     *  - or a mouseDown + (mouseMove or mouseUp),
     * we need to capture everything after on mouse down, and keep track of the
     * down button if we have a mouseMove or mouseDrag
     */
    WStringStream js;

    js << CheckDisabled;

    if (mouseUp && mouseUp->isConnected())
      js << app->javaScriptClass() << "._p_.saveDownPos(event);";

    if ((mouseDrag && mouseDrag->isConnected())
	|| (mouseDown && mouseDown->isConnected()
	    && ((mouseUp && mouseUp->isConnected())
		|| (mouseMove && mouseMove->isConnected()))))
      js << WT_CLASS ".capture(this);";

    if ((mouseMove && mouseMove->isConnected())
	|| (mouseDrag && mouseDrag->isConnected()))
      js << WT_CLASS ".mouseDown(e);";

    if (mouseDown) {
      js << mouseDown->javaScript();
      element.setEvent("mousedown", js.str(),
		       mouseDown->encodeCmd(), mouseDown->isExposedSignal());
      mouseDown->updateOk();
    } else
      element.setEvent("mousedown", js.str(), std::string(), false);
  }

  if (updateMouseUp) {
    WStringStream js;

    /*
     * when we have a mouseMove or mouseDrag event, we need to keep track
     * of unpressing the button.
     */
    js << CheckDisabled;

    if ((mouseMove && mouseMove->isConnected())
	|| (mouseDrag && mouseDrag->isConnected()))
      js << WT_CLASS ".mouseUp(e);";
      
    if (mouseUp) {
      js << mouseUp->javaScript();
      element.setEvent("mouseup", js.str(),
		       mouseUp->encodeCmd(), mouseUp->isExposedSignal());
      mouseUp->updateOk();
    } else
      element.setEvent("mouseup", js.str(), std::string(), false);
  }

  if (updateMouseMove) {
    /*
     * We need to mix mouseDrag and mouseMove events.
     */
    std::vector<DomElement::EventAction> actions;
    
    if (mouseMove) {
      actions.push_back
	(DomElement::EventAction(std::string(),
				 mouseMove->javaScript(),
				 mouseMove->encodeCmd(),
				 mouseMove->isExposedSignal()));
      mouseMove->updateOk();
    }

    if (mouseDrag) {
      actions.push_back
	(DomElement::EventAction(WT_CLASS ".buttons",
				 mouseDrag->javaScript()
				 + WT_CLASS ".drag(e);",
				 mouseDrag->encodeCmd(),
				 mouseDrag->isExposedSignal()));
      mouseDrag->updateOk();
    }

    element.setEvent("mousemove", actions);
  }
  /*
   * -- allow computation of dragged touch distance
   */
  EventSignal<WTouchEvent> *touchStart
    = touchEventSignal(TOUCH_START_SIGNAL, false);
  EventSignal<WTouchEvent> *touchEnd
    = touchEventSignal(TOUCH_END_SIGNAL, false);
  EventSignal<WTouchEvent> *touchMove
    = touchEventSignal(TOUCH_MOVE_SIGNAL, false);

  bool updateTouchMove
    = (touchMove && touchMove->needsUpdate(all));

  bool updateTouchStart
    = (touchStart && touchStart->needsUpdate(all))
    || updateTouchMove;

  bool updateTouchEnd
    = (touchEnd && touchEnd->needsUpdate(all))
    || updateTouchMove;

  if (updateTouchStart) {
    /*
     * when we have a touchStart event, we also need a touchEnd event
     * to be able to compute dragDX/Y.
     *
     * When we have:
     *  - a touchStart + (touchMove or touchEnd),
     * we need to capture everything after on touch start, and keep track of the
     * down button if we have a touchMove 
     */
    WStringStream js;

    js << CheckDisabled;

    if (touchEnd && touchEnd->isConnected())
      js << app->javaScriptClass() << "._p_.saveDownPos(event);";

    if ((touchStart && touchStart->isConnected()
	    && ((touchEnd && touchEnd->isConnected())
		|| (touchMove && touchMove->isConnected()))))
      js << WT_CLASS ".capture(this);";

    if (touchStart) {
      js << touchStart->javaScript();
      element.setEvent("touchstart", js.str(),
		       touchStart->encodeCmd(), touchStart->isExposedSignal());
      touchStart->updateOk();
    } else
      element.setEvent("touchstart", js.str(), std::string(), false);
  }

  if (updateTouchEnd) {
    WStringStream js;

    /*
     * when we have a touchMove, we need to keep track
     * of removing touch.
     */
    js << CheckDisabled;

    if (touchEnd) {
      js << touchEnd->javaScript();
      element.setEvent("touchend", js.str(),
		         touchEnd->encodeCmd(), touchEnd->isExposedSignal());
      touchEnd->updateOk();
    } else
      element.setEvent("touchend", js.str(), std::string(), false);
  }

  if (updateTouchMove) {
    
    if (touchMove) {
      element.setEvent("touchmove", touchMove->javaScript(),
			touchMove->encodeCmd(), touchMove->isExposedSignal());
      touchMove->updateOk();
    }
  }

  /*
   * -- mix mouseClick and mouseDblClick events in mouseclick since we
   *    only want to fire one of both
   */
  EventSignal<WMouseEvent> *mouseClick
    = mouseEventSignal(M_CLICK_SIGNAL, false);
  EventSignal<WMouseEvent> *mouseDblClick
    = mouseEventSignal(DBL_CLICK_SIGNAL, false);  

  bool updateMouseClick
    = (mouseClick && mouseClick->needsUpdate(all))
    || (mouseDblClick && mouseDblClick->needsUpdate(all));

  if (updateMouseClick) {
    WStringStream js;

    js << CheckDisabled;

    if (mouseDrag)
      js << "if (" WT_CLASS ".dragged()) return;";

    if (mouseDblClick && mouseDblClick->needsUpdate(all)) {
      /*
       * Click: if timer is running:
       *  - clear timer, dblClick()
       *  - start timer, clear timer and click()
       */

      /* We have to prevent this immediately ! */
      if (mouseClick) {
	if (mouseClick->defaultActionPrevented() ||
	    mouseClick->propagationPrevented()) {
	  js << WT_CLASS ".cancelEvent(e";
	  if (mouseClick->defaultActionPrevented() &&
	      mouseClick->propagationPrevented())
	    js << ");";
	  else if (mouseClick->defaultActionPrevented())
	    js << ",0x2);";
	  else
	    js << ",0x1);";
	}
      }

      js << "if(" WT_CLASS ".isDblClick(o, e)) {"
	 << mouseDblClick->javaScript();

      if (mouseDblClick->isExposedSignal())
	js << app->javaScriptClass()
		 << "._p_.update(o,'" << mouseDblClick->encodeCmd()
		 << "',e,true);";

      mouseDblClick->updateOk();

      js <<
	"}else{"
	"""if (" WT_CLASS ".isIElt9 && document.createEventObject) "
	""  "e = document.createEventObject(e);"
	"""o.wtE1 = e;"
	"""o.wtClickTimeout = setTimeout(function() {"
	""   "o.wtClickTimeout = null; o.wtE1 = null;";

      if (mouseClick) {
	js << mouseClick->javaScript();

	if (mouseClick->isExposedSignal()) {
	  js << app->javaScriptClass()
		   << "._p_.update(o,'" << mouseClick->encodeCmd()
		   << "',e,true);";
	}

	mouseClick->updateOk();
      }

      const Configuration& conf = app->environment().server()->configuration();
      js << "}," << conf.doubleClickTimeout() << ");}";
    } else {
      if (mouseClick && mouseClick->needsUpdate(all)) {
	js << mouseClick->javaScript();

	if (mouseClick->isExposedSignal()) {
	  js << app->javaScriptClass()
	     << "._p_.update(o,'" << mouseClick->encodeCmd()
	     << "',e,true);";
	}

	mouseClick->updateOk();
      }
    }

    element.setEvent(CLICK_SIGNAL, js.str(),
		     mouseClick ? mouseClick->encodeCmd() : "");

    if (mouseDblClick) {
      if (app->environment().agentIsIElt(9))
	element.setEvent("dblclick", "this.onclick()");
    }
  }

  /*
   * -- mouseOver with delay
   */
  EventSignal<WMouseEvent> *mouseOver
    = mouseEventSignal(MOUSE_OVER_SIGNAL, false);
  EventSignal<WMouseEvent> *mouseOut
    = mouseEventSignal(MOUSE_OUT_SIGNAL, false); 

  bool updateMouseOver = mouseOver && mouseOver->needsUpdate(all);

  if (mouseOverDelay_) {
    if (updateMouseOver) {
      WStringStream js;
      js << "o.over=setTimeout(function() {"
	 << "o.over = null;"
	 << mouseOver->javaScript();

      if (mouseOver->isExposedSignal()) {
	js << app->javaScriptClass()
	   << "._p_.update(o,'" << mouseOver->encodeCmd() << "',e,true);";
      }

      js << "}," << mouseOverDelay_ << ");";

      element.setEvent("mouseover", js.str(), "");

      mouseOver->updateOk();

      if (!mouseOut)
	mouseOut = mouseEventSignal(MOUSE_OUT_SIGNAL, true);

      element.setEvent("mouseout",
		       "clearTimeout(o.over); o.over=null;"
		       + mouseOut->javaScript(),
		       mouseOut->encodeCmd(), mouseOut->isExposedSignal());
      mouseOut->updateOk();
    }
  } else {
    if (updateMouseOver) {
      element.setEventSignal("mouseover", *mouseOver);
      mouseOver->updateOk();
    }

    bool updateMouseOut = mouseOut && mouseOut->needsUpdate(all);

    if (updateMouseOut) {
      element.setEventSignal("mouseout", *mouseOut);
      mouseOut->updateOk();
    }
  }

  EventSignal<> *dragStart = voidEventSignal(DRAGSTART_SIGNAL, false);
  if (dragStart && dragStart->needsUpdate(all)) {
    element.setEventSignal("dragstart", *dragStart);
    dragStart->updateOk();
  }

  updateEventSignals(element, all);

  WWebWidget::updateDom(element, all);
}

void WInteractWidget::setMouseOverDelay(int delay)
{
  mouseOverDelay_ = delay;

  EventSignal<WMouseEvent> *mouseOver
    = mouseEventSignal(MOUSE_OVER_SIGNAL, false);
  if (mouseOver)
    mouseOver->ownerRepaint();
}

int WInteractWidget::mouseOverDelay() const
{
  return mouseOverDelay_;
}

void WInteractWidget::updateEventSignals(DomElement& element, bool all)
{
  EventSignalList& other = eventSignals();

  for (EventSignalList::iterator i = other.begin(); i != other.end(); ++i) {
    EventSignalBase& s = **i;

    if (s.name() == WInteractWidget::M_CLICK_SIGNAL
	&& flags_.test(BIT_REPAINT_TO_AJAX))
      element.unwrap();

    updateSignalConnection(element, s, s.name(), all);
  }
}

void WInteractWidget::propagateRenderOk(bool deep)
{
  EventSignalList& other = eventSignals();

  for (EventSignalList::iterator i = other.begin(); i != other.end(); ++i) {
    EventSignalBase& s = **i;
    s.updateOk();
  }

  WWebWidget::propagateRenderOk(deep);
}

void WInteractWidget::load()
{
  if (!isDisabled()) {
    if (parent())
      flags_.set(BIT_ENABLED, parent()->isEnabled());
    else
      flags_.set(BIT_ENABLED, true);
  } else
    flags_.set(BIT_ENABLED, false);

  WWebWidget::load();
}

bool WInteractWidget::isEnabled() const
{
  return !isDisabled() && flags_.test(BIT_ENABLED);
}

void WInteractWidget::propagateSetEnabled(bool enabled)
{
  flags_.set(BIT_ENABLED, enabled);

  WApplication *app = WApplication::instance();
  std::string disabledClass = app->theme()->disabledClass();
  toggleStyleClass(disabledClass, !enabled, true);

  WWebWidget::propagateSetEnabled(enabled);
}

void WInteractWidget::setDraggable(const std::string& mimeType,
				   WWidget *dragWidget, bool isDragWidgetOnly,
				   WObject *sourceObject)
{
  if (dragWidget == nullptr)
    dragWidget = this;

  if (sourceObject == nullptr)
    sourceObject = this;

  if (isDragWidgetOnly) {
    dragWidget->hide();
  }

  WApplication *app = WApplication::instance();

  setAttributeValue("dmt", mimeType);
  setAttributeValue("dwid", dragWidget->id());
  setAttributeValue("dsid", app->encodeObject(sourceObject));

  if (!dragSlot_) {
    dragSlot_.reset(new JSlot());
    dragSlot_->setJavaScript("function(o,e){" + app->javaScriptClass()
			     + "._p_.dragStart(o,e);" + "}");
  }

  if (!dragTouchSlot_) {
    dragTouchSlot_.reset(new JSlot());
    dragTouchSlot_->setJavaScript("function(o,e){" + app->javaScriptClass()
				+ "._p_.touchStart(o,e);" + "}");
  }

  if (!dragTouchEndSlot_) {
    dragTouchEndSlot_.reset(new JSlot());
    dragTouchEndSlot_->setJavaScript("function(){" + app->javaScriptClass()
				+ "._p_.touchEnded();" + "}");
  }

  voidEventSignal(DRAGSTART_SIGNAL, true)->preventDefaultAction(true);

  mouseWentDown().connect(*dragSlot_);
  touchStarted().connect(*dragTouchSlot_);
  touchStarted().preventDefaultAction(true);
  touchEnded().connect(*dragTouchEndSlot_);
}

void WInteractWidget::unsetDraggable()
{
  if (dragSlot_) {
    mouseWentDown().disconnect(*dragSlot_);
    dragSlot_.reset();
  }

  if (dragTouchSlot_) {
    touchStarted().disconnect(*dragTouchSlot_);
    dragTouchSlot_.reset();
  }

  if (dragTouchEndSlot_) {
    touchEnded().disconnect(*dragTouchEndSlot_);
    dragTouchEndSlot_.reset();
  }

  EventSignal<> *dragStart = voidEventSignal(DRAGSTART_SIGNAL, false);
  if (dragStart) {
    dragStart->preventDefaultAction(false);
  }
}

}
