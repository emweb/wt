/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>

#include <Wt/WIOService.h>
#include <Wt/WLocale.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Http/Client.h>
#include <Wt/Http/Response.h>
#include <Wt/Http/ResponseContinuation.h>
#include <Wt/Http/Request.h>

#include <web/Configuration.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace Wt;


namespace {

  class LocaleResource : public WResource
  {
  public:
    virtual ~LocaleResource() {
      beingDeleted();
    }

    void handleRequest(const Http::Request& request,
                       Http::Response& response) override
    {
      response.setMimeType("text/plain");
      response.out() << WLocale::currentLocale().name();
    }
  };

  class Server : public WServer
  {
  public:
    Server() {
      int argc = 7;
      const char *argv[]
        = { "test",
            "--http-address", "127.0.0.1",
            "--http-port", "0",
            "--docroot", "."
          };
      setServerConfiguration(argc, (char **)argv);
      resource_ = std::make_shared<LocaleResource>();
      addResource(resource_, "/test");
    }

    std::string address()
    {
      return "127.0.0.1:" + std::to_string(httpPort());
    }

    LocaleResource& resource() { return *resource_; }

  private:
    std::shared_ptr<LocaleResource> resource_;
  };

  class Client : public Wt::WObject {
  public:
    Client()
      : done_(true),
        abortAfterHeaders_(false)
    {
      impl_.done().connect(this, &Client::onDone);
      impl_.headersReceived().connect(this, &Client::onHeadersReceived);
      impl_.bodyDataReceived().connect(this, &Client::onDataReceived);
    }

    bool get(const std::string &url)
    {
      done_ = false;
      return impl_.get(url);
    }

    bool get(const std::string &url,
             const std::vector<Http::Message::Header> &headers)
    {
      done_ = false;
      return impl_.get(url, headers);
    }

    bool post(const std::string &url,
              const Http::Message &message)
    {
      done_ = false;
      return impl_.post(url, message);
    }

    void abort()
    {
      impl_.abort();
    }

    void abortAfterHeaders()
    {
      abortAfterHeaders_ = true;
    }

    void waitDone()
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      while (!done_)
        doneCondition_.wait(guard);
    }

    bool isDone()
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      return done_;
    }

    void onDone(Wt::AsioWrapper::error_code err, const Http::Message& m)
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      err_ = err;
      message_ = m;

      done_ = true;
      doneCondition_.notify_one();
    }

    void onHeadersReceived(WT_MAYBE_UNUSED const Http::Message& m)
    {
      if (abortAfterHeaders_)
        abort();
    }

    void onDataReceived(WT_MAYBE_UNUSED const std::string& d)
    { }

    Wt::AsioWrapper::error_code err() { return err_; }
    const Http::Message& message() { return message_; }

  private:
    Http::Client impl_;
    bool done_;
    bool abortAfterHeaders_;
    std::condition_variable doneCondition_;
    std::mutex doneMutex_;

    Wt::AsioWrapper::error_code err_;
    Http::Message message_;
  };
}

BOOST_AUTO_TEST_CASE( static_resource_locale_test )
{
  Server server;
  const std::string localName2 = "en-US";
  const std::string localName1 = "fr-FR";

  BOOST_REQUIRE(server.start());

  Client client;
  client.get("http://" + server.address() + "/test", { {"Accept-Language", localName1} });
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);
  BOOST_TEST(client.message().body() == localName1);

  client.get("http://" + server.address() + "/test", { {"Accept-Language", localName2} });
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);
  BOOST_TEST(client.message().body() == localName2);
}


#endif // WT_THREADED