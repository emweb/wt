#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Character.h"

Character::Character(const std::string& name, WContainerWidget *parent)
  : WText(parent),
    name_(name),
    redDrops_(0),
    blueDrops_(0)
{
  setText(name_ + " got no pills");

  setStyleClass("character");

  /*
   * Accept drops, and indicate this with a change in CSS style class.
   */
  acceptDrops("red-pill", "red-drop-site");
  acceptDrops("blue-pill", "blue-drop-site");

  setInline(false);
}

void Character::dropEvent(WDropEvent event)
{
  if (event.mimeType() == "red-pill")
    ++redDrops_;
  if (event.mimeType() == "blue-pill")
    ++blueDrops_;

  std::string text = name_ + " got ";

  if (redDrops_ != 0)
    text += boost::lexical_cast<std::string>(redDrops_) + " red pill";
  if (redDrops_ > 1)
    text += "s";

  if (redDrops_ != 0 && blueDrops_ != 0)
    text += " and ";

  if (blueDrops_ != 0)
    text += boost::lexical_cast<std::string>(blueDrops_) + " blue pill";
  if (blueDrops_ > 1)
    text += "s";

  setText(text);
}
