/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/Configuration.h"
#include "web/EntryPoint.h"

#include <string>

using namespace std::string_literals;

BOOST_AUTO_TEST_CASE( test_entrypoint_empty )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("", "", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 0);
  BOOST_REQUIRE(entryPointMatch.depth() == 0);

  entryPointMatch = configuration.matchEntryPoint("", "/", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 0);
  BOOST_REQUIRE(entryPointMatch.depth() == 0);
}

BOOST_AUTO_TEST_CASE( test_entrypoint_slash )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("", "", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 0);
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("", "/", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 1);
  BOOST_REQUIRE(entryPointMatch.depth() == 1);
}

BOOST_AUTO_TEST_CASE( test_entrypoint_with_urlparams )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/users", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/users/${user}", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("", "", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint == nullptr);

  entryPointMatch = configuration.matchEntryPoint("", "/users", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("", "/users/", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("", "/users/jos", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  auto urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  entryPointMatch = configuration.matchEntryPoint("", "/users/jos/extra", false);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint == nullptr);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/jos", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/jos/extra", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/", // path
      "" // favicon
    )
  );

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 0);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 6);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users");
  BOOST_REQUIRE(entryPointMatch.depth() == 1);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/jos", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  entryPointMatch = configuration.matchEntryPoint("/entrypoint.dll", "/users/jos/extra", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  // Add an entrypoint that will require backtracking
  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/users/${user}/posts/all", // path
      "" // favicon
    )
  );

  // /users/jos/posts matches entrypoint /users/jos, since there's no entrypoint for /users/jos/posts,
  // so it will backtrack from depth 3 to 2
  entryPointMatch = configuration.matchEntryPoint("", "/users/jos/posts", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 10);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);

  entryPointMatch = configuration.matchEntryPoint("", "/users/jos/posts/all", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 20);
  urlParams = std::cref(entryPointMatch.urlParams);
  BOOST_REQUIRE(urlParams.get().size() == 1);
  BOOST_REQUIRE(std::find(urlParams.get().begin(), urlParams.get().end(), std::make_pair("user"s, "jos"s)) != urlParams.get().end());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/users/${user}/posts/all");
  BOOST_REQUIRE(entryPointMatch.depth() == 4);
}

BOOST_AUTO_TEST_CASE( test_entrypoint_prefer_deepest )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/a", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/a/a", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("/a", "/a", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 2);
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/a/a");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);
}

BOOST_AUTO_TEST_CASE( test_entrypoint_prefer_least_dynamic1 )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/foo/bar", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/${arg}/foo/bar", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("/foo", "/foo/bar", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 8);
  BOOST_REQUIRE(entryPointMatch.urlParams.empty());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/foo/bar");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);
}

BOOST_AUTO_TEST_CASE( test_entrypoint_prefer_least_dynamic2 )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/${var1}/${var2}/${var3}", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    Wt::EntryPoint(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/b/c", // path
      "" // favicon
    )
  );

  auto entryPointMatch = configuration.matchEntryPoint("/a", "/b/c", true);
  BOOST_REQUIRE(entryPointMatch.entryPoint != nullptr);
  BOOST_REQUIRE(entryPointMatch.extraStartIndex == 4);
  BOOST_REQUIRE(entryPointMatch.urlParams.empty());
  BOOST_REQUIRE(entryPointMatch.entryPoint->path() == "/b/c");
  BOOST_REQUIRE(entryPointMatch.depth() == 2);
}
