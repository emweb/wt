/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WLink.h>

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WException.h>
#include <Wt/WInteractWidget.h>
#include <Wt/WResource.h>
#include <Wt/WString.h>

#include <boost/algorithm/string.hpp>

#include "WebSession.h"

namespace Wt {

WLink::WLink()
  : type_(LinkType::Url),
    target_(LinkTarget::Self)
{ 
#ifdef WT_TARGET_JAVA
  setUrl("");
#endif
}

WLink::WLink(const char *url)
  : target_(LinkTarget::Self)
{ 
  setUrl(url);
}

WLink::WLink(const std::string& url)
  : target_(LinkTarget::Self)
{ 
  setUrl(url);
}

WLink::WLink(LinkType type, const std::string& value)
  : target_(LinkTarget::Self)
{
  switch (type) {
  case LinkType::Url: setUrl(value); break;
  case LinkType::InternalPath: setInternalPath(WString::fromUTF8(value)); break;
  default:
    throw WException("WLink::WLink(type) cannot be used for a Resource");
  }
}

WLink::WLink(const std::shared_ptr<WResource>& resource)
  : target_(LinkTarget::Self)
{
  setResource(resource);
}

bool WLink::isNull() const
{
  return type_ == LinkType::Url && url().empty();
}

void WLink::setUrl(const std::string& url)
{
  type_ = LinkType::Url;
  stringValue_ = url;
  resource_.reset();
}

std::string WLink::url() const
{
  switch (type_) {
  case LinkType::Url:
    return stringValue_;
  case LinkType::Resource:
    return resource()->url();
  case LinkType::InternalPath:
    return WApplication::instance()->bookmarkUrl(internalPath().toUTF8());
  }

  return std::string();
}

void WLink::setResource(const std::shared_ptr<WResource>& resource)
{
  type_ = LinkType::Resource;
  resource_ = resource;
  stringValue_.clear();
}

std::shared_ptr<WResource> WLink::resource() const
{
  return resource_;
}

void WLink::setInternalPath(const WT_USTRING& internalPath)
{
  type_ = LinkType::InternalPath;
  std::string path = internalPath.toUTF8();

  if (boost::starts_with(path, "#/"))
    path = path.substr(1);

  stringValue_ = path;
  resource_.reset();
}

WT_USTRING WLink::internalPath() const
{
  if (type_ == LinkType::InternalPath) {
    return WT_USTRING::fromUTF8(stringValue_);
  } else {
#ifndef WT_TARGET_JAVA
    return WT_USTRING::Empty;
#else
    return std::string();
#endif
  }
}

void WLink::setTarget(LinkTarget target)
{
  target_ = target;
}

bool WLink::operator==(const WLink& other) const
{
  return type_ == other.type_ &&
    stringValue_ == other.stringValue_ &&
    resource_ == other.resource_;
}

bool WLink::operator!=(const WLink& other) const
{
  return !(*this == other);
}

std::string WLink::resolveUrl(WApplication *app) const
{
  std::string relativeUrl;

  switch (type_) {
  case LinkType::InternalPath: {
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
  if (type_ == LinkType::InternalPath) {
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
