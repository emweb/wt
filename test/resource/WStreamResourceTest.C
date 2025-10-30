/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>

#include <Wt/WIOService.h>
#include <Wt/WStreamResource.h>
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
  constexpr auto SimulatedWorkTime = std::chrono::milliseconds{200};
  constexpr auto fullData = "This is a test stream resource. This stream is used to test it so it must be quite long as the bigger it is, the better the test will be.";

  class TestResource : public WStreamResource
  {
  public:
    virtual ~TestResource() {
      beingDeleted();
    }

    void handleRequest(const Http::Request& request,
                       Http::Response& response) override
    {
      std::istringstream input(fullData);
      handleRequestPiecewise(request, response, input);
      std::this_thread::sleep_for(SimulatedWorkTime);
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
      resource_ = std::make_shared<TestResource>();
      addResource(resource_, "/test");
    }

    std::string address()
    {
      return "127.0.0.1:" + std::to_string(httpPort());
    }

    TestResource& resource() { return *resource_; }

  private:
    std::shared_ptr<TestResource> resource_;
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

BOOST_AUTO_TEST_CASE( wstreamresource_concurrent_mixed_get )
{
  /* Tests that both a request for the full resource and a request for
   * a part of the resource can be handled concurrently.
   */
  Server server;
  server.resource().setBufferSize(10);

  BOOST_REQUIRE(server.start());

  Client fullClient;
  Client partClient;
  fullClient.get("http://" + server.address() + "/test");
  std::this_thread::sleep_for(SimulatedWorkTime);
  partClient.get("http://" + server.address() + "/test", { {"Range", "bytes=10-59"} });
  fullClient.waitDone();
  partClient.waitDone();

  BOOST_TEST(!fullClient.err());
  BOOST_TEST(fullClient.message().status() == 200);
  BOOST_TEST(fullClient.message().body() == fullData);

  BOOST_TEST(!partClient.err());
  BOOST_TEST(partClient.message().status() == 206);
  BOOST_TEST(partClient.message().body() == "test stream resource. This stream is used to test ");
}

BOOST_AUTO_TEST_CASE( wstreamresource_concurrent_full_get )
{
  /* Tests that several requests for the full resource can be handled
   * concurrently.
   */
  Server server;
  server.resource().setBufferSize(10);

  BOOST_REQUIRE(server.start());

  Client fullClient1;
  Client fullClient2;
  fullClient1.get("http://" + server.address() + "/test");
  std::this_thread::sleep_for(SimulatedWorkTime);
  fullClient2.get("http://" + server.address() + "/test");
  fullClient1.waitDone();
  fullClient2.waitDone();

  BOOST_TEST(!fullClient1.err());
  BOOST_TEST(fullClient1.message().status() == 200);
  BOOST_TEST(fullClient1.message().body() == fullData);

  BOOST_TEST(!fullClient2.err());
  BOOST_TEST(fullClient2.message().status() == 200);
  BOOST_TEST(fullClient2.message().body() == fullData);
}

BOOST_AUTO_TEST_CASE( wstreamresource_concurrent_partial_get )
{
  /* Tests that several requests for different parts of the resource
   * can be handled concurrently.
   */
  Server server;
  server.resource().setBufferSize(10);

  BOOST_REQUIRE(server.start());

  Client partClient1;
  Client partClient2;
  partClient1.get("http://" + server.address() + "/test", { {"Range", "bytes=40-100"} });
  std::this_thread::sleep_for(SimulatedWorkTime);
  partClient2.get("http://" + server.address() + "/test", { {"Range", "bytes=10-59"} });
  partClient1.waitDone();
  partClient2.waitDone();

  BOOST_TEST(!partClient1.err());
  BOOST_TEST(partClient1.message().status() == 206);
  BOOST_TEST(partClient1.message().body() == "eam is used to test it so it must be quite long as the bigger");

  BOOST_TEST(!partClient2.err());
  BOOST_TEST(partClient2.message().status() == 206);
  BOOST_TEST(partClient2.message().body() == "test stream resource. This stream is used to test ");
}


#endif // WT_THREADED