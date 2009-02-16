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

WInteractWidget::WInteractWidget(WContainerWidget *parent)
  : WWebWidget(parent),
    keyWentDown(this),
    keyPressed(this),
    keyWentUp(this),
    enterPressed(this),
    escapePressed(this),
    clicked(this),
    doubleClicked(this),
    mouseWentDown(this),
    mouseWentUp(this),
    mouseWentOut(this),
    mouseWentOver(this),
    mouseMoved(this),
    dragSlot_(0)
{ }

WInteractWidget::~WInteractWidget()
{
  delete dragSlot_;
}

void WInteractWidget::updateDom(DomElement& element, bool all)
{
  if ((all && (enterPressed.isConnected()
	       || escapePressed.isConnected()
	       || keyWentDown.isConnected()))
      || keyWentDown.needUpdate()
      || enterPressed.needUpdate()
      || escapePressed.needUpdate()) {
    std::vector<DomElement::EventAction> actions;

    if (enterPressed.isConnected()) {
      /*
       * prevent enterPressed from triggering a changed event on all
       * browsers except for Opera and IE
       */
      std::string extraJS;
      const WEnvironment& env = WApplication::instance()->environment();

      if (dynamic_cast<WFormWidget *>(this)
	  && env.userAgent().find("Opera") == std::string::npos
	  && !env.agentIE())
	extraJS
	  = "var g=this.onchange; this.onchange=function(){this.onchange=g;};";

      actions.push_back
	(DomElement::EventAction("(e.keyCode && e.keyCode == 13)",
				 enterPressed.javaScript() + extraJS,
				 enterPressed.encodeCmd(),
				 enterPressed.isExposedSignal()));
    }

    if (escapePressed.isConnected()) {
      actions.push_back
	(DomElement::EventAction("(e.keyCode && e.keyCode == 27)",
				 escapePressed.javaScript(),
				 escapePressed.encodeCmd(),
				 escapePressed.isExposedSignal()));
    }

    if (keyWentDown.isConnected()) {
      actions.push_back
	(DomElement::EventAction(std::string(),
				 keyWentDown.javaScript(),
				 keyWentDown.encodeCmd(),
				 keyWentDown.isExposedSignal()));
    }

    if (!actions.empty())
      element.setEvent("keydown", actions);
    else if (!all)
      element.setEvent("keydown", "", "");

    enterPressed.updateOk();
    escapePressed.updateOk();
    keyWentDown.updateOk();
  }

  updateSignalConnection(element, keyPressed, "keypress", all);
  updateSignalConnection(element, keyWentUp, "keyup", all);

  updateSignalConnection(element, clicked, "click", all);
  updateSignalConnection(element, doubleClicked, "dblclick", all);
  updateSignalConnection(element, mouseWentDown, "mousedown", all);
  updateSignalConnection(element, mouseMoved, "mousemove", all);
  updateSignalConnection(element, mouseWentOut, "mouseout", all);
  updateSignalConnection(element, mouseWentOver, "mouseover", all);
  updateSignalConnection(element, mouseWentUp, "mouseup", all);

  WWebWidget::updateDom(element, all);
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
  setAttributeValue("dwid", dragWidget->formName());
  setAttributeValue("dsid", app->encodeObject(sourceObject));

  if (!dragSlot_) {
    dragSlot_ = new JSlot();
    dragSlot_->setJavaScript("function(o,e){" + app->javaScriptClass()
			     + "._p_.dragStart(o,e);" + "}");
  }

  mouseWentDown.connect(*dragSlot_);
}

}
