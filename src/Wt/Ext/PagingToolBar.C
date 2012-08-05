/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/Ext/PagingToolBar"
#include "Wt/Ext/TableView"

namespace Wt {
  namespace Ext {

PagingToolBar::PagingToolBar(const std::string& storeRef,
			     TableView *tableView)
  : storeRef_(storeRef),
    tableView_(tableView)
{ }

std::string PagingToolBar::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar()
	 << "=new Ext.PagingToolbar({store:" << storeRef_ << ",pageSize:"
	 << boost::lexical_cast<std::string>(tableView_->pageSize())
	 << "});";

  if (inContainer) {
    result << elVar() << ".render('" << id() << "');";
    jsAfterPanelRendered(result);
  }

  return result.str();
}

void PagingToolBar::jsAfterPanelRendered(std::stringstream& js)
{
  std::string refs = createMixed(items_, js);
  js << elRef() << ".add(" << refs << ");";
}

}
}
