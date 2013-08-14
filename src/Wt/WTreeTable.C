/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WString"
#include "Wt/WTree"
#include "Wt/WTreeTable"
#include "Wt/WText"
#include "Wt/WTreeTableNode"

#ifndef WT_DEBUG_JS
#include "js/WTreeTable.min.js"
#endif

namespace Wt {

WTreeTable::WTreeTable(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  setImplementation(impl_ = new WContainerWidget());

  setStyleClass("Wt-treetable");
  setPositionScheme(Relative);

  headers_ = new WContainerWidget(impl_);
  headers_->setStyleClass("Wt-header header");

  /*
   * spacer for when a scroll bar is visible
   */
  WContainerWidget *spacer = new WContainerWidget(headers_);
  spacer->setStyleClass("Wt-sbspacer");

  headerContainer_ = new WContainerWidget(headers_);
  headerContainer_->setFloatSide(Right);

  headers_->addWidget(new WText());
  columnWidths_.push_back(WLength::Auto);

  WContainerWidget *content = new WContainerWidget(impl_);
  content->setStyleClass("Wt-content");
  if (!wApp->environment().agentIsIE())
    content->setOverflow(WContainerWidget::OverflowAuto);
  else
    content->setAttributeValue
      ("style", "overflow-y: auto; overflow-x: hidden; zoom: 1");

  content->addWidget(tree_ = new WTree());

  tree_->setMargin(3, Top);
  tree_->resize(WLength(100, WLength::Percentage), WLength::Auto);
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
  if (flags & RenderFull) {
    defineJavaScript();

    setJavaScriptMember(WT_RESIZE_JS,
			"$('#" + id() + "').data('obj').wtResize");

    resize(width(), height());

    WApplication::instance()->addAutoJavaScript
      ("{var obj = $('#" + id() + "').data('obj');"
       "if (obj) obj.autoJavaScript();}");
  }

  WCompositeWidget::render(flags);
}

WWidget *WTreeTable::headerWidget() const
{
  return headers_;
}

void WTreeTable::setTreeRoot(WTreeTableNode *root, const WString& h)
{
  tree_->setTreeRoot(root);
  header(0)->setText(h);
  root->setTable(this);
}

WTreeTableNode *WTreeTable::treeRoot()
{
  return dynamic_cast<WTreeTableNode *>(tree_->treeRoot());
}

void WTreeTable::setTree(WTree *root, const WString& h)
{
  WContainerWidget *parent = dynamic_cast<WContainerWidget *>(tree_->parent());

  delete tree_;
  parent->addWidget(tree_ = root);
  header(0)->setText(h);
  tree_->resize(WLength(100, WLength::Percentage), WLength::Auto);

  treeRoot()->setTable(this);
}

void WTreeTable::addColumn(const WString& header, const WLength& width)
{
  if (treeRoot())
    throw WException("WTreeTable::addColumn(): must be called before "
		     "setTreeRoot()");

  WText *t = new WText(header);
  t->resize(width, WLength::Auto);
  t->setInline(false);
  t->setFloatSide(Left);
  headerContainer_->addWidget(t);

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
