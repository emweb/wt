/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WInteractWidget"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WFormWidget"
#include "Wt/WJavaScript"

#include "DomElement.h"

/*
 * FIXME: provide a cross-browser mechanism to "cancel" the default
 * action for a keyboard event (and other events!).
 *
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
const char *WInteractWidget::DBL_CLICK_SIGNAL = "dblclick";
const char *WInteractWidget::MOUSE_DOWN_SIGNAL = "M_mousedown";
const char *WInteractWidget::MOUSE_UP_SIGNAL = "M_mouseup";
const char *WInteractWidget::MOUSE_OUT_SIGNAL = "mouseout";
const char *WInteractWidget::MOUSE_OVER_SIGNAL = "mouseover";
const char *WInteractWidget::MOUSE_MOVE_SIGNAL = "M_mousemove";
const char *WInteractWidget::MOUSE_DRAG_SIGNAL = "M_mousedrag";
const char *WInteractWidget::MOUSE_WHEEL_SIGNAL = "mousewheel";
const char *WInteractWidget::TOUCH_START_SIGNAL = "touchstart";
const char *WInteractWidget::TOUCH_MOVE_SIGNAL = "touchmove";
const char *WInteractWidget::TOUCH_END_SIGNAL = "touchend";
const char *WInteractWidget::GESTURE_START_SIGNAL = "gesturestart";
const char *WInteractWidget::GESTURE_CHANGE_SIGNAL = "gesturechange";
const char *WInteractWidget::GESTURE_END_SIGNAL = "gestureend";

WInteractWidget::WInteractWidget(WContainerWidget *parent)
  : WWebWidget(parent),
    dragSlot_(0)
{ }

WInteractWidget::~WInteractWidget()
{
  delete dragSlot_;
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
  return *mouseEventSignal(CLICK_SIGNAL, true);
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
  return *mouseEventSignal(MOUSE_WHEEL_SIGNAL, true);
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

  EventSignal<> *enterPress = voidEventSignal(ENTER_PRESS_SIGNAL, false);
  EventSignal<> *escapePress = voidEventSignal(ESCAPE_PRESS_SIGNAL, false);
  EventSignal<WKeyEvent> *keyDown = keyEventSignal(KEYDOWN_SIGNAL, false);

  updateKeyDown = (enterPress && enterPress->needsUpdate(all))
    || (escapePress && escapePress->needsUpdate(all))
    || (keyDown && keyDown->needsUpdate(all));

  if (updateKeyDown) {
    std::vector<DomElement::EventAction> actions;

    if (enterPress) {
      if (enterPress->isConnected()) {
	/*
	 * prevent enterPressed from triggering a changed event on all
	 * browsers except for Opera and IE
	 */
	std::string extraJS;
	const WEnvironment& env = WApplication::instance()->environment();

	if (dynamic_cast<WFormWidget *>(this)
	    && !env.agentIsOpera() && !env.agentIsIE())
	  extraJS = "var g=this.onchange;"
	    ""      "this.onchange=function(){this.onchange=g;};";

	actions.push_back
	  (DomElement::EventAction("(e.keyCode && e.keyCode == 13)",
				   enterPress->javaScript() + extraJS,
				   enterPress->encodeCmd(),
				   enterPress->isExposedSignal()));
      }
      enterPress->updateOk();
    }

    if (escapePress) {
      if (escapePress->isConnected()) {
	actions.push_back
	  (DomElement::EventAction("(e.keyCode && e.keyCode == 27)",
				   escapePress->javaScript(),
				   escapePress->encodeCmd(),
				   escapePress->isExposedSignal()));
      }
      escapePress->updateOk();
    }

    if (keyDown) {
      if (keyDown->isConnected()) {
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
    std::string js;
    if (mouseUp && mouseUp->isConnected())
      js += WApplication::instance()->javaScriptClass()
	+ "._p_.saveDownPos(event);";

    if ((mouseDrag && mouseDrag->isConnected())
	|| (mouseDown && mouseDown->isConnected()
	    && ((mouseUp && mouseUp->isConnected())
		|| (mouseMove && mouseMove->isConnected()))))
      js += WT_CLASS ".capture(this);";

    if ((mouseMove && mouseMove->isConnected())
	|| (mouseDrag && mouseDrag->isConnected()))
      js += WT_CLASS ".mouseDown(e);";

    if (mouseDown) {
      js += mouseDown->javaScript();
      element.setEvent("mousedown", js,
		       mouseDown->encodeCmd(), mouseDown->isExposedSignal());
      mouseDown->updateOk();
    } else
      element.setEvent("mousedown", js, std::string(), false);
  }

  if (updateMouseUp) {
    /*
     * when we have a mouseMove or mouseDrag event, we need to keep track
     * of unpressing the button.
     */
    std::string js;
    if ((mouseMove && mouseMove->isConnected())
	|| (mouseDrag && mouseDrag->isConnected()))
      js += WT_CLASS ".mouseUp(e);";
      
    if (mouseUp) {
      js += mouseUp->javaScript();
      element.setEvent("mouseup", js,
		       mouseUp->encodeCmd(), mouseUp->isExposedSignal());
      mouseUp->updateOk();
    } else
      element.setEvent("mouseup", js, std::string(), false);
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
				 mouseDrag->javaScript(),
				 mouseDrag->encodeCmd(),
				 mouseDrag->isExposedSignal()));
      mouseDrag->updateOk();
    }

    element.setEvent("mousemove", actions);
  }

  updateEventSignals(element, all);

  WWebWidget::updateDom(element, all);
}

