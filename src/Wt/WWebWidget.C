/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <mxml.h>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WLogger"
#include "Wt/WWebWidget"
#include "Wt/WJavaScript"

#include "DomElement.h"
#include "WebRenderer.h"
#include "WebSession.h"
#include "WebSession.h"
#include "WtException.h"
#include "Utils.h"

using namespace Wt;

std::vector<WWidget *> WWebWidget::emptyWidgetList_;

WWebWidget::TransientImpl::TransientImpl()
{ }

WWebWidget::TransientImpl::~TransientImpl()
{
  for (unsigned i = 0; i < childRemoveChanges_.size(); ++i)
    delete childRemoveChanges_[i];
}

WWebWidget::LayoutImpl::LayoutImpl()
  : positionScheme_(Static),
    floatSide_(None),
    clearSides_(None),
    popup_(false),
    verticalAlignment_(AlignBaseline),
    marginsChanged_(false)
{ 
  for (unsigned i = 0; i < 4; ++i)
    margin_[i] = WLength(0);
}

WWebWidget::LookImpl::LookImpl()
  : decorationStyle_(0),
    toolTip_(0),
    styleClassChanged_(false),
    toolTipChanged_(false)
{ }

WWebWidget::LookImpl::~LookImpl()
{
  delete decorationStyle_;
  delete toolTip_;
}

WWebWidget::OtherImpl::OtherImpl()
  : attributes_(0),
    attributesSet_(0),
    id_(0),
    dropSignal_(0),
    acceptedDropMimeTypes_(0)
{ }

WWebWidget::OtherImpl::~OtherImpl()
{
  delete attributes_;
  delete attributesSet_;
  delete dropSignal_;
  delete acceptedDropMimeTypes_;
  delete id_;
}

WWebWidget::WWebWidget(WContainerWidget *parent)
  : WWidget(parent),
    width_(0),
    height_(0),
    transientImpl_(0),
    layoutImpl_(0),
    lookImpl_(0),
    otherImpl_(0),
    children_(0)
{
  flags_.set(BIT_INLINE);
  flags_.set(BIT_NEED_RERENDER);

  if (parent)
    parent->addWidget(this);
}

void WWebWidget::setFormObject(bool how)
{
  flags_.set(BIT_FORM_OBJECT, how);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);
}

void WWebWidget::setId(const std::string& id)
{
  if (!otherImpl_)
    otherImpl_ = new OtherImpl();

  if (!otherImpl_->id_)
    otherImpl_->id_ = new std::string();

  *otherImpl_->id_ = id;
}

const std::string WWebWidget::formName() const
{
  if (otherImpl_ && otherImpl_->id_)
    return *otherImpl_->id_;
  else
    return WWidget::formName();
}

void WWebWidget::repaint(unsigned int flags)
{
  if (!flags_.test(BIT_NEED_RERENDER)) {
    flags_.set(BIT_NEED_RERENDER);
    WApplication::instance()->session()->renderer().needUpdate(this);
  }

  flags_ |= flags;
}

void WWebWidget::renderOk()
{
  if (flags_.test(BIT_NEED_RERENDER)) {
    flags_.reset(BIT_NEED_RERENDER);
    flags_ &= ~RepaintAll;
    WApplication::instance()->session()->renderer().doneUpdate(this);
  }
}

void WWebWidget::signalConnectionsChanged()
{
  repaint(RepaintPropertyAttribute);
}

WWebWidget::~WWebWidget()
{
  flags_.set(BIT_BEING_DELETED);

  if (flags_.test(BIT_FORM_OBJECT))
    WApplication::instance()
      ->session()->renderer().updateFormObjects(this, false);

  setParent(0);

  delete width_;
  delete height_;

  if (children_) {
    while (children_->size())
      delete (*children_)[0];
    delete children_;
  }

  delete transientImpl_;
  delete layoutImpl_;
  delete lookImpl_;

  WApplication::instance()->session()->renderer().doneUpdate(this);

  delete otherImpl_;
}

WCssDecorationStyle& WWebWidget::decorationStyle()
{
  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  if (!lookImpl_->decorationStyle_) {
    lookImpl_->decorationStyle_ = new WCssDecorationStyle();
    lookImpl_->decorationStyle_->setWebWidget(this);
  }

  return *lookImpl_->decorationStyle_;
}

DomElement *WWebWidget::renderRemove()
{
  DomElement *e = DomElement::getForUpdate(this, DomElement_DIV);
  e->removeFromParent();
  return e;
}

