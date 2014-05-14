/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/Calendar"

#include "Wt/WLogger"

#include "DomElement.h"

namespace Wt {

LOGGER("Ext.Calendar");

  namespace Ext {

Calendar::Calendar(bool i18n, WContainerWidget *parent)
  : Component(parent),
    selectionChanged_(this),
    selected_(this),
    extDateSelected_(this, "select", false)
{ }

void Calendar::select(const WDate& date)
{
  selection_.clear();
  selection_.insert(date);

  if (isRendered())
    addUpdateJS(elVar() + ".setValue(Date.parseDate('"
		+ date.toString("dd/MM/yyyy").toUTF8() + "','d/m/Y');");
}

void Calendar::onSelect(std::string date)
{ 
  selection_.clear();

  WDate d = WDate::fromString(date, "dd/MM/yyyy");

  if (d.isValid()) {
    selection_.insert(d);
    selectionChanged_.emit();
    selected_.emit();
  } else
    LOG_ERROR("could not parse date: '" << date << "'");
}

void Calendar::updateExt()
{
  updateWtSignal(&extDateSelected_, extDateSelected_.name(),
		 "dp,d", "d.format('d/m/Y')");
  Component::updateExt();
}

std::string Calendar::createJS(DomElement *inContainer)
{
  assert(inContainer);

  if (!extDateSelected_.isConnected())
    extDateSelected_.connect(this, &Calendar::onSelect);

  std::stringstream result;
  result << elVar() << " = new Ext.DatePicker(" << configStruct() << ");";
  result << elVar() << ".render('" << id() << "');";

  if (selection_.size() == 1)
    result << elVar() << ".setValue(Date.parseDate('"
	   << (*selection_.begin()).toString("dd/MM/yyyy").toUTF8()
	   << "','d/m/Y'));";

  bindEventHandler("select", "selectH", result);
  
  return result.str();
}

void Calendar::createConfig(std::ostream& config)
{
  Component::createConfig(config);

  addWtSignalConfig("selectH", &extDateSelected_, extDateSelected_.name(),
		    "dp,d", "d.format('d/m/Y')", config);
}

  }
}
