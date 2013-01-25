/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EventsDemo.h"

#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <Wt/WLineEdit>
#include <Wt/WHBoxLayout>
#include <Wt/Utils>
#include "DragExample.h"

EventsDemo::EventsDemo(EventDisplayer *ed)
  : TopicWidget(ed),
    keyEventRepeatCounter_(0)
{ }

void EventsDemo::populateSubMenu(WMenu *menu)
{
  menu->addItem("Overview", addText(tr("events-intro")));
  menu->addItem("Keyboard Events", wKeyEvent());
  menu->addItem("Mouse Events", wMouseEvent());
  menu->addItem("Drag & Drop", wDropEvent());
  menu->addItem("Other events", addText(tr("other-events")));
}

WWidget *EventsDemo::wKeyEvent()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WKeyEvent", result);
  addText(tr("events-WKeyEvent-1"), result);
  WLineEdit *l = new WLineEdit(result);
  l->setTextSize(50);
  l->keyWentUp().connect(this, &EventsDemo::showKeyWentUp);
  l->keyWentDown().connect(this, &EventsDemo::showKeyWentDown);
  
  addText(tr("events-WKeyEvent-2"), result);
  l = new WLineEdit(result);
  l->setTextSize(50);
  l->keyPressed().connect(this, &EventsDemo::showKeyPressed);
  
  addText(tr("events-WKeyEvent-3"), result);
  l = new WLineEdit(result);
  l->setTextSize(50);
  l->enterPressed().connect(this, &EventsDemo::showEnterPressed);
  l->escapePressed().connect(this, &EventsDemo::showEscapePressed);
  new WBreak(result);
  addText("Last event: ", result);
  keyEventType_ = new WText(result);
  new WBreak(result);
  keyEventDescription_ = new WText(result);

  return result;
}

WWidget *EventsDemo::wMouseEvent()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WMouseEvent", result);
  addText(tr("events-WMouseEvent"), result);
  WContainerWidget *c = new WContainerWidget(result);
  WHBoxLayout *hlayout = new WHBoxLayout;
  c->setLayout(hlayout);
  WContainerWidget *l = new WContainerWidget;
  WContainerWidget *r = new WContainerWidget;
  new WText("clicked<br/>doubleClicked<br/>mouseWentOut<br/>mouseWentOver",
	    l);
  new WText("mouseWentDown<br/>mouseWentUp<br/>mouseMoved<br/>mouseWheel", r);
  hlayout->addWidget(l);
  hlayout->addWidget(r);
  c->resize(600, 300);
  l->decorationStyle().setBackgroundColor(Wt::gray);
  r->decorationStyle().setBackgroundColor(Wt::gray);
  // prevent that firefox interprets drag as drag&drop action
  l->setStyleClass("unselectable");
  r->setStyleClass("unselectable");
  l->clicked().connect(this, &EventsDemo::showClicked);
  l->doubleClicked().connect(this, &EventsDemo::showDoubleClicked);
  l->mouseWentOut().connect(this, &EventsDemo::showMouseWentOut);
  l->mouseWentOver().connect(this, &EventsDemo::showMouseWentOver);
  r->mouseMoved().connect(this, &EventsDemo::showMouseMoved);
  r->mouseWentUp().connect(this, &EventsDemo::showMouseWentUp);
  r->mouseWentDown().connect(this, &EventsDemo::showMouseWentDown);
  r->mouseWheel().connect(this, &EventsDemo::showMouseWheel);
  r->mouseWheel().preventDefaultAction(true);

  l->setAttributeValue
    ("oncontextmenu",
     "event.cancelBubble = true; event.returnValue = false; return false;");
  r->setAttributeValue
    ("oncontextmenu",
     "event.cancelBubble = true; event.returnValue = false; return false;");

  new WBreak(result);
  new WText("Last event: ", result);
  mouseEventType_ = new WText(result);
  new WBreak(result);
  mouseEventDescription_ = new WText(result);

  return result;
}

WWidget *EventsDemo::wDropEvent()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WDropEvent", result);
  addText(tr("events-WDropEvent"), result);
  new DragExample(result);

  return result;
}

namespace {
  std::ostream &operator<<(std::ostream &o, Wt::WMouseEvent::Button b)
  {
    switch (b) {
    case WMouseEvent::NoButton:
      return o << "No button";
    case WMouseEvent::LeftButton:
      return o << "LeftButton";
    case WMouseEvent::RightButton:
      return o << "RightButton";
    case WMouseEvent::MiddleButton:
      return o << "MiddleButton";
    default:
      return o << "Unknown Button";
    }
  }

