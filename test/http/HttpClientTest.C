/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <Wt/WApplication.h>
#include <Wt/WIOService.h>
#include <Wt/Http/Client.h>
#include <Wt/Test/WTestEnvironment.h>

using namespace Wt;
using namespace Wt::Http;

namespace {

  class TestFixture : public WApplication
  {
  public:
    TestFixture(const WEnvironment& env)
      : WApplication(env),
	done_(false)
    { }

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

    void onDone(Wt::AsioWrapper::error_code err, const Message& m)
    {
      assert (WApplication::instance() == this);

      std::unique_lock<std::mutex> guard(doneMutex_);

      err_ = err;
      message_ = m;

      if (err_)
	std::cerr << "Http client error: " << err_.message() << std::endl;
      else {
	std::cerr << "Http client result status: " << m.status() << std::endl;
	for (unsigned i = 0; i < m.headers().size(); ++i) {
	  const Message::Header& h = m.headers()[i];
	  std::cerr << " " << h.name() << ": " << h.value() << std::endl;
	}
	std::cerr << " Body: -----" << std::endl;
	std::cerr << m.body();
	std::cerr << "-----" << std::endl;
      }

      done_ = true;
      doneCondition_.notify_one();
    }

  private:
    bool done_;
    std::condition_variable doneCondition_;
    std::mutex doneMutex_;

    Wt::AsioWrapper::error_code err_;
    Message message_;
  };
}

BOOST_AUTO_TEST_CASE( http_client_test1 )
{
  Wt::Test::WTestEnvironment environment;
  TestFixture app(environment);

  std::unique_ptr<Client> c(new Client());
  c->done().connect(std::bind(&TestFixture::onDone, &app,
			      std::placeholders::_1,
			      std::placeholders::_2));

  std::string ok = "www.google.com/";

  if (c->get("https://" + ok)) {
    environment.endRequest();
    app.waitDone();
    environment.startRequest();
  }
}

BOOST_AUTO_TEST_CASE( http_client_test2 )
{
  Wt::Test::WTestEnvironment environment;
  TestFixture app(environment);

  std::unique_ptr<Client> c(new Client());
  c->done().connect(std::bind(&TestFixture::onDone, &app,
			      std::placeholders::_1,
			      std::placeholders::_2));

  std::string verifyFail = "pause.perl.org/";

  /* Does no longer fail! Too bad */
  if (c->get("https://" + verifyFail)) {
    environment.endRequest();
    app.waitDone();
    environment.startRequest();
  }
}

BOOST_AUTO_TEST_CASE( http_client_test3 )
{
  Wt::Test::WTestEnvironment environment;
  TestFixture app(environment);

  std::unique_ptr<Client> c(new Client());
  c->done().connect(std::bind(&TestFixture::onDone, &app,
			      std::placeholders::_1,
			      std::placeholders::_2));

  std::string asioFail = "www.google.be/";

  /* Asio no longer fails. Good! */
  if (c->get("https://" + asioFail)) {
    environment.endRequest();
    app.waitDone();
    environment.startRequest();
  }
}

BOOST_AUTO_TEST_CASE( http_client_test4 )
{
  Wt::Test::WTestEnvironment environment;
  TestFixture app(environment);

  environment.server()->ioService().start();
  
  std::unique_ptr<Client> c(new Client());
  c->done().connect(std::bind(&TestFixture::onDone, &app,
			      std::placeholders::_1,
			      std::placeholders::_2));

  std::string ok = "www.google.com/";

  if (c->get("https://" + ok)) {
    environment.endRequest();
    app.waitDone();
    environment.startRequest();
  }

  environment.server()->ioService().stop();
  environment.server()->ioService().start();
  
  app.reset();

  if (c->get("https://" + ok)) {
    environment.endRequest();
    app.waitDone();
    environment.startRequest();
  }
}

BOOST_AUTO_TEST_CASE( http_client_test5 )
{
  // Test parseUrl
  Wt::Http::Client::URL url;
  bool result = Wt::Http::Client::parseUrl(
        "https://www.googleapis.com/calendar/v3/calendars/abcdefgh112233344444@group.calendar.google.com",
        url);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(url.auth, "");
  BOOST_CHECK_EQUAL(url.protocol, "https");
  BOOST_CHECK_EQUAL(url.host, "www.googleapis.com");
  BOOST_CHECK_EQUAL(url.port, 443);
  BOOST_CHECK_EQUAL(url.path, "/calendar/v3/calendars/abcdefgh112233344444@group.calendar.google.com");
}

BOOST_AUTO_TEST_CASE( http_client_test6 )
{
  // Test parseUrl with auth
  Wt::Http::Client::URL url;
  bool result = Wt::Http::Client::parseUrl(
        "http://foo:bar@example.com/a/b/c",
        url);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(url.auth, "foo:bar");
  BOOST_CHECK_EQUAL(url.protocol, "http");
  BOOST_CHECK_EQUAL(url.host, "example.com");
  BOOST_CHECK_EQUAL(url.port, 80);
  BOOST_CHECK_EQUAL(url.path, "/a/b/c");
}

#endif // WT_THREADED
