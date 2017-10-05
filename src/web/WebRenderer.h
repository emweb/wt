// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEBRENDERER_H_
#define WEBRENDERER_H_

#include <string>
#include <vector>
#include <set>

#include "Wt/WDateTime.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStatelessSlot.h"

namespace Wt {

class WebRequest;
class WebResponse;
class WebStream;
class DomElement;
class EscapeOStream;
class FileServe;
class MetaHeader;

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
  bool isRendered() const { return rendered_; }
  void setRendered(bool how);

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
  int scriptId() const { return scriptId_; }
  int pageId() const { return pageId_; }

  void serveResponse(WebResponse& request);
  void serveError(int status, WebResponse& request, 
		  const std::string& message);
  void serveLinkedCss(WebResponse& request);

  void setCookie(const std::string name, const std::string value,
		 const WDateTime& expires, const std::string domain,
		 const std::string path, bool secure);

  bool preLearning() const { return learning_; }
  void learningIncomplete();

  void updateLayout() { updateLayout_ = true; }

  enum AckState {
    CorrectAck,
    ReasonableAck,
    BadAck
  };
  
  AckState ackUpdate(int updateId);

  void streamRedirectJS(WStringStream& out, const std::string& redirect);

  bool checkResponsePuzzle(const WebRequest& request);

  bool jsSynced() const;

  void setJSSynced(bool invisibleToo);

  void setStatelessSlotNotStateless() { currentStatelessSlotIsActuallyStateless_ = false; }

  // Keep stubbed widget in vector marking it as stubbed,
  // so we don't discard the JavaScript effects of a stateless
  // slot executed before the widget was unstubbed.
  void markAsStubbed(const WWidget *widget);

  // Check whether the given object (which should be a widget)
  // was marked as stubbed.
  bool wasStubbed(const WObject *widget) const;

private:
  struct CookieValue {
    std::string value;
    std::string path;
    std::string domain;
    WDateTime expires;
    bool secure;

    CookieValue();
    CookieValue(const std::string& v, const std::string& p,
		const std::string& d, const WDateTime& e, bool secure);
  };

  WebSession& session_;

  bool visibleOnly_, rendered_, initialStyleRendered_;
  int twoPhaseThreshold_, pageId_, expectedAckId_, scriptId_, ackErrs_;
  int linkedCssCount_;
  std::string solution_;

  bool currentStatelessSlotIsActuallyStateless_;

  std::map<std::string, CookieValue> cookiesToSet_;

  FormObjectsMap currentFormObjects_;
  std::string currentFormObjectsList_;
  bool formObjectsChanged_;
  bool updateLayout_;

  std::vector<int> wsRequestsToHandle_;
  bool multiSessionCookieUpdateNeeded_;

  // A vector of all widgets that were stubbed.
  // Kept around until the first event after they're unstubbed,
  // then the vector is cleared.
  std::vector<const WWidget*> stubbedWidgets_;

  void setHeaders(WebResponse& request, const std::string mimeType);
  void setCaching(WebResponse& response, bool allowCache);

  void serveJavaScriptUpdate(WebResponse& response);
  void serveMainscript(WebResponse& response);
  void serveBootstrap(WebResponse& request);
  void serveMainpage(WebResponse& response);
  void serveMainAjax(WStringStream& out);
  void serveWidgetSet(WebResponse& request);
  void collectJavaScript();

  void collectChanges(std::vector<DomElement *>& changes);

  void collectJavaScriptUpdate(WStringStream& out);
  void loadStyleSheet(WStringStream& out, WApplication *app,
		      const WLinkedCssStyleSheet& sheet);
  void loadStyleSheets(WStringStream& out, WApplication *app);
  void removeStyleSheets(WStringStream& out, WApplication *app);
  int loadScriptLibraries(WStringStream& out, WApplication *app,
			  int count = -1);
  void updateLoadIndicator(WStringStream& out, WApplication *app, bool all);
  void renderSetServerPush(WStringStream& out);
  void renderStyleSheet(WStringStream& out, const WLinkedCssStyleSheet& sheet,
			WApplication *app);

  std::string createFormObjectsList(WApplication *app);

  void preLearnStateless(WApplication *app, WStringStream& out);
  WStringStream collectedJS1_, collectedJS2_, invisibleJS_, statelessJS_,
    beforeLoadJS_;
  void collectJS(WStringStream *js);

  void setPageVars(FileServe& page);
  void streamBootContent(WebResponse& response, 
			 FileServe& boot, bool hybrid);
  void addResponseAckPuzzle(WStringStream& out);
  void addContainerWidgets(WWebWidget *w,
			   std::vector<WContainerWidget *>& v);

  std::string headDeclarations() const;
  std::string bodyClassRtl() const;
  std::string sessionUrl() const;

  typedef std::set<WWidget *> UpdateMap;
  UpdateMap updateMap_;
  bool learning_, learningIncomplete_, moreUpdates_;

  std::string safeJsStringLiteral(const std::string& value);

  void addWsRequestId(int wsRqId);
  void renderWsRequestsDone(WStringStream &out);

  void updateMultiSessionCookie(const WebRequest &request);
  void renderMultiSessionCookieUpdate(WStringStream &out);

  // If we're already past the loading phase, and the first non-load
  // event was handled, we can clear the vector of stubbed widgets
  void clearStubbedWidgets();

public:
  virtual std::string learn(WStatelessSlot* slot) final override;

  friend class WApplication;
  friend class WebSession;
};

}

#endif // WEBRENDERER_H_
