/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WText>

#include "Option.h"
#include "OptionList.h"

Option::Option(const WString& text, WContainerWidget *parent)
  : WContainerWidget(parent),
    sep_(0),
    list_(0)
{
  setInline(true);

  option_ = new WText(text, this);
  option_->setStyleClass("option");
}

void Option::setText(const WString& text)
{
  option_->setText(text);
}

void Option::setOptionList(OptionList *l)
{
  list_ = l;
}

void Option::addSeparator()
{
  sep_ = new WText("|", this);
  sep_->setStyleClass("sep");
}

void Option::hideSeparator()
{
  sep_->hide();
}

void Option::showSeparator()
{
  sep_->show();
}

void Option::setHidden(bool hidden, const WAnimation& animation)
{
  WContainerWidget::setHidden(hidden, animation);

  if (list_)
    list_->optionVisibilityChanged(this, hidden);
}
