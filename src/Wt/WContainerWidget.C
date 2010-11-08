/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <algorithm>

#include "StdWidgetItemImpl.h"
#include "StdGridLayoutImpl.h"
#include "WtException.h"

#include "Wt/WApplication"
#include "Wt/WBorderLayout"
#include "Wt/WBoxLayout"
#include "Wt/WContainerWidget"
#include "Wt/WGridLayout"
#include "Wt/WWidgetItem"
#include "Utils.h"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WebSession.h"

namespace Wt {

const char *WContainerWidget::SCROLL_SIGNAL = "scroll";

/*
 * Should addedChildren move to WWebWidget ?
 * Should a WWebWidget know itself that it has been rendered to
 * avoid the ugly setIgnoreChildRemoves() hack ?
 * I think so ?
 *
 * The problem still exists with a child added with insertWidget since
 * it is not in the addedChildren list even if if it is not rendered ?
 * What if before gets deleted after adding a sibling ? then also
 * things go wrong. So many wrong things !
 */

WContainerWidget::WContainerWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    contentAlignment_(AlignLeft),
    overflow_(0),
    padding_(0),
    layout_(0)
{
  setInline(false);
  setLoadLaterWhenInvisible(false);

  children_ = new std::vector<WWidget *>;
}

EventSignal<WScrollEvent>& WContainerWidget::scrolled()
{
  return *scrollEventSignal(SCROLL_SIGNAL, true);
}

WContainerWidget::~WContainerWidget()
{
#ifndef WT_NO_LAYOUT
  WLayout *layout = layout_;
  layout_ = 0;
  delete layout;
#endif // WT_NO_LAYOUT
  delete[] padding_;
  delete[] overflow_;
}

#ifndef WT_NO_LAYOUT
StdLayoutImpl *WContainerWidget::layoutImpl() const
{
  return dynamic_cast<StdLayoutImpl *>(layout_->impl());
}
#endif // WT_NO_LAYOUT

void WContainerWidget::setLayout(WLayout *layout)
{
  setLayout(layout, AlignJustify);
}

void WContainerWidget::setLayout(WLayout *layout,
				 WFlags<AlignmentFlag> alignment)
{
#ifndef WT_NO_LAYOUT
  if (layout_ && layout != layout_)
    delete layout_;

  contentAlignment_ = alignment;

  if (layout != layout_) {
    layout_ = layout;
    flags_.set(BIT_LAYOUT_CHANGED);

    if (layout) {
      WWidget::setLayout(layout);
      layoutImpl()->setContainer(this);

      /*
       * Normally, scrollbars are not used automatically for a container,
       * which applies to when a layout overflows.
       *
       * Only for IE we really need to set this otherwise the parent
       * increases its size automatically and then we cannot reduce in
       * size (standard behaviour is overflow visible which says the
       * parent size should not be affected). Luckily, IE does not show the
       * scrollbars unless really needed
       */
      if (WApplication::instance()->environment().agentIsIE()) {
	AlignmentFlag vAlign = alignment & AlignVerticalMask;
	if (vAlign == 0)
	  setOverflow(WContainerWidget::OverflowHidden);
      }
    }
  }
#else
  assert(false);
#endif
}

void WContainerWidget::childResized(WWidget *child,
				    WFlags<Orientation> directions)
{
#ifndef WT_NO_LAYOUT
  AlignmentFlag vAlign = contentAlignment_ & AlignVerticalMask;
  if (layout_
      && (directions & Vertical)
      && (vAlign == 0)) {
    if (!flags_.test(BIT_LAYOUT_NEEDS_UPDATE)) {
      WWidgetItem *item = layout_->findWidgetItem(child);
      if (item)
	if (dynamic_cast<StdLayoutImpl *>(item->parentLayout()->impl())
	    ->itemResized(item)) {
	  flags_.set(BIT_LAYOUT_NEEDS_UPDATE);
	  repaint(RepaintInnerHtml);
	}
    }
  } else
#endif
    WInteractWidget::childResized(child, directions);
}

WLayoutItemImpl *WContainerWidget::createLayoutItemImpl(WLayoutItem *item)
{
#ifndef WT_NO_LAYOUT
  {
    WWidgetItem *wi = dynamic_cast<WWidgetItem *>(item);
    if (wi)
      return new StdWidgetItemImpl(wi);
  }

  {
    WBorderLayout *l = dynamic_cast<WBorderLayout *>(item);
    if (l)
      return new StdGridLayoutImpl(l, l->grid());
  }

  {
    WBoxLayout *l = dynamic_cast<WBoxLayout *>(item);
    if (l)
      return new StdGridLayoutImpl(l, l->grid());
  }

  {
    WGridLayout *l = dynamic_cast<WGridLayout *>(item);
    if (l)
      return new StdGridLayoutImpl(l, l->grid());
  }
#endif

  assert(false);

  return 0;
}

void WContainerWidget::addWidget(WWidget *widget)
{
  if (widget->parent()) {
    if (widget->parent() != this) {
      wApp->log("warn")
	<< "WContainerWidget::addWidget(): reparenting widget";
      widget->setParentWidget(0);
    } else
      return;
  }

  if (!transientImpl_) {
    transientImpl_ = new TransientImpl();

    // IE cannot replace a TD node using DOM API
    if (domElementType() != DomElement_TD
	|| !WApplication::instance()->environment().agentIsIE())
      setLoadLaterWhenInvisible(true);
  }

  transientImpl_->addedChildren_.push_back(widget);
  flags_.set(BIT_ADJUST_CHILDREN_ALIGN); // children margins hacks
  repaint(RepaintInnerHtml);

  widget->setParentWidget(this);
}

void WContainerWidget::insertWidget(int index, WWidget *widget)
{
  if (index == (int)children_->size())
    addWidget(widget);
  else
    insertBefore(widget, children()[index]);
}

void WContainerWidget::insertBefore(WWidget *widget, WWidget *before)
{
  if (before->parent() != this) {
    wApp->log("error") << "WContainerWidget::insertBefore(): 'before' not "
      "in this container";
    return;
  }

  if (widget->parent()) {
    if (widget->parent() != this) {
      wApp->log("warn")
	<< "WContainerWidget::insertWidget(): reparenting widget";
      widget->setParentWidget(0);
    } else
      return;
  }

  int i = Utils::indexOf(*children_, before);
  if (i == -1)
    i = children_->size();

  children_->insert(children_->begin() + i, widget);
  flags_.set(BIT_ADJUST_CHILDREN_ALIGN); // children margins hacks
  repaint(RepaintInnerHtml);

  if (!transientImpl_)
    transientImpl_ = new TransientImpl();
  transientImpl_->addedChildren_.push_back(widget);

  // would try to add the widget again to children_
  // widget->setParent(this);
  // so instead, we copy the code from WWebWidget::addChild() here:
  widget->setParent(this);

  if (loaded())
    doLoad(widget);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);
}

