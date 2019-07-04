/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "StdWidgetItemImpl.h"
#include "StdGridLayoutImpl2.h"

#include "Wt/WApplication.h"
#include "Wt/WBorderLayout.h"
#include "Wt/WBoxLayout.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WFitLayout.h"
#include "Wt/WGridLayout.h"
#include "Wt/WWidgetItem.h"
#include "WebUtils.h"

#include "DomElement.h"
#include "WebSession.h"

#include <boost/algorithm/string.hpp>

namespace Wt {

LOGGER("WContainerWidget");

const char *WContainerWidget::SCROLL_SIGNAL = "scroll";

WContainerWidget::WContainerWidget()
  : contentAlignment_(AlignmentFlag::Left),
    overflow_(nullptr),
    padding_(nullptr),
    globalUnfocused_(false),
    scrollTop_(0),
    scrollLeft_(0)
{
  setInline(false);
  setLoadLaterWhenInvisible(false);
}

EventSignal<WScrollEvent>& WContainerWidget::scrolled()
{
  return *scrollEventSignal(SCROLL_SIGNAL, true);
}

WContainerWidget::~WContainerWidget()
{
  beingDeleted();
  clear();

  delete[] padding_;
  delete[] overflow_;
}

#ifndef WT_NO_LAYOUT
StdLayoutImpl *WContainerWidget::layoutImpl() const
{
  return dynamic_cast<StdLayoutImpl *>(layout_->impl());
}
#endif // WT_NO_LAYOUT

void WContainerWidget::setLayout(std::unique_ptr<WLayout> layout)
{
#ifndef WT_NO_LAYOUT
  // make sure old layout is deleted first, std::unique_ptr assignment
  // changes the value of the pointer and then deletes the old value, but
  // we need to delete the old value first, otherwise we run into problems
  // FIXME: maybe the code should be fixed so this is not necessary? Having
  //        to call reset() first feels dirty
  clear();

  layout_ = std::move(layout);
  if (layout_)
    layout_->setParentWidget(this);
  contentAlignment_ = AlignmentFlag::Justify;
  flags_.set(BIT_LAYOUT_NEEDS_RERENDER);
  repaint();
#else
  assert(false);
#endif
}

void WContainerWidget::childResized(WWidget *child,
				    WFlags<Orientation> directions)
{
#ifndef WT_NO_LAYOUT
  if (layout_) {
    WWidgetItem *item = layout_->findWidgetItem(child);
    if (item) {
      if (dynamic_cast<StdLayoutImpl *>(item->parentLayout()->impl())
	  ->itemResized(item)) {
	flags_.set(BIT_LAYOUT_NEEDS_UPDATE);
	repaint();
      }
    }
  } else
    WInteractWidget::childResized(child, directions);
#endif
}

void WContainerWidget::parentResized(WWidget *parent,
				     WFlags<Orientation> directions)
{
#ifndef WT_NO_LAYOUT
  if (layout_) {
    if (dynamic_cast<StdLayoutImpl *>(layout_->impl())->parentResized()) {
      flags_.set(BIT_LAYOUT_NEEDS_UPDATE);
      repaint();
    }
  } else
    WInteractWidget::parentResized(parent, directions);
#endif
}

void WContainerWidget::addWidget(std::unique_ptr<WWidget> widget)
{
  insertWidget(children_.size(), std::move(widget));
}

void WContainerWidget::insertBefore(std::unique_ptr<WWidget> widget,
				    WWidget *before)
{
  int index = indexOf(before);

  if (index == -1) {
    LOG_ERROR("insertBefore(): before is not in container, appending at back");
    index = children_.size();
  }

  insertWidget(index, std::move(widget));
}

void WContainerWidget::insertWidget(int index, std::unique_ptr<WWidget> widget)
{
  WWidget *w = widget.get();

  if (!addedChildren_) {
    addedChildren_.reset(new std::vector<WWidget *>());

    // A TD/TH node cannot be stubbed
    if (domElementType() != DomElementType::TD &&
	domElementType() != DomElementType::TH)
      setLoadLaterWhenInvisible(true);
  }

  addedChildren_->push_back(widget.get());
  children_.insert(children_.begin() + index, widget.get());
  addChild(std::move(widget));
  flags_.set(BIT_ADJUST_CHILDREN_ALIGN); // children margins hacks
  repaint(RepaintFlag::SizeAffected);

  widgetAdded(w);
}

std::unique_ptr<WWidget> WContainerWidget::removeWidget(WWidget *widget)
{
#ifndef WT_NO_LAYOUT
  if (layout_) {
    auto result = layout_->removeWidget(widget);
    if (result)
      widgetRemoved(result.get(), false);
    return result;
  }
#endif // WT_NO_LAYOUT

  int index = indexOf(widget);

  if (index != -1) {
    bool renderRemove = true;

    if (addedChildren_ && Utils::erase(*addedChildren_, widget))
      renderRemove = false;

    children_.erase(children_.begin() + index);

    // NOTE: result may be null if the widget is not owned by this WContainerWidget!
    std::unique_ptr<WWidget> result = removeChild(widget);

    repaint(RepaintFlag::SizeAffected);

    widgetRemoved(widget, renderRemove);

    return result;
  } else {
    LOG_ERROR("removeWidget(): widget not in container");
    return std::unique_ptr<WWidget>();
  }
}

void WContainerWidget::clear()
{
#ifndef WT_NO_LAYOUT
  layout_.reset();
#endif

  while (!children_.empty())
    removeWidget(children_.back());
}

void WContainerWidget::iterateChildren(const HandleWidgetMethod& method) const
{
  // It's possible that a child is added during iteration,
  // e.g. when load() is called on a widget that adds a
  // new global widget to the domroot. This would invalidate
  // the iterator. That's why we're iterating over children_
  // the old school way, with an index. Then there's no iterator
  // that ends up invalidated.
  for (std::size_t i = 0;
       i < children_.size(); ++i)
#ifndef WT_TARGET_JAVA
    method(children_[i]);
#else
    method.handle(children_[i]);
#endif

  if (layout_)
    layout_->iterateWidgets(method);
}

int WContainerWidget::indexOf(WWidget *widget) const
{
  for (unsigned i = 0; i < children_.size(); ++i)
    if (children_[i] == widget)
      return i;

  return -1;
}

WWidget *WContainerWidget::widget(int index) const
{
  return children_[index];
}

int WContainerWidget::count() const
{
  return children_.size();
}

void WContainerWidget::setContentAlignment(WFlags<AlignmentFlag> alignment)
{
  contentAlignment_ = alignment;

  /* Make sure vertical alignment is always specified */
  AlignmentFlag vAlign = contentAlignment_ & AlignVerticalMask;
  if (vAlign == static_cast<AlignmentFlag>(0))
    contentAlignment_ |= AlignmentFlag::Top;

  flags_.set(BIT_CONTENT_ALIGNMENT_CHANGED);

  repaint();
}

void WContainerWidget::setList(bool list, bool ordered)
{
  flags_.set(BIT_LIST, list);
  flags_.set(BIT_ORDERED_LIST, ordered);
}

bool WContainerWidget::isList() const
{
  return flags_.test(BIT_LIST);
}

bool WContainerWidget::isOrderedList() const
{
  return flags_.test(BIT_LIST) && flags_.test(BIT_ORDERED_LIST);
}

bool WContainerWidget::isUnorderedList() const
{
  return flags_.test(BIT_LIST) && !flags_.test(BIT_ORDERED_LIST);
}

void WContainerWidget::setPadding(const WLength& length, WFlags<Side> sides)
{
  if (!padding_) {
    padding_ = new WLength[4];
#ifdef WT_TARGET_JAVA
    padding_[0] = padding_[1] = padding_[2] = padding_[3] = WLength::Auto;
#endif // WT_TARGET_JAVA
  }

  if (sides.test(Side::Top))
    padding_[0] = length;
  if (sides.test(Side::Right))
    padding_[1] = length;
  if (sides.test(Side::Bottom))
    padding_[2] = length;
  if (sides.test(Side::Left))
    padding_[3] = length;

  flags_.set(BIT_PADDINGS_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

void WContainerWidget::setOverflow(Overflow value,
				   WFlags<Orientation> orientation)
{
  if (!overflow_) {
    overflow_ = new Overflow[2];
    overflow_[0] = overflow_[1] = Overflow::Visible;
  }

  if (orientation.test(Orientation::Horizontal))
    overflow_[0] = value;
  if (orientation.test(Orientation::Vertical))
    overflow_[1] = value;
  
  // Could be a workaround for IE, but sometimes causes other problems:
  // if (value == OverflowScroll || value == OverflowAuto)
  //   setPositionScheme(PositionScheme::Relative);

  flags_.set(BIT_OVERFLOW_CHANGED);
  repaint();
}

WLength WContainerWidget::padding(Side side) const
{
  if (!padding_)
    return WLength::Auto;

  switch (side) {
  case Side::Top:
    return padding_[0];
  case Side::Right:
    return padding_[1];
  case Side::Bottom:
    return padding_[2];
  case Side::Left:
    return padding_[3];
  default:
    LOG_ERROR("padding(): improper side.");
    return WLength();
  }
}

void WContainerWidget::updateDom(DomElement& element, bool all)
{
  element.setGlobalUnfocused(globalUnfocused_);
  if (all && element.type() == DomElementType::LI && isInline())
    element.setProperty(Property::StyleDisplay, "inline");

  if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED) || all) {
    AlignmentFlag hAlign = contentAlignment_ & AlignHorizontalMask;

    bool ltr = WApplication::instance()->layoutDirection() 
      == LayoutDirection::LeftToRight;

    switch (hAlign) {
    case AlignmentFlag::Left:
      if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED))
	element.setProperty(Property::StyleTextAlign, ltr ? "left" : "right");
      break;
    case AlignmentFlag::Right:
      element.setProperty(Property::StyleTextAlign, ltr ? "right" : "left");
      break;
    case AlignmentFlag::Center:
      element.setProperty(Property::StyleTextAlign, "center");
      break;
    case AlignmentFlag::Justify:
#ifndef WT_NO_LAYOUT
      if (!layout_)
#endif // WT_NO_LAYOUT
	element.setProperty(Property::StyleTextAlign, "justify");
      break;
    default:
      break;
    }

