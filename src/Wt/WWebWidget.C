/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#include "Wt/WApplication.h"
#include "Wt/WCompositeWidget.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WLogger.h"
#include "Wt/WJavaScript.h"
#include "Wt/WStringStream.h"
#include "Wt/WTheme.h"
#include "Wt/WWebWidget.h"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WebRenderer.h"
#include "WebSession.h"
#include "WebUtils.h"
#include "StringUtils.h"
#include "XSSFilter.h"

#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_xhtml.hpp"

#ifndef WT_DEBUG_JS
#include "js/WWebWidget.min.js"
#include "js/ToolTip.min.js"
#include "js/ScrollVisibility.min.js"
#endif

#ifdef max
#undef max
#endif

namespace Wt {

LOGGER("WWebWidget");

namespace {

  WLength nonNegative(const WLength& w) {
    if (w.isAuto())
      return w;
    else
      return WLength(std::fabs(w.value()), w.unit());
  }

}

const char *WWebWidget::FOCUS_SIGNAL = "focus";
const char *WWebWidget::BLUR_SIGNAL = "blur";
const int WWebWidget::DEFAULT_BASE_Z_INDEX = 1000;

#ifndef WT_TARGET_JAVA
const std::bitset<38> WWebWidget::AllChangeFlags = std::bitset<38>()
  .set(BIT_FLEX_BOX_CHANGED)
  .set(BIT_HIDDEN_CHANGED)
  .set(BIT_GEOMETRY_CHANGED)
  .set(BIT_FLOAT_SIDE_CHANGED)
  .set(BIT_TOOLTIP_CHANGED)
  .set(BIT_MARGINS_CHANGED)
  .set(BIT_STYLECLASS_CHANGED)
  .set(BIT_SELECTABLE_CHANGED)
  .set(BIT_WIDTH_CHANGED)
  .set(BIT_HEIGHT_CHANGED)
  .set(BIT_DISABLED_CHANGED)
  .set(BIT_ZINDEX_CHANGED)
  .set(BIT_TABINDEX_CHANGED)
  .set(BIT_SCROLL_VISIBILITY_CHANGED)
  .set(BIT_OBJECT_NAME_CHANGED);
#endif // WT_TARGET_JAVA

WWebWidget::TransientImpl::TransientImpl()
  : addedChildren_(0),
    specialChildRemove_(false)
{ }

WWebWidget::TransientImpl::~TransientImpl()
{ }

WWebWidget::LayoutImpl::LayoutImpl()
  : positionScheme_(PositionScheme::Static),
    floatSide_(static_cast<Side>(0)),
    clearSides_(None),
    minimumWidth_(0),
    minimumHeight_(0),
    baseZIndex_(DEFAULT_BASE_Z_INDEX),
    zIndex_(0),
    verticalAlignment_(AlignmentFlag::Baseline)
{ 
  for (unsigned i = 0; i < 4; ++i) {
#ifdef WT_TARGET_JAVA
    offsets_[i] = WLength::Auto;
#endif // WT_TARGET_JAVA
    margin_[i] = WLength(0);
  }
}

WWebWidget::LookImpl::LookImpl(WWebWidget *w)
  : toolTipTextFormat_(TextFormat::Plain),
    loadToolTip_(w, "Wt-loadToolTip")
{ }

WWebWidget::LookImpl::~LookImpl()
{ }

WWebWidget::OtherImpl::JavaScriptStatement::JavaScriptStatement
(JavaScriptStatementType aType, const std::string& aData)
  : type(aType),
    data(aData)
{ }

WWebWidget::OtherImpl::OtherImpl(WWebWidget *const self)
  : elementTagName_(nullptr),
    id_(nullptr),
    tabIndex_(std::numeric_limits<int>::min()),
    scrollVisibilityMargin_(0),
    jsScrollVisibilityChanged_(self, "scrollVisibilityChanged")
{
  jsScrollVisibilityChanged_.connect(self, &WWebWidget::jsScrollVisibilityChanged);
}

WWebWidget::OtherImpl::~OtherImpl()
{ }

WWebWidget::WWebWidget()
{
  flags_.set(BIT_INLINE);
  flags_.set(BIT_ENABLED);
}


#ifndef WT_TARGET_JAVA
WStatelessSlot *WWebWidget::getStateless(Method method)
{
  typedef void (WWebWidget::*Type)();

  Type focusMethod = &WWebWidget::setFocus;

  if (method == static_cast<WObject::Method>(focusMethod))
    return implementStateless(focusMethod,
			      &WWebWidget::undoSetFocus);
  else
    return WWidget::getStateless(method);
}
#endif

void WWebWidget::setFormObject(bool how)
{
  flags_.set(BIT_FORM_OBJECT, how);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);
}

void WWebWidget::setId(const std::string& id)
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  WApplication* app = WApplication::instance();
  for (std::size_t i = 0; i < jsignals_.size(); ++i) {
    EventSignalBase* signal = jsignals_[i];
    if (signal->isExposedSignal())
      app->removeExposedSignal(signal);
  }

  if (!otherImpl_->id_)
    otherImpl_->id_.reset(new std::string());

  *otherImpl_->id_ = id;

  for (std::size_t i = 0; i < jsignals_.size(); ++i) {
    EventSignalBase* signal = jsignals_[i];
    if (signal->isExposedSignal())
      app->addExposedSignal(signal);
  }
}

void WWebWidget::setSelectable(bool selectable)
{
  flags_.set(BIT_SET_SELECTABLE, selectable);
  flags_.set(BIT_SET_UNSELECTABLE, !selectable);
  flags_.set(BIT_SELECTABLE_CHANGED);

  repaint();
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
   * If the widget is currently stubbed, we need to redo the slot
   * learning while unstubbing.
   */
  if (isStubbed()) {
    WebRenderer& renderer = WApplication::instance()->session()->renderer();
    if (renderer.preLearning())
      renderer.learningIncomplete();
  }

  /*
   * We ignore repaints to an unrendered widget.
   */
  if (!flags_.test(BIT_RENDERED))
    return;

  WWidget::scheduleRerender(false, flags);

  if (flags.test(RepaintFlag::ToAjax))
    flags_.set(BIT_REPAINT_TO_AJAX);
}

void WWebWidget::renderOk()
{
  WWidget::renderOk();

  flags_.reset(BIT_REPAINT_TO_AJAX);
}

void WWebWidget::signalConnectionsChanged()
{
  repaint();
}

void WWebWidget::beingDeleted()
{
  // flag that we are being deleted, this allows some optimalizations
  flags_.set(BIT_BEING_DELETED);
}

WWebWidget::~WWebWidget()
{
  beingDeleted();
  std::unique_ptr<WWidget> unique_this = removeFromParent();
#ifndef WT_TARGET_JAVA
  // removeFromParent should always return a nullptr if it is called in the destructor!
  assert(unique_this == nullptr);
#endif // WT_TARGET_JAVA
}

WCssDecorationStyle& WWebWidget::decorationStyle()
{
  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  if (!lookImpl_->decorationStyle_) {
    lookImpl_->decorationStyle_.reset(new WCssDecorationStyle());
    lookImpl_->decorationStyle_->setWebWidget(this);
  }

  return *lookImpl_->decorationStyle_;
}

const WCssDecorationStyle& WWebWidget::decorationStyle() const
{
  return const_cast<WWebWidget *>(this)->decorationStyle();
}

void WWebWidget::setDecorationStyle(const WCssDecorationStyle& style)
{
#ifndef WT_TARGET_JAVA
  decorationStyle() = style;
#else
  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  lookImpl_->decorationStyle_.reset(&style);
#endif // WT_TARGET_JAVA
}

void WWebWidget::iterateChildren(const HandleWidgetMethod& method) const
{ }

std::string WWebWidget::renderRemoveJs(bool recursive)
{
  Wt::WStringStream result;

  if (isRendered() && scrollVisibilityEnabled()) {
    result << WT_CLASS ".scrollVisibility.remove("
        << jsStringLiteral(id()) << ");";
    flags_.set(BIT_SCROLL_VISIBILITY_CHANGED);
    flags_.reset(BIT_SCROLL_VISIBILITY_LOADED);
  }

  iterateChildren
    ([&](WWidget *c) {
      result << c->webWidget()->renderRemoveJs(true);
    });

  if (!recursive) {
    if (result.empty())
      result << "_" << id();
    else
      result << WT_CLASS ".remove('" << id() << "');";
  }

  return result.str();
}

