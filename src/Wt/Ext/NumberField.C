/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iomanip>
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
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(decimals_) << value;
  lineEdit()->setText(oss.str());
}

double NumberField::value() const
{
  try {
    return boost::lexical_cast<double>(lineEdit()->text());
  } catch (boost::bad_lexical_cast& e) {
    // You wouldn't expect this to happen anless a user is tampering
    // 0 with do in that case
    return 0;
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
