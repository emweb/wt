/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Process.h"

#include "Service.h"

#include "ProcessImpl.h"
#include "ServiceImpl.h"

#include "Wt/PopupWindow.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WResource.h"

#include "Wt/Auth/Identity.h"

#include "Wt/Http/Client.h"
#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"

#include "web/WebUtils.h"

namespace Wt {

LOGGER("Auth.Saml.Process");

}

namespace Wt {
namespace Auth {
namespace Saml {

class Process::AuthnRequestResource final : public WResource {
public:
  explicit AuthnRequestResource(Process &process);

  ~AuthnRequestResource() final;

protected:
  void handleRequest(const Http::Request &request, Http::Response &response) final;

private:
  Process &process_;
};

class Process::PrivateAcsResource final : public WResource {
public:
  explicit PrivateAcsResource(Process &process);

  ~PrivateAcsResource() final;

protected:
  void handleRequest(const Http::Request &request, Http::Response &response) final;

private:
  Process &process_;
};

Process::AuthnRequestResource::AuthnRequestResource(Process &process)
  : process_(process)
{
  setTakesUpdateLock(true);
}

Process::AuthnRequestResource::~AuthnRequestResource()
{
  beingDeleted();
}

void Process::AuthnRequestResource::handleRequest(const Http::Request &,
                                                  Http::Response &response)
{
  bool success = process_.createAuthnRequest(response);
  if (!success) {
    response.setStatus(500);
    response.setMimeType("text/html");
    response.out() << "<html><body>"
                   << "<h1>SAML Authentication error</h1>"
                   << "</body></html>";
  }
}

Process::PrivateAcsResource::PrivateAcsResource(Process &process)
  : process_(process)
{
  setTakesUpdateLock(true);
}

Process::PrivateAcsResource::~PrivateAcsResource()
{
  beingDeleted();
}

void Process::PrivateAcsResource::handleRequest(const Http::Request &request,
                                                Http::Response &response)
{
  if (process_.handleResponse(request)) {
#ifndef WT_TARGET_JAVA
    std::ostream &o = response.out();
#else // WT_TARGET_JAVA
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA
    WApplication *app = WApplication::instance();
    const bool usePopup = app->environment().ajax() && process_.service_.popupEnabled();

    if (!usePopup) {
#ifndef WT_TARGET_JAVA
      WApplication::UpdateLock lock(app);
#endif
      process_.doneCallbackConnection_ =
        app->unsuspended().connect(&process_, &Process::onSamlDone);

      std::string redirectTo = app->makeAbsoluteUrl(app->url(process_.startInternalPath_));
      response.setStatus(303);
      response.addHeader("Location", redirectTo);
    } else {
      std::string appJs = app->javaScriptClass();
      o <<
        "<!DOCTYPE html>"
        "<html lang=\"en\" dir=\"ltr\">\n"
        "<head><title></title>\n"
        "<script type=\"text/javascript\">\n"
        "function load() { "
        """if (window.opener." << appJs << ") {"
        ""  "var " << appJs << "= window.opener." << appJs << ";"
#ifndef WT_TARGET_JAVA
        << process_.redirected_.createCall({}) << ";"
#else // WT_TARGET_JAVA
        <<  process_.redirected_.createCall() << ";"
#endif // WT_TARGET_JAVA
        ""  "window.close();"
        "}\n"
        "}\n"
        "</script></head>"
        "<body onload=\"load();\"></body></html>";
    }
  } else {
    response.setStatus(500);
    response.setMimeType("text/html");
    response.out() << "<html><body>"
                   << "<h1>SAML Authentication error</h1>"
                   << "</body></html>";
  }
}

Process::Process(const Service &service)
  : service_(service),
    redirected_(this, "redirected")
{
  impl_ = std::make_unique<ProcessImpl>(*this);
  authnRequestResource_ = std::make_unique<AuthnRequestResource>(*this);
  privateAcsResource_ = std::make_unique<PrivateAcsResource>(*this);

  WApplication *app = WApplication::instance();
  PopupWindow::loadJavaScript(app);

  std::string url = app->makeAbsoluteUrl(authnRequestResource_->url());

  redirected_.connect(this, &Process::onSamlDone);

#ifndef WT_TARGET_JAVA
  if (service_.popupEnabled()) {
    WStringStream js;
    js << WT_CLASS << ".PopupWindow(" WT_CLASS
       << "," << WWebWidget::jsStringLiteral(url)
       << ", " << service.popupWidth()
       << ", " << service.popupHeight() << ");";

    implementJavaScript(&Process::startAuthenticate, js.str());
  }
#endif
}

Process::~Process()
{ }

void Process::startAuthenticate()
{
  WApplication *app = WApplication::instance();
  if (app->environment().javaScript() && service_.popupEnabled()) {
    return;
  }

  app->suspend(service_.redirectTimeout_);

  startInternalPath_ = app->internalPath();
  app->redirect(authnRequestResource_->url());
}

#ifdef WT_TARGET_JAVA
void Process::connectStartAuthenticate(EventSignalBase &s)
{
  WApplication *app = WApplication::instance();
  if (app->environment().javaScript()) {
    std::string url = app->makeAbsoluteUrl(authnRequestResource_->url());
    WStringStream js;
    js << "function(object, event) {"
       << WT_CLASS ".PopupWindow(" WT_CLASS
       << "," << WWebWidget::jsStringLiteral(url)
       << ", " << service_.popupWidth()
       << ", " << service_.popupHeight() << ");"
       << "}";

    s.connect(js.str());
  }

  s.connect(this, &Process::startAuthenticate);
}
#endif

bool Process::createAuthnRequest(Http::Response &response)
{
  return impl_->createAuthnRequest(response);
}

bool Process::handleResponse(const Http::Request &request)
{
  return impl_->handleResponse(request);
}

void Process::onSamlDone()
{
  bool success = error_.empty();

  authenticated().emit(success ? service_.assertionToIdentity(assertion_) : Identity());

  if (doneCallbackConnection_.isConnected())
    doneCallbackConnection_.disconnect();
}

std::string Process::privateAcsResourceUrl() const
{
  return privateAcsResource_->url();
}

}
}
}
