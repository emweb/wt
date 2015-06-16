/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCheckBox"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WJavaScript"
#include "DomElement.h"

namespace Wt {

WCheckBox::WCheckBox(WContainerWidget *parent)
  : WAbstractToggleButton(parent),
    triState_(false),
	partialStateSelectable_(false)
{
  setFormObject(true);
}

WCheckBox::WCheckBox(const WString& text, WContainerWidget *parent)
  : WAbstractToggleButton(text, parent),
    triState_(false),
	partialStateSelectable_(false)
{
  setFormObject(true);
}

void WCheckBox::setTristate(bool tristate)
{
  triState_ = tristate;

  if (triState_) {
    if (!supportsIndeterminate(WApplication::instance()->environment()))
	  updateJSlot();
  }
}

void WCheckBox::setPartialStateSelectable(bool t) {
  if(t && !isTristate()) setTristate(true);
  partialStateSelectable_ = t;

  updateJSlot();
  updateNextState();
}

void WCheckBox::updateJSlot() {
  JSlot *slot = 0;
  std::string partialOn, partialOff;
  if(!supportsIndeterminate(WApplication::instance()->environment())) {
	partialOff = "obj.style.opacity='';";
	partialOn = "obj.style.opacity='0.5';";
	if(triState_ && !partialStateSelectable_) 
	  slot = new JSlot("function(obj, e) { " + partialOff + "}", this);
  } else {
	partialOn  = "obj.indeterminate=true;";
	partialOff = "obj.indeterminate=false;";
  }

  if(partialStateSelectable_) {
	std::stringstream ss;
	ss << "function(obj, e) {\n"
	   << "if(obj.nextState == 'c'){\n"
	   << "obj.checked=true;"
	   <<  partialOff
	   << " obj.nextState='u';"
	   << "} else if( obj.nextState=='i') {\n"
	   << "obj.nextState='c';"
	   << partialOn 
	   << " } else if( obj.nextState=='u') {\n"
	   << "obj.nextState='i';"
	   << "obj.checked=false;"
	   << partialOff 
	   << " } else obj.nextState='i';"
	   << "}";
	slot = new JSlot(ss.str(), this);
  }
  if(slot)
	changed().connect(*slot);
}

void WCheckBox::updateNextState() {
  std::string nextState;
  switch(state_) {
	case Checked:
	  nextState="u";
	  break;
	case Unchecked:
	  nextState="i";
	  break;
	case PartiallyChecked:
	  nextState="c";
	  break;
  }
  if(partialStateSelectable_)
	doJavaScript(this->jsRef() + ".nextState='"+nextState+"'");
  else 
	doJavaScript(this->jsRef() + ".nextState=null");
}


void WCheckBox::setCheckState(CheckState state)
{
  WAbstractToggleButton::setCheckState(state);
  updateNextState();
}

void WCheckBox::updateInput(DomElement& input, bool all)
{
  if (all)
    input.setAttribute("type", "checkbox");
}

}
