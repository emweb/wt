/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>

#include <Wt/WResource.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>
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

  enum class TestType {
    Simple,
    Continuation,
    ClientAddress,
    Exception,
  };

  class TestResource : public WResource
  {
  public:
    TestResource()
      : delaySendingBody_(false),
        haveEverMoreData_(false),
        haveRandomMoreData_(false),
        aborted_(0)
    { }

    virtual ~TestResource() {
      beingDeleted();
    }

    void setType(const TestType type) {
      type_ = type;
    }

    void delaySendingBody() {
      delaySendingBody_ = true;
    }

    void haveEverMoreData() {
      haveEverMoreData_ = true;
    }

    void haveRandomMoreData() {
      haveRandomMoreData_ = true;
    }

    int abortedCount() const {
      return aborted_;
    }

    virtual void handleRequest(const Http::Request& request,
                               Http::Response& response) override
    {
      switch (type_) {
      case TestType::Simple:
        return handleSimple(request, response);
      case TestType::Continuation:
        return handleWithContinuation(request, response);
      case TestType::ClientAddress:
        return handleClientAddress(request, response);
      case TestType::Exception:
        throw Wt::WException("Test exception");
      }
    }

    virtual void handleAbort(const Http::Request& request) override
    {
      ++aborted_;
    }

  private:
    bool delaySendingBody_;
    bool haveEverMoreData_;
    bool haveRandomMoreData_;
    int aborted_;
    TestType type_ = TestType::Simple;

    void handleSimple(const Http::Request& request,
                      Http::Response& response)
    {
      response.setStatus(200);
      response.out() << "Hello";
    }

    void handleClientAddress(const Http::Request& request,
                             Http::Response &response)
    {
      response.setStatus(200);
      response.out() << request.clientAddress();
    }

    void handleWithContinuation(const Http::Request& request,
                                Http::Response& response) 
    {
      if (request.continuation()) {
        response.out() << "Hello";
        if (haveEverMoreData_ ||
            (haveRandomMoreData_ && (rand() % 10 != 0)))
          response.createContinuation();
      } else {
        response.setStatus(200);
        Http::ResponseContinuation *c = response.createContinuation();
        if (delaySendingBody_)
          c->waitForMoreData();
      }
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
      addResource(&resource_, "/test");
    }

    std::string address() 
    {
      return "127.0.0.1:" + std::to_string(httpPort());
    }

    TestResource& resource() { return resource_; }

  private:
    TestResource resource_;
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

    bool isDone() const
    {
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

    void onHeadersReceived(const Http::Message& m)
    {
      if (abortAfterHeaders_)
        abort();
    }

    void onDataReceived(const std::string& d)
    {
    }

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

BOOST_AUTO_TEST_CASE( http_client_server_test1 )
{
  Server server;

  if (server.start()) {
    Client client;
    client.get("http://" + server.address() + "/test");
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);
    BOOST_REQUIRE(client.message().body() == "Hello");
  }
}

BOOST_AUTO_TEST_CASE( http_client_server_test2 )
{
  Server server;

  server.resource().setType(TestType::Continuation);

  if (server.start()) {
    Client client;
    client.get("http://" + server.address() + "/test");
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);
    BOOST_REQUIRE(client.message().body() == "Hello");
  }
}

BOOST_AUTO_TEST_CASE( http_client_server_test3 )
{
  Server server;

  server.resource().setType(TestType::Continuation);
  server.resource().delaySendingBody();
  server.resource().haveEverMoreData();

  if (server.start()) {
    Client client;
    client.abortAfterHeaders();
    client.get("http://" + server.address() + "/test");
    client.waitDone();

    BOOST_REQUIRE(client.err() == Wt::AsioWrapper::asio::error::bad_descriptor ||
                  client.err() == Wt::AsioWrapper::asio::error::operation_aborted);
  }

  server.resource().haveMoreData();
  std::this_thread::sleep_for(std::chrono::milliseconds{50});
  BOOST_REQUIRE(server.resource().abortedCount() == 1);
}

