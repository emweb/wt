/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EventsDemo.h"

#include <Wt/WText.h>
#include <Wt/WBreak.h>
#include <Wt/WCssDecorationStyle.h>
#include <sstream>
#include <Wt/WLineEdit.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/Utils.h>
#include "DragExample.h"

EventsDemo::EventsDemo(EventDisplayer *ed)
  : TopicWidget(ed),
    keyEventRepeatCounter_(0)
{ }

void EventsDemo::populateSubMenu(WMenu *menu)
{
  menu->addItem("Overview", std::move(addText(tr("events-intro"))));
  menu->addItem("Keyboard Events", std::move(wKeyEvent()));
  menu->addItem("Mouse Events", std::move(wMouseEvent()));
  menu->addItem("Drag & Drop", std::move(wDropEvent()));
  menu->addItem("Other events", std::move(addText(tr("other-events"))));
}

std::unique_ptr<WWidget> EventsDemo::wKeyEvent()
{
  auto result = std::make_unique<WContainerWidget>();

  topic("WKeyEvent", result.get());
  result->addWidget(std::move(addText(tr("events-WKeyEvent-1"))));
  WLineEdit *l = result->addWidget(std::make_unique<WLineEdit>());
  l->setTextSize(50);
  l->keyWentUp().connect(this, &EventsDemo::showKeyWentUp);
  l->keyWentDown().connect(this, &EventsDemo::showKeyWentDown);
  
  result->addWidget(std::move(addText(tr("events-WKeyEvent-2"))));
  l = result->addWidget(std::make_unique<WLineEdit>());
  l->setTextSize(50);
  l->keyPressed().connect(this, &EventsDemo::showKeyPressed);
  
  result->addWidget(std::move(addText(tr("events-WKeyEvent-3"))));
  l = result->addWidget(std::make_unique<WLineEdit>());
  l->setTextSize(50);
  l->enterPressed().connect(this, &EventsDemo::showEnterPressed);
  l->escapePressed().connect(this, &EventsDemo::showEscapePressed);

  result->addWidget(std::make_unique<WBreak>());
  result->addWidget(std::move(addText("Last event: ")));
  keyEventType_ = result->addWidget(std::make_unique<WText>());
  result->addWidget(std::make_unique<WBreak>());
  keyEventDescription_ = result->addWidget(std::make_unique<WText>());

  return result;
}

WWidget *EventsDemo::wMouseEvent()
{
  auto result = std::make_unique<WContainerWidget>();

  topic("WMouseEvent", result.get());
  result->addWidget(std::move(addText(tr("events-WMouseEvent"))));

  WContainerWidget *c =
      result->addWidget(std::make_unique<WContainerWidget>());
  auto hlayout = c->setLayout(std::make_unique<WHBoxLayout>());

  auto l = hlayout->addWidget(std::make_unique<WContainerWidget>());
  auto r = hlayout->addWidget(std::make_unique<WContainerWidget>());
  l->addWidget(std::make_unique<WText>("clicked<br/>doubleClicked<br/>mouseWentOut<br/>mouseWentOver"));
  r->addWidget(std::make_unique<WText>("mouseWentDown<br/>mouseWentUp<br/>mouseMoved<br/>mouseWheel"));

  c->resize(600, 300);
  l->decorationStyle().setBackgroundColor(WColor(StandardColor::Gray));
  r->decorationStyle().setBackgroundColor(WColor(StandardColor::Gray));
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

  result->addWidget(std::make_unique<WBreak>());
  result->addWidget(std::make_unique<WText>("Last event: "));
  mouseEventType_ = result->addWidget(std::make_unique<WText>());
  result->addWidget(std::make_unique<WBreak>());
  mouseEventDescription_ = result->addwidget(std::make_unique<WText>());

  return result;
}

std::unique_ptr<WWidget> EventsDemo::wDropEvent()
{
  auto result = std::make_unique<WContainerWidget>();

  topic("WDropEvent", result.get());
  result->addWidget(std::move(addText(tr("events-WDropEvent"))));
  result->addWidget(std::make_unique<DragExample>());

  return result;
}

namespace {
  std::ostream &operator<<(std::ostream &o, WMouseEvent::Button b)
  {
    switch (b) {
    case MouseButton::None:
      return o << "No button";
    case MouseButton::Left:
      return o << "LeftButton";
    case MouseButton::Right:
      return o << "RightButton";
    case MouseButton::Middle:
      return o << "MiddleButton";
    default:
      return o << "Unknown Button";
    }
  }

  std::ostream &operator<<(std::ostream &o, Key k)
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

  std::ostream &operator<<(std::ostream &o, WMouseEvent::Coordinates c)
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
      + asString(keyEventRepeatCounter_) + " times)";
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

void EventsDemo::describe(const WKeyEvent &e)
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

void EventsDemo::describe(const WMouseEvent &e)
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
