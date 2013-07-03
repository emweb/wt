/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
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
  text_->setParentWidget(this);
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
  image_->setParentWidget(this);
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
  repaint();
}

void WLabel::setText(const WString& text)
{
  if (this->text() == text)
    return;

  if (!text_) {
    text_ = new WText();
    text_->setWordWrap(false);
    text_->setParentWidget(this);
    newText_ = true;
    repaint(RepaintSizeAffected);
  }

  text_->setText(text);
}

bool WLabel::setTextFormat(TextFormat format)
{
  if (!text_) {
    setText("A");
    setText("");
  }

  return text_->setTextFormat(format);
}

TextFormat WLabel::textFormat() const
{
  if (!text_)
    return XHTMLText;
  else
    return text_->textFormat();
}

const WString& WLabel::text() const
{
  static WString empty("");
  if (text_)
    return text_->text();
  else
    return empty;
}

void WLabel::setImage(WImage *image, Side side)
{
  delete image_;
  image_ = image;
  if (image_) {
    image_->setParentWidget(this);
    imageSide_ = side;
  }

  newImage_ = true;
  repaint(RepaintSizeAffected);
}

void WLabel::setWordWrap(bool wordWrap)
{
  if (!text_) {
    text_ = new WText();
    text_->setParentWidget(this);
    newText_ = true;
    repaint(RepaintSizeAffected);
  }

  text_->setWordWrap(wordWrap);
}

bool WLabel::wordWrap() const
{
  return text_ ? text_->wordWrap() : false;
}

void WLabel::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();

  if (image_ && text_)
    if (imageSide_ == Left) {
      updateImage(element, all, app, 0);
      updateText(element, all, app, 1);
    } else {
      updateText(element, all, app, 0);
      updateImage(element, all, app, 1);
    }
  else {
    updateText(element, all, app, 0);
    updateImage(element, all, app, 0);
  }

  if (buddyChanged_ || all) {
    if (buddy_)
      element.setAttribute("for", buddy_->formName());
    buddyChanged_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

void WLabel::updateImage(DomElement& element, bool all, WApplication *app,
			 int pos)
{
  if (newImage_ || all) {
    if (image_)
      element.insertChildAt(image_->createSDomElement(app), pos);
    newImage_ = false;
  }
}

void WLabel::updateText(DomElement& element, bool all, WApplication *app,
			int pos)
{
  if (newText_ || all) {
    if (text_)
      element.insertChildAt(text_->createSDomElement(app), pos);
    newText_ = false;
  }
}

void WLabel::propagateSetEnabled(bool enabled)
{
  if (text_)
    text_->propagateSetEnabled(enabled);

  WInteractWidget::propagateSetEnabled(enabled);
}

void WLabel::propagateRenderOk(bool deep)
{
  newImage_ = false;
  newText_ = false;
  buddyChanged_ = false;

  WInteractWidget::propagateRenderOk(deep);
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
