/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/LineEdit"
#include "Wt/WLineEdit"

#include "DomElement.h"

namespace Wt {

  namespace Ext {

LineEdit::LineEdit(WContainerWidget *parent)
  : FormField(parent),
    lineEdit_(new WLineEdit())
{
  addOrphan(lineEdit_);
}

LineEdit::LineEdit(const WString& content, WContainerWidget *parent)
  : FormField(parent),
    lineEdit_(new WLineEdit())
{
  addOrphan(lineEdit_);
  setText(content);
}

EventSignal<WKeyEvent>& LineEdit::keyWentDown()
{
  return lineEdit_->keyWentDown();
}

EventSignal<WKeyEvent>& LineEdit::keyPressed()
{
  return lineEdit_->keyPressed();
}

EventSignal<WKeyEvent>& LineEdit::keyWentUp()
{
  return lineEdit_->keyWentUp();
}

EventSignal<>& LineEdit::enterPressed()
{
  return lineEdit_->enterPressed();
}

WFormWidget *LineEdit::formWidget() const
{
  return lineEdit_;
}

void LineEdit::setText(const WString& value)
{
  lineEdit_->setText(value);
}

const WString& LineEdit::text() const
{
  return lineEdit_->text();
}

void LineEdit::setTextSize(int numChars)
{
  lineEdit_->setTextSize(numChars);
}

int LineEdit::textSize() const
{
  return lineEdit_->textSize();
}

void LineEdit::setMaxLength(int numChars)
{
  lineEdit_->setMaxLength(numChars);
}

int LineEdit::maxLength() const
{
  return lineEdit_->maxLength();
}

void LineEdit::setEchoMode(EchoMode echoMode)
{
  lineEdit_->setEchoMode(static_cast<WLineEdit::EchoMode>(echoMode));
}

LineEdit::EchoMode LineEdit::echoMode() const
{
  return static_cast<EchoMode>(lineEdit_->echoMode());
}

void LineEdit::setEmptyDisplayText(const WString& text)
{ 
  // NYI
}

void LineEdit::setGrowToContent(bool grow, int minWidth, int maxWidth)
{
  // NYI
}

WValidator::State LineEdit::validate()
{
  if (validator()) {
    return validator()->validate(lineEdit_->text()).state();
  } else
    return WValidator::Valid;
}

void LineEdit::useAsTableViewEditor()
{
  lineEdit_->setFormObject(false);
}

std::string LineEdit::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar() << " = new Ext.form.TextField(" << configStruct() << ");";

  applyToWidget(lineEdit_, result, inContainer);

  return result.str();
}

void LineEdit::createConfig(std::ostream& config)
{
  FormField::createConfig(config);
}

  }
}
