/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WIconPair.h"
#include "Wt/WString.h"
#include "Wt/WTree.h"
#include "Wt/WTreeTable.h"
#include "Wt/WText.h"
#include "Wt/WTreeTableNode.h"

#ifndef WT_DEBUG_JS
#include "js/WTreeTable.min.js"
#endif

namespace Wt {

WTreeTable::WTreeTable()
{
  setImplementation(std::unique_ptr<WContainerWidget>(impl_ = new WContainerWidget()));

  setStyleClass("Wt-treetable");
  setPositionScheme(PositionScheme::Relative);

  headers_ = impl_->addWidget(cpp14::make_unique<WContainerWidget>());
  headers_->setStyleClass("Wt-header header");

  /*
   * spacer for when a scroll bar is visible
   */
  WContainerWidget *spacer
    = headers_->addWidget(cpp14::make_unique<WContainerWidget>());
  spacer->setStyleClass("Wt-sbspacer");

  headerContainer_
    = headers_->addWidget(cpp14::make_unique<WContainerWidget>());
  headerContainer_->setFloatSide(Side::Right);

  headers_->addWidget(cpp14::make_unique<WText>());
  columnWidths_.push_back(WLength::Auto);

  WContainerWidget *content
    = impl_->addWidget(cpp14::make_unique<WContainerWidget>());
  content->setStyleClass("Wt-content");
  if (!wApp->environment().agentIsIE())
    content->setOverflow(Overflow::Auto);
  else
    content->setAttributeValue
      ("style", "overflow-y: auto; overflow-x: hidden; zoom: 1");

  tree_ = content->addWidget(cpp14::make_unique<WTree>());
  tree_->setMargin(3, Side::Top);
  tree_->resize(WLength(100, LengthUnit::Percentage), WLength::Auto);
}

void WTreeTable::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WTreeTable.js", "WTreeTable", wtjs1);

  setJavaScriptMember(" WTreeTable", "new " WT_CLASS ".WTreeTable("
		      + app->javaScriptClass() + "," + jsRef() + ");");
}

void WTreeTable::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    defineJavaScript();

    setJavaScriptMember(WT_RESIZE_JS,
                        jsRef() + ".wtObj.wtResize");

    resize(width(), height());

    WApplication::instance()->addAutoJavaScript
      ("{var obj = " + jsRef() + ";"
       "if (obj && obj.wtObj) obj.wtObj.autoJavaScript();}");
  }

  WCompositeWidget::render(flags);
}

WWidget *WTreeTable::headerWidget() const
{
  return headers_;
}

void WTreeTable::setTreeRoot(std::unique_ptr<WTreeTableNode> root,
			     const WString& h)
{
  root->setTable(this);
  tree_->setTreeRoot(std::move(root));

  header(0)->setText(h.empty() ? "&nbsp;" : h);
}

WTreeTableNode *WTreeTable::treeRoot()
{
  return dynamic_cast<WTreeTableNode *>(tree_->treeRoot());
}

void WTreeTable::setTree(std::unique_ptr<WTree> root, const WString& h)
{
  WContainerWidget *parent = dynamic_cast<WContainerWidget *>(tree_->parent());
  parent->removeWidget(tree_);
  tree_ = root.get();
  parent->addWidget(std::move(root));

  header(0)->setText(h);
  tree_->resize(WLength(100, LengthUnit::Percentage), WLength::Auto);

  treeRoot()->setTable(this);
}

void WTreeTable::addColumn(const WString& header, const WLength& width)
{
  if (treeRoot())
    throw WException("WTreeTable::addColumn(): must be called before "
		     "setTreeRoot()");

  std::unique_ptr<WText> t(new WText(header));
  t->resize(width, WLength::Auto);
  t->setInline(false);
  t->setFloatSide(Side::Left);
  headerContainer_->addWidget(std::move(t));

  columnWidths_.push_back(width);
}

WText *WTreeTable::header(int column) const
{
  if (column == 0)
    return dynamic_cast<WText *>
      (dynamic_cast<WContainerWidget *>(impl_->children()[0])
       ->children()[2]);
  else
    return dynamic_cast<WText *>(headerContainer_->children()[column - 1]);
}

}
