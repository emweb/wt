// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBRENDERER_H_
#define WEBRENDERER_H_

#include <string>
#include <sstream>
#include <vector>
#include <set>
#include "Wt/WEnvironment"
#include "Wt/WStatelessSlot"

namespace Wt {

class WebRequest;
class WebStream;
class DomElement;

class WApplication;
class WWidget;
class WObject;
class WResource;
class WStatelessSlot;
class WWebWidget;

class WT_API WebRenderer : public Wt::SlotLearnerInterface
{
public:
  WebRenderer(WebSession& session);

  void setTwoPhaseThreshold(int bytes);

  bool visibleOnly() const { return visibleOnly_; }
  void setVisibleOnly(bool how) { visibleOnly_ = how; }

  void needUpdate(WWebWidget *w);
  void doneUpdate(WWebWidget *w);
  void updateFormObjects(WWebWidget *w, bool checkDescendants);

  enum ResponseType { UpdateResponse, FullResponse };

  const std::vector<WObject *>& formObjects() const;

  void prepare();
  void saveChanges();
  void discardChanges();
  void letReloadJS(WebRequest& request, bool newSession, bool embedded = false);
  void letReloadHTML(WebRequest& request, bool newSession);

  void streamJavaScriptUpdate(std::ostream& out, int id, bool doTwoPhaze);

  void serveMainWidget(WebRequest& request, ResponseType responseType);
  void serveBootstrap(WebRequest& request);
  void serveError(WebRequest& request, const std::exception& error,
		  ResponseType responseType);
  void serveError(WebRequest& request, const std::string& message,
		  ResponseType responseType);

  void setCookie(const std::string name, const std::string value,
		 int maxAge, const std::string domain,
		 const std::string path);

  static std::string appSessionCookie(std::string url);

  bool preLearning() const { return learning_; }

  void streamRedirectJS(std::ostream& out, const std::string& redirect);

private:
  struct Cookie {
    std::string name;
    std::string value;
    std::string path;
    std::string domain;
    int maxAge;

    Cookie(std::string n, std::string v, std::string p, std::string d, int m)
      : name(n), value(v), path(p), domain(d), maxAge(m) { }
  };

  WebSession& session_;
  bool        visibleOnly_;
  int         twoPhaseThreshold_;

  std::vector<Cookie> cookiesToSet_;

  std::set<WWebWidget *> rootWebWidgets_;
  std::vector<WObject *> currentFormObjects_;
  std::string		 currentFormObjectsList_;
  bool                   formObjectsChanged_;

  void setHeaders(WebRequest& request, const std::string mimeType);

  void serveJavaScriptUpdate(WebRequest& request);
  void serveMainpage(WebRequest& request);
  void serveMainscript(WebRequest& request);
  void serveWidgetSet(WebRequest& request);
  void streamCommJs(WApplication *app, std::ostream& out);

  void collectChanges(std::vector<DomElement *>& changes);

  void collectJavaScriptUpdate(std::ostream& out);
  void loadStyleSheets(std::ostream& out, WApplication *app);
  void loadScriptLibraries(std::ostream& out, WApplication *app, bool start);
  void updateLoadIndicator(std::ostream& out, WApplication *app, bool all);

  std::string createFormObjectsList(WApplication *app);

  std::string       learn(WStatelessSlot* slot);
  void              preLearnStateless(WApplication *app);
  std::stringstream collectedChanges_;
  void              collectJS(std::ostream *js);

  typedef std::set<WWebWidget *> UpdateMap;
  UpdateMap updateMap_;
  bool      learning_;
};

}

#endif // WEBRENDERER_H_
