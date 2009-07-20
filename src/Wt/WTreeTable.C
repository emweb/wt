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

namespace Wt {

WTreeTable::WTreeTable(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  setImplementation(impl_ = new WContainerWidget());
  setPositionScheme(Relative);

  headers_ = new WContainerWidget(impl_);
  headers_->setStyleClass("header");
  headers_->resize(WLength(100, WLength::Percentage), WLength::Auto);

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
 
  /*
   * Ugly JavaScript hack to make headers stay on top of content when
   * scrollbars appear, and resize contents to fit in parent - header.
   */
  WApplication::instance()->doJavaScript
    ("function sb" + id() + "() {"
     """var c=" + impl_->jsRef() + ";"
     """var h=" + headers_->jsRef() + ";"
     """var e=" + content->jsRef() + ";"
     """var sp=" + spacer->jsRef() + ";"
     """if (e && sp) {"
     ""  "if (" WT_CLASS ".pxself(" + impl_->jsRef() + ",'height') != 0)"
     ""    "e.style.height=(c.offsetHeight - h.offsetHeight) + 'px';"
     ""  "if (e.scrollHeight > e.offsetHeight) {"
     ""    "sp.style.display='block';"
     ""  "} else {"
     ""    "sp.style.display='none';"
     ""  "}"
     ""  "setTimeout(sb" + id() + ", 20);"
     """}"
     "}"
     "sb" + id() + "();");
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
  if (tree_)
    delete tree_;

  header(0)->setText(h);
  impl_->addWidget(tree_ = new WTree());
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
