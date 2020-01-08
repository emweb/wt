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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace Wt;

namespace {

  class TestResource : public WResource
  {
  public:
    TestResource()
      : continuation_(false),
	delaySendingBody_(false),
	haveEverMoreData_(false),
	haveRandomMoreData_(false),
	aborted_(0)
    { }

    virtual ~TestResource() {
      beingDeleted();
    }

    void useContinuation() {
      continuation_ = true;
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
      if (continuation_)
	handleWithContinuation(request, response);
      else
	handleSimple(request, response);
    }

    virtual void handleAbort(const Http::Request& request) override
    {
      ++aborted_;
    }

  private:
    bool continuation_;
    bool delaySendingBody_;
    bool haveEverMoreData_;
    bool haveRandomMoreData_;
    int aborted_;

    void handleSimple(const Http::Request& request,
		      Http::Response& response)
    {
      response.setStatus(200);
      response.out() << "Hello";
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

  class Client : public Http::Client
  {
  public:
    Client()
      : done_(false),
	abortAfterHeaders_(false)
    { 
      done().connect(this, &Client::onDone);
      headersReceived().connect(this, &Client::onHeadersReceived);
      bodyDataReceived().connect(this, &Client::onDataReceived);
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

    void reset() 
    {
      done_ = false;
    }

    bool isDone()
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

  server.resource().useContinuation();

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

  server.resource().useContinuation();
  server.resource().delaySendingBody();
  server.resource().haveEverMoreData();

  if (server.start()) {
    Client client;
    client.abortAfterHeaders();
    client.get("http://" + server.address() + "/test");
    client.waitDone();

    BOOST_REQUIRE(client.err() == Wt::AsioWrapper::asio::error::bad_descriptor);
  }

  server.resource().haveMoreData();
  std::this_thread::sleep_for(std::chrono::milliseconds{50});
  BOOST_REQUIRE(server.resource().abortedCount() == 1);
}

BOOST_AUTO_TEST_CASE( http_client_server_test4 )
{
  Server server;

  server.resource().useContinuation();
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
      delete clients[i];
    }
  }
}

BOOST_AUTO_TEST_CASE( http_client_server_test5 )
{
  Server server;
  server.resource().useContinuation();
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

      client.reset();
    }
  }
}

#endif // WT_THREADED







