/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WLink>

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WException>
#include <Wt/WInteractWidget>
#include <Wt/WResource>
#include <Wt/WString>

#include <boost/algorithm/string.hpp>

#include "WebSession.h"

namespace Wt {

WLink::WLink()
  : type_(Url)
{ 
#ifdef WT_TARGET_JAVA
  setUrl("");
#endif
}

WLink::WLink(const char *url)
{ 
  setUrl(url);
}

WLink::WLink(const std::string& url)
{ 
  setUrl(url);
}

WLink::WLink(Type type, const std::string& value)
{
  switch (type) {
  case Url: setUrl(value); break;
  case InternalPath: setInternalPath(WString::fromUTF8(value)); break;
  default:
    throw WException("WLink::WLink(type) cannot be used for a Resource");
  }
}

WLink::WLink(WResource *resource)
{
  setResource(resource);
}

bool WLink::isNull() const
{
  return type_ == Url && url().empty();
}

void WLink::setUrl(const std::string& url)
{
  type_ = Url;
  value_ = url;
}

std::string WLink::url() const
{
  switch (type_) {
  case Url:
#ifndef WT_CNOR
    return boost::get<std::string>(value_);
#else
    return boost::any_cast<std::string>(value_);
#endif
  case Resource:
    return resource()->url();
  case InternalPath:
    return WApplication::instance()->bookmarkUrl(internalPath().toUTF8());
  }

  return std::string();
}

void WLink::setResource(WResource *resource)
{
  type_ = Resource;
  value_ = resource;
}

WResource *WLink::resource() const
{
  if (type_ == Resource) {
#ifndef WT_CNOR
    return boost::get<WResource *>(value_);
#else
    return boost::any_cast<WResource *>(value_);
#endif
  } else
    return 0;
}

void WLink::setInternalPath(const WT_USTRING& internalPath)
{
  type_ = InternalPath;
  std::string path = internalPath.toUTF8();

  if (boost::starts_with(path, "#/"))
    path = path.substr(1);

  value_ = path;
}

WT_USTRING WLink::internalPath() const
{
  if (type_ == InternalPath) {
#ifndef WT_CNOR
    return WT_USTRING::fromUTF8(boost::get<std::string>(value_));
#else
    return WT_USTRING::fromUTF8(boost::any_cast<std::string>(value_));
#endif
  } else {
#ifndef WT_TARGET_JAVA
    return WT_USTRING::Empty;
#else
    return std::string();
#endif
  }
}

bool WLink::operator==(const WLink& other) const
{
  return type_ == other.type_ && value_ == other.value_;
}

bool WLink::operator!=(const WLink& other) const
{
  return !(*this == other);
}

std::string WLink::resolveUrl(WApplication *app) const
{
  std::string relativeUrl;

  switch (type_) {
  case InternalPath: {
    if (app->environment().ajax())
      relativeUrl = app->bookmarkUrl(internalPath().toUTF8());
    else if (app->environment().agentIsSpiderBot())
      relativeUrl = app->bookmarkUrl(internalPath().toUTF8());
    else
      // If no JavaScript is available, then we still add the session
      // so that when used in WAnchor it will be handled by the same
      // session.
      relativeUrl = app->session()->mostRelativeUrl(internalPath().toUTF8());
  }
    break;
  default:
    relativeUrl = url();
  }

  return app->resolveRelativeUrl(relativeUrl);
}

JSlot *WLink::manageInternalPathChange(WApplication *app,
				       WInteractWidget *widget,
				       JSlot *slot) const
{
  if (type_ == InternalPath) {
    if (app->environment().ajax()) {
      if (!slot) {
	slot = new JSlot();
	widget->clicked().connect(*slot);
	widget->clicked().preventDefaultAction();
      }

      slot->setJavaScript
	("function(){" +
	 app->javaScriptClass() + "._p_.setHash("
	 + WWebWidget::jsStringLiteral(internalPath()) + ",true);"
	 "}");

#ifdef WT_TARGET_JAVA
      widget->clicked().senderRepaint();
#endif // WT_TARGET_JAVA

      return slot;
    }
  }

  delete slot;

  return 0;
}

}