void WWebWidget::removeChild(WWidget *w)
{
  assert(children_);

  int i = Utils::indexOf(*children_, w);

  assert (i != -1);
    
  if (!flags_.test(BIT_IGNORE_CHILD_REMOVES)
      && !flags_.test(BIT_BEING_DELETED)) {
    DomElement *e = w->webWidget()->renderRemove();

    if (e) {
      if (!transientImpl_)
	transientImpl_ = new TransientImpl();

      transientImpl_->childRemoveChanges_.push_back(e);

      repaint(RepaintInnerHtml);
    }
  }

  /*
    -- does not work properly: should in reality propagate the render
    remove to all grand children; but perhaps we don't need this

    std::vector<DomElement *> *nestedRemoveChanges
      = w->webWidget()->childRemoveChanges_;

    if (nestedRemoveChanges_) {
      for (unsigned k = 0; k < w->nestedRemoveChanges_->size(); ++k) {
	DomElement *f = (*nestedRemoveChanges_)[k];

	if (!f->discardWithParent()) {
	  if (!childRemoveChanges_)
	    childRemoveChanges_ = new std::vector<DomElement *>;
	  childRemoveChanges_->push_back(f);

	  nestedRemoveChanges_->erase(nestedRemoveChanges_->begin() + k);
	  --k;
	}
      }
    }
    */

  w->WObject::setParent(0);
    
  /*
   * When the child is about to be deleted, all of its descendants
   * properly removes itself from the renderer "dirty" list. If not,
   * we here force this propagation.
   */
  if (!w->webWidget()->flags_.test(BIT_BEING_DELETED))
    w->webWidget()->quickPropagateRenderOk();

  children_->erase(children_->begin() + i);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(w->webWidget(), true);
}

void WWebWidget::setPositionScheme(PositionScheme scheme)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->positionScheme_ = scheme;

  if ((scheme == Absolute) || (scheme == Fixed))
    flags_.reset(BIT_INLINE);

  flags_.set(BIT_GEOMETRY_CHANGED);
  repaint(RepaintPropertyAttribute);
}

WWidget::PositionScheme WWebWidget::positionScheme() const
{
  return layoutImpl_ ? layoutImpl_->positionScheme_ : Static;
}

