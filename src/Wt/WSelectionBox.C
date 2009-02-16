/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WSelectionBox"

#include "CgiParser.h"
#include "DomElement.h"
#include "WtException.h"

namespace Wt {

WSelectionBox::WSelectionBox(WContainerWidget *parent)
  : WComboBox(parent),
    verticalSize_(5),
    selectionMode_(SingleSelection),
    configChanged_(false)
{ }

void WSelectionBox::setVerticalSize(int items)
{
  verticalSize_ = items;
  configChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WSelectionBox::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    selectionMode_ = mode;
    configChanged_ = true;
    repaint(RepaintPropertyAttribute);

    if (mode == ExtendedSelection) {
      selection_.clear();
      if (currentIndex() != -1)
	selection_.insert(currentIndex());
    } else {
      if (selection_.size() == 1)
	setCurrentIndex(*selection_.begin());
      else
	setCurrentIndex(-1);
      selection_.clear();
    }
  }
}

void WSelectionBox::setSelectedIndexes(const std::set<int>& selection)
{
  if (selectionMode_ != ExtendedSelection)
    throw WtException("WSelectionBox::setSelectedIndexes() can only be used "
		      "for an ExtendedSelection mode");

  selection_ = selection;
  selectionChanged_ = true;
  repaint(RepaintInnerHtml);
}

void WSelectionBox::clearSelection()
{
  if (selectionMode_ == ExtendedSelection)
    setSelectedIndexes(std::set<int>());
  else
    setCurrentIndex(-1);
}

bool WSelectionBox::isSelected(int index) const
{
  if (selectionMode_ == ExtendedSelection)
    return selection_.find(index) != selection_.end();
  else
    return WComboBox::isSelected(index);
}

void WSelectionBox::updateDom(DomElement& element, bool all)
{
  if (configChanged_ || all) {
    element.setAttribute("size",
			 boost::lexical_cast<std::string>(verticalSize_));

    if (!all || (selectionMode_ == ExtendedSelection)) {
      element.setProperty(PropertyMultiple, selectionMode_ == ExtendedSelection
			  ? "true" : "false");
      if (!all)
	selectionChanged_ = true;
    }

    configChanged_ = false;
  }

  if (selectionMode_ == ExtendedSelection)
    if (selectionChanged_ && !all) {
      for (int i = 0; i < count(); ++i) {
	element.callMethod("options[" + boost::lexical_cast<std::string>(i)
			+ "].selected=" + (isSelected(i) ? "true" : "false"));
      }
      selectionChanged_ = false;
    }

  WComboBox::updateDom(element, all);
}

void WSelectionBox::setFormData(CgiEntry *entry)
{
  if (selectionMode_ == SingleSelection)
    WComboBox::setFormData(entry);
  else {
    selection_.clear();

    for (CgiEntry *e = entry; e; e = e->next()) {
      if (!e->value().empty()) {
	try {
	  int i = boost::lexical_cast<int>(e->value());
	  selection_.insert(i);
	} catch (boost::bad_lexical_cast&) {
	  wApp->log("error") << "WSelectionBox received illegal form value: '"
			     << entry->value() << "'";
	}
      }
    }
  }
}

}
