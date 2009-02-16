/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WLabel"
#include "Wt/WText"
#include "Wt/WImage"
#include "Wt/WFormWidget"

#include "DomElement.h"

namespace Wt {

WLabel::WLabel(WContainerWidget *parent)
  : WInteractWidget(parent),
    buddy_(0),
    text_(0),
    image_(0),
    buddyChanged_(false),
    newImage_(false),
    newText_(false)
{ }

WLabel::WLabel(const WString& text, WContainerWidget *parent)
  : WInteractWidget(parent),
    buddy_(0),
    image_(0),
    buddyChanged_(false),
    newImage_(false),
    newText_(false)
{
  text_ = new WText(text);
  text_->setWordWrap(false);
  text_->setParent(this);
}

WLabel::WLabel(WImage *image, WContainerWidget *parent)
  : WInteractWidget(parent),
    buddy_(0),
    text_(0),
    buddyChanged_(false),
    newImage_(false),
    newText_(false)
{ 
  image_ = image;
  image_->setParent(this);
}

WLabel::~WLabel()
{
  setBuddy((WFormWidget *) 0);
}

void WLabel::setBuddy(WFormWidget *buddy)
{
  if (buddy_)
    buddy_->setLabel(0);

  buddy_ = buddy;
  if (buddy_)
    buddy_->setLabel(this);

  buddyChanged_ = true;
  repaint(RepaintPropertyAttribute);
}

void WLabel::setText(const WString& text)
{
  if (this->text() == text)
    return;

  if (!text_) {
    text_ = new WText();
    text_->setWordWrap(false);
    text_->setParent(this);
    newText_ = true;
    repaint(RepaintInnerHtml);
  }

  text_->setText(text);
}

const WString& WLabel::text() const
{
  static WString empty("");
  if (text_)
    return text_->text();
  else
    return empty;
}

void WLabel::setImage(WImage *image)
{
  if (image_)
    delete image_;
  image_ = image;
  if (image_)
    image_->setParent(this);

  newImage_ = true;
  repaint(RepaintInnerHtml);
}

void WLabel::setWordWrap(bool how)
{
  if (!text_) {
    text_ = new WText();
    text_->setParent(this);
    newText_ = true;
    repaint(RepaintInnerHtml);
  }

  text_->setWordWrap(how);
}

bool WLabel::wordWrap() const
{
  return text_ ? text_->wordWrap() : false;
}

void WLabel::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();

  if (newImage_ || all) {
    if (image_)
      element.insertChildAt(((WWebWidget *)image_)->createDomElement(app), 0);
    newImage_ = false;
  }

  if (newText_ || all) {
    if (text_)
      element.addChild(((WWebWidget *)text_)->createDomElement(app));
    newText_ = false;
  }

  if (buddyChanged_ || all) {
    if (buddy_)
      element.setAttribute("for", buddy_->formName());
    buddyChanged_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

DomElementType WLabel::domElementType() const
{
  // The label in <a><label /></a> eats an onclick event in IE
  // We should explicitly continue to propagate the onclick event ? 
  // For now we avoid to create a LABEL element if no buddy is set
  // (This is used e.g. in WTreeView)
  if (buddy_)
    return DomElement_LABEL;
  else
    return isInline() ? DomElement_SPAN : DomElement_DIV;
}

void WLabel::getDomChanges(std::vector<DomElement *>& result,
			   WApplication *app)
{
  WInteractWidget::getDomChanges(result, app);

  if (text_)
    ((WWebWidget *)text_)->getDomChanges(result, app);
  if (image_)
    ((WWebWidget *)image_)->getDomChanges(result, app);
}

}