BOOST_AUTO_TEST_CASE( http_client_server_test4 )
{
  Server server;

  server.resource().setType(TestType::Continuation);
  server.resource().delaySendingBody();
  server.resource().haveRandomMoreData();

  int abortedCount = 0;

  if (server.start()) {
    std::vector<Client *> clients;

    for (unsigned i = 0; i < 1000; ++i) {
      Client *client = new Client();
      client->get("http://" + server.address() + "/test");
      clients.push_back(client);
    }

    for (;;) {
      bool alldone = true;

      for (unsigned i = 0; i < clients.size(); ++i) {        
        if (!clients[i]->isDone()) {
          if (i % 100 == 0) {
            clients[i]->abort();
            ++abortedCount;
          }
          alldone = false;
          break;
        }
      }

      if (!alldone)
        server.resource().haveMoreData();
      else
        break;
    }

    for (unsigned i = 0; i < 1000; ++i) {
      clients[i]->waitDone();
      delete clients[i];
    }
  }
}

BOOST_AUTO_TEST_CASE( http_client_server_test5 )
{
  Server server;
  server.resource().setType(TestType::Continuation);
  server.resource().delaySendingBody();

  Http::Message msg;
  msg.addHeader("connection", "keep-alive");

  if (server.start()) {
    Client client;

    for (int i = 0; i < 1000; ++i) {
      client.post("http://" + server.address() + "/test", msg);

      std::this_thread::sleep_for(std::chrono::milliseconds(i == 0 ? 500 : 100));
      server.resource().haveMoreData();

      client.waitDone();
    }
  }
}

// Reserved example IP ranges:
// - 192.0.2.0/24
// - 198.51.100.0/24
// - 203.0.113.0/24
// - 2001:db8::/32

BOOST_AUTO_TEST_CASE( http_client_address_not_behind_reverse_proxy )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "198.51.100.1"},
            {"Client-IP", "203.0.113.1"}
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Client-IP and X-Forwarded-For headers are ignored if not behind reverse proxy
    BOOST_REQUIRE(client.message().body() == "127.0.0.1");
  }
}

BOOST_AUTO_TEST_CASE( http_client_address_client_ip )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);
  server.configuration().setBehindReverseProxy(true);

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "198.51.100.1"},
            {"Client-IP", "203.0.113.1"}
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Should get IP address from Client-IP
    BOOST_REQUIRE(client.message().body() == "203.0.113.1");
  }
}

BOOST_AUTO_TEST_CASE( http_client_address_forwarded_for )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);
  server.configuration().setOriginalIPHeader("X-Forwarded-For");
  server.configuration().setTrustedProxies({
                                             Configuration::Network::fromString("127.0.0.1"),
                                           });

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "198.51.100.1"},
            {"Client-IP", "203.0.113.1"}
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Should get IP address from X-Forwarded-For
    BOOST_REQUIRE(client.message().body() == "198.51.100.1");
  }
}

BOOST_AUTO_TEST_CASE( http_multiple_proxies )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);
  server.configuration().setTrustedProxies({
                                                   Configuration::Network::fromString("127.0.0.1"),
                                                   Configuration::Network::fromString("198.51.100.0/24")
                                           });

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "192.0.2.1, 203.0.113.1, 198.51.100.1"},
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Should get IP address from X-Forwarded-For
    BOOST_REQUIRE(client.message().body() == "203.0.113.1");
  }
}


BOOST_AUTO_TEST_CASE( http_multiple_proxies2 )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);
  server.configuration().setTrustedProxies({
                                                   Configuration::Network::fromString("127.0.0.1"),
                                                   Configuration::Network::fromString("198.51.100.0/24"),
                                                   Configuration::Network::fromString("203.0.113.0/24")
                                           });

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "192.0.2.1, 203.0.113.1, 198.51.100.1"},
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Should get IP address from X-Forwarded-For
    BOOST_REQUIRE(client.message().body() == "192.0.2.1");
  }
}


