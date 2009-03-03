/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WWidget"
#include "Wt/WWebWidget"
#include "Wt/WContainerWidget"
#include "Wt/WLayout"
#include "Wt/WJavaScript"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WtException.h"

namespace Wt {

#ifndef WIN32
/*
 * Why do we need these and not the others ?
 */
const Side WWidget::Top;
const Side WWidget::Left;
#endif

WWidget::WWidget(WContainerWidget* parent)
  : WResource(0)
{
  implementStateless(&WWidget::hide, &WWidget::undoHideShow);
  implementStateless(&WWidget::show, &WWidget::undoHideShow);
}

WWidget::~WWidget()
{ }

void WWidget::setParent(WWidget *p)
{
  if (parent())
    parent()->removeChild(this);
  if (p)
    p->addChild(this);
}

void WWidget::setStyleClass(const char *value)
{
  setStyleClass(WString(value, UTF8));
}

void WWidget::hide()
{
  wasHidden_ = isHidden();
  setHidden(true);
}

void WWidget::show()
{ 
  wasHidden_ = isHidden();
  setHidden(false);
}

void WWidget::undoHideShow()
{
  setHidden(wasHidden_);
}

void WWidget::setOffset(int sides, const WLength& length)
{
  setOffsets(length, sides);
}

std::string WWidget::jsRef() const
{
  return WT_CLASS ".getElement('" + formName() + "')";
}

const std::string WWidget::resourceMimeType() const
{
  return "text/html; charset=utf-8";
}

void WWidget::htmlText(std::ostream& out)
{
  DomElement *element
    = webWidget()->createSDomElement(WApplication::instance());
  DomElement::TimeoutList timeouts;
  EscapeOStream sout(out);
  element->asHTML(sout, timeouts);
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

void WWidget::setArguments(const ArgumentMap& arguments)
{ }

bool WWidget::streamResourceData(std::ostream& stream,
				 const ArgumentMap& arguments)
{
  stream 
    << "<!DOCTYPE html PUBLIC "
       "\"-//W3C//DTD HTML 4.01 Transitional//EN\" "
       "\"http://www.w3.org/TR/html4/loose.dtd\">"
       "<html lang=\"en\" dir=\"ltr\">\n"
       "<head><title></title>\n"
       "<script type=\"text/javascript\">\n"
       "function load() { ";

  if (!resourceTriggerUpdate_.empty())
    stream << "window.parent."
	   << WApplication::instance()->javaScriptClass()
	   << "._p_.update(null, '" << resourceTriggerUpdate_
	   << "', null, true);";
  stream
    << "}\n"
       "</script></head>"
       "<body onload=\"load();\""
    << " style=\"margin:0;padding:0;\">";

  resourceTriggerUpdate_.clear();
  htmlText(stream);

  stream << "</body></html>"; 

  return true;
}

WWidget *WWidget::adam()
{
  if (parent())
    return parent()->adam();
  else
    return this;
}

WString WWidget::tr(const char *key)
{
  return WString::tr(key);
}

void WWidget::acceptDrops(const std::string& mimeType,
			  const WString& hoverStyleClass)
{
  WWebWidget *thisWebWidget = webWidget();

  if (thisWebWidget->setAcceptDropsImpl(mimeType, true, hoverStyleClass)) {
    thisWebWidget->otherImpl_
      ->dropSignal_->connect(SLOT(this, WWidget::getDrop));
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

std::string WWidget::createJavaScript(std::stringstream& js,
				      const std::string& insertJS)
{
  DomElement *de = webWidget()->createSDomElement(WApplication::instance());
  std::string var = de->asJavaScript(js, true);
  if (!insertJS.empty())
    js << insertJS << var << ");";
  de->asJavaScript(js, false);
  delete de;

  return var;
}

void WWidget::setLayout(WLayout *layout)
{ 
  layout->setParent(this);
}

WLayout *WWidget::layout()
{
  return 0;
}

WLayoutItemImpl *WWidget::createLayoutItemImpl(WLayoutItem *layoutItem)
{
  throw WtException("WWidget::setLayout(): widget does not support "
		    "layout managers");
}

}