void WWebWidget::widgetRemoved(WWidget *child, bool renderRemove)
{
  if (!flags_.test(BIT_BEING_DELETED) && renderRemove) {
    std::string js = child->webWidget()->renderRemoveJs(false);

    if (!transientImpl_)
      transientImpl_.reset(new TransientImpl());

    transientImpl_->childRemoveChanges_.push_back(js);
    if (js[0] != '_')
      transientImpl_->specialChildRemove_ = true;

    repaint(RepaintFlag::SizeAffected);
  }

  child->setParentWidget(nullptr);

  if (transientImpl_ &&
      !child->webWidget()->isRendered() &&
      !child->webWidget()->isStubbed())
    --transientImpl_->addedChildren_;
    
  /*
   * When the child is about to be deleted, all of its descendants
   * properly removes itself from the renderer "dirty" list. If not,
   * we here force this propagation.
   */
  if (!child->webWidget()->flags_.test(BIT_BEING_DELETED))
    child->webWidget()->setRendered(false);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(child->webWidget(), true);

  emitChildrenChanged();
}

Signal<>& WWebWidget::childrenChanged()
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  return otherImpl_->childrenChanged_;
}

void WWebWidget::setPositionScheme(PositionScheme scheme)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->positionScheme_ = scheme;

  if ((scheme == PositionScheme::Absolute) || (scheme == PositionScheme::Fixed))
    flags_.reset(BIT_INLINE);

  flags_.set(BIT_GEOMETRY_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

PositionScheme WWebWidget::positionScheme() const
{
  return layoutImpl_ ? layoutImpl_->positionScheme_ : PositionScheme::Static;
}

void WWebWidget::resize(const WLength& width, const WLength& height)
{
  bool changed = false;

  if (!width_ && !width.isAuto())
    width_.reset(new WLength());

  if (width_ && *width_ != width) {
    changed = true;
    *width_ = nonNegative(width);
    flags_.set(BIT_WIDTH_CHANGED);
  }

  if (!height_ && !height.isAuto())
    height_.reset(new WLength());

  if (height_ && *height_ != height) {
    changed = true;
    *height_ = nonNegative(height);
    flags_.set(BIT_HEIGHT_CHANGED);
  }

  if (changed) {
    repaint(RepaintFlag::SizeAffected);
    WWidget::resize(width, height);
  }
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
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->minimumWidth_ = nonNegative(width);
  layoutImpl_->minimumHeight_ = nonNegative(height);

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintFlag::SizeAffected);
}

WLength WWebWidget::minimumWidth() const
{
  return layoutImpl_ ? layoutImpl_->minimumWidth_ : WLength(0);
}

WLength WWebWidget::minimumHeight() const
{
  return layoutImpl_ ? layoutImpl_->minimumHeight_ : WLength(0);
}

void WWebWidget::setMaximumSize(const WLength& width, const WLength& height)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->maximumWidth_ = nonNegative(width);
  layoutImpl_->maximumHeight_ = nonNegative(height);

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintFlag::SizeAffected);
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
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->lineHeight_ = height;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint(RepaintFlag::SizeAffected);
}

WLength WWebWidget::lineHeight() const
{
  return layoutImpl_ ? layoutImpl_->lineHeight_ : WLength::Auto;
}

void WWebWidget::setFloatSide(Side s)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->floatSide_ = s;

  flags_.set(BIT_FLOAT_SIDE_CHANGED);

  repaint();
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
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->clearSides_ = sides;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint();
}

WFlags<Side> WWebWidget::clearSides() const
{
  if (layoutImpl_)
    return layoutImpl_->clearSides_;
  else
    return None;
}

void WWebWidget::setVerticalAlignment(AlignmentFlag alignment,
				      const WLength& length)
{
  if (AlignHorizontalMask.test(alignment))
    LOG_ERROR("setVerticalAlignment(): alignment " 
	      << static_cast<int>(alignment)
	      << " is not vertical");

  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->verticalAlignment_ = alignment;
  layoutImpl_->verticalAlignmentLength_ = length;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint();
}

AlignmentFlag WWebWidget::verticalAlignment() const
{
  return layoutImpl_ 
    ? layoutImpl_->verticalAlignment_ : AlignmentFlag::Baseline;
}

WLength WWebWidget::verticalAlignmentLength() const
{
  return layoutImpl_ ? layoutImpl_->verticalAlignmentLength_ : WLength::Auto;
}

void WWebWidget::setOffsets(const WLength& offset, WFlags<Side> sides)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  if (sides.test(Side::Top))
    layoutImpl_->offsets_[0] = offset;
  if (sides.test(Side::Right))
    layoutImpl_->offsets_[1] = offset;
  if (sides.test(Side::Bottom))
    layoutImpl_->offsets_[2] = offset;
  if (sides.test(Side::Left))
    layoutImpl_->offsets_[3] = offset;

  flags_.set(BIT_GEOMETRY_CHANGED);

  repaint();
}

WLength WWebWidget::offset(Side s) const
{
  if (!layoutImpl_)
    return WLength::Auto;

  switch (s) {
  case Side::Top:
    return layoutImpl_->offsets_[0];
  case Side::Right:
    return layoutImpl_->offsets_[1];
  case Side::Bottom:
    return layoutImpl_->offsets_[2];
  case Side::Left:
    return layoutImpl_->offsets_[3];
  default:
    LOG_ERROR("offset(Side) with invalid side: " << (int)s);
    return WLength();
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

  repaint();
}

bool WWebWidget::isInline() const
{
  return flags_.test(BIT_INLINE);
}

void WWebWidget::setFlexBox(bool enabled)
{
  flags_.set(BIT_FLEX_BOX, enabled);
  flags_.set(BIT_FLEX_BOX_CHANGED);
}

void WWebWidget::setPopup(bool popup)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->zIndex_ = popup ? -1 : 0;

  if (popup && parent())
    calcZIndex();

  flags_.set(BIT_ZINDEX_CHANGED);

  repaint();
}

void WWebWidget::setZIndex(int zIndex)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->zIndex_ = zIndex;

  flags_.set(BIT_ZINDEX_CHANGED);

  repaint();
}

void WWebWidget::setParentWidget(WWidget *parent)
{
  WWidget::setParentWidget(parent);

  if (parent)
    if (isPopup())
      calcZIndex();
}

WWebWidget *WWebWidget::parentWebWidget() const
{
  /*
   * Returns the parent webwidget, i.e. skipping composite widgets
   */
  WWidget *p = this->parent();
  while (p != nullptr && dynamic_cast<WCompositeWidget *>(p) != nullptr)
    p = p->parent();

  return p ? p->webWidget() : nullptr;
}

WWidget *WWebWidget::selfWidget()
{
  /*
   * Returns the composite widget implemented by this web widget
   */
  WWidget *p = nullptr, *p_parent = this;
  do {
    p = p_parent;
    p_parent = p->parent();
  } while (p_parent != nullptr && 
	   dynamic_cast<WCompositeWidget *>(p_parent) != nullptr);

  return p;
}

void WWebWidget::calcZIndex()
{
  layoutImpl_->zIndex_ = -1;

  WWebWidget *ww = parentWebWidget();
  if (ww) {
    const std::vector<WWidget *>& children = ww->children();

    int maxZ = 0;
    for (unsigned i = 0; i < children.size(); ++i) {
      WWebWidget *wi = children[i]->webWidget();
      if (wi->baseZIndex() <= baseZIndex())
        maxZ = std::max(maxZ, wi->zIndex());
    }

    layoutImpl_->zIndex_ = std::max(baseZIndex(), maxZ + 1000);
  }
}

bool WWebWidget::isPopup() const
{
  return layoutImpl_ ? layoutImpl_->zIndex_ != 0 : false;
}

void WWebWidget::setMargin(const WLength& margin, WFlags<Side> sides)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  if (sides.test(Side::Top))
    layoutImpl_->margin_[0] = margin;
  if (sides.test(Side::Right))
    layoutImpl_->margin_[1] = margin;
  if (sides.test(Side::Bottom))
    layoutImpl_->margin_[2] = margin;
  if (sides.test(Side::Left))
    layoutImpl_->margin_[3] = margin;

  flags_.set(BIT_MARGINS_CHANGED);

  repaint(RepaintFlag::SizeAffected);
}

WLength WWebWidget::margin(Side side) const
{
  if (!layoutImpl_)
    return WLength(0);

  switch (side) {
  case Side::Top:
    return layoutImpl_->margin_[0];
  case Side::Right:
    return layoutImpl_->margin_[1];
  case Side::Bottom:
    return layoutImpl_->margin_[2];
  case Side::Left:
    return layoutImpl_->margin_[3];
  default:
    LOG_ERROR("margin(Side) with invalid side: " << (int)side);
    return WLength();
  }
}

