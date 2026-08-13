/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Test/WTestEnvironment.h"

#include "Wt/WEvent.h"
#include "Wt/WPointF.h"
#include "Wt/WSslInfo.h"
#include "web/CgiParser.h"
#include "web/WebRequest.h"

namespace {
  class MockRequest : public Wt::WebRequest {
  public:
    bool supportsTransferWebSocketResourceSocket() override
    {
      return false;
    }

    virtual void flush(WT_MAYBE_UNUSED ResponseState state, WT_MAYBE_UNUSED const WriteCallback& callback) override
    { }

    virtual std::istream &in() override
    {
      return in_;
    }

    virtual std::ostream &out() override
    {
      return out_;
    }

    virtual std::ostream &err() override
    {
      return err_;
    }

    virtual void setRedirect(WT_MAYBE_UNUSED const std::string& url) override
    { }

    virtual void setStatus(WT_MAYBE_UNUSED int status) override
    { }

    int status() override
    {
      return 0;
    }

    virtual void setContentType(const std::string &value) override
    {
      contentType_ = value;
    }

    virtual const char *contentType() const override
    {
      return contentType_.c_str();
    }

    virtual void setContentLength(int64_t length) override
    {
      contentLength_ = length;
    }

    virtual int64_t contentLength() const override
    {
      return contentLength_;
    }

    virtual void addHeader(WT_MAYBE_UNUSED const std::string& name, WT_MAYBE_UNUSED const std::string& value) override
    { }

    void insertHeader(WT_MAYBE_UNUSED const std::string &name, WT_MAYBE_UNUSED const std::string &value) override
    { }

    virtual const char *envValue(WT_MAYBE_UNUSED const char *name) const override
    {
      return nullptr;
    }

    virtual const std::string &serverName() const override
    {
      return serverName_;
    }

    virtual const std::string &serverPort() const override
    {
      return serverPort_;
    }

    virtual const std::string &scriptName() const override
    {
      return scriptName_;
    }

    virtual const char *requestMethod() const override
    {
      return requestMethod_.c_str();
    }

    virtual const std::string &queryString() const override
    {
      return queryString_;
    }

    virtual const std::string &pathInfo() const override
    {
      return pathInfo_;
    }

    virtual const std::string &remoteAddr() const override
    {
      return remoteAddr_;
    }

    virtual const char *urlScheme() const override
    {
      return urlScheme_.c_str();
    }

    virtual const char *headerValue(WT_MAYBE_UNUSED const char* name) const override
    {
      return nullptr;
    }

    virtual std::vector<Wt::Http::Message::Header> headers() const override
    {
      return std::vector<Wt::Http::Message::Header>{};
    }

    virtual std::unique_ptr<Wt::WSslInfo> sslInfo(const Wt::Configuration &) const override
    {
      return nullptr;
    }

    std::string contentType_;
    int64_t contentLength_;
    std::stringstream in_;
    std::stringstream out_;
    std::stringstream err_;
    std::string serverName_;
    std::string serverPort_;
    std::string scriptName_;
    std::string requestMethod_;
    std::string queryString_;
    std::string pathInfo_;
    std::string remoteAddr_;
    std::string urlScheme_;
  };
}