    if (domElementType() == DomElementType::TD) {
      AlignmentFlag vAlign = contentAlignment_ & AlignVerticalMask;
      switch (vAlign) {
      case AlignmentFlag::Top:
	if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED))
	  element.setProperty(Property::StyleVerticalAlign, "top");
	break;
      case AlignmentFlag::Middle:
	element.setProperty(Property::StyleVerticalAlign, "middle");
	break;
      case AlignmentFlag::Bottom:
	element.setProperty(Property::StyleVerticalAlign, "bottom");
      default:
	break;
      }
    }
  }

  if (flags_.test(BIT_ADJUST_CHILDREN_ALIGN) || 
      flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED) || all) {
    /*
     * Welcome to CSS hell.
     *
     * Apparently, the text-align property only applies to inline elements.
     * To center non-inline children, the standard says to set its left and
     * right margin to 'auto'.
     *
     * I assume the same applies for aligning to the right ?
     */
    for (unsigned i = 0; i < children_.size(); ++i) {
      WWidget *child = children_[i];

      if (!child->isInline()) {
	AlignmentFlag ha = contentAlignment_ & AlignHorizontalMask;
	if (ha == AlignmentFlag::Center) {
	  if (!child->margin(Side::Left).isAuto())
	    child->setMargin(WLength::Auto, Side::Left);
	  if (!child->margin(Side::Right).isAuto())
	    child->setMargin(WLength::Auto, Side::Right);
	} else if (ha == AlignmentFlag::Right) {
	  if (!child->margin(Side::Left).isAuto())
	    child->setMargin(WLength::Auto, Side::Left);
	}
      }
    }

    flags_.reset(BIT_CONTENT_ALIGNMENT_CHANGED);
    flags_.reset(BIT_ADJUST_CHILDREN_ALIGN);
  }

  if (flags_.test(BIT_PADDINGS_CHANGED)
      || (all && padding_ &&
	  !(   padding_[0].isAuto() && padding_[1].isAuto()
	    && padding_[2].isAuto() && padding_[3].isAuto()))) {

    if ((padding_[0] == padding_[1]) && (padding_[0] == padding_[2])
	&& (padding_[0] == padding_[3]))
      element.setProperty(Property::StylePadding, padding_[0].cssText());
    else {
      WStringStream s;
      for (unsigned i = 0; i < 4; ++i) {
	if (i != 0)
	  s << ' ';
	s << (padding_[i].isAuto() ? "0" : padding_[i].cssText());
      }
      element.setProperty(Property::StylePadding, s.str());
    }

    flags_.reset(BIT_PADDINGS_CHANGED);
  }

  WInteractWidget::updateDom(element, all);

  if (flags_.test(BIT_OVERFLOW_CHANGED) ||
      (all && overflow_ &&
       !(overflow_[0] == Overflow::Visible &&
	 overflow_[1] == Overflow::Visible))) {
    static const char *cssText[] = { "visible", "auto", "hidden", "scroll" };

    element.setProperty(Property::StyleOverflowX, 
			cssText[static_cast<unsigned int>(overflow_[0])]);
    element.setProperty(Property::StyleOverflowY,
			cssText[static_cast<unsigned int>(overflow_[1])]);
    // enable form object to retrieve scroll state
    setFormObject(true);
    
    //declare javascript function Wt.encodeValue()
    this->doJavaScript(this->jsRef()
	+ ".wtEncodeValue = function() {"
	+ "return " + this->jsRef() + ".scrollTop"
	+ " + ';' + " + this->jsRef() + ".scrollLeft;"
	+ "}");

    flags_.reset(BIT_OVERFLOW_CHANGED);

    /* If a container widget has overflow, then, if ever something
     * inside it has position scheme relative/absolute, it will not
     * scroll properly unless every element up to the container and including 
     * the container itself has overflow: relative.
     *
     * The following fixes the common case:
     * container (overflow) - container - layout
     */
    WApplication *app = WApplication::instance();
    if (app->environment().agentIsIE()
	&& (overflow_[0] == Overflow::Auto || 
	    overflow_[0] == Overflow::Scroll))
      if (positionScheme() == PositionScheme::Static)
	element.setProperty(Property::StylePosition, "relative");
  }
}