void WContainerWidget::removeFromLayout(WWidget *widget)
{
#ifndef WT_NO_LAYOUT
  if (layout_)
    removeWidget(widget);
#endif // WT_NO_LAYOUT
}

void WContainerWidget::removeWidget(WWidget *widget)
{
  widget->setParentWidget(0);

  repaint(RepaintInnerHtml);
}

void WContainerWidget::clear()
{
  // first delete the widgets, this will also remove them from
  // the layout
  while (!children().empty()) {
    WWidget *w = children().back();
    delete w;
  }

#ifndef WT_NO_LAYOUT
  delete layout_;
  layout_ = 0;
#endif // WT_NO_LAYOUT
}

int WContainerWidget::indexOf(WWidget *widget) const
{
  return Utils::indexOf(children(), widget);
}

WWidget *WContainerWidget::widget(int index) const
{
  return children()[index];
}

int WContainerWidget::count() const
{
  return children().size();
}

void WContainerWidget::removeChild(WWidget *child)
{
  bool ignoreThisChildRemove = false;

  if (transientImpl_) {
    if (Utils::erase(transientImpl_->addedChildren_, child)) {
      /*
       * Child was just added: do not render a child remove, since it
       * is not yet part of the DOM
       */
      ignoreThisChildRemove = true;
    }
  }

#ifndef WT_NO_LAYOUT
  if (layout_) {
    ignoreThisChildRemove = true; // will be re-rendered by layout
    if (layout_->removeWidget(child))
      return;
  }
#endif // WT_NO_LAYOUT

  if (ignoreThisChildRemove)
    if (ignoreChildRemoves())
      ignoreThisChildRemove = false; // was already ignoring them

  if (ignoreThisChildRemove)
    setIgnoreChildRemoves(true);

  WWebWidget::removeChild(child);

  if (ignoreThisChildRemove)
    setIgnoreChildRemoves(false);
}

