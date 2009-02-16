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
    scrolled(this),
    contentAlignment_(AlignLeft),
    overflow_(0),
    padding_(0),
    layout_(0)
{
  setInline(false);
  setLoadLaterWhenInvisible(false);

  children_ = new std::vector<WWidget *>;
}

WContainerWidget::~WContainerWidget()
{
  delete layout_;
  delete[] padding_;
  delete[] overflow_;
}

StdLayoutImpl *WContainerWidget::layoutImpl() const
{
  return dynamic_cast<StdLayoutImpl *>(layout_->impl());
}

void WContainerWidget::setLayout(WLayout *layout)
{
  setLayout(layout, AlignJustify);
}

void WContainerWidget::setLayout(WLayout *layout, int alignment)
{
  if (layout_ && layout != layout_) {
    wApp->log("error") << "WContainerWidget::setLayout: already have a layout.";
    return;
  }

  contentAlignment_ = alignment;

  if (!layout_) {
    layout_ = layout;
    flags_.set(BIT_LAYOUT_CHANGED);

    WWidget::setLayout(layout);

    layoutImpl()->setContainer(this);
  }
}

WLayoutItemImpl *WContainerWidget::createLayoutItemImpl(WLayoutItem *item)
{
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

  assert(false);

  return 0;
}

void WContainerWidget::addWidget(WWidget *widget)
{
  if (widget->parent()) {
    if (widget->parent() != this) {
      wApp->log("warn")
	<< "WContainerWidget::addWidget(): reparenting widget";
      widget->setParent(0);
    } else
      return;
  }

  if (!transientImpl_) {
    transientImpl_ = new TransientImpl();

    // IE cannot replace a TD node using DOM API
    if (domElementType() != DomElement_TD
	|| !WApplication::instance()->environment().agentIE())
      setLoadLaterWhenInvisible(true);
  }

  transientImpl_->addedChildren_.push_back(widget);
  flags_.set(BIT_ADJUST_CHILDREN_ALIGN); // children margins hacks
  repaint(RepaintInnerHtml);

  widget->setParent(this);
}

void WContainerWidget::insertWidget(int index, WWidget *w)
{
  if (index == (int)children_->size())
    addWidget(w);
  else
    insertBefore(w, widget(index));
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
      widget->setParent(0);
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

  widget->WObject::setParent(this);

  if (loaded())
    doLoad(widget);

  WApplication::instance()
    ->session()->renderer().updateFormObjects(this, false);
}

void WContainerWidget::removeWidget(WWidget *widget)
{
  widget->setParent((WWidget *)0);

  repaint(RepaintInnerHtml);
}

void WContainerWidget::clear()
{
  while (!children().empty())
    delete children().back();

  // FIXME: we do not support deleting the layout_ without deleting all
  // children _FIRST_, since deleting the layout automatically removes the
  // children too in the DOM.
  delete layout_;
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

  if (ignoreThisChildRemove)
    if (WWebWidget::flags_.test(BIT_IGNORE_CHILD_REMOVES))
      ignoreThisChildRemove = false; // was already ignoring them

  if (ignoreThisChildRemove)
    setIgnoreChildRemoves(true);

  WWebWidget::removeChild(child);

  if (ignoreThisChildRemove)
    setIgnoreChildRemoves(false);
}

void WContainerWidget::setContentAlignment(HorizontalAlignment ha)
{
  contentAlignment_ = ha;

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

void WContainerWidget::setPadding(const WLength& length, int sides)
{
  if (!padding_)
    padding_ = new WLength[4];

  if (sides & Top)
    padding_[0] = length;
  if (sides & Right)
    padding_[1] = length;
  if (sides & Bottom)
    padding_[2] = length;
  if (sides & Left)
    padding_[3] = length;

  flags_.set(BIT_PADDINGS_CHANGED);
  repaint(RepaintPropertyAttribute);
}

void WContainerWidget::setOverflow(Overflow value, int orientation)
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
    return WLength();

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
    switch (contentAlignment_ & 0x0F) {
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
      if (!layout_)
	element.setProperty(PropertyStyleTextAlign, "justify");
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
	if (contentAlignment_ == AlignCenter) {
	  if (!child->margin(Left).isAuto())
	    child->setMargin(WLength(), Left);
	  if (!child->margin(Right).isAuto())
	    child->setMargin(WLength(), Right);
	}
	if (contentAlignment_ == AlignRight) {
	  if (!child->margin(Left).isAuto())
	    child->setMargin(WLength(), Left);
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
      if (padding_)
	element.setProperty(PropertyStylePadding,
			    padding_[0].cssText()
			    + " " + padding_[1].cssText()
			    + " " + padding_[2].cssText()
			    + " " + padding_[3].cssText());

    flags_.reset(BIT_PADDINGS_CHANGED);
  }

  if (!layout_) {
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

	WWidget *w = (*children_)[pos];
	DomElement *c = w->webWidget()->createSDomElement(app);

	if (pos + (addedCount - insertCount) == totalCount)
	  element.addChild(c);
	else
	  element.insertChildAt(c, pos);

	++insertCount;
      }

      transientImpl_->addedChildren_.clear();
    }
  }

  updateSignalConnection(element, scrolled, "scroll", all);

  WInteractWidget::updateDom(element, all);

  if (flags_.test(BIT_OVERFLOW_CHANGED)
      || (all && overflow_ &&
	  !(overflow_[0] == OverflowVisible
	    && overflow_[1] == OverflowVisible))) {
    static const char *cssText[] = { "visible", "auto", "hidden", "scroll" };

    element.setProperty(PropertyStyleOverflowX, cssText[overflow_[0]]);
    //element.setProperty(PropertyStyleOverflowY, cssText[overflow_[1]]);

    flags_.reset(BIT_OVERFLOW_CHANGED);
  }
}

