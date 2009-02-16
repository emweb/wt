/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/NumberField"
#include "Wt/WLineEdit"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

NumberField::NumberField(WContainerWidget *parent)
  : LineEdit(parent),
    decimals_(2)
{ }

void NumberField::setValue(double value)
{
  lineEdit()->setText(boost::lexical_cast<std::string>(value));
}

double NumberField::value() const
{
  try {
    return boost::lexical_cast<int>(lineEdit()->text());
  } catch (boost::bad_lexical_cast&) {
    return 0; // FIXME ?
  }
}

void NumberField::setDecimalPrecision(int decimals)
{
  decimals_ = decimals;
}

std::string NumberField::createJS(DomElement *inContainer)
{
  std::stringstream result;

  result << elVar() << " = new Ext.form.NumberField(" << configStruct() << ");";

  applyToWidget(lineEdit(), result, inContainer);

  return result.str();
}

void NumberField::createConfig(std::ostream& config)
{
  LineEdit::createConfig(config);

  if (decimals_ != 2)
    config << ",decimalPrecision:" << decimals_;
}
  }
}