int WContainerWidget::firstChildIndex() const
{
  return 0;
}

void WContainerWidget::propagateLayoutItemsOk(WLayoutItem *item)
{
  if (!item)
    return;

  if (item->layout()) {
    WLayout *layout = item->layout();
    const int c = layout->count();
    for (int i = 0; i < c; ++i)
      propagateLayoutItemsOk(layout->itemAt(i));
  } else if (item->widget()) {
    WWidget *w = item->widget();
    w->webWidget()->propagateRenderOk(true);
  }
}

void WContainerWidget::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_CONTENT_ALIGNMENT_CHANGED);
  flags_.reset(BIT_PADDINGS_CHANGED);
  flags_.reset(BIT_OVERFLOW_CHANGED);
  flags_.reset(BIT_LAYOUT_NEEDS_RERENDER);
  flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);

#ifndef WT_NO_LAYOUT
  if (layout_ && deep)
    propagateLayoutItemsOk(layout());
  else
#endif
    addedChildren_.reset();

  WInteractWidget::propagateRenderOk(deep);
}

bool WContainerWidget::wasEmpty() const
{
  /*
   * First case: on IE6, a popup widget has a shim child.
   * Second case: WGroupBox always has a legend
   */
  if (isPopup() || firstChildIndex() > 0)
    return false;
  else
    return (addedChildren_ ? addedChildren_->size() : 0) == children_.size();
}

