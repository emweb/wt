/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <string>
#include <boost/lexical_cast.hpp>

#include "Wt/WResource"
#include "Wt/WApplication"
#include "Wt/WEnvironment"

#include "WebRequest.h"
#include "WebSession.h"
#include "WtRandom.h"
#include "WtException.h"

namespace Wt {

WResource::WResource(WObject* parent)
  : WObject(parent),
    dataChanged(this),
    reentrant_(false),
    webRequest_(0)
{ }

WResource::~WResource()
{ 
  if (wApp)
    wApp->removeExposedResource(this);
}

void WResource::setArguments(const ArgumentMap&)
{ 
  if (!fileName_.empty())
    addHeader("Content-Disposition", "attachment;filename=" + fileName_);

  /*
   * A resource may always be cached: a change is indicated by generating a
   * new url
   */
  addHeader("Expires", "Sun, 14 Jun 2020 00:00:00 GMT");
  if (WApplication::instance()->environment().agentIE())
    addHeader("Cache-Control", "post-check=900,pre-check=3600");
  else
    addHeader("Cache-Control", "max-age=3600");
}

void WResource::flush()
{
  if (!webRequest_)
    throw WtException("WResource::flush(): last streamData() did not indicate "
		      "pending data");

  ArgumentMap arguments;
  bool done = streamResourceData(webRequest_->out(), arguments);
  webRequest_->setKeepConnectionOpen(!done);
  webRequest_->flush();

  if (done)
    setRequest(0);
}

void WResource::setReentrant(bool how)
{
  reentrant_ = how;
}

void WResource::setRequest(WebRequest *request)
{
  if (webRequest_ && request) {
    webRequest_->setKeepConnectionOpen(false);
    webRequest_->flush();
  }

  webRequest_ = request;
}

void WResource::suggestFileName(const std::string& name)
{
  fileName_ = name;
}

void WResource::addHeader(const std::string& name, const std::string& value)
{
  if (webRequest_) {
    webRequest_->addHeader(name, value);
  } else
    throw WtException("WResource::setHeader() must be called from within "
		      "setArguments()");
}

const std::string WResource::generateUrl() const
{
  WApplication *app = WApplication::instance();

  return app->addExposedResource(const_cast<WResource *>(this));
}

void WResource::write(std::ostream& out)
{
  ArgumentMap arguments;
  streamResourceData(out, arguments);
}

}