void WWebWidget::resize(const WLength& width, const WLength& height)
{
  if (!width_ && !width.isAuto())
    width_ = new WLength();

  if (width_)
    *width_ = width;

  if (!height_ && !height.isAuto())
    height_ = new WLength();

  if (height_)
    *height_ = height;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::width() const
{
  return width_ ? *width_ : WLength();
}

WLength WWebWidget::height() const
{
  return height_ ? *height_ : WLength();
}

void WWebWidget::setMinimumSize(const WLength& width, const WLength& height)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->minimumWidth_ = width;
  layoutImpl_->minimumHeight_ = height;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::minimumWidth() const
{
  return layoutImpl_ ? layoutImpl_->minimumWidth_ : WLength();
}

WLength WWebWidget::minimumHeight() const
{
  return layoutImpl_ ? layoutImpl_->minimumHeight_ : WLength();
}

void WWebWidget::setMaximumSize(const WLength& width, const WLength& height)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->maximumWidth_ = width;
  layoutImpl_->maximumHeight_ = height;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::maximumWidth() const
{
  return layoutImpl_ ? layoutImpl_->maximumWidth_ : WLength();
}

WLength WWebWidget::maximumHeight() const
{
  return layoutImpl_ ? layoutImpl_->maximumHeight_ : WLength();
}

void WWebWidget::setLineHeight(const WLength& height)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->lineHeight_ = height;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::lineHeight() const
{
  return layoutImpl_ ? layoutImpl_->lineHeight_ : WLength();
}

void WWebWidget::setFloatSide(Side s)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->floatSide_ = s;

  flags_.set(BIT_FLOAT_SIDE_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WWidget::Side WWebWidget::floatSide() const
{
  if (layoutImpl_)
    return layoutImpl_->floatSide_;
  else
    return None;
}

void WWebWidget::setClearSides(int sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->clearSides_ = sides;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

int WWebWidget::clearSides() const
{
  if (layoutImpl_)
    return layoutImpl_->clearSides_;
  else
    return None;
}

void WWebWidget::setVerticalAlignment(VerticalAlignment va,
				      const WLength& length)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->verticalAlignment_ = va;
  layoutImpl_->verticalAlignmentLength_ = length;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WWidget::VerticalAlignment WWebWidget::verticalAlignment() const
{
  return layoutImpl_ ? layoutImpl_->verticalAlignment_ : AlignBaseline;
}

WLength WWebWidget::verticalAlignmentLength() const
{
  return layoutImpl_ ? layoutImpl_->verticalAlignmentLength_ : WLength();
}

void WWebWidget::setOffsets(const WLength& length, int sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();  

  if (sides & Top)
    layoutImpl_->offsets_[0] = length;
  if (sides & Right)
    layoutImpl_->offsets_[1] = length;
  if (sides & Bottom)
    layoutImpl_->offsets_[2] = length;
  if (sides & Left)
    layoutImpl_->offsets_[3] = length;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::offset(Side s) const
{
  if (!layoutImpl_)
    return WLength();

  switch (s) {
  case Top:
    return layoutImpl_->offsets_[0];
  case Right:
    return layoutImpl_->offsets_[1];
  case Bottom:
    return layoutImpl_->offsets_[2];
  case Left:
    return layoutImpl_->offsets_[3];
  default:
    throw WtException("WWebWidget::offset(Side) with invalid side.");
  }
}

int WWebWidget::zIndex() const
{
  if (layoutImpl_)
    return layoutImpl_->popup_ ? 10 : 0;
  else
    return 0;
}

void WWebWidget::setInline(bool inl)
{
  flags_.set(BIT_INLINE, inl);

  resetLearnedSlot(&WWidget::show);

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

bool WWebWidget::isInline() const
{
  return flags_.test(BIT_INLINE);
}

void WWebWidget::setPopup(bool popup)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->popup_ = popup;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

bool WWebWidget::isPopup() const
{
  return layoutImpl_ ? layoutImpl_->popup_ : false;
}


void WWebWidget::setMargin(const WLength& length, int sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();  

  if (sides & Top)
    layoutImpl_->margin_[0] = length;
  if (sides & Right)
    layoutImpl_->margin_[1] = length;
  if (sides & Bottom)
    layoutImpl_->margin_[2] = length;
  if (sides & Left)
    layoutImpl_->margin_[3] = length;

  layoutImpl_->marginsChanged_ = true;

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::margin(Side side) const
{
  if (!layoutImpl_)
    return WLength(0);

  switch (side) {
  case Top:
    return layoutImpl_->margin_[0];
  case Right:
    return layoutImpl_->margin_[1];
  case Bottom:
    return layoutImpl_->margin_[2];
  case Left:
    return layoutImpl_->margin_[3];
  default:
    throw WtException("WWebWidget::margin(Side) with invalid side");
  }
}

void WWebWidget::setStyleClass(const WString& styleClass)
{
  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  lookImpl_->styleClass_ = styleClass;
  lookImpl_->styleClassChanged_ = true;

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::setStyleClass(const char *value)
{
  setStyleClass(WString(value, UTF8));
}

WString WWebWidget::styleClass() const
{
  return lookImpl_ ? lookImpl_->styleClass_ : WString();
}

void WWebWidget::setAttributeValue(const std::string& name,
				   const WString& value)
{
  if (!otherImpl_)
    otherImpl_ = new OtherImpl();

  if (!otherImpl_->attributes_)
    otherImpl_->attributes_ = new std::map<std::string, WString>;
  (*otherImpl_->attributes_)[name] = value;

  if (!otherImpl_->attributesSet_)
    otherImpl_->attributesSet_ = new std::vector<std::string>;

  otherImpl_->attributesSet_->push_back(name);

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::setToolTip(const WString& message)
{
  if (canOptimizeUpdates() && (toolTip() == message))
    return;

  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  if (!lookImpl_->toolTip_)
    lookImpl_->toolTip_ = new WString();

  *lookImpl_->toolTip_ = message;

  lookImpl_->toolTipChanged_ = true;

  repaint(RepaintPropertyAttribute);
}

WString WWebWidget::toolTip() const
{
  return lookImpl_ ? (lookImpl_->toolTip_ ? *lookImpl_->toolTip_ : WString())
    : WString();
}

void WWebWidget::setHidden(bool hidden)
{
  if (canOptimizeUpdates() && (hidden == isHidden()))
    return;

  if (hidden)
    flags_.set(BIT_HIDDEN);
  else
    flags_.reset(BIT_HIDDEN);

  flags_.set(BIT_HIDDEN_CHANGED);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, true);

  repaint(RepaintPropertyAttribute);
}

bool WWebWidget::isHidden() const
{
  return flags_.test(BIT_HIDDEN);
}

void WWebWidget::addChild(WWidget *child)
{
  if (child->parent() != 0) {
    child->setParent(0);
    wApp->log("warn") << "WWebWidget::addChild(): reparenting child";
  }

  if (!children_)
    children_ = new std::vector<WWidget *>;

  children_->push_back(child);

  child->WObject::setParent(this);

  if (flags_.test(BIT_LOADED))
    doLoad(child);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);
}

const std::vector<WWidget *>& WWebWidget::children() const
{
  return children_ ? *children_ : emptyWidgetList_;
}

void WWebWidget::setHideWithOffsets(bool how)
{
  if (how) {
    if (!flags_.test(BIT_HIDE_WITH_OFFSETS)) {
      flags_.set(BIT_HIDE_WITH_OFFSETS);

      resetLearnedSlot(&WWidget::show);
      resetLearnedSlot(&WWidget::hide);

      if (parent())
	parent()->setHideWithOffsets();
    }
  }
}

void WWebWidget::updateDom(DomElement& element, bool all)
{
  /*
   * determine display
   */
  if (flags_.test(BIT_GEOMETRY_CHANGED)
      || (!flags_.test(BIT_HIDE_WITH_OFFSETS)
	  && flags_.test(BIT_HIDDEN_CHANGED))
      || all) {
    if (flags_.test(BIT_HIDE_WITH_OFFSETS) || !flags_.test(BIT_HIDDEN)) {
      if (element.isDefaultInline() != flags_.test(BIT_INLINE)) {
	if (flags_.test(BIT_INLINE)) {
	  if (element.type() == DomElement_TABLE)
	    element.setProperty(PropertyStyleDisplay, "inline-table");
	  if (element.type() == DomElement_LI)
	    element.setProperty(PropertyStyleDisplay, "inline");
	  else if (element.type() != DomElement_TD)
	    element.setProperty(PropertyStyleDisplay, "inline-block");
	} else {
	  element.setProperty(PropertyStyleDisplay, "block");
	}
      } else if (!all && flags_.test(BIT_HIDDEN_CHANGED))
	if (element.isDefaultInline() == flags_.test(BIT_INLINE))
	  element.setProperty(PropertyStyleDisplay, "");
	else
	  element.setProperty(PropertyStyleDisplay,
			      flags_.test(BIT_INLINE) ? "inline" : "block");
    } else
      element.setProperty(PropertyStyleDisplay, "none");

    if (!flags_.test(BIT_HIDE_WITH_OFFSETS))
      flags_.reset(BIT_HIDDEN_CHANGED);
  }

  if (flags_.test(BIT_GEOMETRY_CHANGED) || all) {
    if (layoutImpl_) {
      /*
       * set position
       */
      switch (layoutImpl_->positionScheme_) {
      case Static:
	break;
      case Relative:
	element.setProperty(PropertyStylePosition, "relative"); break;
      case Absolute:
	element.setProperty(PropertyStylePosition, "absolute"); break;
      case Fixed:
	element.setProperty(PropertyStylePosition, "fixed"); break;
      }

      /*
       * set z-index
       */
      if (layoutImpl_->popup_)
	element.setProperty(PropertyStyleZIndex,
			    boost::lexical_cast<std::string>(zIndex()));

      /*
       * set clear: FIXME: multiple values
       */
      switch (layoutImpl_->clearSides_) {
      case None:
	break;
      case Left:
	element.setProperty(PropertyStyleClear, "left");
	break;
      case Right:
	element.setProperty(PropertyStyleClear, "right");
	break;
      case Verticals:
	element.setProperty(PropertyStyleClear, "both");
	break;
      default:
	/* illegal values */
	;
      }

      if (!layoutImpl_->minimumWidth_.isAuto()
	  && (layoutImpl_->minimumWidth_.value() != 0))
	element.setProperty(PropertyStyleMinWidth,
			    layoutImpl_->minimumWidth_.cssText());
      if (!layoutImpl_->minimumHeight_.isAuto()
	  && (layoutImpl_->minimumHeight_.value() != 0))
	element.setProperty(PropertyStyleMinHeight,
			    layoutImpl_->minimumHeight_.cssText());
      if (!layoutImpl_->maximumWidth_.isAuto()) // == none
	element.setProperty(PropertyStyleMaxWidth,
			    layoutImpl_->maximumWidth_.cssText());
      if (!layoutImpl_->maximumHeight_.isAuto()) // == none
	element.setProperty(PropertyStyleMaxHeight,
			    layoutImpl_->maximumHeight_.cssText());

      /*
       * set offsets
       */
      if (layoutImpl_->positionScheme_ != Static) {
	static const Property properties[] = { PropertyStyleTop,
					       PropertyStyleRight,
					       PropertyStyleBottom,
					       PropertyStyleLeft };

	for (unsigned i = 0; i < 4; ++i) {
	  if (!layoutImpl_->offsets_[i].isAuto())
	    element.setProperty(properties[i],
				layoutImpl_->offsets_[i].cssText());
	}
      }

      /*
       * set vertical alignment
       */
      switch (layoutImpl_->verticalAlignment_) {
      case AlignBaseline:
	break;
      case AlignSub:
	element.setProperty(PropertyStyleVerticalAlign, "sub"); break;
      case AlignSuper:
	element.setProperty(PropertyStyleVerticalAlign, "super"); break;
      case AlignTop:
	element.setProperty(PropertyStyleVerticalAlign, "top"); break;
      case AlignTextTop:
	element.setProperty(PropertyStyleVerticalAlign, "text-top"); break;
      case AlignMiddle:
	element.setProperty(PropertyStyleVerticalAlign, "middle"); break;
      case AlignBottom:
	element.setProperty(PropertyStyleVerticalAlign, "bottom"); break;
      case AlignTextBottom:
	element.setProperty(PropertyStyleVerticalAlign, "text-bottom"); break;
      case AlignLength:
	element.setProperty(PropertyStyleVerticalAlign,
			    layoutImpl_->verticalAlignmentLength_.cssText());
	break;
      }

      if (!layoutImpl_->lineHeight_.isAuto()) // == none
	element.setProperty(PropertyStyleLineHeight,
			    layoutImpl_->lineHeight_.cssText());

    }

    /*
     * set width & height
     */
    if (width_ && !width_->isAuto())
      element.setProperty(PropertyStyleWidth, width_->cssText());
    if (height_ && !height_->isAuto())
      element.setProperty(PropertyStyleHeight, height_->cssText());

    flags_.reset(BIT_GEOMETRY_CHANGED);
  }


  if (flags_.test(BIT_FLOAT_SIDE_CHANGED) || all) {
    if (layoutImpl_) {
      /*
       * set float
       */
      switch (layoutImpl_->floatSide_) {
      case None:
	if (flags_.test(BIT_FLOAT_SIDE_CHANGED))
	  element.setProperty(PropertyStyleFloat, "none");
	break;
      case Left:
	element.setProperty(PropertyStyleFloat, "left");
	break;
      case Right:
	element.setProperty(PropertyStyleFloat, "right");
	break;
      default:
	/* illegal values */
	;
      }
    }

    flags_.reset(BIT_FLOAT_SIDE_CHANGED);
  }      

  if (layoutImpl_) {
    if (layoutImpl_->marginsChanged_ || all) {
      if (layoutImpl_->marginsChanged_
	  || (layoutImpl_->margin_[0].value() != 0))
	element.setProperty(PropertyStyleMarginTop,
			    layoutImpl_->margin_[0].cssText());
      if (layoutImpl_->marginsChanged_
	  || (layoutImpl_->margin_[1].value() != 0))
	element.setProperty(PropertyStyleMarginRight,
			    layoutImpl_->margin_[1].cssText());
      if (layoutImpl_->marginsChanged_
	  || (layoutImpl_->margin_[2].value() != 0))
	element.setProperty(PropertyStyleMarginBottom,
			    layoutImpl_->margin_[2].cssText());
      if (layoutImpl_->marginsChanged_
	  || (layoutImpl_->margin_[3].value() != 0))
	element.setProperty(PropertyStyleMarginLeft,
			    layoutImpl_->margin_[3].cssText());

      layoutImpl_->marginsChanged_ = false;
    }
  }

  if (lookImpl_) {
    if (lookImpl_->toolTip_
	&& (lookImpl_->toolTipChanged_ || all)) {
      if ((lookImpl_->toolTip_->value().length() > 0)
	  || lookImpl_->toolTipChanged_)
	element.setAttribute("title", lookImpl_->toolTip_->toUTF8());

      lookImpl_->toolTipChanged_ = false;
    }

    if (lookImpl_->decorationStyle_)
      lookImpl_->decorationStyle_->updateDomElement(element, all);

    if (((!all) && lookImpl_->styleClassChanged_)
	|| (all && !lookImpl_->styleClass_.empty()))
      element.setAttribute("class", lookImpl_->styleClass_.toUTF8());

    lookImpl_->styleClassChanged_ = false;
  }

  if (otherImpl_ && otherImpl_->attributes_) {
    if (all) {
      for (std::map<std::string, WString>::const_iterator i
	     = otherImpl_->attributes_->begin();
	   i != otherImpl_->attributes_->end(); ++i)
	element.setAttribute(i->first, i->second.toUTF8());
    } else if (otherImpl_->attributesSet_) {
      for (unsigned i = 0; i < otherImpl_->attributesSet_->size(); ++i) {
	std::string attr = (*otherImpl_->attributesSet_)[i];
	element.setAttribute(attr, (*otherImpl_->attributes_)[attr].toUTF8());
      }
    }

    delete otherImpl_->attributesSet_;
    otherImpl_->attributesSet_ = 0;
  }

  if (flags_.test(BIT_HIDE_WITH_OFFSETS)) {
    if (flags_.test(BIT_HIDDEN_CHANGED)
	|| (all && flags_.test(BIT_HIDDEN))) {

      if (flags_.test(BIT_HIDDEN)) {
	element.setProperty(PropertyStylePosition, "absolute");
	element.setProperty(PropertyStyleLeft, "-10000px");
	element.setProperty(PropertyStyleTop, "-10000px");
	element.setProperty(PropertyStyleVisibility, "hidden");
      } else {
	if (layoutImpl_) {
	  switch (layoutImpl_->positionScheme_) {
	  case Static:
	    element.setProperty(PropertyStylePosition, "static"); break;
	  case Relative:
	    element.setProperty(PropertyStylePosition, "relative"); break;
	  case Absolute:
	    element.setProperty(PropertyStylePosition, "absolute"); break;
	  case Fixed:
	    element.setProperty(PropertyStylePosition, "fixed"); break;
	  }
	  element.setProperty(PropertyStyleTop,
			      layoutImpl_->offsets_[0].cssText());
	  element.setProperty(PropertyStyleLeft,
			      layoutImpl_->offsets_[3].cssText());
	} else {
	  element.setProperty(PropertyStylePosition, "static");
	}
	element.setProperty(PropertyStyleVisibility, "visible");
	element.setProperty(PropertyStyleTop, "0px");
	element.setProperty(PropertyStyleLeft, "0px");
	element.setProperty(PropertyStyleDisplay, ""); // XXX
      }

      flags_.reset(BIT_HIDDEN_CHANGED);
    }
  }

  renderOk();

  delete transientImpl_;
  transientImpl_ = 0;
}

bool WWebWidget::isStubbed() const
{
  if (flags_.test(BIT_STUBBED))
    return true;
  else
    if (parent())
      return parent()->isStubbed();
    else
      return false;
}

bool WWebWidget::isVisible() const
{
  if (flags_.test(BIT_STUBBED) || flags_.test(BIT_HIDDEN))
    return false;
  else
    if (parent())
      return parent()->isVisible();
    else
      return true;
}

void WWebWidget::getSFormObjects(std::vector<WObject *>& result)
{
  if (!flags_.test(BIT_STUBBED) && !flags_.test(BIT_HIDDEN))
    getFormObjects(result);
}

void WWebWidget::getFormObjects(std::vector<WObject *>& formObjects)
{
  if (flags_.test(BIT_FORM_OBJECT))
    formObjects.push_back(this);

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->getSFormObjects(formObjects);
}

void WWebWidget::getDomChanges(std::vector<DomElement *>& result,
			       WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, domElementType());
  updateDom(*e, false);
  result.push_back(e);
}

void WWebWidget::getSDomChanges(std::vector<DomElement *>& result,
				WApplication *app)
{
  bool isIEMobile = app->environment().agentIEMobile();

  if (flags_.test(BIT_STUBBED)) {
    /*
     * If we are prelearning, we still want to catch changes to *this*
     * widget, since it could be related to the prelearning. Note that
     * this assumes that getDomChanges() does not attempt to do things
     * like recreating the widget...
     *
     * ... which is what happens in WContainerWidget, we make the exception
     * there...
     */
    if (app->session()->renderer().preLearning()) {
      getDomChanges(result, app);
      repaint();
    } else {
      flags_.reset(BIT_STUBBED);

      if (!isIEMobile) {
	DomElement *stub = DomElement::getForUpdate(this, DomElement_SPAN);
	DomElement *realElement = createDomElement(app);
	stub->replaceWith(realElement, !flags_.test(BIT_HIDE_WITH_OFFSETS));
	result.push_back(stub);
      } else
	propagateRenderOk();
    }
  } else {
    if (isIEMobile) {
      if (flags_.test(BIT_REPAINT_PROPERTY_ATTRIBUTE)) {
	WWidget *p = this;
	WWebWidget *w = this;
	do {
	  p = p->parent();
	  if (p)
	    w = p->webWidget();
	} while (p && w == this);

	w ->getSDomChanges(result, app);
      } else if (flags_.test(BIT_REPAINT_INNER_HTML)
		 || !flags_.test(BIT_REPAINT_PROPERTY_IEMOBILE)) {
	// the last condition results from repainting the parent, in which
	// case no change flags are set
	DomElement *e = createDomElement(app);
	e->updateInnerHtmlOnly();
	result.push_back(e);
      } else { // BIT_REPAINT_PROPERTY_IEMOBILE only
	getDomChanges(result, app);
      }

      return;
    }

    if (transientImpl_) {
      result.insert(result.end(), transientImpl_->childRemoveChanges_.begin(),
		    transientImpl_->childRemoveChanges_.end());
      transientImpl_->childRemoveChanges_.clear();
    }

    getDomChanges(result, app);
  }
}

void WWebWidget::prepareRerender()
{
}

void WWebWidget::doneRerender()
{
  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->doneRerender();
}

void WWebWidget::propagateRenderOk(bool deep)
{
  if (flags_.test(BIT_STUBBED))
    return;

  if (needRerender()) {
    DomElement *v = DomElement::createNew(DomElement_SPAN);
    updateDom(*v, false);
    delete v;
  }

  if (deep && children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->propagateRenderOk();

  delete transientImpl_;
  transientImpl_ = 0;
}

void WWebWidget::quickPropagateRenderOk()
{
  renderOk();

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->quickPropagateRenderOk();
}

void WWebWidget::setLoadLaterWhenInvisible(bool how)
{
  flags_.set(BIT_DONOT_STUB, !how);
}

void WWebWidget::setId(DomElement *element, WApplication *app)
{
  if (!app->environment().agentIsSpiderBot()
      || (otherImpl_ && otherImpl_->id_))
    element->setId(this, flags_.test(BIT_FORM_OBJECT));
}

DomElement *WWebWidget::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);

  updateDom(*result, true);

  return result;
}

DomElement *WWebWidget::createSDomElement(WApplication *app)
{
  if (!flags_.test(BIT_DONOT_STUB)
      && flags_.test(BIT_HIDDEN)
      && WApplication::instance()->session()->renderer().visibleOnly()) {
    propagateRenderOk();

    flags_.set(BIT_STUBBED);

    DomElement *stub = DomElement::createNew(DomElement_SPAN);
    if (!flags_.test(BIT_HIDE_WITH_OFFSETS)) {
      stub->setProperty(Wt::PropertyStyleDisplay, "none");
    } else {
      stub->setProperty(PropertyStylePosition, "absolute");
      stub->setProperty(PropertyStyleLeft, "-10000px");
      stub->setProperty(PropertyStyleTop, "-10000px");
      stub->setProperty(PropertyStyleVisibility, "hidden");
    }
    if (wApp->environment().javaScript())
      stub->setProperty(Wt::PropertyInnerHTML, "...");

    if (!app->environment().agentIsSpiderBot()
	|| (otherImpl_ && otherImpl_->id_))
      stub->setId(this);

    repaint();

    return stub;
  } else {
    flags_.reset(BIT_STUBBED);

    prepareRerender();

    return createDomElement(app);
  }
}

void WWebWidget::setNoFormData()
{
  WObject::setNoFormData();
}

void WWebWidget::refresh()
{
  if (lookImpl_ && lookImpl_->toolTip_)
    if (lookImpl_->toolTip_->refresh()) {
      lookImpl_->toolTipChanged_ = true;
      repaint(RepaintPropertyAttribute);
    }

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->refresh();
}

void WWebWidget::setIgnoreChildRemoves(bool how)
{
  if (how)
    flags_.set(BIT_IGNORE_CHILD_REMOVES);
  else
    flags_.reset(BIT_IGNORE_CHILD_REMOVES);
}

WString WWebWidget::escapeText(const WString& text,
			       bool newlinestoo)
{
  std::string result = text.toUTF8();

  Wt::Utils::replace(result, '&', "&amp;");
  Wt::Utils::replace(result, '<', "&lt;");
  Wt::Utils::replace(result, '>', "&gt;");
  // replace(result, '"', "&quot;");
  // replace(result, '\'', "&apos;");
  if (newlinestoo)
    Wt::Utils::replace(result, '\n', "<br />");

  return WString(result, UTF8);
}

std::string WWebWidget::jsStringLiteral(const std::string& value,
					char delimiter)
{
  std::stringstream result;
  DomElement::jsStringLiteral(result, value, delimiter);
  return result.str();
}

void WWebWidget::load()
{
  flags_.set(BIT_LOADED);

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      doLoad((*children_)[i]);

  if (flags_.test(BIT_HIDE_WITH_OFFSETS))
    parent()->setHideWithOffsets();
}

void WWebWidget::doLoad(WWidget *w)
{
  w->load();
  if (!w->loaded())
    std::cerr << "Improper load() implementation: base implementation not "
      "called?" << std::endl;
}

bool WWebWidget::loaded() const
{
  return flags_.test(BIT_LOADED);
}

WWebWidget::DropMimeType::DropMimeType(const WString& aHoverStyleClass)
  : hoverStyleClass(aHoverStyleClass)
{ }

WWebWidget::DropMimeType::DropMimeType()
{ }

bool WWebWidget::setAcceptDropsImpl(const std::string& mimeType, bool accept,
				    const WString& hoverStyleClass)
{
  bool result  = false; // whether the signal needs to be connected.
  bool changed = false;

  if (!otherImpl_)
    otherImpl_ = new OtherImpl();
  if (!otherImpl_->acceptedDropMimeTypes_)
    otherImpl_->acceptedDropMimeTypes_ = new OtherImpl::MimeTypesMap;

  OtherImpl::MimeTypesMap::iterator i
    = otherImpl_->acceptedDropMimeTypes_->find(mimeType);

  if (i == otherImpl_->acceptedDropMimeTypes_->end()) {
    if (accept) {
      result = otherImpl_->acceptedDropMimeTypes_->empty();
      (*otherImpl_->acceptedDropMimeTypes_)[mimeType]
	= DropMimeType(hoverStyleClass);
      changed = true;
    }
  } else {
    if (!accept) {
      otherImpl_->acceptedDropMimeTypes_->erase(i);
      changed = true;
    }
  }

  if (changed) {
    std::string mimeTypes = "";

    for (OtherImpl::MimeTypesMap::const_iterator i
	   = otherImpl_->acceptedDropMimeTypes_->begin();
	 i != otherImpl_->acceptedDropMimeTypes_->end(); ++i) {
      mimeTypes
	+= "{" + i->first + ":" + i->second.hoverStyleClass.toUTF8() + "}";
    }

    setAttributeValue("amts", mimeTypes);
  }

  if (result && !otherImpl_->dropSignal_)
    otherImpl_->dropSignal_
      = new JSignal<std::string,std::string, WMouseEvent>(this, "_drop");

  return result;
}

void WWebWidget::updateSignalConnection(DomElement& element,
					EventSignalBase &signal,
					const char *eventName,
					bool all)
{
  if (all || signal.needUpdate()) {

    if (signal.isConnected())
      element.setEventSignal(eventName, signal);
    else
      if (!all)
	element.setEvent(eventName, std::string(), std::string());

    signal.updateOk();
  }
}

bool WWebWidget::canOptimizeUpdates()
{
  return !WApplication::instance()->session()->renderer().preLearning();
}

std::string WWebWidget::fixRelativeUrl(const std::string& url)
{
  return WApplication::instance()->fixRelativeUrl(url);
}

/*
 * XML parsing for removing illegal and dangerous tags to prevent XSS.
 */
namespace {

class MyHandler
{
public:
  MyHandler();

  static void sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data);

private:
  void saxCallback(mxml_node_t *node, mxml_sax_event_t event);

  bool isBadTag(const std::string& name);
  bool isBadAttribute(const std::string& name);
  bool isBadAttributeValue(const std::string& name, const std::string& value);

  int discard_;
};

MyHandler::MyHandler()
  : discard_(0)
{ }

bool MyHandler::isBadTag(const std::string& name)
{
  return (boost::iequals(name, "script")
	  || boost::iequals(name, "applet")
	  || boost::iequals(name, "object")
	  || boost::iequals(name, "iframe")
	  || boost::iequals(name, "frame")
	  || boost::iequals(name, "layer")
	  || boost::iequals(name, "ilayer")
	  || boost::iequals(name, "frameset")
	  || boost::iequals(name, "link")
	  || boost::iequals(name, "meta")
	  || boost::iequals(name, "title")
	  || boost::iequals(name, "base")
	  || boost::iequals(name, "basefont")
	  || boost::iequals(name, "bgsound")
	  || boost::iequals(name, "head")
	  || boost::iequals(name, "body")
	  || boost::iequals(name, "embed")
	  || boost::iequals(name, "style")
	  || boost::iequals(name, "blink"));
}

bool MyHandler::isBadAttribute(const std::string& name)
{
  return (boost::istarts_with(name, "on")
	  || boost::istarts_with(name, "data")
	  || boost::iequals(name, "dynsrc")
	  || boost::iequals(name, "id")
	  || boost::iequals(name, "name"));
}

bool MyHandler::isBadAttributeValue(const std::string& name,
				    const std::string& value)
{
  if (boost::iequals(name, "action")
      || boost::iequals(name, "background")
      || boost::iequals(name, "codebase")
      || boost::iequals(name, "dynsrc")
      || boost::iequals(name, "href")
      || boost::iequals(name, "src"))
    return (boost::istarts_with(value, "javascript:")
	    || boost::istarts_with(value, "vbscript:")
	    || boost::istarts_with(value, "about:")
	    || boost::istarts_with(value, "chrome:")
	    || boost::istarts_with(value, "data:")
	    || boost::istarts_with(value, "disk:")
	    || boost::istarts_with(value, "hcp:")
	    || boost::istarts_with(value, "help:")
	    || boost::istarts_with(value, "livescript")
	    || boost::istarts_with(value, "lynxcgi:")
	    || boost::istarts_with(value, "lynxexec:")
	    || boost::istarts_with(value, "ms-help:")
	    || boost::istarts_with(value, "ms-its:")
	    || boost::istarts_with(value, "mhtml:")
	    || boost::istarts_with(value, "mocha:")
	    || boost::istarts_with(value, "opera:")
	    || boost::istarts_with(value, "res:")
	    || boost::istarts_with(value, "resource:")
	    || boost::istarts_with(value, "shell:")
	    || boost::istarts_with(value, "view-source:")
	    || boost::istarts_with(value, "vnd.ms.radio:")
	    || boost::istarts_with(value, "wysiwyg:"));
  else
    if (boost::iequals(name, "style"))
      return boost::icontains(value, "absolute")
	|| boost::icontains(value, "behaviour")
	|| boost::icontains(value, "content")
	|| boost::icontains(value, "expression")
	|| boost::icontains(value, "fixed")
	|| boost::icontains(value, "include-source")
	|| boost::icontains(value, "moz-binding")
	|| boost::icontains(value, "javascript");
    else
      return false;
}

void MyHandler::saxCallback(mxml_node_t *node, mxml_sax_event_t event)
{
  if (event == MXML_SAX_ELEMENT_OPEN) {
    const char *name = node->value.element.name;

    if (isBadTag(name)) {
      wApp->log("warn") << "(XSS) discarding invalid tag: " << name;
      ++discard_;
    }

    if (discard_ == 0) {
      for (int i = 0; i < node->value.element.num_attrs; ++i) {
	const char *aname = node->value.element.attrs[i].name;
	char *v = node->value.element.attrs[i].value;
	
	if (isBadAttribute(aname) || isBadAttributeValue(aname, v)) {
	  wApp->log("warn") << "(XSS) discarding invalid attribute: "
			    << aname << ": " << v;
	  mxmlElementDeleteAttr(node, aname);
	  --i;
	}
      }
    }
  } else if (event == MXML_SAX_ELEMENT_CLOSE) {
    std::string name = node->value.element.name;
    if (isBadTag(name))
      --discard_;

    if (!discard_) {
      if (!node->child && !DomElement::isSelfClosingTag(name)) {
	mxmlNewText(node, 0, "");
      }
    }
  }

  if (!discard_) {
    if (event == MXML_SAX_ELEMENT_OPEN)
      mxmlRetain(node);
    else if (event == MXML_SAX_DIRECTIVE)
      mxmlRetain(node);
    else if (event == MXML_SAX_DATA && node->parent->ref_count > 1) {
      /*
       * If the parent was retained, then retain
       * this data node as well.
       */
      mxmlRetain(node);
    }
  }
}

void MyHandler::sax_cb(mxml_node_t *node, mxml_sax_event_t event,
			     void *data)
{
  MyHandler *instance = (MyHandler *)data;

  instance->saxCallback(node, event);
}

}

bool WWebWidget::removeScript(WString& text)
{
  if (text.empty())
    return true;

  std::string result = "<span>" + text.toUTF8() + "</span>";

  MyHandler handler;

  mxml_node_t *top = mxmlNewElement(MXML_NO_PARENT, "span");
  mxml_node_t *first
    = mxmlSAXLoadString(top, result.c_str(), MXML_NO_CALLBACK,
			MyHandler::sax_cb, &handler);

  if (first) {
    char *r = mxmlSaveAllocString(top, MXML_NO_CALLBACK);
    result = r;
    free(r);
  } else {
    mxmlDelete(top);
    wApp->log("error") << "Error parsing: " << text;

    return false;
  }

  mxmlDelete(top);

  /*
   * 27 is the length of '<span><span>x</span></span>\n'
   */

  if (result.length() < 28)
    result.clear();
  else
    result = result.substr(12, result.length() - 27);

  text = WString::fromUTF8(result);

  return true;
}
