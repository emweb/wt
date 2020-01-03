/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WImage.h>
#include <Wt/WText.h>
#include <Wt/WCssDecorationStyle.h>

#include "TreeNode.h"
#include "IconPair.h"

using namespace Wt;

std::string TreeNode::imageLine_[] = { "icons/line-middle.gif",
				       "icons/line-last.gif" };
std::string TreeNode::imagePlus_[] = { "icons/nav-plus-line-middle.gif",
				       "icons/nav-plus-line-last.gif" };
std::string TreeNode::imageMin_[] = { "icons/nav-minus-line-middle.gif",
				      "icons/nav-minus-line-last.gif" };

TreeNode::TreeNode(const std::string labelText,
		   TextFormat labelFormat,
		   std::unique_ptr<IconPair> labelIcon)
  : parentNode_(nullptr),
    labelIcon_(labelIcon.get())
{
  // pre-learned stateless implementations ...
  implementStateless(&TreeNode::expand, &TreeNode::undoExpand);
  implementStateless(&TreeNode::collapse, &TreeNode::undoCollapse);

  // ... or auto-learned stateless implementations
  // which do not need undo functions
  //implementStateless(&TreeNode::expand);
  //implementStateless(&TreeNode::collapse);

  layout_ = setNewImplementation<WTable>();

  expandIcon_ = layout_->elementAt(0, 0)->addNew<IconPair>(imagePlus_[Last], imageMin_[Last]);
  expandIcon_->hide();

  noExpandIcon_ = layout_->elementAt(0, 0)->addNew<WImage>(imageLine_[Last]);

  expandedContent_ = layout_->elementAt(1, 1)->addNew<WContainerWidget>();
  expandedContent_->hide();

  auto labelTextWidget = cpp14::make_unique<WText>(labelText);
  labelTextWidget->setTextFormat(labelFormat);
  labelTextWidget->setStyleClass("treenodelabel");

  auto childCountLabel = cpp14::make_unique<WText>();
  childCountLabel_ = childCountLabel.get();
  childCountLabel_->setMargin(7, Side::Left);
  childCountLabel_->setStyleClass("treenodechildcount");

  if (labelIcon) {
    layout_->elementAt(0, 1)->addWidget(std::move(labelIcon));
    labelIcon_->setVerticalAlignment(AlignmentFlag::Middle);
  }
  layout_->elementAt(0, 1)->addWidget(std::move(labelTextWidget));
  layout_->elementAt(0, 1)->addWidget(std::move(childCountLabel));

  layout_->elementAt(0, 0)->setContentAlignment(AlignmentFlag::Top);
  layout_->elementAt(0, 1)->setContentAlignment(AlignmentFlag::Middle);

  expandIcon_->icon1Clicked->connect(this, &TreeNode::expand);
  expandIcon_->icon2Clicked->connect(this, &TreeNode::collapse);
} //

bool TreeNode::isLastChildNode() const
{
  if (parentNode_) {
    return parentNode_->childNodes_.back() == this;
  } else
    return true;
}

void TreeNode::addChildNode(std::unique_ptr<TreeNode> node)
{
  childNodes_.push_back(node.get());
  node->parentNode_ = this;

  expandedContent_->addWidget(std::move(node));

  childNodesChanged();
}

void TreeNode::removeChildNode(TreeNode *node, int index)
{
  childNodes_.erase(childNodes_.begin() + index);

  node->parentNode_ = nullptr;

  expandedContent_->removeWidget(node);

  childNodesChanged();
} //

void TreeNode::childNodesChanged()
{
  for (auto node : childNodes_)
    node->adjustExpandIcon();

  adjustExpandIcon();

  if (!childNodes_.empty()) {
    childCountLabel_ ->setText("(" + std::to_string(childNodes_.size()) + ")");
  } else {
    childCountLabel_->setText("");
  }

  resetLearnedSlots();
} //

void TreeNode::collapse()
{
  wasCollapsed_ = expandedContent_->isHidden();

  expandIcon_->setState(0);
  expandedContent_->hide();
  if (labelIcon_)
    labelIcon_->setState(0);
} //

void TreeNode::expand()
{
  wasCollapsed_ = expandedContent_->isHidden();

  expandIcon_->setState(1);
  expandedContent_->show();
  if (labelIcon_)
    labelIcon_->setState(1);

  /*
   * collapse all children
   */
  for (auto node : childNodes_)
    node->collapse();
} //

void TreeNode::undoCollapse()
{
  if (!wasCollapsed_) {
    // re-expand
    expandIcon_->setState(1);
    expandedContent_->show();
    if (labelIcon_)
      labelIcon_->setState(1);    
  }
}

void TreeNode::undoExpand()
{
  if (wasCollapsed_) {
    // re-collapse
    expandIcon_->setState(0);
    expandedContent_->hide();
    if (labelIcon_)
      labelIcon_->setState(0);
  }

  /*
   * undo collapse of children
   */
  for (auto node : childNodes_)
    node->undoCollapse();
} //

void TreeNode::adjustExpandIcon()
{
  ImageIndex index = isLastChildNode() ? Last : Middle;

  if (expandIcon_->icon1()->imageLink().url() != imagePlus_[index])
    expandIcon_->icon1()->setImageLink(imagePlus_[index]);
  if (expandIcon_->icon2()->imageLink().url() != imageMin_[index])
    expandIcon_->icon2()->setImageLink(imageMin_[index]);
  if (noExpandIcon_->imageLink().url() != imageLine_[index])
    noExpandIcon_->setImageLink(imageLine_[index]);

  if (index == Last) {
    layout_->elementAt(0, 0)
      ->decorationStyle().setBackgroundImage("");
    layout_->elementAt(1, 0)
      ->decorationStyle().setBackgroundImage("");
  } else {
    layout_->elementAt(0, 0)
      ->decorationStyle().setBackgroundImage("icons/line-trunk.gif",
                                             Orientation::Vertical);
    layout_->elementAt(1, 0)
      ->decorationStyle().setBackgroundImage("icons/line-trunk.gif",
                                             Orientation::Vertical);
  } //

  if (childNodes_.empty()) {
    if (noExpandIcon_->isHidden()) {
      noExpandIcon_->show();
      expandIcon_->hide();
    }
  } else {
    if (expandIcon_->isHidden()) {
      noExpandIcon_->hide();
      expandIcon_->show();
    }
  }
} //