void WInteractWidget::updateEventSignals(DomElement& element, bool all)
{
  EventSignalList& other = eventSignals();

  for (EventSignalList::iterator i = other.begin(); i != other.end(); ++i) {
#ifndef WT_NO_BOOST_INTRUSIVE
    EventSignalBase& s = *i;
#else
    EventSignalBase& s = **i;
#endif

    if (s.name() == WInteractWidget::CLICK_SIGNAL
	&& flags_.test(BIT_REPAINT_TO_AJAX))
      element.unwrap();

    if ((s.name() != WInteractWidget::CLICK_SIGNAL
	 && s.name() != WInteractWidget::DBL_CLICK_SIGNAL)
	|| flags_.test(BIT_ENABLED))
      updateSignalConnection(element, s, s.name(), all);
    else
      element.setEvent(s.name(), WT_CLASS ".cancelEvent(event||window.event);");
  }
}

void WInteractWidget::propagateRenderOk(bool deep)
{
  EventSignalList& other = eventSignals();

  for (EventSignalList::iterator i = other.begin(); i != other.end(); ++i) {
#ifndef WT_NO_BOOST_INTRUSIVE
    EventSignalBase& s = *i;
#else
    EventSignalBase& s = **i;
#endif
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

  EventSignal<WMouseEvent> *s;

  s = mouseEventSignal(CLICK_SIGNAL, false);
  if (s)
    s->senderRepaint();

  s = mouseEventSignal(DBL_CLICK_SIGNAL, false);
  if (s)
    s->senderRepaint();

  WWebWidget::propagateSetEnabled(enabled);
}

void WInteractWidget::setDraggable(const std::string& mimeType,
				   WWidget *dragWidget, bool isDragWidgetOnly,
				   WObject *sourceObject)
{
  if (dragWidget == 0)
    dragWidget = this;

  if (sourceObject == 0)
    sourceObject = this;

  if (isDragWidgetOnly) {
    dragWidget->hide();
  }

  WApplication *app = WApplication::instance();

  setAttributeValue("dmt", mimeType);
  setAttributeValue("dwid", dragWidget->id());
  setAttributeValue("dsid", app->encodeObject(sourceObject));

  if (!dragSlot_) {
    dragSlot_ = new JSlot();
    dragSlot_->setJavaScript("function(o,e){" + app->javaScriptClass()
			     + "._p_.dragStart(o,e);" + "}");
  }

  mouseWentDown().connect(*dragSlot_);
}

}
