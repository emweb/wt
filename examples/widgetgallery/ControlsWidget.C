/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "ControlsWidget.h"
#include <Wt/WText>
#include <sstream>

#include <boost/algorithm/string.hpp>

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

std::string ControlsWidget::docAnchor(const std::string &classname) const
{
  std::stringstream ss;

#if !defined(WT_TARGET_JAVA)
  ss << "<a href=\"http://www.webtoolkit.eu/wt/doc/reference/html/class"
     << escape("Wt::" + classname)
     << ".html\" target=\"_blank\">doc</a>";
#else
  std::string cn = boost::replace_all(classname, "Chart::","chart/");
  ss << "<a href=\"http://www.webtoolkit.eu/"
     << "jwt/latest/doc/javadoc/eu/webtoolkit/jwt/"
     << classname
    << ".html\" target=\"_blank\">doc</a>";
#endif

  return ss.str();
}

std::string ControlsWidget::title(const std::string& classname) const
{
  std::string cn;
#if defined(WT_TARGET_JAVA)
    cn = boost::replace_all(classname, "Chart::","");
#else
    cn = classname;
#endif

    return std::string("<span class=\"title\">") + cn + "</span> "
      + "<span class=\"doc\">["
      + docAnchor(classname) + "]</span>";
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