// Tests a particular crash scenario where the touches vector
// became empty due to there being a negative touch id
BOOST_AUTO_TEST_CASE( EventDecodeTest_negativeTouchId )
{
  MockRequest request;
  request.setContentLength(660);
  request.setContentType("application/x-www-form-urlencoded");
  request.requestMethod_ = "POST";
  request.queryString_ = "wtd=Jacx1tBYN9YMuZ8FIUsvzCJW1u8SshgO";
  request.in_.str(""
                 "ackId=-298847068&"
                 "an=0&"
                 "button=0&"
                 "charCode=0&"
                 "ctouches=-1697150354;322;75;322;91;322;91;101;39&"
                 "documentX=322&"
                 "documentY=91&"
                 "dragdX=199&"
                 "dragdY=-92&"
                 "focus=&"
                 "height=80&"
                 "id=ou1u7z4&"
                 "name=itemTouchEvent&"
                 "ou1u7rc=0;0&"
                 "ou1u7rd=0;0&"
                 "ou1u7re=0;0&"
                 "ou1u7rf=0;0&"
                 "ou1u7rg=0;0&"
                 "ou1u7rh=0;0&"
                 "ou1u7ta=0;0&"
                 "ou1u7tg=0;0&"
                 "ou1u7tm=0;0&"
                 "ou1u7ts=0;0&"
                 "ou1u7ty=0;0&"
                 "ou1u7u4=0;0&"
                 "ou1u7x9=0;0&"
                 "ou1u7yt=0;0&"
                 "ou1u7yv=0;0&"
                 "ou1u7zf=0;0&"
                 "ou1u7zg=0;0&"
                 "pageId=1&"
                 "request=jsupdate&"
                 "scale=1&"
                 "scrollX=0&"
                 "scrollY=0&"
                 "signal=user&"
                 "tid=ou1u7r2&"
                 "touches=-1697150354;322;75;322;91;322;91;101;39&"
                 "ttouches=-1697150354;322;75;322;91;322;91;101;39&"
                 "type=touchstart&"
                 "wheel=0&"
                 "widgetX=101&"
                 "widgetY=39&"
                 "width=652&"
                 "wtd=Jacx1tBYN9YMuZ8FIUsvzCJW1u8SshgO");

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);
  Wt::JavaScriptEvent jsEvent;
  jsEvent.get(request, "");

  BOOST_REQUIRE(jsEvent.touches.size() == 1);
  BOOST_REQUIRE(jsEvent.changedTouches.size() == 1);
  BOOST_REQUIRE(jsEvent.targetTouches.size() == 1);

  for (const Wt::Touch &touch : {
         jsEvent.touches[0],
         jsEvent.changedTouches[0],
         jsEvent.targetTouches[0]
       }) {
    BOOST_REQUIRE(Wt::WPointF(touch.window()) == Wt::WPointF(Wt::Coordinates(322,75)));
    BOOST_REQUIRE(Wt::WPointF(touch.document()) == Wt::WPointF(Wt::Coordinates(322,91)));
    BOOST_REQUIRE(Wt::WPointF(touch.screen()) == Wt::WPointF(Wt::Coordinates(322,91)));
    BOOST_REQUIRE(Wt::WPointF(touch.widget()) == Wt::WPointF(Wt::Coordinates(101,39)));
  }
}

// Verifies that the dedicated form-encoded unload request is parsed into
// request parameters. This is request-format coverage; it does not exercise
// browser lifecycle behavior or navigator.sendBeacon. See issue #14709.
BOOST_AUTO_TEST_CASE( EventDecodeTest_beaconUnload )
{
  MockRequest request;
  const std::string body =
    "request=jsupdate&"
    "ackId=-1&"
    "pageId=1&"
    "signal=user&"
    "id=app&"
    "name=Wt-unload&"
    "an=0";
  request.setContentLength(static_cast<int64_t>(body.size()));
  request.setContentType("application/x-www-form-urlencoded");
  request.requestMethod_ = "POST";
  request.queryString_ = "wtd=Jacx1tBYN9YMuZ8FIUsvzCJW1u8SshgO";
  request.in_.str(body);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  const std::string *req = request.getParameter("request");
  const std::string *ackId = request.getParameter("ackId");
  const std::string *pageId = request.getParameter("pageId");
  const std::string *signal = request.getParameter("signal");
  const std::string *id = request.getParameter("id");
  const std::string *name = request.getParameter("name");
  const std::string *an = request.getParameter("an");
  BOOST_REQUIRE(req && *req == "jsupdate");
  BOOST_REQUIRE(ackId && *ackId == "-1");
  BOOST_REQUIRE(pageId && *pageId == "1");
  BOOST_REQUIRE(signal && *signal == "user");
  BOOST_REQUIRE(id && *id == "app");
  BOOST_REQUIRE(name && *name == "Wt-unload");
  BOOST_REQUIRE(an && *an == "0");
}
