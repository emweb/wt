/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication"
#include "Wt/WCompositeWidget"
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
#include "XSSFilter.h"
#ifdef max
#undef max
#endif
using namespace Wt;

std::vector<WWidget *> WWebWidget::emptyWidgetList_;

#ifndef WT_TARGET_JAVA
const std::bitset<29> WWebWidget::AllChangeFlags = std::bitset<29>()
  .set(BIT_HIDDEN_CHANGED)
  .set(BIT_GEOMETRY_CHANGED)
  .set(BIT_FLOAT_SIDE_CHANGED)
  .set(BIT_TOOLTIP_CHANGED)
  .set(BIT_MARGINS_CHANGED)
  .set(BIT_STYLECLASS_CHANGED)
  .set(BIT_SELECTABLE_CHANGED)
  .set(BIT_WIDTH_CHANGED)
  .set(BIT_HEIGHT_CHANGED)
  .set(BIT_DISABLED_CHANGED);
#endif // WT_TARGET_JAVA

WWebWidget::TransientImpl::TransientImpl()
{ 
  specialChildRemove_ = false;
}

WWebWidget::TransientImpl::~TransientImpl()
{
}

WWebWidget::LayoutImpl::LayoutImpl()
  : positionScheme_(Static),
    floatSide_(static_cast<Side>(0)),
    clearSides_(0),
    zIndex_(0),
    verticalAlignment_(AlignBaseline)
{ 
  for (unsigned i = 0; i < 4; ++i) {
#ifdef WT_TARGET_JAVA
    offsets_[i] = WLength::Auto;
#endif // WT_TARGET_JAVA
    margin_[i] = WLength(0);
  }
}

WWebWidget::LookImpl::LookImpl()
  : decorationStyle_(0),
    toolTip_(0)
{ }

WWebWidget::LookImpl::~LookImpl()
{
  delete decorationStyle_;
  delete toolTip_;
}

WWebWidget::OtherImpl::OtherImpl()
  : id_(0),
    attributes_(0),
    attributesSet_(0),
    jsMembers_(0),
    jsMembersSet_(0),
    jsMemberCalls_(0),
    dropSignal_(0),
    acceptedDropMimeTypes_(0),
    delayedDoJavaScript_(0)
{ }