DomElementType WContainerWidget::domElementType() const
{
  DomElementType type = isInline() ? DomElementType::SPAN : DomElementType::DIV;

  WContainerWidget *p = dynamic_cast<WContainerWidget *>(parentWebWidget());
  if (p && p->isList())
    type = DomElementType::LI;

  if (isList())
    type = isOrderedList() ? DomElementType::OL : DomElementType::UL;

  return type;
}

void WContainerWidget::getDomChanges(std::vector<DomElement *>& result,
				     WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, domElementType());

#ifndef WT_NO_LAYOUT
  if (!app->session()->renderer().preLearning()) {
    if (flags_.test(BIT_LAYOUT_NEEDS_RERENDER)) {
      e->removeAllChildren(firstChildIndex());
      createDomChildren(*e, app);

      flags_.reset(BIT_LAYOUT_NEEDS_RERENDER);
      flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);
    }
  }
#endif // WT_NO_LAYOUT

  updateDomChildren(*e, app);

  updateDom(*e, false);

  result.push_back(e);
}

DomElement *WContainerWidget::createDomElement(WApplication *app)
{
  return createDomElement(app, true);
}

DomElement *WContainerWidget::createDomElement(WApplication *app,
					       bool addChildren)
{
  addedChildren_.reset();

  DomElement *result = WWebWidget::createDomElement(app);

  if (addChildren)
    createDomChildren(*result, app);

  return result;
}