BOOST_AUTO_TEST_CASE( http_client_address_forward_for_includes_us_on_subnet )
{
  Server server;
  server.resource().setType(TestType::ClientAddress);
  server.configuration().setTrustedProxies({
                                                   Configuration::Network::fromString("127.0.0.0/8"),
                                                   Configuration::Network::fromString("198.51.100.0/24"),
                                                   Configuration::Network::fromString("203.0.113.0/24")
                                           });

  if (server.start()) {
    Client client;

    std::vector<Http::Message::Header> headers {
            {"X-Forwarded-For", "127.0.0.10, 203.0.113.1, 198.51.100.1"},
    };

    client.get("http://" + server.address() + "/test", headers);
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 200);

    // Should get IP address from X-Forwarded-For
    BOOST_REQUIRE(client.message().body() == "127.0.0.10");
  }
}

BOOST_AUTO_TEST_CASE( resource_invalid_after_changed )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  WApplication *app = nullptr;
  server.addEntryPoint(EntryPointType::Application,
                       [&app] (const WEnvironment& env) {
                         auto app_ = std::make_unique<WApplication>(env);
                         app = app_.get();
                         return app_;
                       });
  if (server.start()) {
    // create application
    Client client;
    client.get("http://" + server.address());
    client.waitDone();

    // do resource test
    std::shared_ptr<WResource> resource;
    {
      WApplication::UpdateLock lock(app);
      resource = std::make_shared<TestResource>();
      resource->generateUrl();
    }

    std::string resourceUrl = "http://" + server.address() + "/" + resource->url().substr(1);

    Client client1;
    client1.get(resourceUrl);
    client1.waitDone();

    BOOST_REQUIRE(!client1.err());
    BOOST_REQUIRE(client1.message().status() == 200);
    BOOST_REQUIRE(client1.message().body() == "Hello");

    {
      WApplication::UpdateLock lock(app);
      resource->setChanged();
    }

    Client client2;
    client2.get(resourceUrl);
    client2.waitDone();

    BOOST_REQUIRE(!client2.err());
    BOOST_REQUIRE(client2.message().status() == 200);

    resource->setInvalidAfterChanged(true);

    Client client3;
    client3.get(resourceUrl);
    client3.waitDone();

    BOOST_REQUIRE(!client3.err());
    BOOST_REQUIRE(client3.message().status() == 404);
  }
}


BOOST_AUTO_TEST_CASE( application_expired_while_newid )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);
  auto& conf = server.configuration();
  conf.setSessionTimeout(2);

  WApplication *app = nullptr;
  server.addEntryPoint(EntryPointType::Application,
                       [&app] (const WEnvironment& env) {
                         auto app_ = std::make_unique<WApplication>(env);
                         app = app_.get();
                         return app_;
                       });
  if (server.start()) {
    Client client;
    client.get("http://" + server.address());
    client.waitDone();

    auto webSession = app->session();
    auto controller = webSession->controller();
    std::thread t([app] {
                    WApplication::UpdateLock lock(app);
                    std::this_thread::sleep_for(std::chrono::milliseconds(2200));
                    app->changeSessionId();
                  });
    std::this_thread::sleep_for(std::chrono::milliseconds(2100));
    controller->expireSessions();

    t.join();
  }
}

BOOST_AUTO_TEST_CASE( http_wresource_exception )
{
  Server server;
  server.resource().setType(TestType::Exception);

  if (server.start()) {
    Client client;

    client.get("http://" + server.address() + "/test");
    client.waitDone();

    BOOST_REQUIRE(!client.err());
    BOOST_REQUIRE(client.message().status() == 500);
  }
}

#endif // WT_THREADED
