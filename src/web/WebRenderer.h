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
class WebResponse;
class WebStream;
class DomElement;
class FileServe;

class WApplication;
class WWidget;
class WWebWidget;
class WObject;
class WResource;
class WStatelessSlot;
class WWidget;

class WT_API WebRenderer : public Wt::SlotLearnerInterface
{
public:
  typedef std::map<std::string, WObject *> FormObjectsMap;

  WebRenderer(WebSession& session);

  void setTwoPhaseThreshold(int bytes);

  bool visibleOnly() const { return visibleOnly_; }
  void setVisibleOnly(bool how) { visibleOnly_ = how; }

  void needUpdate(WWidget *w, bool laterOnly);
  void doneUpdate(WWidget *w);
  void updateFormObjects(WWebWidget *w, bool checkDescendants);

  void updateFormObjectsList(WApplication *app);
  const FormObjectsMap& formObjects() const;

  void saveChanges();
  void discardChanges();
  void letReloadJS(WebResponse& request, bool newSession,
		   bool embedded = false);
  void letReloadHTML(WebResponse& request, bool newSession);

  bool isDirty() const;

  void serveResponse(WebResponse& request);
  void serveError(WebResponse& request, const std::exception& error);
  void serveError(WebResponse& request, const std::string& message);

  void setCookie(const std::string name, const std::string value,
		 int maxAge, const std::string domain,
		 const std::string path);

  bool preLearning() const { return learning_; }
  void learningIncomplete();

  void ackUpdate(int updateId);

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
  bool        visibleOnly_, rendered_;
  int         twoPhaseThreshold_;
  int         expectedAckId_;

  std::vector<Cookie> cookiesToSet_;

  FormObjectsMap currentFormObjects_;
  std::string	 currentFormObjectsList_;
  bool           formObjectsChanged_;

  void setHeaders(WebResponse& request, const std::string mimeType);

  void serveJavaScriptUpdate(WebResponse& response);
  void serveMainscript(WebResponse& response);
  void serveBootstrap(WebResponse& request);
  void serveMainpage(WebResponse& response);
  void serveMainAjax(WebResponse& response);
  void serveWidgetSet(WebResponse& request);
  void streamCommJs(WApplication *app, std::ostream& out);
  void collectJavaScript();

  void collectChanges(std::vector<DomElement *>& changes);

  void collectJavaScriptUpdate(std::ostream& out);
  void loadStyleSheets(std::ostream& out, WApplication *app);
  void loadScriptLibraries(std::ostream& out, WApplication *app, bool start);
  void updateLoadIndicator(std::ostream& out, WApplication *app, bool all);
  void setJSSynced(bool invisibleToo);

  std::string createFormObjectsList(WApplication *app);

  void              preLearnStateless(WApplication *app, std::ostream& out);
  std::stringstream collectedJS1_, collectedJS2_, invisibleJS_, statelessJS_,
                    beforeLoadJS_;
  void              collectJS(std::ostream *js);

  void setPageVars(FileServe& page);
  void setBootVars(WebResponse& response, FileServe& boot);

  std::string headDeclarations() const;

  typedef std::set<WWidget *> UpdateMap;
  UpdateMap updateMap_;
  bool      learning_, learningIncomplete_;
  bool      moreUpdates_;

  std::string safeJsStringLiteral(const std::string& value);

public:
  std::string       learn(WStatelessSlot* slot);

  friend class WApplication;
};

}

#endif // WEBRENDERER_H_