void WContainerWidget::createDomChildren(DomElement& parent, WApplication *app)
{
  if (layout_) {
#ifndef WT_NO_LAYOUT
    containsLayout();
    bool fitWidth = true;
    bool fitHeight = true;

    DomElement *c = layoutImpl()->createDomElement(&parent,
						   fitWidth, fitHeight, app);

    if (c != &parent)
      parent.addChild(c);

    flags_.reset(BIT_LAYOUT_NEEDS_RERENDER);
    flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);
#endif // WT_NO_LAYOUT
  } else {
    for (unsigned i = 0; i < children_.size(); ++i)
      parent.addChild(children_[i]->createSDomElement(app));
  }

  addedChildren_.reset();
}

void WContainerWidget::updateDomChildren(DomElement& parent, WApplication *app)
{
  if (!app->session()->renderer().preLearning() && !layout_) {
    if (parent.mode() == DomElement::Mode::Update)
      parent.setWasEmpty(wasEmpty());

    if (addedChildren_) {
      for (;;) {
	std::vector<int> orderedInserts;
	std::vector<WWidget *>& ac = *addedChildren_;

	for (unsigned i = 0; i < ac.size(); ++i)
	  orderedInserts.push_back(indexOf(ac[i]));

	Utils::sort(orderedInserts);

	int addedCount = addedChildren_->size();
	int totalCount = children_.size();
	int insertCount = 0;

	addedChildren_.reset();

	for (unsigned i = 0; i < orderedInserts.size(); ++i) {
	  int pos = orderedInserts[i];
	
	  DomElement *c = (children_)[pos]->createSDomElement(app);

	  if (pos + (addedCount - insertCount) == totalCount)
	    parent.addChild(c);
	  else
	    parent.insertChildAt(c, pos + firstChildIndex());

	  ++insertCount;
	}

	if (!addedChildren_ || addedChildren_->empty())
	  break;
      }

      addedChildren_.reset();
    }
  }

#ifndef WT_NO_LAYOUT
  if (flags_.test(BIT_LAYOUT_NEEDS_UPDATE)) {
    if (layout_)
      layoutImpl()->updateDom(parent);

    flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);
  }
#endif // WT_NO_LAYOUT
}

void WContainerWidget::layoutChanged(bool rerender)
{
#ifndef WT_NO_LAYOUT
  if (rerender)
    flags_.set(BIT_LAYOUT_NEEDS_RERENDER);
  else
    flags_.set(BIT_LAYOUT_NEEDS_UPDATE);

  repaint(RepaintFlag::SizeAffected);
#endif // WT_NO_LAYOUT
}

void WContainerWidget::rootAsJavaScript(WApplication *app, WStringStream& out,
					bool all)
{
  std::vector<WWidget *> *toAdd = all ? &children_ : addedChildren_.get();

  if (toAdd)
    for (unsigned i = 0; i < toAdd->size(); ++i) {
      DomElement *c = (*toAdd)[i]->createSDomElement(app);

      app->streamBeforeLoadJavaScript(out, false);

      c->callMethod("omousemove=function(e) {"
		    "if (!e) e = window.event;"
		    "return " + app->javaScriptClass()
		    + "._p_.dragDrag(event); }");
      c->callMethod("mouseup=function(e) {"
		    "if (!e) e = window.event;"
		    "return " + app->javaScriptClass()
		  + "._p_.dragEnd(event);}");
      c->callMethod("dragstart=function(){return false;}");
      c->asJavaScript(out);
      delete c;
    }

  addedChildren_.reset();

  if (!all) {
    /* Note: we ignore rendering of deletion of a bound widget... */
  }

  // FIXME
  propagateRenderOk(false);
}

void WContainerWidget::setGlobalUnfocused(bool b)
{
  globalUnfocused_ = b;
}

bool WContainerWidget::isGlobalUnfocussed() const 
{
  return globalUnfocused_;
}

void WContainerWidget::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    std::vector<std::string> attributes;
    boost::split(attributes, formData.values[0], boost::is_any_of(";"));

    if (attributes.size() == 2) {
      try {
        scrollTop_ = (int)Utils::stod(attributes[0]);
        scrollLeft_ = (int)Utils::stod(attributes[1]);

      }catch (const std::exception& e) {
	throw WException("WContainerWidget: error parsing: " + formData.values[0] + ": " + e.what());
      }
    } else 
      throw WException("WContainerWidget: error parsing: " + formData.values[0]);
	  }

}

}
