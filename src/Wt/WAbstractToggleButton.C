/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLabel"
#include "DomElement.h"
#include "CgiParser.h"

namespace Wt {

WAbstractToggleButton::WAbstractToggleButton(WContainerWidget *parent)
  : WFormWidget(parent),
    checked(this),
    unChecked(this),
    checked_(false),
    checkedChanged_(false)
{
  WLabel *label = new WLabel(parent);
  label->setBuddy(this);

  implementStateless(&WAbstractToggleButton::setChecked,
		     &WAbstractToggleButton::undoSetChecked);
  implementStateless(&WAbstractToggleButton::setUnChecked,
		     &WAbstractToggleButton::undoSetUnChecked);
}

WAbstractToggleButton::WAbstractToggleButton(const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    checked(this),
    unChecked(this),
    checked_(false),
    checkedChanged_(false)
{ 
  WLabel *label = new WLabel(text, parent);
  label->setBuddy(this);

  implementStateless(&WAbstractToggleButton::setChecked,
		     &WAbstractToggleButton::undoSetChecked);
  implementStateless(&WAbstractToggleButton::setUnChecked,
		     &WAbstractToggleButton::undoSetUnChecked);
}

WAbstractToggleButton::WAbstractToggleButton(bool withLabel,
					     const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    checked(this),
    unChecked(this),
    checked_(false),
    checkedChanged_(false)
{
  if (withLabel) {
    WLabel *label = new WLabel(text, parent);
    label->setBuddy(this);
  }
}

void WAbstractToggleButton::load()
{
  WLabel *l = label();
  if (l) {
    WWidget *w = l->parent();
    if (!w) {
      WContainerWidget *p = dynamic_cast<WContainerWidget *>(parent());
      if (p)
	p->insertWidget(p->indexOf(this) + 1, l);
    }
  }
  WFormWidget::load();
}

void WAbstractToggleButton::setText(const WString& text)
{
  if (label())
    label()->setText(text);
}

const WString WAbstractToggleButton::text() const
{
  if (label())
    return label()->text();
  else
    return WString();
}

void WAbstractToggleButton::setChecked(bool how)
{
  if (canOptimizeUpdates() && checked_ == how)
    return;

  checked_ = how;
  checkedChanged_ = true;

  /*
  if (how)
    checked.emit();
  else
    unChecked.emit();
  */

  repaint(RepaintPropertyIEMobile);
}

void WAbstractToggleButton::setChecked()
{
  wasChecked_ = checked_;
  setChecked(true);
}

void WAbstractToggleButton::setUnChecked()
{
  wasChecked_ = checked_;
  setChecked(false);
}

void WAbstractToggleButton::undoSetChecked()
{
  setChecked(checked_);
}

void WAbstractToggleButton::undoSetUnChecked()
{
  undoSetChecked();
}

void WAbstractToggleButton::updateDom(DomElement& element, bool all)
{
  if (checkedChanged_ || all) {
    element.setProperty(Wt::PropertyChecked, checked_ ? "true" : "false");
    checkedChanged_ = false;
  }

  const WEnvironment& env = WApplication::instance()->environment();

  bool needUpdateClickedSignal =
    (clicked.needUpdate()
     || (env.agentIE() && changed.needUpdate()) // onchange does not work on IE
     || checked.needUpdate()
     || unChecked.needUpdate());

  WFormWidget::updateDom(element, all);

  if (needUpdateClickedSignal || all) {
    DomElement *e = DomElement::getForUpdate(this, DomElement_INPUT);

    std::vector<DomElement::EventAction> actions;

    if (checked.isConnected())
      actions.push_back
	(DomElement::EventAction(e->createReference() + ".checked == true",
				 checked.javaScript(),
				 checked.encodeCmd(),
				 checked.isExposedSignal()));
    if (unChecked.isConnected())
      actions.push_back
	(DomElement::EventAction(e->createReference() + ".checked == false",
				 unChecked.javaScript(),
				 unChecked.encodeCmd(),
				 unChecked.isExposedSignal()));

    if (env.agentIE() && changed.isConnected())
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   changed.javaScript(),
				   changed.encodeCmd(),
				   changed.isExposedSignal()));

    if (clicked.isConnected())
      actions.push_back
	(DomElement::EventAction(std::string(),
				 clicked.javaScript(),
				 clicked.encodeCmd(),
				 clicked.isExposedSignal()));

    if (!(all && actions.empty()))
      element.setEvent("click", actions);

    clicked.updateOk();
    checked.updateOk();
    unChecked.updateOk();
    changed.updateOk();

    delete e;
  }
}

void WAbstractToggleButton::setFormData(CgiEntry *entry)
{
  checked_ = (entry->value() != "0");
}

void WAbstractToggleButton::setNoFormData()
{
  if (isEnabled() && isVisible()) {
    checked_ = false;
  }
}

}