bool WContainerWidget::wasEmpty() const
{
  return (transientImpl_ ? transientImpl_->addedChildren_.size() : 0)
    == children_->size();
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

  if (!wApp->session()->renderer().preLearning()) {
    if (flags_.test(BIT_LAYOUT_CHANGED)) {
      if (layout_)
	createDomChildren(*e, app);
      else
	flags_.reset(BIT_LAYOUT_CHANGED);
    }
  }

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
    int additionalVerticalPadding;
    bool fitWidth = (contentAlignment_ & 0x0F) == AlignJustify;
    bool fitHeight = (contentAlignment_ & 0xF0) == 0;

    DomElement *c
      = (layoutImpl()->createDomElement(fitWidth, fitHeight,
					additionalVerticalPadding, app));

    /*
     * Take the hint: if the container is relative, then we can use an absolute
     * layout for its contents, under the assumption that also the width
     * is handle using JavaScript (like in WTreeView)
     */
    if (positionScheme() == Relative) {
      c->setProperty(PropertyStylePosition, "absolute");
      c->setProperty(PropertyStyleLeft, "0");
      c->setProperty(PropertyStyleRight, "0");
    }

    switch (contentAlignment_ & 0x0F) {
    case AlignCenter: {
      DomElement *itable = DomElement::createNew(DomElement_TABLE);
      itable->setAttribute("class", "Wt-hcenter");
      if (fitHeight)
	itable->setAttribute("style", "height:100%;");
      DomElement *irow = DomElement::createNew(DomElement_TR);
      DomElement *itd = DomElement::createNew(DomElement_TD);
      if (fitHeight)
	itd->setAttribute("style", "height:100%;");
      itd->addChild(c);
      irow->addChild(itd);
      itable->addChild(irow);
      itable->setId(formName() + "l");
      c = itable;

      break;
    }
    case AlignLeft:
      //c->setProperty(PropertyStyleFloat, "left");
      break;
    case AlignRight:
      c->setProperty(PropertyStyleFloat, "right");
      break;
    }

    parent.addChild(c);
    
    flags_.reset(BIT_LAYOUT_CHANGED);
  } else {
    for (unsigned i = 0; i < children_->size(); ++i) {
      WWidget *w = (*children_)[i];
      parent.addChild(w->webWidget()->createSDomElement(app));
    }

    if (transientImpl_)
      transientImpl_->addedChildren_.clear();
  }
}

void WContainerWidget::rootAsJavaScript(WApplication *app, std::ostream& out,
					bool all)
{
  prepareRerender();

  std::vector<WWidget *> *toAdd
    = all ? children_ : (transientImpl_ ? &transientImpl_->addedChildren_ : 0);

  if (toAdd)
    for (unsigned i = 0; i < toAdd->size(); ++i) {
      DomElement *c = (*toAdd)[i]->webWidget()->createSDomElement(app);
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
    /* ignore rendering of deletion of a bound widget... */
    if (false && transientImpl_
	&& !transientImpl_->childRemoveChanges_.empty()) {
      EscapeOStream sout(out);
      for (unsigned i = 0; i < transientImpl_->childRemoveChanges_.size();
	   ++i) {
	DomElement *c = transientImpl_->childRemoveChanges_[i];
	c->asJavaScript(sout, DomElement::Delete);
	delete c;
      }

      transientImpl_->childRemoveChanges_.clear();
    }
  }

  propagateRenderOk(false);
}

void WContainerWidget::layoutChanged(bool deleted)
{
  if (!flags_.test(BIT_LAYOUT_CHANGED)) {
    if (!transientImpl_)
      transientImpl_ = new TransientImpl();

    std::string fn = (contentAlignment_ & 0x0F) == AlignCenter ?
      formName() + "l" : layoutImpl()->formName();

    DomElement *e = DomElement::getForUpdate(fn, DomElement_TABLE);
    e->removeFromParent();
    transientImpl_->childRemoveChanges_.push_back(e);

    flags_.set(BIT_LAYOUT_CHANGED);

    repaint(RepaintInnerHtml);
  }

  if (deleted)
    layout_ = 0;
}

}
