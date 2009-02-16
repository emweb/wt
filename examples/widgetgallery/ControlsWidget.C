/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "ControlsWidget.h"
#include <Wt/WText>
#include <sstream>

using namespace Wt;

ControlsWidget::ControlsWidget(EventDisplayer *ed, bool hasSubMenu)
  : WContainerWidget(),
    ed_(ed),
    hasSubMenu_(hasSubMenu)
{ }

void ControlsWidget::populateSubMenu(Wt::WMenu *menu)
{
  
}

std::string ControlsWidget::escape(const std::string &name) const
{
  std::stringstream ss;
  for(unsigned int i = 0; i < name.size(); ++i) {
    if (name[i] != ':') {
      ss << name[i];
    } else {
      ss << "_1";
    }
  }
  return ss.str();
}

std::string ControlsWidget::doxygenAnchor(const std::string &classname) const
{
  std::stringstream ss;
  ss << "<a href=\"http://www.webtoolkit.eu/wt/doc/reference/html/class"
     << escape("Wt::" + classname)
     << ".html\" target=\"_blank\">doc</a>";

  return ss.str();
}

std::string ControlsWidget::title(const std::string& classname) const
{
  return "<span class=\"title\">" + classname + "</span> "
    + "<span class=\"doc\">["
    + doxygenAnchor(classname) + "]</span>";
}

void ControlsWidget::topic(const std::string &classname,
			   WContainerWidget *parent) const
{
  new WText(title(classname) + "<br/>", parent);
}

void ControlsWidget::topic(const std::string &classname1,
			   const std::string &classname2,
			   WContainerWidget *parent) const
{
  new WText(title(classname1) + " and " + title(classname2) + "<br/>",
            parent);
}

void ControlsWidget::topic(const std::string &classname1,
			   const std::string &classname2,
			   const std::string &classname3,
			   WContainerWidget *parent) const
{
  new WText(title(classname1) + ", " + title(classname2) + " and "
	    + title(classname3) + "<br/>", parent);
}

void ControlsWidget::topic(const std::string &classname1,
			   const std::string &classname2,
			   const std::string &classname3,
			   const std::string &classname4,
			   WContainerWidget *parent) const
{
  new WText(title(classname1) + ", " + title(classname2) + ", "
	    + title(classname3) + " and " + title(classname4) + "<br/>",
            parent);
}