void WContainerWidget::setContentAlignment(WFlags<AlignmentFlag> alignment)
{
  contentAlignment_ = alignment;

  /* Make sure vertical alignment is always specified */
  AlignmentFlag vAlign = contentAlignment_ & AlignVerticalMask;
  if (vAlign == 0)
    contentAlignment_ |= AlignTop;

  flags_.set(BIT_CONTENT_ALIGNMENT_CHANGED);

  repaint(RepaintPropertyAttribute);
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

  if (sides.testFlag(Top))
    padding_[0] = length;
  if (sides.testFlag(Right))
    padding_[1] = length;
  if (sides.testFlag(Bottom))
    padding_[2] = length;
  if (sides.testFlag(Left))
    padding_[3] = length;

  flags_.set(BIT_PADDINGS_CHANGED);
  repaint(RepaintPropertyAttribute);
}

void WContainerWidget::setOverflow(Overflow value,
				   WFlags<Orientation> orientation)
{
  if (!overflow_) {
    overflow_ = new Overflow[2];
    overflow_[0] = overflow_[1] = OverflowVisible;
  }

  if (orientation & Horizontal)
    overflow_[0] = value;
  if (orientation & Vertical)
    overflow_[1] = value;
  
  // Could be a workaround for IE, but sometimes causes other problems:
  // if (value == OverflowScroll || value == OverflowAuto)
  //   setPositionScheme(Relative);

  flags_.set(BIT_OVERFLOW_CHANGED);
  repaint(RepaintPropertyAttribute);
}

WLength WContainerWidget::padding(Side side) const
{
  if (!padding_)
    return WLength::Auto;

  switch (side) {
  case Top:
    return padding_[0];
  case Right:
    return padding_[1];
  case Bottom:
    return padding_[2];
  case Left:
    return padding_[3];
  default:
    throw WtException("WContainerWidget::padding(Side) with invalid side.");
  }
}

void WContainerWidget::updateDom(DomElement& element, bool all)
{
  if (all && element.type() == DomElement_LI && isInline())
    element.setProperty(PropertyStyleDisplay, "inline");

  if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED) || all) {
    AlignmentFlag hAlign = contentAlignment_ & AlignHorizontalMask;
    switch (hAlign) {
    case AlignLeft:
      if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED))
	element.setProperty(PropertyStyleTextAlign, "left");
      break;
    case AlignRight:
      element.setProperty(PropertyStyleTextAlign, "right");
      break;
    case AlignCenter:
      element.setProperty(PropertyStyleTextAlign, "center");
      break;
    case AlignJustify:
#ifndef WT_NO_LAYOUT
      if (!layout_)
