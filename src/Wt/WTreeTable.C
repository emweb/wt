/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WVBoxLayout"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WString"
#include "Wt/WTree"
#include "Wt/WTreeTable"
#include "Wt/WText"
#include "Wt/WTreeTableNode"

#include "JavaScriptLoader.h"

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
  content->resize(WLength(100, WLength::Percentage),
		  WLength(100, WLength::Percentage));
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

  if (!app->environment().ajax())
    return;

  const char *THIS_JS = "js/WTreeTable.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WTreeTable", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  /*
   * We should really resolve this: we use setJavaScriptMember()
   * instead of doJavaScript(), because setJavaScriptMember() is streamed
   * before doJavaScript()
   */
  setJavaScriptMember("_a", "0;new " WT_CLASS ".WTreeTable("
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

  header(0)->setText(h);
  parent->addWidget(tree_ = new WTree());
  tree_->resize(WLength(100, WLength::Percentage), WLength::Auto);

  treeRoot()->setTable(this);
}

void WTreeTable::addColumn(const WString& header, const WLength& width)
{
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
