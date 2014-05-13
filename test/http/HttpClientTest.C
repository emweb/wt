/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include <Wt/WApplication>
#include <Wt/WIOService>
#include <Wt/Http/Client>
#include <Wt/Test/WTestEnvironment>

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
      boost::mutex::scoped_lock guard(doneMutex_);

      while (!done_)
	doneCondition_.wait(guard);
    }

    void reset() 
    {
      done_ = false;
    }

    void onDone(boost::system::error_code err, const Message& m)
    {
      assert (WApplication::instance() == this);

      boost::mutex::scoped_lock guard(doneMutex_);

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
    boost::condition doneCondition_;
    boost::mutex doneMutex_;

    boost::system::error_code err_;
    Message message_;
  };
}

BOOST_AUTO_TEST_CASE( http_client_test1 )
{
  Wt::Test::WTestEnvironment environment;
  TestFixture app(environment);

  Client *c = new Client(&app);
  c->done().connect(boost::bind(&TestFixture::onDone, &app, _1, _2));

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

  Client *c = new Client(&app);
  c->done().connect(boost::bind(&TestFixture::onDone, &app, _1, _2));

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

  Client *c = new Client(&app);
  c->done().connect(boost::bind(&TestFixture::onDone, &app, _1, _2));

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
  
  Client *c = new Client(&app);
  c->done().connect(boost::bind(&TestFixture::onDone, &app, _1, _2));

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

#endif // WT_THREADED
