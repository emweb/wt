/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/DateField"
#include "Wt/WLineEdit"
#include "Wt/WDate"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

DateField::DateField(WContainerWidget *parent)
  : LineEdit(parent),
    format_("dd/MM/yyyy")
{
  //extjs: datefield doesn't stand a parent with display: none 
  setHideWithOffsets(true);
}

void DateField::setFormat(const WString& format)
{
  WDate d = date();
  format_ = format;
  setDate(d);
}

void DateField::setDate(const WDate& date)
{
  lineEdit()->setText(date.toString(format_));
}

WDate DateField::date() const
{
  return WDate::fromString(lineEdit()->text(), format_);
}

std::string DateField::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar() << " = new Ext.form.DateField(" << configStruct() << ");";

  applyToWidget(lineEdit(), result, inContainer);

  return result.str();
}

void DateField::createConfig(std::ostream& config)
{
  if (!validator())
    config << ",format:" << jsStringLiteral(WDate::extFormat(format_), '\'');

  LineEdit::createConfig(config);
}
  }
}
