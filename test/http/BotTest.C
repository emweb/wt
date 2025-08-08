/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WConfig.h"

#ifdef WT_THREADED

#include "Wt/cpp17/filesystem.hpp"

#include "Wt/Http/Client.h"
#include "Wt/Http/Message.h"

#include "Wt/WServer.h"

#include "web/Configuration.h"

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>

using namespace Wt;

namespace {
  const char* TEST_WT_CONFIG = "tmp_wt_test_config.xml";

  // For newline / carriage return
#ifdef _WIN32
  const std::string doctype = "<!DOCTYPE html>\r\n<html ";
#else
  const std::string doctype = "<!DOCTYPE html>\n<html ";
#endif
  // Appended as minified or not
#ifdef WT_DEBUG_JS
  const std::string loadScript = "function loadScript(url)";
#else
  const std::string loadScript = "function loadScript(e)";
#endif

  const std::string iframe = "<iframe id='wt_iframe_dl_id' name='wt_iframe_dl' style='display:none;'></iframe>";
  const std::string domroot = "<div class=\"Wt-domRoot\" style=\"height:100.0%;\">";

  class Server : public WServer
  {
  public:
    Server() {
      int argc = 9;
      const char *argv[]
        = { "test",
            "--http-address", "127.0.0.1",
            "--http-port", "0",
            "--docroot", ".",
            "--config", TEST_WT_CONFIG
          };
      createBotConfig();
      setServerConfiguration(argc, (char **)argv);
    }

    std::string address()
    {
      return "127.0.0.1:" + std::to_string(httpPort());
    }

    ~Server()
    {
      Wt::cpp17::filesystem::remove(TEST_WT_CONFIG);
    }

  private:
    void createBotConfig()
    {
      std::fstream config(TEST_WT_CONFIG, std::ios_base::out);
      config << "<server>"
             << "  <application-settings location=\"*\">"
             << "    <user-agents type=\"bot\">"
             << "      <user-agent>.*bot.*</user-agent>"
             << "   </user-agents>"
             << "  </application-settings>"
             << "</server>";
      config.flush();
    }
  };

  class Client : public Wt::WObject {
  public:
    Client()
      : done_(true)
    {
      impl_.done().connect(this, &Client::onDone);
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

    void waitDone()
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      while (!done_)
        doneCondition_.wait(guard);
    }

    void onDone(Wt::AsioWrapper::error_code err, const Http::Message& m)
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      err_ = err;
      message_ = m;

      done_ = true;
      doneCondition_.notify_one();
    }

    Wt::AsioWrapper::error_code err() { return err_; }
    const Http::Message& message() { return message_; }

  private:
    Http::Client impl_;
    bool done_;
    std::condition_variable doneCondition_;
    std::mutex doneMutex_;

    Wt::AsioWrapper::error_code err_;
    Http::Message message_;
  };
}

void detectPlainHTML(const std::string& content)
{
  // A request body should contain the opening basic HTML tags, and then the Plain.html without a form
  // This does contain the iframe, and the DOM root.
  BOOST_TEST(content.find(doctype) != std::string::npos);
  BOOST_TEST(content.find(iframe) != std::string::npos);
  BOOST_TEST(content.find(domroot) != std::string::npos);

  // No Boot.js is present
  BOOST_TEST(content.find(loadScript) == std::string::npos);
  // No form is present
  BOOST_TEST(content.find("<form method='post'") == std::string::npos);
  // This does NOT CONTAIN ANY REFERENCE TO A WTD!
  BOOST_TEST(content.find("wtd=") == std::string::npos);
}

BOOST_AUTO_TEST_CASE( non_bot_access_delayed_boot )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  // Using delayed boot, so WebSession isn't linked to WApp
  Client client;
  client.get("http://" + server.address());
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Not detected as bot
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);

  // Non-bot, so session is alive (but missing WApp)
  std::string sessionId = server.sessions()[0].sessionId;
  // A request body should contain the opening basic HTML tags, and then the Boot.js script segment
  // After that the Boot.html (without a form)
  BOOST_TEST(client.message().body().find(doctype) != std::string::npos);
  // Boot.js
  BOOST_TEST(client.message().body().find(loadScript) != std::string::npos);

  // Boot.html
  BOOST_TEST(client.message().body().find("<link href=\"?wtd=" + sessionId + "&amp;request=style&amp;page=1") != std::string::npos);
  // No form
  BOOST_TEST(client.message().body().find("<form method='post' action=") == std::string::npos);

  // We now force the WApp creation
  client.post("http://" + server.address() + "/?wtd=" + sessionId + "&js=no&signal=load", Http::Message());
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Not detected as bot, inside application
  BOOST_TEST(!isBotUser);
  BOOST_TEST(hasApplicationStarted);

  BOOST_TEST(client.message().body().find(doctype) != std::string::npos);
  // Now we have the Plain.html form
  BOOST_TEST(client.message().body().find("<form method='post' action='?wtd=" + sessionId + "' id='form'") != std::string::npos);
}