  std::ostream &operator<<(std::ostream &o, Wt::Key k)
  {
    switch(k) {
    default:
    case Key_unknown : return o << "Key_unknown";
    case Key_Enter : return o << "Key_Enter";
    case Key_Tab : return o << "Key_Tab";
    case Key_Backspace : return o << "Key_Backspace";
    case Key_Shift : return o << "Key_Shift";
    case Key_Control : return o << "Key_Control";
    case Key_Alt : return o << "Key_Alt";
    case Key_PageUp : return o << "Key_PageUp";
    case Key_PageDown : return o << "Key_PageDown";
    case Key_End : return o << "Key_End";
    case Key_Home : return o << "Key_Home";
    case Key_Left : return o << "Key_Left";
    case Key_Up : return o << "Key_Up";
    case Key_Right : return o << "Key_Right";
    case Key_Down : return o << "Key_Down";
    case Key_Insert : return o << "Key_Insert";
    case Key_Delete : return o << "Key_Delete";
    case Key_Escape : return o << "Key_Escape";
    case Key_F1 : return o << "Key_F1";
    case Key_F2 : return o << "Key_F2";
    case Key_F3 : return o << "Key_F3";
    case Key_F4 : return o << "Key_F4";
    case Key_F5 : return o << "Key_F5";
    case Key_F6 : return o << "Key_F6";
    case Key_F7 : return o << "Key_F7";
    case Key_F8 : return o << "Key_F8";
    case Key_F9 : return o << "Key_F9";
    case Key_F10 : return o << "Key_F10";
    case Key_F11 : return o << "Key_F11";
    case Key_F12 : return o << "Key_F12";
    case Key_Space : return o << "Key_Space";
    case Key_A : return o << "Key_A";
    case Key_B : return o << "Key_B";
    case Key_C : return o << "Key_C";
    case Key_D : return o << "Key_D";
    case Key_E : return o << "Key_E";
    case Key_F : return o << "Key_F";
    case Key_G : return o << "Key_G";
    case Key_H : return o << "Key_H";
    case Key_I : return o << "Key_I";
    case Key_J : return o << "Key_J";
    case Key_K : return o << "Key_K";
    case Key_L : return o << "Key_L";
    case Key_M : return o << "Key_M";
    case Key_N : return o << "Key_N";
    case Key_O : return o << "Key_O";
    case Key_P : return o << "Key_P";
    case Key_Q : return o << "Key_Q";
    case Key_R : return o << "Key_R";
    case Key_S : return o << "Key_S";
    case Key_T : return o << "Key_T";
    case Key_U : return o << "Key_U";
    case Key_V : return o << "Key_V";
    case Key_W : return o << "Key_W";
    case Key_X : return o << "Key_X";
    case Key_Y : return o << "Key_Y";
    case Key_Z : return o << "Key_Z";
    }
  }

  std::ostream &operator<<(std::ostream &o, Wt::WMouseEvent::Coordinates c)
  {
    return o << c.x << ", " << c.y;
  }
  std::string modifiersToString(const WFlags< KeyboardModifier >& modifiers)
  {
    std::stringstream o;
    if (modifiers & ShiftModifier) o << "Shift ";
    if (modifiers & ControlModifier) o << "Control ";
    if (modifiers & AltModifier) o << "Alt ";
    if (modifiers & MetaModifier) o << "Meta ";
    if (modifiers == 0) o << "No modifiers";
    return o.str();
  }
}

void EventsDemo::setKeyType(const std::string &type, const WKeyEvent *e)
{
  std::string repeatString = "";
  if (lastKeyType_ == type) {
    keyEventRepeatCounter_++;
    repeatString = " ("
      + boost::lexical_cast<std::string>(keyEventRepeatCounter_) + " times)";
  } else {
    lastKeyType_ = type;
    keyEventRepeatCounter_ = 0;
  }
  keyEventType_->setText(type + repeatString);
  if (e) {
    describe(*e);
  } else {
    keyEventDescription_->setText("");
  }
}

void EventsDemo::showKeyWentUp(const WKeyEvent &e)
{
  setKeyType("keyWentUp", &e);
}

void EventsDemo::showKeyWentDown(const WKeyEvent &e)
{
  setKeyType("keyWentDown", &e);
}

void EventsDemo::showKeyPressed(const WKeyEvent &e)
{
  setKeyType("keyPressed", &e);
}

void EventsDemo::showEnterPressed()
{
  setKeyType("enterPressed");
}

void EventsDemo::showEscapePressed()
{
  setKeyType("escapePressed");
}

void EventsDemo::describe(const Wt::WKeyEvent &e)
{
  std::stringstream ss;
  ss << "Key: " << e.key() << "<br/>"
     << "Modifiers: " << modifiersToString(e.modifiers()) << "<br/>";
  int charCode = (int)e.charCode();
  if (charCode) {
    ss << "Char code: " << charCode << "<br/>"
       << "text: " << Utils::htmlEncode(e.text()) << "<br/>";
  }
  keyEventDescription_->setText(ss.str());
}

void EventsDemo::showClicked(const WMouseEvent &e)
{
  mouseEventType_->setText("clicked");
  describe(e);
}

void EventsDemo::showDoubleClicked(const WMouseEvent &e)
{
  mouseEventType_->setText("doubleClicked");
  describe(e);
}

void EventsDemo::showMouseWentOut(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseWentOut");
  describe(e);
}

void EventsDemo::showMouseWheel(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseWheel");
  describe(e);
}

void EventsDemo::showMouseWentOver(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseWentOver");
  describe(e);
}

void EventsDemo::showMouseMoved(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseMoved");
  describe(e);
}

void EventsDemo::showMouseWentUp(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseWentUp");
  describe(e);
}

void EventsDemo::showMouseWentDown(const WMouseEvent &e)
{
  mouseEventType_->setText("mouseWentDown");
  describe(e);
}

void EventsDemo::describe(const Wt::WMouseEvent &e)
{
  std::stringstream ss;
  ss << "Button: " << e.button() << "<br/>"
     << "Modifiers: " << modifiersToString(e.modifiers()) << "<br/>"
     << "Document coordinates: " << e.document() << "<br/>"
     << "Window coordinates: " << e.window() << "<br/>"
     << "Screen coordinates: " << e.screen() << "<br/>"
     << "Widget coordinates: " << e.widget() << "<br/>"
     << "DragDelta coordinates: " << e.dragDelta() << "<br/>"
     << "Wheel delta: " << e.wheelDelta() << "<br/>";
  mouseEventDescription_->setText(ss.str());
}
