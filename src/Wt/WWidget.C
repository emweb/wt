/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WWidget"
#include "Wt/WWebWidget"
#include "Wt/WCompositeWidget"
#include "Wt/WContainerWidget"
#include "Wt/WLayout"
#include "Wt/WJavaScript"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WebRenderer.h"
#include "WebSession.h"
#include "WtException.h"

namespace Wt {

const char *WWidget::WT_RESIZE_JS = "wtResize";

WWidget::WWidget(WContainerWidget* parent)
  : WObject(0),
    resized_(0)
{ 
  flags_.set(BIT_NEED_RERENDER);
}

WWidget::~WWidget()
{
  while (!eventSignals_.empty()) {
#ifndef WT_NO_BOOST_INTRUSIVE
    EventSignalBase *s = &eventSignals_.front();
#else
    EventSignalBase *s = eventSignals_.front();
#endif
    eventSignals_.pop_front();
#ifndef WT_TARGET_JAVA
    delete s;
#else
    s->~EventSignalBase();
#endif
  }

  delete resized_;
  resized_ = 0;

  renderOk();
}

void WWidget::setParentWidget(WWidget *p)
{
  if (p == parent())
    return;

  if (parent())
    parent()->removeChild(this);
  if (p)
    p->addChild(this);
}

void WWidget::refresh()
{
  setJsSize();
}

void WWidget::resize(const WLength& width, const WLength& height)
{
  setJsSize();
}

void WWidget::setJsSize()
{
  if (!height().isAuto() && height().unit() != WLength::Percentage
      && !javaScriptMember(WT_RESIZE_JS).empty())
    callJavaScriptMember
      (WT_RESIZE_JS, jsRef() + ","
       + boost::lexical_cast<std::string>(width().toPixels()) + ","
       + boost::lexical_cast<std::string>(height().toPixels()));
}

void WWidget::render(WFlags<RenderFlag> flags)
{ }

bool WWidget::isRendered() const
{
  WWidget *self = const_cast<WWidget *>(this);
  return self->webWidget()->isRendered();
}

void WWidget::renderOk()
{
  if (flags_.test(BIT_NEED_RERENDER)) {
    flags_.reset(BIT_NEED_RERENDER);
    WApplication *app = WApplication::instance();
    if (app)
      app->session()->renderer().doneUpdate(this);
  }
}

void WWidget::askRerender(bool laterOnly)
{
  if (!flags_.test(BIT_NEED_RERENDER)) {
    flags_.set(BIT_NEED_RERENDER);
    WApplication::instance()->session()->renderer().needUpdate(this, laterOnly);

    /*
     * Let's start with assuming that every change is a potential resize
     *
     * Propagate event up, this will be caught by a container widget
     * with a layout manager.
     */
    WWidget *p = parent();

    if (p)
      p->childResized(this, Vertical);
  }
}

void WWidget::childResized(WWidget *child, WFlags<Orientation> directions)
{
  WWidget *p = parent();

  if (p)
    p->childResized(this, directions);
}

void WWidget::setStyleClass(const char *styleClass)
{
  setStyleClass(WString::fromUTF8(styleClass));
}

void WWidget::addStyleClass(const char *styleClass, bool force)
{
  addStyleClass(WString::fromUTF8(styleClass), force);
}

void WWidget::removeStyleClass(const char *styleClass, bool force)
{
  removeStyleClass(WString::fromUTF8(styleClass), force);
}

void WWidget::hide()
{
  flags_.set(BIT_WAS_HIDDEN, isHidden());
  setHidden(true);
}

void WWidget::show()
{ 
  flags_.set(BIT_WAS_HIDDEN, isHidden());
  setHidden(false);
}

void WWidget::disable()
{
  flags_.set(BIT_WAS_DISABLED, isDisabled());
  setDisabled(true);
}

void WWidget::enable()
{ 
  flags_.set(BIT_WAS_DISABLED, isDisabled());
  setDisabled(false);
}

#ifndef WT_TARGET_JAVA
WStatelessSlot *WWidget::getStateless(Method method)
{
  if (method == static_cast<WObject::Method>(&WWidget::hide))
    return implementStateless(&WWidget::hide, &WWidget::undoHideShow);
  else if (method == static_cast<WObject::Method>(&WWidget::show))
    return implementStateless(&WWidget::show, &WWidget::undoHideShow);
  else if (method == static_cast<WObject::Method>(&WWidget::enable))
    return implementStateless(&WWidget::enable, &WWidget::undoDisableEnable);
  else if (method == static_cast<WObject::Method>(&WWidget::disable))
    return implementStateless(&WWidget::disable, &WWidget::undoDisableEnable);
  else
    return WObject::getStateless(method);
}
#endif // WT_TARGET_JAVA

void WWidget::undoHideShow()
{
  setHidden(flags_.test(BIT_WAS_HIDDEN));
}

void WWidget::undoDisableEnable()
{
  setDisabled(flags_.test(BIT_WAS_DISABLED));
}

#ifdef WT_TARGET_JAVA
void WWidget::resize(int widthPixels, int heightPixels)
{
  resize(WLength(widthPixels), WLength(heightPixels));
}

void WWidget::setMargin(int pixels, WFlags<Side> sides)
{
  setMargin(WLength(pixels), sides);
}

void WWidget::setOffsets(int pixels, WFlags<Side> sides)
{
  setOffsets(WLength(pixels), sides);
}
#endif // WT_TARGET_JAVA

std::string WWidget::jsRef() const
{
  return "$('#" + id() + "').get(0)";
}

void WWidget::htmlText(std::ostream& out)
{
  DomElement *element = createSDomElement(WApplication::instance());

  DomElement::TimeoutList timeouts;
  EscapeOStream sout(out);
  EscapeOStream js;
  element->asHTML(sout, js, timeouts);

  WApplication::instance()->doJavaScript(js.str());

  delete element;
}

std::string WWidget::inlineCssStyle()
{
  WWebWidget *ww = webWidget();
  DomElement *e = DomElement::getForUpdate(ww, ww->domElementType());
  ww->updateDom(*e, true);
  std::string result = e->cssStyle();
  delete e;
  return result;
}

WWidget *WWidget::adam()
{
  WWidget *p = parent();
  return p ? p->adam() : this;
}

WString WWidget::tr(const char *key)
{
  return WString::tr(key);
}

WString WWidget::tr(const std::string& key)
{
  return WString::tr(key);
}

void WWidget::acceptDrops(const std::string& mimeType,
			  const WT_USTRING& hoverStyleClass)
{
  WWebWidget *thisWebWidget = webWidget();

  if (thisWebWidget->setAcceptDropsImpl(mimeType, true, hoverStyleClass)) {
    thisWebWidget->otherImpl_->dropSignal_->connect(this, &WWidget::getDrop);
  }
}

void WWidget::stopAcceptDrops(const std::string& mimeType)
{
  WWebWidget *thisWebWidget = webWidget();

  thisWebWidget->setAcceptDropsImpl(mimeType, false, "");
}

void WWidget::getDrop(const std::string sourceId, const std::string mimeType,
		      WMouseEvent event)
{
  WDropEvent e(WApplication::instance()->decodeObject(sourceId), mimeType,
	       event);

  dropEvent(e);
}

void WWidget::dropEvent(WDropEvent event)
{ }

DomElement *WWidget::createSDomElement(WApplication *app)
{
  if (!needsToBeRendered()) {
    DomElement *result = webWidget()->createStubElement(app);
    renderOk();
    askRerender(true);
    return result;
  } else {
    webWidget()->setRendered(true);
    render(RenderFull);
    return webWidget()->createActualElement(app);
  }
}

std::string WWidget::createJavaScript(std::stringstream& js,
				      std::string insertJS)
{
  WApplication *app = WApplication::instance();
  DomElement *de = createSDomElement(app);

  std::string var = de->createVar();
  if (!insertJS.empty())
    insertJS += var + ");";
  de->createElement(js, app, insertJS);

  delete de;

  return var;
}

void WWidget::setLayout(WLayout *layout)
{
  layout->setParentWidget(this);
}

WLayout *WWidget::layout()
{
  return 0;
}

WLayoutItemImpl *WWidget::createLayoutItemImpl(WLayoutItem *item)
{
  throw WtException("WWidget::setLayout(): widget does not support "
		    "layout managers");
}

void WWidget::addEventSignal(EventSignalBase& s)
{
#ifndef WT_NO_BOOST_INTRUSIVE
  eventSignals_.push_back(s);
#else
  eventSignals_.push_back(&s);
#endif
}

EventSignalBase *WWidget::getEventSignal(const char *name)
{
  for (EventSignalList::iterator i = eventSignals_.begin();
       i != eventSignals_.end(); ++i) {
#ifndef WT_NO_BOOST_INTRUSIVE
    EventSignalBase& s = *i;
#else
    EventSignalBase& s = **i;
#endif
    if (s.name() == name)
      return &s;
  }

  return 0;
}

int WWidget::boxPadding(Orientation orientation) const
{
  return 0;
}

int WWidget::boxBorder(Orientation orientation) const
{
  return 0;
}

void WWidget::positionAt(const WWidget *widget, Orientation orientation)
{
  std::string side = (orientation == Horizontal ? ".Horizontal" : ".Vertical");

  WApplication::instance()->doJavaScript
    (WT_CLASS ".positionAtWidget('"
     + id() + "','"
     + widget->id() + "',"
     WT_CLASS + side + ");");
}

void WWidget::setLayoutSizeAware(bool aware)
{
  if (aware == (resized_ != 0))
    return;

  if (aware && WApplication::instance()) {
    resized_ = new JSignal<int, int>(this, "resized");
    resized_->connect(this, &WContainerWidget::layoutSizeChanged);

    setJavaScriptMember
      (WT_RESIZE_JS,
       "function(self, w, h) {"
       ""  "if (!self.wtWidth || self.wtWidth!=w "
       ""      "|| !self.wtHeight || self.wtHeight!=h) {"
       ""    "self.wtWidth=w; self.wtHeight=h;"
       ""    "self.style.height=h + 'px';"
       + resized_->createCall("Math.round(w)", "Math.round(h)") +
       ""  "}"
       "};");
  } else {
    if (!javaScriptMember(WT_RESIZE_JS).empty())
      setJavaScriptMember(WT_RESIZE_JS, "");
    delete resized_;
    resized_ = 0;
  }
}

bool WWidget::layoutSizeAware() const
{
  return resized_ != 0;
}

void WWidget::layoutSizeChanged(int width, int height)
{ } 

bool WWidget::isInLayout() const
{
  WWidget *p = parent();
  if (p != 0 &&
      (dynamic_cast<WCompositeWidget *>(p) != 0 ||
       !p->javaScriptMember(WT_RESIZE_JS).empty()))
    return p->isInLayout();

  WContainerWidget *c = dynamic_cast<WContainerWidget *>(p);

  return c != 0 && c->layout() != 0;
}

void WWidget::setTabOrder(WWidget *first, WWidget *second)
{
  second->setTabIndex(first->tabIndex() + 1);
}

void WWidget::setHasParent(bool hasParent)
{
  flags_.set(BIT_HAS_PARENT, hasParent);

  setParent(parent()); // since hasParent() has changed
}

bool WWidget::hasParent() const
{
  if (flags_.test(BIT_HAS_PARENT))
    return true;
  else
    return WObject::hasParent();
}

}