void WWebWidget::addStyleClass(const WT_USTRING& styleClass, bool force)
{
  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  std::string currentClass = lookImpl_->styleClass_.toUTF8();
  Utils::SplitSet classes;
  Utils::split(classes, currentClass, " ", true);
  
  if (classes.find(styleClass.toUTF8()) == classes.end()) {
    lookImpl_->styleClass_
      = WT_USTRING::fromUTF8(Utils::addWord(lookImpl_->styleClass_.toUTF8(),
					    styleClass.toUTF8()));

    if (!force) {
      flags_.set(BIT_STYLECLASS_CHANGED);
      repaint(RepaintFlag::SizeAffected);
    }
  }

  if (force && isRendered()) {
    if (!transientImpl_)
      transientImpl_.reset(new TransientImpl());

    Utils::add(transientImpl_->addedStyleClasses_, styleClass);
    Utils::erase(transientImpl_->removedStyleClasses_, styleClass);

    repaint(RepaintFlag::SizeAffected);
  }
}

void WWebWidget::removeStyleClass(const WT_USTRING& styleClass, bool force)
{
  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  if (hasStyleClass(styleClass)) {
    // perhaps it is quicker to join the classes back, but then we need to
    // make sure we keep the original order ?
    lookImpl_->styleClass_
      = WT_USTRING::fromUTF8(Utils::eraseWord(lookImpl_->styleClass_.toUTF8(),
					      styleClass.toUTF8()));
    if (!force) {
      flags_.set(BIT_STYLECLASS_CHANGED);
      repaint(RepaintFlag::SizeAffected);
    }
  }

  if (force && isRendered()) {
    if (!transientImpl_)
      transientImpl_.reset(new TransientImpl());

    Utils::add(transientImpl_->removedStyleClasses_, styleClass);
    Utils::erase(transientImpl_->addedStyleClasses_, styleClass);

    repaint(RepaintFlag::SizeAffected);
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

bool WWebWidget::hasStyleClass(const WT_USTRING& styleClass) const
{
  if (!lookImpl_)
    return false;

  std::string currentClass = lookImpl_->styleClass_.toUTF8();
  Utils::SplitSet classes;
  Utils::split(classes, currentClass, " ", true);

  return classes.find(styleClass.toUTF8()) != classes.end();
}

void WWebWidget::setStyleClass(const WT_USTRING& styleClass)
{
  if (canOptimizeUpdates() && (styleClass == this->styleClass()))
    return;

  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  lookImpl_->styleClass_ = styleClass;

  flags_.set(BIT_STYLECLASS_CHANGED);

  repaint(RepaintFlag::SizeAffected);
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
    otherImpl_.reset(new OtherImpl(this));

  if (!otherImpl_->attributes_)
    otherImpl_->attributes_.reset(new std::map<std::string, WT_USTRING>);

  std::map<std::string, WT_USTRING>::const_iterator i
    = otherImpl_->attributes_->find(name);
  
  if (i != otherImpl_->attributes_->end() && i->second == value)
    return;

  (*otherImpl_->attributes_)[name] = value;

  if (!transientImpl_)
    transientImpl_.reset(new TransientImpl());

  transientImpl_->attributesSet_.push_back(name);

  repaint();
}

WT_USTRING WWebWidget::attributeValue(const std::string& name) const
{
  if (otherImpl_ && otherImpl_->attributes_) {
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
    otherImpl_.reset(new OtherImpl(this));

  if (!otherImpl_->jsMembers_)
    otherImpl_->jsMembers_.reset(new std::vector<OtherImpl::Member>);
  
  std::vector<OtherImpl::Member>& members = *otherImpl_->jsMembers_;
  int index = indexOfJavaScriptMember(name);
  
  if (index != -1 && (members[index].value == value))
    return;

  if (value.empty()) {
    if (index != -1)
      members.erase(members.begin() + index);
    else
      return;
  } else {
    if (index == -1) {
      OtherImpl::Member m;
      m.name = name;
      m.value = value;
      members.push_back(m);
    } else {
      members[index].value = value;
    }
  }

  addJavaScriptStatement(JavaScriptStatementType::SetMember, name);

  repaint();
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
  addJavaScriptStatement(JavaScriptStatementType::CallMethod, 
			 name + "(" + args + ");");

  repaint();
}

void WWebWidget::addJavaScriptStatement(JavaScriptStatementType type,
					const std::string& data)
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  if (!otherImpl_->jsStatements_)
    otherImpl_->jsStatements_.reset
      (new std::vector<OtherImpl::JavaScriptStatement>());

  std::vector<OtherImpl::JavaScriptStatement>& v = *otherImpl_->jsStatements_;

  /*
   * a SetMember is idempotent, if one is already scheduled we do not need
   * to add another statement.
   */
  if (type == JavaScriptStatementType::SetMember) {
    for (unsigned i = 0; i < v.size(); ++i) {
      if (v[i].type == JavaScriptStatementType::SetMember && v[i].data == data)
	return;
    }
  }

  /*
   * If the last statement is exactly the same, then it's a dupe, discard it
   * too.
   */
  if (v.empty() ||
      v.back().type != type ||
      v.back().data != data)
    v.push_back(OtherImpl::JavaScriptStatement(type, data));
}


void WWebWidget::loadToolTip()
{
  if (!lookImpl_->toolTip_)
    lookImpl_->toolTip_.reset(new WString());
  *lookImpl_->toolTip_ = toolTip();

  flags_.set(BIT_TOOLTIP_CHANGED);
  repaint();
}

void WWebWidget::setToolTip(const WString& text, TextFormat textFormat)
{
  flags_.reset(BIT_TOOLTIP_DEFERRED);

  if (canOptimizeUpdates() && text == storedToolTip())
    return;

  if (!lookImpl_)
    lookImpl_.reset(new LookImpl(this));

  if (!lookImpl_->toolTip_)
    lookImpl_->toolTip_.reset(new WString());

  *lookImpl_->toolTip_ = text;
  lookImpl_->toolTipTextFormat_ = textFormat;

  flags_.set(BIT_TOOLTIP_CHANGED);

  repaint();
}

WString WWebWidget::toolTip() const
{
  return storedToolTip();
}

WString WWebWidget::storedToolTip() const
{
  return lookImpl_ && lookImpl_->toolTip_
    ? *lookImpl_->toolTip_
    : WString::Empty;
}

void WWebWidget::setDeferredToolTip(bool enable, TextFormat textFormat)
{
  flags_.set(BIT_TOOLTIP_DEFERRED, enable);

  if (!enable)
    setToolTip("", textFormat);
  else{
    if (!lookImpl_)
      lookImpl_.reset(new LookImpl(this));

    if (!lookImpl_->toolTip_)
      lookImpl_->toolTip_.reset(new WString());
    else
      *lookImpl_->toolTip_ = WString();

    lookImpl_->toolTipTextFormat_ = textFormat;

    flags_.set(BIT_TOOLTIP_CHANGED);

    repaint();
  }
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

void WWebWidget::setHidden(bool hidden, const WAnimation& animation)
{
  if (canOptimizeUpdates() && (animation.empty() && hidden == isHidden()))
    return;

  bool wasVisible = isVisible();

  flags_.set(BIT_HIDDEN, hidden);
  flags_.set(BIT_HIDDEN_CHANGED);

  if (!animation.empty()
      && WApplication::instance()->environment().supportsCss3Animations()
      && WApplication::instance()->environment().ajax()) {
    if (!transientImpl_)
      transientImpl_.reset(new TransientImpl());
    transientImpl_->animation_ = animation;
  }

  bool shouldBeVisible = !hidden;
  if (shouldBeVisible && parent())
    shouldBeVisible = parent()->isVisible();

  if (!canOptimizeUpdates() || shouldBeVisible != wasVisible)
    propagateSetVisible(shouldBeVisible);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, true);

  repaint(RepaintFlag::SizeAffected);
}

void WWebWidget::parentResized(WWidget *parent, WFlags<Orientation> directions)
{
  if (flags_.test(BIT_CONTAINS_LAYOUT)) {
    iterateChildren
      ([=](WWidget *c) {
	if (!c->isHidden())
	  c->webWidget()->parentResized(parent, directions);
      });
  }
}    

void WWebWidget::containsLayout()
{
  if (!flags_.test(BIT_CONTAINS_LAYOUT)) {
    flags_.set(BIT_CONTAINS_LAYOUT);
    WWebWidget *p = parentWebWidget();
    if (p)
      p->containsLayout();
  }
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
      return this == WApplication::instance()->domRoot() ||
	     this == WApplication::instance()->domRoot2();
}

void WWebWidget::setDisabled(bool disabled)
{
  if (canOptimizeUpdates() && (disabled == flags_.test(BIT_DISABLED)))
    return;

  bool wasEnabled = isEnabled();

  flags_.set(BIT_DISABLED, disabled);
  flags_.set(BIT_DISABLED_CHANGED);

  bool shouldBeEnabled = !disabled;
  if (shouldBeEnabled && parent())
    shouldBeEnabled = parent()->isEnabled();

  if (shouldBeEnabled != wasEnabled)
    propagateSetEnabled(shouldBeEnabled);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, true);

  repaint();
}

void WWebWidget::propagateSetEnabled(bool enabled)
{
  iterateChildren
    ([=](WWidget *c) {
      if (!c->isDisabled())
	c->webWidget()->propagateSetEnabled(enabled);
      });
}

void WWebWidget::propagateSetVisible(bool visible)
{
  iterateChildren
    ([=](WWidget *c) {
      if (!c->isHidden())
        c->webWidget()->propagateSetVisible(visible);
      });
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

void WWebWidget::widgetAdded(WWidget *child)
{
  child->setParentWidget(this);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);

  if (!transientImpl_)
    transientImpl_.reset(new TransientImpl());
  ++transientImpl_->addedChildren_;

  emitChildrenChanged();
}

std::vector<WWidget *> WWebWidget::children() const
{
  std::vector<WWidget *> result;

  iterateChildren
    ([&](WWidget *c) {
      result.push_back(c);
    });

  return result;
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

void WWebWidget::setImplementLayoutSizeAware(bool aware)
{
  if (!aware) {
    if (otherImpl_) {
      if (otherImpl_->resized_) {
	otherImpl_->resized_.reset();

	std::string v = javaScriptMember(WT_RESIZE_JS);
	if (v.length() == 1)
	  setJavaScriptMember(WT_RESIZE_JS, std::string());
	else
	  addJavaScriptStatement(JavaScriptStatementType::SetMember, WT_RESIZE_JS);
      }
    }
  }
}

JSignal<int, int>& WWebWidget::resized()
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  if (!otherImpl_->resized_) {
    otherImpl_->resized_.reset(new JSignal<int, int>(this, "resized"));
    otherImpl_->resized_->connect(this, &WWidget::layoutSizeChanged);

    std::string v = javaScriptMember(WT_RESIZE_JS);
    if (v.empty())
      setJavaScriptMember(WT_RESIZE_JS, "0");
    else
      addJavaScriptStatement(JavaScriptStatementType::SetMember, WT_RESIZE_JS);
  }

  return *otherImpl_->resized_;
}

void WWebWidget::updateDom(DomElement& element, bool all)
{
  WApplication *app = nullptr;

  /*
   * determine display
   */

  if (flags_.test(BIT_GEOMETRY_CHANGED) || 
      flags_.test(BIT_FLEX_BOX_CHANGED) ||
      (!flags_.test(BIT_HIDE_WITH_VISIBILITY) &&
        flags_.test(BIT_HIDDEN_CHANGED)) ||
      all) {
    if (flags_.test(BIT_HIDE_WITH_VISIBILITY) || !flags_.test(BIT_HIDDEN)) {
      const char *Inline = "inline";
      const char *InlineTable = "inline-table";
      const char *InlineBlock = "inline-block";
      const char *Block = "block";
      const char *Flex = "flex";
      const char *FlexInline = "flex-inline";
      const char *Empty = "";
      const char *display = nullptr;

      const bool defaultInline = element.type() == DomElementType::OTHER ? DomElement::isDefaultInline(domElementType()) : element.isDefaultInline();
      if (defaultInline != flags_.test(BIT_INLINE)) {
	if (flags_.test(BIT_INLINE)) {
	  if (element.type() == DomElementType::TABLE)
	    display = InlineTable;
	  if (element.type() == DomElementType::LI)
	    display = Inline;
	  else if (element.type() != DomElementType::TD) {
	    if (!app) app = WApplication::instance();
	    if (app->environment().agentIsIElt(9)) {
	      display = Inline;
	      element.setProperty(Property::StyleZoom, "1");
	    } else
	      display = InlineBlock;
	  }
	} else {
	  display = Block;
	}
      } else if (!all && flags_.test(BIT_HIDDEN_CHANGED)) {
	if (defaultInline == flags_.test(BIT_INLINE))
	  display = Empty;
	else
	  display = flags_.test(BIT_INLINE) ? Inline : Block;
      }

      if (flags_.test(BIT_FLEX_BOX))
	display = flags_.test(BIT_INLINE) ? FlexInline : Flex;
      else if (flags_.test(BIT_FLEX_BOX_CHANGED) && !display)
        display = Empty;

      if (display)
	element.setProperty(Property::StyleDisplay, display);
    } else
      element.setProperty(Property::StyleDisplay, "none");
  }

  if (flags_.test(BIT_ZINDEX_CHANGED) || all) {
    if (layoutImpl_) {
      /*
       * set z-index
       */
      if (layoutImpl_->zIndex_ > 0) {
	element.setProperty(Property::StyleZIndex,
			    std::to_string(layoutImpl_->zIndex_));
	element.addPropertyWord(Property::Class, "Wt-popup");
	if (!all &&
	    !flags_.test(BIT_STYLECLASS_CHANGED) &&
	    lookImpl_ && !lookImpl_->styleClass_.empty())
	  element.addPropertyWord(Property::Class,
				  lookImpl_->styleClass_.toUTF8());

	if (!app) app = WApplication::instance();
	if (all && app->environment().agent() == UserAgent::IE6
	    && element.type() == DomElementType::DIV) {
	  DomElement *i = DomElement::createNew(DomElementType::IFRAME);
	  i->setId("sh" + id());
	  i->setProperty(Property::Class, "Wt-shim");
	  i->setProperty(Property::Src, "javascript:false;");
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
    }

    flags_.reset(BIT_ZINDEX_CHANGED);
  }

  if (flags_.test(BIT_GEOMETRY_CHANGED) || all) {
    if (layoutImpl_) {
      /*
       * set position
       */
      if (!(flags_.test(BIT_HIDE_WITH_VISIBILITY) && flags_.test(BIT_HIDDEN)))
	switch (layoutImpl_->positionScheme_) {
	case PositionScheme::Static:
	  break;
	case PositionScheme::Relative:
	  element.setProperty(Property::StylePosition, "relative"); break;
	case PositionScheme::Absolute:
	  element.setProperty(Property::StylePosition, "absolute"); break;
	case PositionScheme::Fixed:
	  element.setProperty(Property::StylePosition, "fixed"); break;
	}

      /*
       * set clear: FIXME: multiple values
       */
      if (layoutImpl_->clearSides_ == Side::Left) {
	element.setProperty(Property::StyleClear, "left");
      } else if (layoutImpl_->clearSides_ == Side::Right) {
	element.setProperty(Property::StyleClear, "right");
      } else if (layoutImpl_->clearSides_ == (Side::Left | Side::Right)) {
	element.setProperty(Property::StyleClear, "both");
      }

      if (layoutImpl_->minimumWidth_.value() != 0) {
	std::string text
	  = layoutImpl_->minimumWidth_.isAuto() ? "0px"
	  : layoutImpl_->minimumWidth_.cssText();
	element.setProperty(Property::StyleMinWidth, text);
      }
      if (layoutImpl_->minimumHeight_.value() != 0) {
	std::string text
	  = layoutImpl_->minimumHeight_.isAuto() ? "0px"
	  : layoutImpl_->minimumHeight_.cssText();
	element.setProperty(Property::StyleMinHeight, text);
      }
      if (!layoutImpl_->maximumWidth_.isAuto())
	element.setProperty(Property::StyleMaxWidth,
			    layoutImpl_->maximumWidth_.cssText());
      if (!layoutImpl_->maximumHeight_.isAuto())
	element.setProperty(Property::StyleMaxHeight,
			    layoutImpl_->maximumHeight_.cssText());

      /*
       * set offsets
       */
      if (layoutImpl_->positionScheme_ != PositionScheme::Static) {
	static const Property properties[] = { Property::StyleTop,
					       Property::StyleRight,
					       Property::StyleBottom,
					       Property::StyleLeft };

	if (!layoutImpl_->offsets_[0].isAuto()
	    || !layoutImpl_->offsets_[1].isAuto()
	    || !layoutImpl_->offsets_[2].isAuto()
	    || !layoutImpl_->offsets_[3].isAuto()) {
	  for (unsigned i = 0; i < 4; ++i) {
	    Property property = properties[i];

	    if (!app) app = WApplication::instance();

	    if (app->layoutDirection() == LayoutDirection::RightToLeft) {
	      if (i == 1) property = properties[3];
	      else if (i == 3) property = properties[1];
	    }

	    if ((app->environment().ajax()
		 && !app->environment().agentIsIElt(9))
		|| !layoutImpl_->offsets_[i].isAuto())
	      element.setProperty(property, layoutImpl_->offsets_[i].cssText());
	  }
	}
      }

      /*
       * set vertical alignment
       */
      switch (layoutImpl_->verticalAlignment_) {
      case AlignmentFlag::Baseline:
	break;
      case AlignmentFlag::Sub:
	element.setProperty(Property::StyleVerticalAlign, "sub"); break;
      case AlignmentFlag::Super:
	element.setProperty(Property::StyleVerticalAlign, "super"); break;
      case AlignmentFlag::Top:
	element.setProperty(Property::StyleVerticalAlign, "top"); break;
      case AlignmentFlag::TextTop:
	element.setProperty(Property::StyleVerticalAlign, "text-top"); break;
      case AlignmentFlag::Middle:
	element.setProperty(Property::StyleVerticalAlign, "middle"); break;
      case AlignmentFlag::Bottom:
	element.setProperty(Property::StyleVerticalAlign, "bottom"); break;
      case AlignmentFlag::TextBottom:
	element.setProperty(Property::StyleVerticalAlign, "text-bottom"); break;
      default:
	break;
      }

      if (!layoutImpl_->lineHeight_.isAuto()) // == none
	element.setProperty(Property::StyleLineHeight,
			    layoutImpl_->lineHeight_.cssText());

    }

    flags_.reset(BIT_GEOMETRY_CHANGED);
  }


  /*
   * set width & height
   */
  if (width_ && (flags_.test(BIT_WIDTH_CHANGED) || all)) {
    if (!all || !width_->isAuto())
      element.setProperty(Property::StyleWidth, width_->cssText());
    flags_.reset(BIT_WIDTH_CHANGED);
  }

  if (height_ && (flags_.test(BIT_HEIGHT_CHANGED) || all)) {
    if (!all || !height_->isAuto())
      element.setProperty(Property::StyleHeight, height_->cssText());
    flags_.reset(BIT_HEIGHT_CHANGED);
  }

  if (flags_.test(BIT_FLOAT_SIDE_CHANGED) || all) {
    if (layoutImpl_) {
      if (layoutImpl_->floatSide_ == static_cast<Side>(0)) { 
	if (flags_.test(BIT_FLOAT_SIDE_CHANGED))
	  element.setProperty(Property::StyleFloat, "none");
      }
      else {
        /*
        * set float
        */
	if (!app) app = WApplication::instance();

	bool ltr = app->layoutDirection() == LayoutDirection::LeftToRight;
        switch (layoutImpl_->floatSide_) {
        case Side::Left:
	  element.setProperty(Property::StyleFloat, ltr ? "left" : "right");
	  break;
        case Side::Right:
	  element.setProperty(Property::StyleFloat, ltr ? "right" : "left");
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
    bool changed = flags_.test(BIT_MARGINS_CHANGED);
    if (changed || all) {
      if (changed || (layoutImpl_->margin_[0].value() != 0))
	element.setProperty(Property::StyleMarginTop,
			    layoutImpl_->margin_[0].cssText());
      if (changed || (layoutImpl_->margin_[1].value() != 0))
	element.setProperty(Property::StyleMarginRight,
			    layoutImpl_->margin_[1].cssText());
      if (changed || (layoutImpl_->margin_[2].value() != 0))
	element.setProperty(Property::StyleMarginBottom,
			    layoutImpl_->margin_[2].cssText());
      if (changed || (layoutImpl_->margin_[3].value() != 0))
	element.setProperty(Property::StyleMarginLeft,
			    layoutImpl_->margin_[3].cssText());

      flags_.reset(BIT_MARGINS_CHANGED);
    }
  }

  if (lookImpl_) {
    if ((lookImpl_->toolTip_ || flags_.test(BIT_TOOLTIP_DEFERRED))
        && (flags_.test(BIT_TOOLTIP_CHANGED) || all)) {
      if (!all || (!lookImpl_->toolTip_->empty() ||
                   flags_.test(BIT_TOOLTIP_DEFERRED))) {
        if (!app) app = WApplication::instance();
        if ( (lookImpl_->toolTipTextFormat_ != TextFormat::Plain
              || flags_.test(BIT_TOOLTIP_DEFERRED))
            && app->environment().ajax()) {
          LOAD_JAVASCRIPT(app, "js/ToolTip.js", "toolTip", wtjs10);

	  WString tooltipText = *lookImpl_->toolTip_;
          if (lookImpl_->toolTipTextFormat_ == TextFormat::Plain) {
            tooltipText = escapeText(*lookImpl_->toolTip_);
          } else if (lookImpl_->toolTipTextFormat_ == TextFormat::XHTML) {
	    bool res = removeScript(tooltipText);
	    if (!res) {
	      tooltipText = escapeText(*lookImpl_->toolTip_);
	    }
	  }

          std::string deferred = flags_.test(BIT_TOOLTIP_DEFERRED) ?
                "true" : "false";
          element.callJavaScript(WT_CLASS ".toolTip(" +
                                 app->javaScriptClass() + ","
                                 + jsStringLiteral(id()) + ","
                                 + tooltipText.jsStringLiteral()
                                 + ", " + deferred
                                 + ", " +
                                 jsStringLiteral(app->theme()->
                                                 utilityCssClass(ToolTipInner))
                                 + ", " +
                                 jsStringLiteral(app->theme()->
                                                 utilityCssClass(ToolTipOuter))
                                 + ");");

          if (flags_.test(BIT_TOOLTIP_DEFERRED) &&
              !lookImpl_->loadToolTip_.isConnected())
            lookImpl_->loadToolTip_.connect(this, &WWebWidget::loadToolTip);

          element.removeAttribute("title");
        } else
          element.setAttribute("title", lookImpl_->toolTip_->toUTF8());
      }

      flags_.reset(BIT_TOOLTIP_CHANGED);
    }

    if (lookImpl_->decorationStyle_)
      lookImpl_->decorationStyle_->updateDomElement(element, all);

    if (all || flags_.test(BIT_STYLECLASS_CHANGED))
      if (!all || !lookImpl_->styleClass_.empty())
	element.addPropertyWord(Property::Class, lookImpl_->styleClass_.toUTF8());

    flags_.reset(BIT_STYLECLASS_CHANGED);
  }

  if (!all && transientImpl_) {
    for (unsigned i = 0; i < transientImpl_->addedStyleClasses_.size(); ++i)
      element.callJavaScript("$('#" + id() + "').addClass('"
			     + transientImpl_->addedStyleClasses_[i].toUTF8()
			     +"');");

    for (unsigned i = 0; i < transientImpl_->removedStyleClasses_.size(); ++i)
      element.callJavaScript("$('#" + id() + "').removeClass('"
			     + transientImpl_->removedStyleClasses_[i].toUTF8()
			     +"');");

    if (!transientImpl_->childRemoveChanges_.empty()) {
      if ((int)children().size() != transientImpl_->addedChildren_
	  || transientImpl_->specialChildRemove_) {
	for (unsigned i = 0; i < transientImpl_->childRemoveChanges_.size();
	     ++i) {
	  const std::string& js = transientImpl_->childRemoveChanges_[i];
	  if (js[0] == '_')
	    element.callJavaScript(WT_CLASS ".remove('" + js.substr(1) + "');",
				   true);
	  else
	    element.callJavaScript(js, true);
	}
      } else
	element.removeAllChildren();

      transientImpl_->addedChildren_ = 0;
      transientImpl_->childRemoveChanges_.clear();
      transientImpl_->specialChildRemove_ = false;
    }
  }

  if (all || flags_.test(BIT_SELECTABLE_CHANGED)) {
    if (flags_.test(BIT_SET_UNSELECTABLE)) {
      element.addPropertyWord(Property::Class, "unselectable");
      element.setAttribute("unselectable", "on");
      element.setAttribute("onselectstart", "return false;");
    } else if (flags_.test(BIT_SET_SELECTABLE)) {
      element.addPropertyWord(Property::Class, "selectable");
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
	    element.setProperty(Property::Style, i->second.toUTF8());
	  else
	    element.setAttribute(i->first, i->second.toUTF8());
      } else if (transientImpl_) {
	for (unsigned i = 0; i < transientImpl_->attributesSet_.size(); ++i) {
	  std::string attr = transientImpl_->attributesSet_[i];
	  if (attr == "style")
	    element.setProperty(Property::Style,
				(*otherImpl_->attributes_)[attr].toUTF8());
	  else
	    element.setAttribute(attr,
				 (*otherImpl_->attributes_)[attr].toUTF8());
	}
      }
    }

    if (all && otherImpl_->jsMembers_) {
      for (unsigned i = 0; i < otherImpl_->jsMembers_->size(); i++) {
	OtherImpl::Member member = (*otherImpl_->jsMembers_)[i];

	bool notHere = false;
	if (otherImpl_->jsStatements_) {
	  for (unsigned j = 0; j < otherImpl_->jsStatements_->size(); ++j) {
	    const OtherImpl::JavaScriptStatement& jss 
	      = (*otherImpl_->jsStatements_)[j];

	    if (jss.type == JavaScriptStatementType::SetMember && 
		jss.data == member.name) {
	      notHere = true;
	      break;
	    } 
	  }
	}

	if (notHere)
	  continue;

	declareJavaScriptMember(element, member.name, member.value);
      }
    }

    if (otherImpl_->jsStatements_) {
      for (unsigned i = 0; i < otherImpl_->jsStatements_->size(); ++i) {
	const OtherImpl::JavaScriptStatement& jss 
	  = (*otherImpl_->jsStatements_)[i];

	switch (jss.type) {
	case JavaScriptStatementType::SetMember:
	  declareJavaScriptMember(element, jss.data,
				  javaScriptMember(jss.data));
	  break;
	case JavaScriptStatementType::CallMethod:
	  element.callMethod(jss.data);
	  break;
	case JavaScriptStatementType::Statement:
	  element.callJavaScript(jss.data);
	  break;
	}
      }

      otherImpl_->jsStatements_.reset();
    }
  }

  if (flags_.test(BIT_HIDE_WITH_VISIBILITY)) {
    if (flags_.test(BIT_HIDDEN_CHANGED)
	|| (all && flags_.test(BIT_HIDDEN))) {
      if (flags_.test(BIT_HIDDEN)) {
	element.callJavaScript("$('#" + id() + "').addClass('Wt-hidden');");
	element.setProperty(Property::StyleVisibility, "hidden");
	if (flags_.test(BIT_HIDE_WITH_OFFSETS)) {
	  element.setProperty(Property::StylePosition, "absolute");
	  element.setProperty(Property::StyleTop, "-10000px");
	  element.setProperty(Property::StyleLeft, "-10000px");
	}
      } else {
	if (flags_.test(BIT_HIDE_WITH_OFFSETS)) {
	  if (layoutImpl_) {
	    switch (layoutImpl_->positionScheme_) {
	    case PositionScheme::Static:
	      element.setProperty(Property::StylePosition, "static"); break;
	    case PositionScheme::Relative:
	      element.setProperty(Property::StylePosition, "relative"); break;
	    case PositionScheme::Absolute:
	      element.setProperty(Property::StylePosition, "absolute"); break;
	    case PositionScheme::Fixed:
	      element.setProperty(Property::StylePosition, "fixed"); break;
	    }

	    if (!layoutImpl_->offsets_[0].isAuto())
	      element.setProperty(Property::StyleTop,
				  layoutImpl_->offsets_[0].cssText());
	    else
	      element.setProperty(Property::StyleTop, "");

	    if (!layoutImpl_->offsets_[3].isAuto())
	      element.setProperty(Property::StyleLeft,
				  layoutImpl_->offsets_[3].cssText());
	    else
	      element.setProperty(Property::StyleTop, "");
	  } else {
	    element.setProperty(Property::StylePosition, "static");
	    element.setProperty(Property::StyleTop, "");
	    element.setProperty(Property::StyleLeft, "");
	  }
	}
	element.callJavaScript("$('#" + id() + "').removeClass('Wt-hidden');");
	element.setProperty(Property::StyleVisibility, "visible");
      }
    }
  }

  if ((!all && flags_.test(BIT_HIDDEN_CHANGED))
      || (all && !flags_.test(BIT_HIDDEN))) {
    if (transientImpl_ && !transientImpl_->animation_.empty()) {
      const char *THIS_JS = "js/WWebWidget.js";

      if (!app) app = WApplication::instance();

      LOAD_JAVASCRIPT(app, THIS_JS, "animateDisplay", wtjs1);
      LOAD_JAVASCRIPT(app, THIS_JS, "animateVisible", wtjs2);

      if (!flags_.test(BIT_HIDE_WITH_VISIBILITY)) {
	/*
	 * Not using visibility: delay changing the display property
	 */
	WStringStream ss;
	ss << WT_CLASS << ".animateDisplay("
	   << app->javaScriptClass()
	   << ",'" << id()
	   << "'," << transientImpl_->animation_.effects().value()
	   << "," << (int)transientImpl_->animation_.timingFunction()
	   << "," << transientImpl_->animation_.duration()
	   << ",'" << element.getProperty(Property::StyleDisplay)
	   << "');";
	element.callJavaScript(ss.str());

	if (all)
	  element.setProperty(Property::StyleDisplay, "none");
	else
	  element.removeProperty(Property::StyleDisplay);
      } else {
	/*
	 * Using visibility: remember how to show/hide the widget
	 */
	WStringStream ss;
	ss << WT_CLASS << ".animateVisible('" << id()
	   << "'," << transientImpl_->animation_.effects().value()
	   << "," << (int)transientImpl_->animation_.timingFunction()
	   << "," << transientImpl_->animation_.duration()
	   << ",'" << element.getProperty(Property::StyleVisibility)
	   << "','" << element.getProperty(Property::StylePosition)
	   << "','" << element.getProperty(Property::StyleTop)
	   << "','" << element.getProperty(Property::StyleLeft)
	   << "');";
	element.callJavaScript(ss.str());

	if (all) {
	  element.setProperty(Property::StyleVisibility, "hidden");
	  element.setProperty(Property::StylePosition, "absolute");
	  element.setProperty(Property::StyleTop, "-10000px");
	  element.setProperty(Property::StyleLeft, "-10000px");
	} else {
	  element.removeProperty(Property::StyleVisibility);
	  element.removeProperty(Property::StylePosition);
	  element.removeProperty(Property::StyleTop);
	  element.removeProperty(Property::StyleLeft);
	}
      }
    }
  }

  flags_.reset(BIT_HIDDEN_CHANGED);

  if (flags_.test(BIT_GOT_FOCUS)) {
    if (!app) app = WApplication::instance();
    const WEnvironment& env = app->environment();

    element.callJavaScript("setTimeout(function() {"
			   """var o = " + jsRef() + ";"
			   """if (o) {"
			   ""   "if (!$(o).hasClass('" +
			         app->theme()->disabledClass() + "')) {"
			   ""      "try { "
			   ""          "o.focus();"
			   ""      "} catch (e) {}"
			   ""   "}"
			   """}"
			   "}, " + (env.agentIsIElt(9) ? "500" : "10") + ");");

    flags_.reset(BIT_GOT_FOCUS);
  }

  if (flags_.test(BIT_TABINDEX_CHANGED) || all) {
    if (otherImpl_ && otherImpl_->tabIndex_ != std::numeric_limits<int>::min())
      element.setProperty(Property::TabIndex,
			  std::to_string(otherImpl_->tabIndex_));
    else if (!all)
      element.removeAttribute("tabindex");

    flags_.reset(BIT_TABINDEX_CHANGED);
  }

  if (all || flags_.test(BIT_SCROLL_VISIBILITY_CHANGED)) {
    const char *SCROLL_JS = "js/ScrollVisibility.js";

    if (!app) app = WApplication::instance();

    if (!app->javaScriptLoaded(SCROLL_JS) && scrollVisibilityEnabled()) {
      LOAD_JAVASCRIPT(app, SCROLL_JS, "ScrollVisibility", wtjs3);
      WStringStream ss;
      ss << "if (!" WT_CLASS ".scrollVisibility) {"
	    WT_CLASS ".scrollVisibility = new ";
      ss << WT_CLASS ".ScrollVisibility(" << app->javaScriptClass() + "); }";
      element.callJavaScript(ss.str());
    }

    if (scrollVisibilityEnabled()) {
      WStringStream ss;
      ss << WT_CLASS ".scrollVisibility.add({";
      ss << "el:" << jsRef() << ',';
      ss << "margin:" << scrollVisibilityMargin() << ',';
      ss << "visible:" << isScrollVisible();
      ss << "});";
      element.callJavaScript(ss.str());
      flags_.set(BIT_SCROLL_VISIBILITY_LOADED);
    } else if (flags_.test(BIT_SCROLL_VISIBILITY_LOADED)) {
      element.callJavaScript(WT_CLASS ".scrollVisibility.remove("
			     + jsStringLiteral(id()) + ");");
      flags_.reset(BIT_SCROLL_VISIBILITY_LOADED);
    }
    flags_.reset(BIT_SCROLL_VISIBILITY_CHANGED);
  }

  if (all || flags_.test(BIT_OBJECT_NAME_CHANGED)) {
    if (!objectName().empty()) {
      element.setAttribute("data-object-name", objectName());
    } else if (!all) {
      element.removeAttribute("data-object-name");
    }
    flags_.reset(BIT_OBJECT_NAME_CHANGED);
  }
  
  renderOk();

  transientImpl_.reset();
}

void WWebWidget::declareJavaScriptMember(DomElement& element,
					 const std::string& name,
					 const std::string& value)
{
  if (name[0] != ' ') {
    if (name == WT_RESIZE_JS && otherImpl_->resized_) {
      WStringStream combined;
      if (value.length() > 1) {
	combined << name << "=function(s,w,h) {"
		 << WApplication::instance()->javaScriptClass()
		 << "._p_.propagateSize(s,w,h);"
		 << "(" << value << ")(s,w,h);"
		 << "}";
      } else
	combined << name << "="
		 << WApplication::instance()->javaScriptClass()
		 << "._p_.propagateSize";

      element.callMethod(combined.str());
    } else {
      if (value.length() > 0)
	element.callMethod(name + "=" + value);
      else
	element.callMethod(name + "=null");
    }
  } else
    element.callJavaScript(value);
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

void WWebWidget::setCanReceiveFocus(bool enabled)
{
  setTabIndex(enabled ? 0 : std::numeric_limits<int>::min());
}

bool WWebWidget::canReceiveFocus() const
{
  if (otherImpl_)
    return otherImpl_->tabIndex_ != std::numeric_limits<int>::min();
  else
    return false;
}

void WWebWidget::setTabIndex(int index)
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));
    
  otherImpl_->tabIndex_ = index;

  flags_.set(BIT_TABINDEX_CHANGED);
  repaint();
}

int WWebWidget::tabIndex() const
{
  if (!otherImpl_)
    return canReceiveFocus() ? 0 : std::numeric_limits<int>::min();
  else
    return otherImpl_->tabIndex_;
}

bool WWebWidget::setFirstFocus()
{
  if (isVisible() && isEnabled()) {
    if (canReceiveFocus()) {
      setFocus(true);
      return true;
    }

    bool result[1];
    result[0] = false;
    iterateChildren([&](Wt::WWidget *w){
      if (!result[0])
        result[0] = w->setFirstFocus();
    });
    if (result[0])
      return true;

    return false;
  } else
    return false;
}

EventSignal<>& WWebWidget::blurred()
{
  return *voidEventSignal(BLUR_SIGNAL, true);
}

EventSignal<>& WWebWidget::focussed()
{
  return *voidEventSignal(FOCUS_SIGNAL, true);
}

void WWebWidget::setFocus(bool focus)
{
  flags_.set(BIT_GOT_FOCUS, focus);
  repaint();

  WApplication *app = WApplication::instance();
  if (focus)
    app->setFocus(id(), -1, -1);
  else if (app->focus() == id())
    app->setFocus(std::string(), -1, -1);
}

void WWebWidget::undoSetFocus()
{ }

bool WWebWidget::hasFocus() const
{
  return WApplication::instance()->focus() == id();
}

void WWebWidget::getSFormObjects(FormObjectsMap& result)
{
  if (!flags_.test(BIT_STUBBED) && !flags_.test(BIT_HIDDEN)
      && flags_.test(BIT_RENDERED))
    getFormObjects(result);
}

void WWebWidget::getFormObjects(FormObjectsMap& formObjects)
{
  if (flags_.test(BIT_FORM_OBJECT))
    formObjects[id()] = this;

  iterateChildren
    ([&](WWidget *c) {
      c->webWidget()->getSFormObjects(formObjects);
    });
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
      scheduleRerender(true);
    } else {
      if (!app->session()->renderer().visibleOnly()) {
	flags_.reset(BIT_STUBBED);

	DomElement *stub = DomElement::getForUpdate(this, DomElementType::SPAN);
	WWidget *self = selfWidget();
	setRendered(true);
	self->render(RenderFlag::Full);
	DomElement *realElement = createDomElement(app);
	app->theme()->apply(self, *realElement, 0);
	stub->unstubWith(realElement, !flags_.test(BIT_HIDE_WITH_OFFSETS));
	result.push_back(stub);
      }
    }
  } else {
    render(RenderFlag::Update);

    getDomChanges(result, app);
  }
}

void WWebWidget::doneRerender()
{
  iterateChildren
    ([](WWidget *c) {
      c->webWidget()->doneRerender();
    });
}

void WWebWidget::propagateRenderOk(bool deep)
{
#ifndef WT_TARGET_JAVA
  flags_ &= ~AllChangeFlags;
#else
  flags_.reset(BIT_FLEX_BOX_CHANGED);
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
  flags_.reset(BIT_ZINDEX_CHANGED);
  flags_.reset(BIT_TABINDEX_CHANGED);
  flags_.reset(BIT_SCROLL_VISIBILITY_CHANGED);
  flags_.reset(BIT_OBJECT_NAME_CHANGED);
#endif

  renderOk();

  if (deep)
    iterateChildren
      ([](WWidget *c) {
	c->webWidget()->propagateRenderOk();
      });

  transientImpl_.reset();
}

void WWebWidget::setRendered(bool rendered)
{
  if (rendered)
    flags_.set(BIT_RENDERED);
  else {
    flags_.reset(BIT_RENDERED);

    renderOk();

    iterateChildren
      ([](WWidget *c) {
	c->webWidget()->setRendered(false);
      });
  }
}

void WWebWidget::setLoadLaterWhenInvisible(bool how)
{
  flags_.set(BIT_DONOT_STUB, !how);
}

void WWebWidget::setHtmlTagName(const std::string& tag)
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  if (!otherImpl_->elementTagName_)
    otherImpl_->elementTagName_.reset(new std::string());

  *otherImpl_->elementTagName_ = tag;
}

std::string WWebWidget::htmlTagName() const
{
  if (otherImpl_ && otherImpl_->elementTagName_)
    return *otherImpl_->elementTagName_;
  else {
    DomElementType type = domElementType();
    return DomElement::tagName(type);
  }
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
    WWidget *result[1];
    result[0] = nullptr;
    iterateChildren
      ([&](WWidget *c) {
	if (!result[0])
	  result[0] = c->find(name);
      });

    return result[0];
  }
}

WWidget *WWebWidget::findById(const std::string& id)
{
  if (this->id() == id)
    return this;
  else {
    WWidget *result[1];
    result[0] = nullptr;
    iterateChildren
      ([&](WWidget *c) {
	if (!result[0])
	  result[0] = c->findById(id);
      });
    if (result[0])
      return result[0];
  }

  return nullptr;
}

DomElement *WWebWidget::createDomElement(WApplication *app)
{
  setRendered(true);
  DomElement *result;

  if (otherImpl_ && otherImpl_->elementTagName_) {
    result = DomElement::createNew(DomElementType::OTHER);
    result->setDomElementTagName(*otherImpl_->elementTagName_);
  } else
    result = DomElement::createNew(domElementType());

  setId(result, app);
  updateDom(*result, true);

  return result;
}

bool WWebWidget::domCanBeSaved() const
{
  bool canBeSaved[1];
  canBeSaved[0] = true;
  iterateChildren([&](WWidget *child){
    canBeSaved[0] = canBeSaved[0] && child->webWidget()->domCanBeSaved();
  });
  return canBeSaved[0];
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

  DomElement *stub = DomElement::createNew(DomElementType::SPAN);
  if (!flags_.test(BIT_HIDE_WITH_OFFSETS)) {
    stub->setProperty(Property::StyleDisplay, "none");
  } else {
    stub->setProperty(Property::StylePosition, "absolute");
    stub->setProperty(Property::StyleLeft, "-10000px");
    stub->setProperty(Property::StyleTop, "-10000px");
    stub->setProperty(Property::StyleVisibility, "hidden");
  }
  if (app->environment().javaScript())
    stub->setProperty(Property::InnerHTML, "...");

  if (!app->environment().agentIsSpiderBot()
      || (otherImpl_ && otherImpl_->id_))
    stub->setId(id());

  return stub;
}

DomElement *WWebWidget::createActualElement(WWidget *self, WApplication *app)
{
  flags_.reset(BIT_STUBBED);

  DomElement *result = createDomElement(app);

  app->theme()->apply(self, *result, MainElement);

  /* Make sure addStyleClass() does not mess up later */
  std::string styleClass = result->getProperty(Property::Class);
  if (!styleClass.empty()) {
    if (!lookImpl_)
      lookImpl_.reset(new LookImpl(this));

    lookImpl_->styleClass_ = styleClass;
  }

  return result;
}

void WWebWidget::refresh()
{
  if (lookImpl_ && lookImpl_->toolTip_)
    if (lookImpl_->toolTip_->refresh()) {
      flags_.set(BIT_TOOLTIP_CHANGED);
      repaint();
    }

  iterateChildren
    ([](WWidget *c) {
      c->refresh();
    });

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
      EventSignalBase& s = **i;
      if (s.name() == WInteractWidget::M_CLICK_SIGNAL)
	repaint(RepaintFlag::ToAjax);

      s.ownerRepaint();
    }
  }

  if (flags_.test(BIT_TOOLTIP_DEFERRED) || 
      (lookImpl_ && lookImpl_->toolTipTextFormat_ != TextFormat::Plain)) {
    flags_.set(BIT_TOOLTIP_CHANGED);
    repaint();
  }

  iterateChildren
    ([](WWidget *c) {
      c->enableAjax();
    });
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
    sout.pushEscape(EscapeOStream::Plain);

  Wt::Utils::sanitizeUnicode(sout, text);

#ifndef WT_TARGET_JAVA
  text = sout.str();
  return text;
#else
  return sout.str();
#endif // WT_TARGET_JAVA
}

std::string& WWebWidget::unescapeText(std::string &text)
{
#ifndef WT_TARGET_JAVA
  char *inP = &text[0];
  char *const inEndP = &text[text.size()];
  char *outP = &text[0];
  char *ampP = nullptr;
  do {
    assert(inP >= outP);
    ampP = std::find(inP, inEndP, '&');
    if (inP == outP) {
      inP = ampP;
      outP = ampP;
    } else {
      while (inP != ampP) {
        *(outP++) = *(inP++);
      }
    }
    assert(inP >= outP);
    if (inP != inEndP) {
      char *semiP = std::find(inP, inEndP, ';');
      if (semiP == inEndP)
        *(outP++) = *(inP++);
      else {
        if (*(ampP + 1) == '#') {
          unsigned long codept = 0;
          bool valid = false;
          if (*(ampP + 2) == 'x') {
            char *end = nullptr;
            codept = std::strtoul(ampP + 3, &end, 16);
            valid = end == semiP;
          } else {
            char *end = nullptr;
            codept = std::strtoul(ampP + 2, &end, 10);
            valid = end == semiP;
          }
          if (valid) {
            Wt::rapidxml::xml_document<>::insert_coded_character<0>(outP, codept);
            inP = semiP + 1;
          } else {
            *(outP++) = *(inP++);
          }
        } else {
          if (!rapidxml::translate_xhtml_entity(inP, outP)) {
            *(outP++) = *(inP++);
          }
        }
      }
    }
  } while (inP < inEndP);
  assert(inP >= outP);
  *outP = '\0';
  std::size_t s = (std::size_t)(outP - (&text[0]));
  assert(s <= text.size());
  text.resize(s);
  return text;
#endif // WT_TARGET_JAVA
  return text;
}

std::string WWebWidget::jsStringLiteral(const std::string& value,
					char delimiter)
{
  WStringStream result;
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

  iterateChildren
    ([&](WWidget *c) {
      doLoad(c);
    });

  if (flags_.test(BIT_HIDE_WITH_OFFSETS))
    parent()->setHideWithOffsets(true);
}

void WWebWidget::doLoad(WWidget *w)
{
  w->load();
  if (!w->loaded())
    LOG_ERROR("improper load() implementation: base implementation not called");
}

void WWebWidget::render(WFlags<RenderFlag> flags)
{
  WWidget::render(flags);
}

void WWebWidget::doJavaScript(const std::string& javascript)
{
  addJavaScriptStatement(JavaScriptStatementType::Statement, javascript);
  repaint();
}

bool WWebWidget::loaded() const
{
  return flags_.test(BIT_LOADED);
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
    otherImpl_.reset(new OtherImpl(this));
  if (!otherImpl_->acceptedDropMimeTypes_)
    otherImpl_->acceptedDropMimeTypes_.reset(new OtherImpl::MimeTypesMap);

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
      .reset(new JSignal<std::string,std::string, WMouseEvent>(this, "_drop"));

  if (result && !otherImpl_->dropSignal2_)
    otherImpl_->dropSignal2_
      .reset(new JSignal<std::string,std::string, WTouchEvent>(this, "_drop2"));

  return result;
}

EventSignal<WKeyEvent> *WWebWidget::keyEventSignal(const char *name,
						   bool create)
{
  EventSignalBase *b = getEventSignal(name);
  if (b)
    return static_cast<EventSignal<WKeyEvent> *>(b);
  else if (!create)
    return nullptr;
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
    return nullptr;
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
    return nullptr;
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
    return nullptr;
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
    return nullptr;
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
    return nullptr;
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

std::string WWebWidget::resolveRelativeUrl(const std::string& url)
{
  return WApplication::instance()->resolveRelativeUrl(url);
}

bool WWebWidget::removeScript(WString& text)
{
#ifndef WT_NO_XSS_FILTER
  return XSSFilterRemoveScript(text);
#else
  return true;
#endif
}

bool WWebWidget::scrollVisibilityEnabled() const
{
  return flags_.test(BIT_SCROLL_VISIBILITY_ENABLED);
}

void WWebWidget::setScrollVisibilityEnabled(bool enabled)
{
  if (enabled && !otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  if (scrollVisibilityEnabled() != enabled) {
    flags_.set(BIT_SCROLL_VISIBILITY_ENABLED, enabled);
    flags_.set(BIT_SCROLL_VISIBILITY_CHANGED);
    repaint();
  }
}

int WWebWidget::scrollVisibilityMargin() const
{
  if (!otherImpl_)
    return 0;
  else
    return otherImpl_->scrollVisibilityMargin_;
}

void WWebWidget::setScrollVisibilityMargin(int margin)
{
  if (scrollVisibilityMargin() != margin) {
    if (!otherImpl_)
      otherImpl_.reset(new OtherImpl(this));
    otherImpl_->scrollVisibilityMargin_ = margin;
    if (scrollVisibilityEnabled()) {
      flags_.set(BIT_SCROLL_VISIBILITY_CHANGED);
      repaint();
    }
  }
}

Signal<bool> &WWebWidget::scrollVisibilityChanged()
{
  if (!otherImpl_)
    otherImpl_.reset(new OtherImpl(this));

  return otherImpl_->scrollVisibilityChanged_;
}

bool WWebWidget::isScrollVisible() const
{
  return flags_.test(BIT_IS_SCROLL_VISIBLE);
}

void WWebWidget::jsScrollVisibilityChanged(bool visible)
{
  flags_.set(BIT_IS_SCROLL_VISIBLE, visible);
  if (otherImpl_)
    otherImpl_->scrollVisibilityChanged_.emit(visible);
}

void WWebWidget::setThemeStyleEnabled(bool enabled)
{
  flags_.set(BIT_THEME_STYLE_DISABLED, !enabled);
}

bool WWebWidget::isThemeStyleEnabled() const
{
  return !flags_.test(BIT_THEME_STYLE_DISABLED);
}

void WWebWidget::setObjectName(const std::string& name)
{
  if (objectName() != name) {
    WWidget::setObjectName(name);
    flags_.set(BIT_OBJECT_NAME_CHANGED);
    repaint();
  }
}

int WWebWidget::baseZIndex() const
{
  if (!layoutImpl_)
    return DEFAULT_BASE_Z_INDEX;
  else
    return layoutImpl_->baseZIndex_;
}

void WWebWidget::setBaseZIndex(int zIndex)
{
  if (!layoutImpl_)
    layoutImpl_.reset(new LayoutImpl());

  layoutImpl_->baseZIndex_ = zIndex;
}

void WWebWidget::emitChildrenChanged()
{
  if (!flags_.test(BIT_BEING_DELETED) && otherImpl_) {
    otherImpl_->childrenChanged_.emit();
  }
}

}