#endif // WT_NO_LAYOUT
	element.setProperty(PropertyStyleTextAlign, "justify");
      break;
    default:
      break;
    }

    if (domElementType() == DomElement_TD) {
      AlignmentFlag vAlign = contentAlignment_ & AlignVerticalMask;
      switch (vAlign) {
      case AlignTop:
	if (flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED))
	  element.setProperty(PropertyStyleVerticalAlign, "top");
	break;
      case AlignMiddle:
	element.setProperty(PropertyStyleVerticalAlign, "middle");
	break;
      case AlignBottom:
	element.setProperty(PropertyStyleVerticalAlign, "bottom");
      default:
	break;
      }
    }
  }

  if (flags_.test(BIT_ADJUST_CHILDREN_ALIGN)
      || flags_.test(BIT_CONTENT_ALIGNMENT_CHANGED) || all) {
    /*
     * Welcome to CSS hell.
     *
     * Apparently, the text-align property only applies to inline elements.
     * To center non-inline children, the standard says to set its left and
     * right margin to 'auto'.
     *
     * I assume the same applies for aligning to the right ?
     */
    for (unsigned i = 0; i < children_->size(); ++i) {
      WWidget *child = (*children_)[i];

      if (!child->isInline()) {
	AlignmentFlag ha = contentAlignment_ & AlignHorizontalMask;
	if (ha == AlignCenter) {
	  if (!child->margin(Left).isAuto())
	    child->setMargin(WLength::Auto, Left);
	  if (!child->margin(Right).isAuto())
	    child->setMargin(WLength::Auto, Right);
	} else if (ha == AlignRight) {
	  if (!child->margin(Left).isAuto())
	    child->setMargin(WLength::Auto, Left);
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

    if ((padding_[0] == padding_[1])
	&& (padding_[0] == padding_[2])
	&& (padding_[0] == padding_[3]))
      element.setProperty(PropertyStylePadding,
			  padding_[0].cssText());
    else
      element.setProperty(PropertyStylePadding,
			  padding_[0].cssText()
			  + " " + padding_[1].cssText()
			  + " " + padding_[2].cssText()
			  + " " + padding_[3].cssText());

    flags_.reset(BIT_PADDINGS_CHANGED);
  }

  if (!wApp->session()->renderer().preLearning() && !layout_) {
    element.setWasEmpty(all || wasEmpty());

    if (transientImpl_) {
      WApplication *app = WApplication::instance();
      std::vector<int> orderedInserts;
      std::vector<WWidget *>& ac = transientImpl_->addedChildren_;

      for (unsigned i = 0; i < ac.size(); ++i)
	orderedInserts.push_back(Utils::indexOf(*children_, ac[i]));

      Utils::sort(orderedInserts);

      int addedCount = transientImpl_->addedChildren_.size();
      int totalCount = children_->size();
      int insertCount = 0;
      for (unsigned i = 0; i < orderedInserts.size(); ++i) {
	int pos = orderedInserts[i];
	
	DomElement *c = (*children_)[pos]->createSDomElement(app);

	if (pos + (addedCount - insertCount) == totalCount)
	  element.addChild(c);
	else
	  element.insertChildAt(c, pos + firstChildIndex());

	++insertCount;
      }

      transientImpl_->addedChildren_.clear();
    }
  }

  if (flags_.test(BIT_LAYOUT_NEEDS_UPDATE)) {
    if (layout_)
      layoutImpl()->updateDom();

    flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);
  }

  WInteractWidget::updateDom(element, all);

  if (flags_.test(BIT_OVERFLOW_CHANGED)
      || (all && overflow_ &&
	  !(overflow_[0] == OverflowVisible
	    && overflow_[1] == OverflowVisible))) {
    static const char *cssText[] = { "visible", "auto", "hidden", "scroll" };

    element.setProperty(PropertyStyleOverflowX, cssText[overflow_[0]]);
    //element.setProperty(PropertyStyleOverflowY, cssText[overflow_[1]]);

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
	&& (overflow_[0] == OverflowAuto || overflow_[0] == OverflowScroll))
      element.setProperty(PropertyStylePosition, "relative");
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
  flags_.reset(BIT_LAYOUT_CHANGED);
  flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);

#ifndef WT_NO_LAYOUT
  if (layout_ && deep)
    propagateLayoutItemsOk(layout());
  else
#endif
    if (transientImpl_)
      transientImpl_->addedChildren_.clear();

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
    return ((transientImpl_ ? transientImpl_->addedChildren_.size() : 0)
	    == children_->size());
}

DomElementType WContainerWidget::domElementType() const
{
  DomElementType type = isInline() ? DomElement_SPAN : DomElement_DIV;

  WContainerWidget *p = dynamic_cast<WContainerWidget *>(parent());
  if (p && p->isList())
    type = DomElement_LI;

  if (isList())
    type = isOrderedList() ? DomElement_OL : DomElement_UL;

  return type;
}

void WContainerWidget::getDomChanges(std::vector<DomElement *>& result,
				     WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, domElementType());