BOOST_AUTO_TEST_CASE( non_bot_access_progressive_boot )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  client.get("http://" + server.address());
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Not detected as bot, inside application
  BOOST_TEST(!isBotUser);
  BOOST_TEST(hasApplicationStarted);

  // A request body should contain the opening basic HTML tags, and then the Boot.js script segment.
  // After that a form from the Hybrid.html
  BOOST_TEST(client.message().body().find(doctype) != std::string::npos);
  // Boot.js
  BOOST_TEST(client.message().body().find(loadScript) != std::string::npos);

  // Non-bot, so session is alive
  std::string sessionId = server.sessions()[0].sessionId;
  BOOST_TEST(client.message().body().find(iframe) != std::string::npos);
  BOOST_TEST(client.message().body().find("<form method='post' action='?wtd=" + sessionId + "' id='Wt-form'") != std::string::npos);
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_no_session_info )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address(), headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Detected as bot, inside application
  BOOST_TEST(isBotUser);
  BOOST_TEST(hasApplicationStarted);

  detectPlainHTML(client.message().body());
  // No form is present
  BOOST_TEST(client.message().body().find("<form method='post'") == std::string::npos);
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_no_session_info )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address(), headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Detected as bot, inside application
  BOOST_TEST(isBotUser);
  BOOST_TEST(hasApplicationStarted);

  detectPlainHTML(client.message().body());
  // The form is removed from Plain.html
  BOOST_TEST(client.message().body().find("<form method='post'") == std::string::npos);
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_disallowed_followup )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  // "Messy" sessionId retrieval. Done with method only visible for testing,
  // hidden behind "WT_TEST_VISIBILITY"
  std::string sessionId;
  server.controller()->addedSessionId_.connect([&sessionId](const std::string& id) { sessionId = id; });

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address(), headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Detected as bot, inside application
  BOOST_TEST(isBotUser);
  BOOST_TEST(hasApplicationStarted);

  // Reset
  hasApplicationStarted = false;
  isBotUser = false;

  detectPlainHTML(client.message().body());

  // Using delayed boot, so WebSession isn't linked to WApp
  // Since this is a bot agent, this POST should not result in a "valid" request (like above).
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=" + sessionId + "&js=no&signal=load", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_disallowed_followup )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  // "Messy" sessionId retrieval. Done with method only visible for testing,
  // hidden behind "WT_TEST_VISIBILITY"
  std::string sessionId;
  server.controller()->addedSessionId_.connect([&sessionId](const std::string& id) { sessionId = id; });

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address(), headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 200);

  // Detected as bot, inside application
  BOOST_TEST(isBotUser);
  BOOST_TEST(hasApplicationStarted);

  detectPlainHTML(client.message().body());

  // Reset
  hasApplicationStarted = false;
  isBotUser = false;

  // Using delayed boot, so WebSession isn't linked to WApp
  // Since this is a bot agent, this POST should not result in a "valid" request (like above).
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=" + sessionId + "&js=no&signal=load", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_with_wtd_disallowed_get )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address() + "/?wtd=0123456789101213", headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_with_wtd_disallowed_get )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address() + "/?wtd=0123456789101213", headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_with_wtd_disallowed_post )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=0123456789101213", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_with_wtd_disallowed_post )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=0123456789101213", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_with_wtd_and_signal_disallowed_get )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address() + "/?wtd=0123456789101213&signal=123", headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_with_wtd_and_signal_disallowed_get )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  client.get("http://" + server.address() + "/?wtd=0123456789101213&signal=123", headers);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_delayed_boot_with_wtd_and_signal_disallowed_post )
{
  Server server;

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=0123456789101213&signal=123", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

BOOST_AUTO_TEST_CASE( bot_access_progressive_boot_with_wtd_and_signal_disallowed_post )
{
  Server server;
  server.configuration().setBootstrapMethod(Configuration::Progressive);

  bool hasApplicationStarted = false;
  bool isBotUser = false;
  server.addEntryPoint(EntryPointType::Application,
                       [&hasApplicationStarted, &isBotUser] (const WEnvironment& env) {
                         hasApplicationStarted = true;
                         isBotUser = env.agentIsSpiderBot();
                         return std::make_unique<WApplication>(env);
                       });
  BOOST_REQUIRE(server.start());

  Client client;
  std::vector<Http::Message::Header> headers = {{ "User-Agent", "somebot" }};
  Http::Message message(headers);
  client.post("http://" + server.address() + "/?wtd=0123456789101213&signal=123", message);
  client.waitDone();

  BOOST_TEST(!client.err());
  BOOST_TEST(client.message().status() == 403);

  // Not detected as bot, but no application was launched
  BOOST_TEST(!isBotUser);
  BOOST_TEST(!hasApplicationStarted);
  BOOST_TEST(server.configuration().agentIsBot("somebot"));

  // This does not contain a body
  BOOST_TEST(client.message().body().empty());
}

#endif // WT_THREADED