WWebWidget::OtherImpl::~OtherImpl()
{
  delete id_;
  delete attributes_;
  delete attributesSet_;
  delete jsMembers_;
  delete jsMembersSet_;
  delete jsMemberCalls_;
  delete dropSignal_;
  delete acceptedDropMimeTypes_;
  delete delayedDoJavaScript_;
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
  flags_.set(BIT_ENABLED);

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

void WWebWidget::setSelectable(bool selectable)
{
  flags_.set(BIT_SET_SELECTABLE, selectable);
  flags_.set(BIT_SET_UNSELECTABLE, !selectable);
  flags_.set(BIT_SELECTABLE_CHANGED);

  repaint(RepaintPropertyAttribute);
}

const std::string WWebWidget::id() const
{
  if (otherImpl_ && otherImpl_->id_)
    return *otherImpl_->id_;
  else
    return WWidget::id();  
}

void WWebWidget::repaint(WFlags<RepaintFlag> flags)
{
  /*
   * If the widget is currently within a stubbed widget (but not
   * stubbed itself, since then it is considered to be painted), we need
   * to redo the slot learning while unstubbing.
   */
  if (!flags_.test(BIT_STUBBED) && isStubbed()) {
    WebRenderer& renderer = WApplication::instance()->session()->renderer();
    if (renderer.preLearning())
      renderer.learningIncomplete();
  }

  /*
   * We ignore repaints to an unrendered widget.
   */
  if (!flags_.test(BIT_RENDERED))
    return;

  WWidget::askRerender();

#ifndef WT_TARGET_JAVA
  flags_ |= (int)flags;
#else
  if (flags & RepaintPropertyIEMobile)
    flags_.set(BIT_REPAINT_PROPERTY_IEMOBILE);
  if (flags & RepaintPropertyAttribute)
    flags_.set(BIT_REPAINT_PROPERTY_ATTRIBUTE);
  if (flags & RepaintInnerHtml)
    flags_.set(BIT_REPAINT_INNER_HTML);
  if (flags & RepaintToAjax)
    flags_.set(BIT_REPAINT_TO_AJAX);
#endif // WT_TARGET_JAVA
}

void WWebWidget::renderOk()
{
  WWidget::renderOk();

#ifndef WT_TARGET_JAVA
  flags_ &= ~(int)(RepaintAll | RepaintToAjax);
#else // WT_TARGET_JAVA
  flags_.reset(BIT_REPAINT_PROPERTY_IEMOBILE);
  flags_.reset(BIT_REPAINT_PROPERTY_ATTRIBUTE);
  flags_.reset(BIT_REPAINT_INNER_HTML);
  flags_.reset(BIT_REPAINT_TO_AJAX);
#endif // WT_TARGET_JAVA
}

void WWebWidget::signalConnectionsChanged()
{
  repaint(RepaintPropertyAttribute);
}

void WWebWidget::beingDeleted()
{
  // flag that we are being deleted, this allows some optimalizations
  flags_.set(BIT_BEING_DELETED);
  flags_.set(BIT_IGNORE_CHILD_REMOVES);  
}

WWebWidget::~WWebWidget()
{
  beingDeleted();

  setParentWidget(0);

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

void WWebWidget::setDecorationStyle(const WCssDecorationStyle& style)
{
#ifndef WT_TARGET_JAVA
  decorationStyle() = style;
#else
  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  lookImpl_->decorationStyle_ = &style;
#endif // WT_TARGET_JAVA
}

std::string WWebWidget::renderRemoveJs()
{
  return "_" + id();
}

void WWebWidget::removeChild(WWidget *child)
{
  assert(children_ != 0);

  int i = Utils::indexOf(*children_, child);

  assert (i != -1);

  if (!flags_.test(BIT_IGNORE_CHILD_REMOVES)) {
    std::string js = child->webWidget()->renderRemoveJs();

    if (!transientImpl_)
      transientImpl_ = new TransientImpl();

    transientImpl_->childRemoveChanges_.push_back(js);
    if (js[0] != '_')
      transientImpl_->specialChildRemove_ = true;

    repaint(RepaintInnerHtml);
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

  child->setParent(0);
    
  /*
   * When the child is about to be deleted, all of its descendants
   * properly removes itself from the renderer "dirty" list. If not,
   * we here force this propagation.
   */
  if (!child->webWidget()->flags_.test(BIT_BEING_DELETED))
    child->webWidget()->setRendered(false);

  children_->erase(children_->begin() + i);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(child->webWidget(), true);
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

PositionScheme WWebWidget::positionScheme() const
{
  return layoutImpl_ ? layoutImpl_->positionScheme_ : Static;
}

void WWebWidget::resize(const WLength& width, const WLength& height)
{
  if (!width_ && !width.isAuto())
    width_ = new WLength();

  if (width_ && *width_ != width) {
    *width_ = width;
    flags_.set(BIT_WIDTH_CHANGED);
  }

  if (!height_ && !height.isAuto())
    height_ = new WLength();

  if (height_ && *height_ != height) {
    *height_ = height;
    flags_.set(BIT_HEIGHT_CHANGED);
  }

  repaint(RepaintPropertyAttribute);

  WWidget::resize(width, height);
}

WLength WWebWidget::width() const
{
  return width_ ? *width_ : WLength::Auto;
}

WLength WWebWidget::height() const
{
  return height_ ? *height_ : WLength::Auto;
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
  return layoutImpl_ ? layoutImpl_->minimumWidth_ : WLength::Auto;
}

WLength WWebWidget::minimumHeight() const
{
  return layoutImpl_ ? layoutImpl_->minimumHeight_ : WLength::Auto;
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
  return layoutImpl_ ? layoutImpl_->maximumWidth_ : WLength::Auto;
}

WLength WWebWidget::maximumHeight() const
{
  return layoutImpl_ ? layoutImpl_->maximumHeight_ : WLength::Auto;
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
  return layoutImpl_ ? layoutImpl_->lineHeight_ : WLength::Auto;
}

void WWebWidget::setFloatSide(Side s)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->floatSide_ = s;

  flags_.set(BIT_FLOAT_SIDE_CHANGED);

  repaint(RepaintPropertyAttribute);
}

Side WWebWidget::floatSide() const
{
  if (layoutImpl_)
    return layoutImpl_->floatSide_;
  else
    return static_cast<Side>(0);
}

void WWebWidget::setClearSides(WFlags<Side> sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->clearSides_ = sides;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WFlags<Side> WWebWidget::clearSides() const
{
  if (layoutImpl_)
    return layoutImpl_->clearSides_;
  else
    return WFlags<Side>(None);
}

void WWebWidget::setVerticalAlignment(AlignmentFlag alignment,
				      const WLength& length)
{
  if (AlignHorizontalMask & alignment) {
    wApp->log("warning") << "WWebWidget::setVerticalAlignment(): alignment "
      "(" << alignment << ") is horizontal, expected vertical";
  }
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->verticalAlignment_ = alignment;
  layoutImpl_->verticalAlignmentLength_ = length;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

AlignmentFlag WWebWidget::verticalAlignment() const
{
  return layoutImpl_ ? layoutImpl_->verticalAlignment_ : AlignBaseline;
}

WLength WWebWidget::verticalAlignmentLength() const
{
  return layoutImpl_ ? layoutImpl_->verticalAlignmentLength_ : WLength::Auto;
}

void WWebWidget::setOffsets(const WLength& offset, WFlags<Side> sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();  

  if (sides & Top)
    layoutImpl_->offsets_[0] = offset;
  if (sides & Right)
    layoutImpl_->offsets_[1] = offset;
  if (sides & Bottom)
    layoutImpl_->offsets_[2] = offset;
  if (sides & Left)
    layoutImpl_->offsets_[3] = offset;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WLength WWebWidget::offset(Side s) const
{
  if (!layoutImpl_)
    return WLength::Auto;

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
    return layoutImpl_->zIndex_;
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

  layoutImpl_->zIndex_ = popup ? -1 : 0;

  if (popup && parent())
    calcZIndex();

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::setZIndex(int zIndex)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();

  layoutImpl_->zIndex_ = zIndex;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::gotParent()
{
  if (isPopup())
    calcZIndex();
}

void WWebWidget::calcZIndex()
{
  layoutImpl_->zIndex_ = -1;

  // find parent webwidget, i.e. skipping composite widgets
  WWidget *p = this;
  do {
    p = p->parent();
  } while (p != 0 && dynamic_cast<WCompositeWidget *>(p) != 0);

  if (p == 0)
    return;

  WWebWidget *ww = p->webWidget();
  if (ww) {
    const std::vector<WWidget *>& children = ww->children();

    int maxZ = 0;
    for (unsigned i = 0; i < children.size(); ++i) {
      WWebWidget *wi = children[i]->webWidget();
      maxZ = std::max(maxZ, wi->zIndex());
    }

    layoutImpl_->zIndex_ = maxZ + 5;
  }
}

bool WWebWidget::isPopup() const
{
  return layoutImpl_ ? layoutImpl_->zIndex_ != 0 : false;
}

void WWebWidget::setMargin(const WLength& margin, WFlags<Side> sides)
{
  if (!layoutImpl_)
    layoutImpl_ = new LayoutImpl();  

  if (sides & Top)
    layoutImpl_->margin_[0] = margin;
  if (sides & Right)
    layoutImpl_->margin_[1] = margin;
  if (sides & Bottom)
    layoutImpl_->margin_[2] = margin;
  if (sides & Left)
    layoutImpl_->margin_[3] = margin;

  flags_.set(BIT_MARGINS_CHANGED);

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

void WWebWidget::addStyleClass(const WT_USTRING& styleClass, bool force)
{
  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  std::string currentClass = lookImpl_->styleClass_.toUTF8();
  std::set<std::string> classes;
  Utils::split(classes, currentClass, " ", true);
  
  if (classes.find(styleClass.toUTF8()) == classes.end()) {
    lookImpl_->styleClass_
      = WT_USTRING::fromUTF8(Utils::addWord(lookImpl_->styleClass_.toUTF8(),
					    styleClass.toUTF8()));

    if (!force) {
      flags_.set(BIT_STYLECLASS_CHANGED);
      repaint(RepaintPropertyAttribute);
    }
  }

  if (force && isRendered()) {
    if (!transientImpl_)
      transientImpl_ = new TransientImpl();

    transientImpl_->addedStyleClasses_.push_back(styleClass);
    Utils::erase(transientImpl_->removedStyleClasses_, styleClass);

    repaint(RepaintPropertyAttribute);
  }
}

void WWebWidget::removeStyleClass(const WT_USTRING& styleClass, bool force)
{
  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  std::string currentClass = lookImpl_->styleClass_.toUTF8();
  std::set<std::string> classes;
  Utils::split(classes, currentClass, " ", true);

  if (classes.find(styleClass.toUTF8()) != classes.end()) {
    // perhaps it is quicker to join the classes back, but then we need to
    // make sure we keep the original order ?
    lookImpl_->styleClass_
      = WT_USTRING::fromUTF8(Utils::eraseWord(lookImpl_->styleClass_.toUTF8(),
					      styleClass.toUTF8()));
    if (!force) {
      flags_.set(BIT_STYLECLASS_CHANGED);
      repaint(RepaintPropertyAttribute);
    }
  }

  if (force && isRendered()) {
    if (!transientImpl_)
      transientImpl_ = new TransientImpl();

    transientImpl_->removedStyleClasses_.push_back(styleClass);
    Utils::erase(transientImpl_->addedStyleClasses_, styleClass);

    repaint(RepaintPropertyAttribute);
  }
}

void WWebWidget::addStyleClass(const char *styleClass, bool force)
{
  addStyleClass(WString::fromUTF8(styleClass), force);
}

void WWebWidget::removeStyleClass(const char *styleClass, bool force)
{
  removeStyleClass(WString::fromUTF8(styleClass), force);
}

void WWebWidget::setStyleClass(const WT_USTRING& styleClass)
{
  if (canOptimizeUpdates() && (styleClass == this->styleClass()))
    return;

  if (!lookImpl_)
    lookImpl_ = new LookImpl();

  lookImpl_->styleClass_ = styleClass;

  flags_.set(BIT_STYLECLASS_CHANGED);

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::setStyleClass(const char *styleClass)
{
  setStyleClass(WString::fromUTF8(styleClass));
}

WT_USTRING WWebWidget::styleClass() const
{
  return lookImpl_ ? lookImpl_->styleClass_ : WT_USTRING();
}

void WWebWidget::setAttributeValue(const std::string& name,
				   const WT_USTRING& value)
{
  if (!otherImpl_)
    otherImpl_ = new OtherImpl();

  if (!otherImpl_->attributes_)
    otherImpl_->attributes_ = new std::map<std::string, WT_USTRING>;

  std::map<std::string, WT_USTRING>::const_iterator i
    = otherImpl_->attributes_->find(name);
  
  if (i != otherImpl_->attributes_->end() && i->second == value)
    return;

  (*otherImpl_->attributes_)[name] = value;

  if (!otherImpl_->attributesSet_)
    otherImpl_->attributesSet_ = new std::vector<std::string>;

  otherImpl_->attributesSet_->push_back(name);

  repaint(RepaintPropertyAttribute);
}

WT_USTRING WWebWidget::attributeValue(const std::string& name) const
{
  if (otherImpl_) {
    std::map<std::string, WT_USTRING>::const_iterator i
      = otherImpl_->attributes_->find(name);

    if (i != otherImpl_->attributes_->end())
      return i->second;
  }

  return WT_USTRING();
}

void WWebWidget::setJavaScriptMember(const std::string& name,
				     const std::string& value)
{
  if (!otherImpl_)
    otherImpl_ = new OtherImpl();

  if (!otherImpl_->jsMembers_)
    otherImpl_->jsMembers_ = new std::vector<OtherImpl::Member>;
  
  std::vector<OtherImpl::Member>& members = *otherImpl_->jsMembers_;
  int index = indexOfJavaScriptMember(name);
  
  if (index != -1 && members[index].value == value)
    return;

  if (value.empty()) {
    if (index != -1)
      members.erase(members.begin() + index);
    else
      return;
  } else {
    OtherImpl::Member m;
    m.name = name;
    m.value = value;
    members.push_back(m);
  }

  if (!otherImpl_->jsMembersSet_)
    otherImpl_->jsMembersSet_ = new std::vector<std::string>;

  otherImpl_->jsMembersSet_->push_back(name);

  repaint(RepaintPropertyAttribute);
}

std::string WWebWidget::javaScriptMember(const std::string& name) const
{
  int index = indexOfJavaScriptMember(name);
  if (index != -1)
    return (*otherImpl_->jsMembers_)[index].value;
  else
    return std::string();
}

int WWebWidget::indexOfJavaScriptMember(const std::string& name) const 
{
  if (otherImpl_ && otherImpl_->jsMembers_)
  for (unsigned i = 0; i < otherImpl_->jsMembers_->size(); i++)
    if ((*otherImpl_->jsMembers_)[i].name == name)
      return i;

  return -1;
}

void WWebWidget::callJavaScriptMember(const std::string& name,
				      const std::string& args)
{
  if (!otherImpl_)
    otherImpl_ = new OtherImpl();

  if (!otherImpl_->jsMemberCalls_)
    otherImpl_->jsMemberCalls_ = new std::vector<std::string>;

  otherImpl_->jsMemberCalls_->push_back(name + "(" + args + ");");

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

  flags_.set(BIT_TOOLTIP_CHANGED);

  repaint(RepaintPropertyAttribute);
}

WString WWebWidget::toolTip() const
{
  return lookImpl_ ? (lookImpl_->toolTip_ ? *lookImpl_->toolTip_ : WString())
    : WString();
}

void WWebWidget::setHiddenKeepsGeometry(bool enabled)
{
  flags_.set(BIT_DONOT_STUB);
  flags_.set(BIT_HIDE_WITH_VISIBILITY, enabled);
  flags_.set(BIT_HIDDEN_CHANGED);
}

bool WWebWidget::hiddenKeepsGeometry() const
{
  return flags_.test(BIT_HIDE_WITH_VISIBILITY)
    && !flags_.test(BIT_HIDE_WITH_OFFSETS);
}

void WWebWidget::setHidden(bool hidden)
{
  if (canOptimizeUpdates() && (hidden == isHidden()))
    return;

  flags_.set(BIT_HIDDEN, hidden);
  flags_.set(BIT_HIDDEN_CHANGED);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, true);

  repaint(RepaintPropertyAttribute);
}

bool WWebWidget::isHidden() const
{
  return flags_.test(BIT_HIDDEN);
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

void WWebWidget::setDisabled(bool disabled)
{
  if (canOptimizeUpdates() && (disabled == flags_.test(BIT_DISABLED)))
    return;

  flags_.set(BIT_DISABLED, disabled);
  flags_.set(BIT_DISABLED_CHANGED);

  propagateSetEnabled(!disabled);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, true);

  repaint(RepaintPropertyAttribute);
}

void WWebWidget::propagateSetEnabled(bool enabled)
{
  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i) {
      WWidget *c = (*children_)[i];
      if (!c->isDisabled())
	c->webWidget()->propagateSetEnabled(enabled);
    }
}

bool WWebWidget::isDisabled() const
{
  return flags_.test(BIT_DISABLED);
}

bool WWebWidget::isEnabled() const
{
  if (isDisabled())
    return false;
  else
    if (parent())
      return parent()->isEnabled();
    else
      return true;
}

void WWebWidget::addChild(WWidget *child)
{
  if (child->parent() == this)
    return;

  if (child->parent() != 0) {
    child->setParentWidget(0);
    wApp->log("warn") << "WWebWidget::addChild(): reparenting child";
  }

  if (!children_)
    children_ = new std::vector<WWidget *>;

  children_->push_back(child);

  child->setParent(this);

  WWebWidget *ww = child->webWidget();
  if (ww)
    ww->gotParent();

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
      flags_.set(BIT_HIDE_WITH_VISIBILITY);
      flags_.set(BIT_HIDE_WITH_OFFSETS);

      resetLearnedSlot(&WWidget::show);
      resetLearnedSlot(&WWidget::hide);

      if (parent())
	parent()->setHideWithOffsets(true);
    }
  }
}

void WWebWidget::updateDom(DomElement& element, bool all)
{
  /*
   * determine display
   */
  if (flags_.test(BIT_GEOMETRY_CHANGED)
      || (!flags_.test(BIT_HIDE_WITH_VISIBILITY)
	  && flags_.test(BIT_HIDDEN_CHANGED))
      || all) {
    if (flags_.test(BIT_HIDE_WITH_VISIBILITY) || !flags_.test(BIT_HIDDEN)) {
      if (element.isDefaultInline() != flags_.test(BIT_INLINE)) {
	if (flags_.test(BIT_INLINE)) {
	  if (element.type() == DomElement_TABLE)
	    element.setProperty(PropertyStyleDisplay, "inline-table");
	  if (element.type() == DomElement_LI)
	    element.setProperty(PropertyStyleDisplay, "inline");
	  else if (element.type() != DomElement_TD) {
	    WApplication *app = WApplication::instance();
	    if (app->environment().agentIsIElt(9)) {
	      element.setProperty(PropertyStyleDisplay, "inline");
	      element.setProperty(PropertyStyleZoom, "1");
	    } else
	      element.setProperty(PropertyStyleDisplay, "inline-block");
	  }
	} else {
	  element.setProperty(PropertyStyleDisplay, "block");
	}
      } else if (!all && flags_.test(BIT_HIDDEN_CHANGED)) {
	if (element.isDefaultInline() == flags_.test(BIT_INLINE))
	  element.setProperty(PropertyStyleDisplay, "");
	else
	  element.setProperty(PropertyStyleDisplay,
			      flags_.test(BIT_INLINE) ? "inline" : "block");
      }
    } else
      element.setProperty(PropertyStyleDisplay, "none");

    if (!flags_.test(BIT_HIDE_WITH_VISIBILITY))
      flags_.reset(BIT_HIDDEN_CHANGED);
  }

  if (flags_.test(BIT_GEOMETRY_CHANGED) || all) {
    if (layoutImpl_) {
      /*
       * set position
       */
      if (!(flags_.test(BIT_HIDE_WITH_VISIBILITY) && flags_.test(BIT_HIDDEN)))
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
      if (layoutImpl_->zIndex_ > 0) {
	element.setProperty(PropertyStyleZIndex,
		    boost::lexical_cast<std::string>(layoutImpl_->zIndex_));
	WApplication *app = WApplication::instance();
	if (all && app->environment().agent() == WEnvironment::IE6
	    && element.type() == DomElement_DIV) {
	  DomElement *i = DomElement::createNew(DomElement_IFRAME);
	  i->setId("sh" + id());
	  i->setProperty(PropertyClass, "Wt-shim");
	  i->setProperty(PropertySrc, "javascript:false;");
	  i->setAttribute("title", "Popup Shim");
	  i->setAttribute("tabindex", "-1");
	  i->setAttribute("frameborder", "0");

	  app->addAutoJavaScript
	    ("{var w = " + jsRef() + ";"
	     "if (w && !" WT_CLASS ".isHidden(w)) {"
	     "var i = " WT_CLASS ".getElement('" + i->id() + "');"
	     "i.style.width=w.clientWidth + 'px';"
	     "i.style.height=w.clientHeight + 'px';"
	     "}}");

	  element.addChild(i);
	}
      }

      /*
       * set clear: FIXME: multiple values
       */
      if (layoutImpl_->clearSides_ == Left) {
	element.setProperty(PropertyStyleClear, "left");
      } else if (layoutImpl_->clearSides_ == Right) {
	element.setProperty(PropertyStyleClear, "right");
      } else if (layoutImpl_->clearSides_ == Horizontals) {
	element.setProperty(PropertyStyleClear, "both");
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
      default:
	break;
      }

      if (!layoutImpl_->lineHeight_.isAuto()) // == none
	element.setProperty(PropertyStyleLineHeight,
			    layoutImpl_->lineHeight_.cssText());

    }

    flags_.reset(BIT_GEOMETRY_CHANGED);
  }


  /*
   * set width & height
   */
  if (width_ && (flags_.test(BIT_WIDTH_CHANGED) || all)) {
    if (!all || !width_->isAuto())
      element.setProperty(PropertyStyleWidth, width_->cssText());
    flags_.reset(BIT_WIDTH_CHANGED);
  }

  if (height_ && (flags_.test(BIT_HEIGHT_CHANGED) || all)) {
    if (!all || !height_->isAuto())
      element.setProperty(PropertyStyleHeight, height_->cssText());
    flags_.reset(BIT_HEIGHT_CHANGED);
  }

  if (flags_.test(BIT_FLOAT_SIDE_CHANGED) || all) {
    if (layoutImpl_) {
      if (layoutImpl_->floatSide_ == 0) { 
	if (flags_.test(BIT_FLOAT_SIDE_CHANGED))
	  element.setProperty(PropertyStyleFloat, "none");
      }
      else {
        /*
        * set float
        */
        switch (layoutImpl_->floatSide_) {
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
    }

    flags_.reset(BIT_FLOAT_SIDE_CHANGED);
  }

  if (layoutImpl_) {
    if (flags_.test(BIT_MARGINS_CHANGED) || all) {
      if (!all || (layoutImpl_->margin_[0].value() != 0))
	element.setProperty(PropertyStyleMarginTop,
			    layoutImpl_->margin_[0].cssText());
      if (!all || (layoutImpl_->margin_[1].value() != 0))
	element.setProperty(PropertyStyleMarginRight,
			    layoutImpl_->margin_[1].cssText());
      if (!all || (layoutImpl_->margin_[2].value() != 0))
	element.setProperty(PropertyStyleMarginBottom,
			    layoutImpl_->margin_[2].cssText());
      if (!all || (layoutImpl_->margin_[3].value() != 0))
	element.setProperty(PropertyStyleMarginLeft,
			    layoutImpl_->margin_[3].cssText());

      flags_.reset(BIT_MARGINS_CHANGED);
    }
  }

  if (lookImpl_) {
    if (lookImpl_->toolTip_
	&& (flags_.test(BIT_TOOLTIP_CHANGED) || all)) {
      if (!all || (!lookImpl_->toolTip_->empty()))
	element.setAttribute("title", lookImpl_->toolTip_->toUTF8());

      flags_.reset(BIT_TOOLTIP_CHANGED);
    }

    if (lookImpl_->decorationStyle_)
      lookImpl_->decorationStyle_->updateDomElement(element, all);

    if (all || flags_.test(BIT_STYLECLASS_CHANGED))
      if (!all || !lookImpl_->styleClass_.empty())
	element.setProperty(PropertyClass,
			    Utils::addWord(element.getProperty(PropertyClass),
					   lookImpl_->styleClass_.toUTF8()));

    flags_.reset(BIT_STYLECLASS_CHANGED);
  }

  if (transientImpl_) {
    for (unsigned i = 0; i < transientImpl_->addedStyleClasses_.size(); ++i)
      element.callJavaScript("$('#" + id() + "').addClass('"
			     + transientImpl_->addedStyleClasses_[i].toUTF8()
			     +"');");

    for (unsigned i = 0; i < transientImpl_->removedStyleClasses_.size(); ++i)
      element.callJavaScript("$('#" + id() + "').removeClass('"
			     + transientImpl_->removedStyleClasses_[i].toUTF8()
			     +"');");

    if (!transientImpl_->childRemoveChanges_.empty()) {
      if ((children_
	   && (children_->size() != transientImpl_->addedChildren_.size()))
	  || transientImpl_->specialChildRemove_) {
	for (unsigned i = 0; i < transientImpl_->childRemoveChanges_.size();
	     ++i) {
	  const std::string& js = transientImpl_->childRemoveChanges_[i];
	  if (js[0] == '_')
	    element.callJavaScript(WT_CLASS ".remove('" + js.substr(1) + "');",
				   true);
	  else
	    element.callJavaScript(js);
	}
      } else
	element.removeAllChildren();

      transientImpl_->childRemoveChanges_.clear();
      transientImpl_->specialChildRemove_ = false;
    }
  }

  if (all || flags_.test(BIT_SELECTABLE_CHANGED)) {
    if (flags_.test(BIT_SET_UNSELECTABLE)) {
      element.setProperty(PropertyClass,
			  Utils::addWord(element.getProperty(PropertyClass),
					 "unselectable"));
      element.setAttribute("unselectable", "on");
      element.setAttribute("onselectstart", "return false;");
    } else if (flags_.test(BIT_SET_SELECTABLE)) {
      element.setProperty(PropertyClass,
			  Utils::addWord(element.getProperty(PropertyClass),
					 "selectable"));
      element.setAttribute("unselectable", "off");
      element.setAttribute("onselectstart",
			   "event.cancelBubble=true; return true;");
    }

    flags_.reset(BIT_SELECTABLE_CHANGED);
  }

  if (otherImpl_) {
    if (otherImpl_->attributes_) {
      if (all) {
	for (std::map<std::string, WT_USTRING>::const_iterator i
	       = otherImpl_->attributes_->begin();
	     i != otherImpl_->attributes_->end(); ++i)
	  if (i->first == "style")
	    element.setProperty(PropertyStyle, i->second.toUTF8());
	  else
	    element.setAttribute(i->first, i->second.toUTF8());
      } else if (otherImpl_->attributesSet_) {
	for (unsigned i = 0; i < otherImpl_->attributesSet_->size(); ++i) {
	  std::string attr = (*otherImpl_->attributesSet_)[i];
	  if (attr == "style")
	    element.setProperty(PropertyStyle,
				(*otherImpl_->attributes_)[attr].toUTF8());
	  else
	    element.setAttribute(attr,
				 (*otherImpl_->attributes_)[attr].toUTF8());
	}
      }

      delete otherImpl_->attributesSet_;
      otherImpl_->attributesSet_ = 0;
    }

    if (otherImpl_->jsMembers_) {
      if (all) {
	for (unsigned i = 0; i < otherImpl_->jsMembers_->size(); i++) {
	  OtherImpl::Member member = (*otherImpl_->jsMembers_)[i];
	  element.callMethod(member.name + "=" + member.value);
	}
      } else if (otherImpl_->jsMembersSet_) {
	for (unsigned i = 0; i < otherImpl_->jsMembersSet_->size(); ++i) {
	  std::string m = (*otherImpl_->jsMembersSet_)[i];

	  std::string value = javaScriptMember(m);

	  if (!value.empty())
	    element.callMethod(m + "=" + value);
	  else
	    element.callMethod(m + "= null");
	}
      }

      delete otherImpl_->jsMembersSet_;
      otherImpl_->jsMembersSet_ = 0;
    }

    if (otherImpl_->jsMemberCalls_) {
      for (unsigned i = 0; i < otherImpl_->jsMemberCalls_->size(); ++i) {
	std::string m = (*otherImpl_->jsMemberCalls_)[i];
	element.callMethod(m);
      }

      delete otherImpl_->jsMemberCalls_;
      otherImpl_->jsMemberCalls_ = 0;
    }
  }

  if (flags_.test(BIT_HIDE_WITH_VISIBILITY)) {
    if (flags_.test(BIT_HIDDEN_CHANGED)
	|| (all && flags_.test(BIT_HIDDEN))) {

      if (flags_.test(BIT_HIDDEN)) {
	element.setProperty(PropertyStyleVisibility, "hidden");
	if (flags_.test(BIT_HIDE_WITH_OFFSETS)) {
	  element.setProperty(PropertyStylePosition, "absolute");
	  element.setProperty(PropertyStyleTop, "-10000px");
	  element.setProperty(PropertyStyleLeft, "-10000px");
	}
      } else {
	if (flags_.test(BIT_HIDE_WITH_OFFSETS)) {
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

	  element.setProperty(PropertyStyleTop, "0px");
	  element.setProperty(PropertyStyleLeft, "0px");
	}
	element.setProperty(PropertyStyleVisibility, "visible");
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
  else {
    WWidget *p = parent();
    return p ? p->isStubbed() : false;
  }
}

bool WWebWidget::needsToBeRendered() const
{
  /*
   * Returns whether this widget should be rendered. The only alternative
   * is to be stubbed as an invisible widget.
   */
  return flags_.test(BIT_DONOT_STUB)
    || !flags_.test(BIT_HIDDEN)
    || !WApplication::instance()->session()->renderer().visibleOnly();
}

void WWebWidget::getSFormObjects(FormObjectsMap& result)
{
  if (!flags_.test(BIT_STUBBED) && !flags_.test(BIT_HIDDEN))
    getFormObjects(result);
}

void WWebWidget::getFormObjects(FormObjectsMap& formObjects)
{
  if (flags_.test(BIT_FORM_OBJECT))
    formObjects[id()] = this;

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
  bool isIEMobile = app->environment().agentIsIEMobile();

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
      askRerender(true);
    } else {
      if (!app->session()->renderer().visibleOnly()) {
	flags_.reset(BIT_STUBBED);

	if (!isIEMobile) {
	  DomElement *stub = DomElement::getForUpdate(this, DomElement_SPAN);
	  setRendered(true);
	  render(RenderFull);
	  DomElement *realElement = createDomElement(app);
	  stub->unstubWith(realElement, !flags_.test(BIT_HIDE_WITH_OFFSETS));
	  result.push_back(stub);
	} else
	  propagateRenderOk();
      }
    }
  } else {
    render(RenderUpdate);

    if (isIEMobile) {
      if (flags_.test(BIT_REPAINT_PROPERTY_ATTRIBUTE)) {
	WWidget *p = this;
	WWebWidget *w = this;
	do {
	  p = p->parent();
	  if (p)
	    w = p->webWidget();
	} while (p && w == this);

	if (w != this)
	  w->getSDomChanges(result, app);
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

    getDomChanges(result, app);
  }
}

void WWebWidget::doneRerender()
{
  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->doneRerender();
}

void WWebWidget::propagateRenderOk(bool deep)
{
#ifndef WT_TARGET_JAVA
  flags_ &= ~AllChangeFlags;
#else
  flags_.reset(BIT_HIDDEN_CHANGED);
  flags_.reset(BIT_GEOMETRY_CHANGED);
  flags_.reset(BIT_FLOAT_SIDE_CHANGED);
  flags_.reset(BIT_TOOLTIP_CHANGED);
  flags_.reset(BIT_MARGINS_CHANGED);
  flags_.reset(BIT_STYLECLASS_CHANGED);
  flags_.reset(BIT_SELECTABLE_CHANGED);
  flags_.reset(BIT_WIDTH_CHANGED);
  flags_.reset(BIT_HEIGHT_CHANGED);
  flags_.reset(BIT_DISABLED_CHANGED);
#endif

  renderOk();

  if (deep && children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->webWidget()->propagateRenderOk();

  delete transientImpl_;
  transientImpl_ = 0;
}

void WWebWidget::setRendered(bool rendered)
{
  if (rendered)
    flags_.set(BIT_RENDERED);
  else {
    flags_.reset(BIT_RENDERED);

    renderOk();

    if (children_)
      for (unsigned i = 0; i < children_->size(); ++i)
	(*children_)[i]->webWidget()->setRendered(false);
  }
}

void WWebWidget::setLoadLaterWhenInvisible(bool how)
{
  flags_.set(BIT_DONOT_STUB, !how);
}

void WWebWidget::setId(DomElement *element, WApplication *app)
{
  if (!app->environment().agentIsSpiderBot()
      || (otherImpl_ && otherImpl_->id_)) {
    if (!flags_.test(BIT_FORM_OBJECT))
      element->setId(id());
    else
      element->setName(id());
  }
}

WWidget *WWebWidget::find(const std::string& name)
{
  if (objectName() == name)
    return this;
  else {
    if (children_)
      for (unsigned i = 0; i < children_->size(); ++i) {
	WWidget *result = (*children_)[i]->find(name);
	if (result)
	  return result;
      }
  }

  return 0;
}

DomElement *WWebWidget::createDomElement(WApplication *app)
{
  setRendered(true);

  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);
  updateDom(*result, true);

  return result;
}

bool WWebWidget::isRendered() const
{
  return flags_.test(WWebWidget::BIT_RENDERED);
}

DomElement *WWebWidget::createStubElement(WApplication *app)
{
  /*
   * Make sure the object itself is clean, so that stateless slot
   * learning is not confused.
   */
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
  if (app->environment().javaScript())
    stub->setProperty(Wt::PropertyInnerHTML, "...");

  if (!app->environment().agentIsSpiderBot()
      || (otherImpl_ && otherImpl_->id_))
    stub->setId(id());

  return stub;
}

DomElement *WWebWidget::createActualElement(WApplication *app)
{
  flags_.reset(BIT_STUBBED);

  return createDomElement(app);
}

void WWebWidget::refresh()
{
  if (lookImpl_ && lookImpl_->toolTip_)
    if (lookImpl_->toolTip_->refresh()) {
      flags_.set(BIT_TOOLTIP_CHANGED);
      repaint(RepaintPropertyAttribute);
    }

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->refresh();

  WWidget::refresh();
}

void WWebWidget::enableAjax()
{
  /*
   * What needs to be done ? We want to get to the same state as the normal
   * AJAX bootstrap: thus still leaving stubs as is.
   */
  if (!isStubbed()) {
    for (EventSignalList::iterator i = eventSignals().begin();
	 i != eventSignals().end(); ++i) {
#ifndef WT_NO_BOOST_INTRUSIVE
      EventSignalBase& s = *i;
#else
      EventSignalBase& s = **i;
#endif
      if (s.name() == WInteractWidget::CLICK_SIGNAL)
	repaint(RepaintToAjax);

      s.senderRepaint();
    }
  }

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      (*children_)[i]->enableAjax();
}

void WWebWidget::setIgnoreChildRemoves(bool how)
{
  if (how)
    flags_.set(BIT_IGNORE_CHILD_REMOVES);
  else
    flags_.reset(BIT_IGNORE_CHILD_REMOVES);
}

bool WWebWidget::ignoreChildRemoves() const
{
  return flags_.test(BIT_IGNORE_CHILD_REMOVES);
}

WString WWebWidget::escapeText(const WString& text, bool newlinestoo)
{
  std::string result = text.toUTF8();
  result = escapeText(result, newlinestoo);
  return WString::fromUTF8(result);
}

std::string& WWebWidget::escapeText(std::string& text, bool newlinestoo)
{
  EscapeOStream sout;
  if (newlinestoo)
    sout.pushEscape(EscapeOStream::PlainTextNewLines);
  else
    sout.pushEscape(EscapeOStream::PlainText);

  Wt::Utils::sanitizeUnicode(sout, text);

  text = sout.str();

  return text;
}

std::string WWebWidget::jsStringLiteral(const std::string& value,
					char delimiter)
{
  std::stringstream result;
  DomElement::jsStringLiteral(result, value, delimiter);
  return result.str();
}

std::string WWebWidget::jsStringLiteral(const WString& value,
					char delimiter)
{
  return value.jsStringLiteral(delimiter);
}

void WWebWidget::load()
{
  flags_.set(BIT_LOADED);

  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i)
      doLoad((*children_)[i]);

  if (flags_.test(BIT_HIDE_WITH_OFFSETS))
    parent()->setHideWithOffsets(true);
}

void WWebWidget::doLoad(WWidget *w)
{
  w->load();
  if (!w->loaded())
    std::cerr << "Improper load() implementation: base implementation not "
      "called?" << std::endl;
}

void WWebWidget::render(WFlags<RenderFlag> flags)
{
  WWidget::render(flags);
  if (otherImpl_ && otherImpl_->delayedDoJavaScript_) {
    wApp->doJavaScript(otherImpl_->delayedDoJavaScript_->str());
    delete otherImpl_->delayedDoJavaScript_;
    otherImpl_->delayedDoJavaScript_ = 0;
  }
}

void WWebWidget::doJavaScript(const std::string& javascript)
{
  if (isRendered())
    wApp->doJavaScript(javascript);
  else {
    if (!otherImpl_)
      otherImpl_ = new OtherImpl;
    if (!otherImpl_->delayedDoJavaScript_)
      otherImpl_->delayedDoJavaScript_ = new SStream;
    (*otherImpl_->delayedDoJavaScript_) << javascript;
  }
}

bool WWebWidget::loaded() const
{
  return flags_.test(BIT_LOADED);
}

void WWebWidget::setTabIndex(int index)
{
  if (children_)
    for (unsigned i = 0; i < children_->size(); ++i) {
      WWidget *c = (*children_)[i];
      c->setTabIndex(index);
    }
}

int WWebWidget::tabIndex() const
{
  if (children_) {
    int result = 0;

    for (unsigned i = 0; i < children_->size(); ++i) {
      WWidget *c = (*children_)[i];
      result = std::max(result, c->tabIndex());
    }

    return result;
  } else
    return 0;
}

WWebWidget::DropMimeType::DropMimeType(const WT_USTRING& aHoverStyleClass)
  : hoverStyleClass(aHoverStyleClass)
{ }

WWebWidget::DropMimeType::DropMimeType()
{ }

bool WWebWidget::setAcceptDropsImpl(const std::string& mimeType, bool accept,
				    const WT_USTRING& hoverStyleClass)
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
#ifndef WT_TARGET_JAVA
      otherImpl_->acceptedDropMimeTypes_->erase(i);
#else
      otherImpl_->acceptedDropMimeTypes_->erase(mimeType);
#endif // WT_TARGET_JAVA
      changed = true;
    }
  }

  if (changed) {
    std::string mimeTypes = "";

    for (OtherImpl::MimeTypesMap::const_iterator j
	   = otherImpl_->acceptedDropMimeTypes_->begin();
	 j != otherImpl_->acceptedDropMimeTypes_->end(); ++j) {
      mimeTypes
	+= "{" + j->first + ":" + j->second.hoverStyleClass.toUTF8() + "}";
    }

    setAttributeValue("amts", mimeTypes);
  }

  if (result && !otherImpl_->dropSignal_)
    otherImpl_->dropSignal_
      = new JSignal<std::string,std::string, WMouseEvent>(this, "_drop");

  return result;
}

EventSignal<WKeyEvent> *WWebWidget::keyEventSignal(const char *name,
						   bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WKeyEvent> *>(b);
  else if (!create)
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    EventSignal<WKeyEvent> *result = new EventSignal<WKeyEvent>(name, this);
#else
    EventSignal<WKeyEvent> *result
      = new EventSignal<WKeyEvent>(name, this, WKeyEvent::templateEvent);
#endif // WT_TARGET_JAVA

    addEventSignal(*result);
    return result;
  }
}

EventSignal<> *WWebWidget::voidEventSignal(const char *name, bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<> *>(b);
  else if (!create)
    return 0;
  else {
    EventSignal<> *result = new EventSignal<>(name, this);
    addEventSignal(*result);
    return result;
  }
}

EventSignal<WMouseEvent> *WWebWidget::mouseEventSignal(const char *name,
						       bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WMouseEvent> *>(b);
  else if (!create)
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    EventSignal<WMouseEvent> *result = new EventSignal<WMouseEvent>(name, this);
#else
    EventSignal<WMouseEvent> *result
      = new EventSignal<WMouseEvent>(name, this, WMouseEvent::templateEvent);
#endif // WT_TARGET_JAVA
    addEventSignal(*result);
    return result;
  }
}

EventSignal<WTouchEvent> *WWebWidget::touchEventSignal(const char *name,
						       bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WTouchEvent> *>(b);
  else if (!create)
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    EventSignal<WTouchEvent> *result = new EventSignal<WTouchEvent>(name, this);
#else
    EventSignal<WTouchEvent> *result
      = new EventSignal<WTouchEvent>(name, this, WTouchEvent::templateEvent);
#endif // WT_TARGET_JAVA
    addEventSignal(*result);
    return result;
  }
}

EventSignal<WGestureEvent> *WWebWidget::gestureEventSignal(const char *name,
							   bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WGestureEvent> *>(b);
  else if (!create)
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    EventSignal<WGestureEvent> *result
      = new EventSignal<WGestureEvent>(name, this);
#else
    EventSignal<WGestureEvent> *result
      = new EventSignal<WGestureEvent>(name, this,
				       WGestureEvent::templateEvent);
#endif // WT_TARGET_JAVA
    addEventSignal(*result);
    return result;
  }
}

EventSignal<WScrollEvent> *WWebWidget::scrollEventSignal(const char *name,
							 bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WScrollEvent> *>(b);
  else if (!create)
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    EventSignal<WScrollEvent> *result
      = new EventSignal<WScrollEvent>(name, this);
#else
    EventSignal<WScrollEvent> *result
      = new EventSignal<WScrollEvent>(name, this, WScrollEvent::templateEvent);
#endif // WT_TARGET_JAVA
    addEventSignal(*result);
    return result;
  }
}

void WWebWidget::updateSignalConnection(DomElement& element,
					EventSignalBase &signal,
					const char *name,
					bool all)
{
  if (name[0] != 'M' && signal.needsUpdate(all)) {
    element.setEventSignal(name, signal);
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

bool WWebWidget::removeScript(WString& text)
{
#ifndef WT_NO_XSS_FILTER
  return XSSFilterRemoveScript(text);
#else
  return true;
#endif
}