#ifndef WT_NO_LAYOUT
  if (!app->session()->renderer().preLearning()) {
    if (flags_.test(BIT_LAYOUT_CHANGED)) {
      e->removeAllChildren(firstChildIndex());
      createDomChildren(*e, app);

      flags_.reset(BIT_LAYOUT_CHANGED);
      flags_.reset(BIT_LAYOUT_NEEDS_UPDATE);
    }
  }
#endif // WT_NO_LAYOUT

  updateDom(*e, false);

  result.push_back(e);
}

DomElement *WContainerWidget::createDomElement(WApplication *app)
{
  if (transientImpl_)
    transientImpl_->addedChildren_.clear();

  DomElement *result = WWebWidget::createDomElement(app);
  createDomChildren(*result, app);

  return result;
}

void WContainerWidget::createDomChildren(DomElement& parent, WApplication *app)
{
  if (layout_) {
#ifndef WT_NO_LAYOUT
    bool fitWidth = contentAlignment_ & AlignJustify;
    bool fitHeight = !(contentAlignment_ & AlignVerticalMask);

    DomElement *c = layoutImpl()->createDomElement(fitWidth, fitHeight, app);

    /*
     * Take the hint: if the container is relative, then we can use an absolute
     * layout for its contents, under the assumption that a .wtResize or
     * auto-javascript sets the width too (like in WTreeView, WTableView)
     */
    if (positionScheme() == Relative || positionScheme() == Absolute) {
      c->setProperty(PropertyStylePosition, "absolute");
      c->setProperty(PropertyStyleLeft, "0");
      c->setProperty(PropertyStyleRight, "0");
    } else if (app->environment().agentIsIE()) {
      /*
       * position: relative element needs to be in a position: relative
       * parent otherwise scrolling is broken
       */
      parent.setProperty(PropertyStylePosition, "relative");
    }

    switch (contentAlignment_ & AlignHorizontalMask) {
    case AlignCenter: {
      DomElement *itable = DomElement::createNew(DomElement_TABLE);
      itable->setProperty(PropertyClass, "Wt-hcenter");
      if (fitHeight)
	itable->setProperty(PropertyStyle, "height:100%;");
      DomElement *irow = DomElement::createNew(DomElement_TR);
      DomElement *itd = DomElement::createNew(DomElement_TD);
      if (fitHeight)
	itd->setProperty(PropertyStyle, "height:100%;");
      itd->addChild(c);
      irow->addChild(itd);
      itable->addChild(irow);
      itable->setId(id() + "l");
      c = itable;

      break;
    }
    case AlignLeft:
      //c->setProperty(PropertyStyleFloat, "left");
      break;
    case AlignRight:
      c->setProperty(PropertyStyleFloat, "right");
      break;
    default:
      break;
    }

    parent.addChild(c);

    flags_.reset(BIT_LAYOUT_CHANGED);
#endif // WT_NO_LAYOUT
  } else {
    for (unsigned i = 0; i < children_->size(); ++i)
      parent.addChild((*children_)[i]->createSDomElement(app));
  }

  if (transientImpl_)
    transientImpl_->addedChildren_.clear();
}

void WContainerWidget::rootAsJavaScript(WApplication *app, std::ostream& out,
					bool all)
{
  std::vector<WWidget *> *toAdd
    = all ? children_ : (transientImpl_ ? &transientImpl_->addedChildren_ : 0);

  if (toAdd)
    for (unsigned i = 0; i < toAdd->size(); ++i) {
      DomElement *c = (*toAdd)[i]->createSDomElement(app);
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

  if (transientImpl_)
    transientImpl_->addedChildren_.clear();

  if (!all) {
    /* Note: we ignore rendering of deletion of a bound widget... */
  }

  // FIXME
  propagateRenderOk(false);
}

void WContainerWidget::layoutChanged(bool deleted)
{
#ifndef WT_NO_LAYOUT
  flags_.set(BIT_LAYOUT_CHANGED);

  repaint(RepaintInnerHtml);

  if (deleted)
    layout_ = 0;
#endif // WT_NO_LAYOUT
}

}
