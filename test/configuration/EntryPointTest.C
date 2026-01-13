/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/Configuration.h"
#include "web/EntryPoint.h"
#include "Wt/WMemoryResource.h"

#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

namespace {

struct TreeInfo {
  TreeInfo()
    : staticSegmentsCount(0),
      dynamicSegmentsCount(0),
      entryPointsCount(0)
  { }

  int staticSegmentsCount;
  int dynamicSegmentsCount;
  int entryPointsCount;
};

TreeInfo computeTreeInfo(const Wt::PathSegment* segment) {
  TreeInfo info;

  info.entryPointsCount += (segment->entryPoint ? 1 : 0);
  info.staticSegmentsCount += segment->children.size();
  info.dynamicSegmentsCount += (segment->dynamicChild ? 1 : 0);

  for (const auto& child : segment->children) {
    TreeInfo childInfo = computeTreeInfo(child.get());
    info.staticSegmentsCount += childInfo.staticSegmentsCount;
    info.dynamicSegmentsCount += childInfo.dynamicSegmentsCount;
    info.entryPointsCount += childInfo.entryPointsCount;
  }

  if (segment->dynamicChild) {
    TreeInfo dynInfo = computeTreeInfo(segment->dynamicChild.get());
    info.staticSegmentsCount += dynInfo.staticSegmentsCount;
    info.dynamicSegmentsCount += dynInfo.dynamicSegmentsCount;
    info.entryPointsCount += dynInfo.entryPointsCount;
  }

  return info;

}

// Compares two PathSegment trees for equality, printing differences to std::cerr
bool compareSegments(const Wt::PathSegment* s1, const Wt::PathSegment* s2) {
  if (s1->segment != s2->segment) {
    std::cerr << "Segment mismatch: '" << s1->segment << "' vs '" << s2->segment << "'\n";
    return false;
  }
  if ((s1->entryPoint && !s2->entryPoint) || (!s1->entryPoint && s2->entryPoint)) {
    std::cerr << "EntryPoint presence mismatch at segment: '" << s1->segment << "'\n";
    return false;
  }
  if (s1->entryPoint && s2->entryPoint && s1->entryPoint->path() != s2->entryPoint->path()) {
    std::cerr << "EntryPoint path mismatch: '" << s1->entryPoint->path() << "' vs '" << s2->entryPoint->path() << "'\n";
    return false;
  }
  if (s1->children.size() != s2->children.size()) {
    std::cerr << "Children size mismatch at segment: '" << s1->segment << "' (" << s1->children.size() << " vs " << s2->children.size() << ")\n";
    return false;
  }

  auto it1 = s1->children.begin();
  auto it2 = s2->children.begin();
  size_t idx = 0;
  for (; it1 != s1->children.end() && it2 != s2->children.end(); ++it1, ++it2, ++idx) {
    if (!compareSegments(it1->get(), it2->get())) {
      std::cerr << "Child index " << idx << " mismatch at segment: '" << s1->segment << "'\n";
      return false;
    }
  }

  if ((s1->dynamicChild && !s2->dynamicChild) || (!s1->dynamicChild && s2->dynamicChild)) {
    std::cerr << "Dynamic child presence mismatch at segment: '" << s1->segment << "'\n";
    return false;
  }

  if (s1->dynamicChild && s2->dynamicChild) {
    if (!compareSegments(s1->dynamicChild.get(), s2->dynamicChild.get())) {
      std::cerr << "Dynamic child mismatch at segment: '" << s1->segment << "'\n";
      return false;
    }
  }

  return true;
}

void addPathsToManager(Wt::EntryPointManager& mgr, const std::vector<std::string>& paths)
{
  for (const auto& path : paths) {
    auto ep = std::make_shared<Wt::EntryPoint>(
      Wt::EntryPointType::Application,
      [](const Wt::WEnvironment&) { return nullptr; },
      path,
      ""
    );
    mgr.addEntryPoint(ep);
  }

  TreeInfo info = computeTreeInfo(&mgr.rootPathSegment());
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, paths.size());
}

// Helper to build two EntryPointManagers: one with all paths, one with all except the strings in pathsToRemove
std::pair<Wt::EntryPointManager, Wt::EntryPointManager>
buildManagersWithRemoved(const std::vector<std::string>& paths, const std::vector<std::string>& pathsToRemove) {
  Wt::EntryPointManager mgr1;
  Wt::EntryPointManager mgr2;

  addPathsToManager(mgr1, paths);

  std::vector<std::string> pathsWithoutRemoved = paths;
  for (const auto& toRemove : pathsToRemove) {
    pathsWithoutRemoved.erase(std::remove(pathsWithoutRemoved.begin(), pathsWithoutRemoved.end(), toRemove), pathsWithoutRemoved.end());
    mgr1.removeEntryPoint(toRemove);
  }

  addPathsToManager(mgr2, pathsWithoutRemoved);

  return std::pair<Wt::EntryPointManager, Wt::EntryPointManager>(std::move(mgr1), std::move(mgr2));
}

std::pair<Wt::EntryPointManager, Wt::EntryPointManager>
buildManagersWithRemoved(const std::vector<std::string>& paths, const std::string& toRemove) {
  return buildManagersWithRemoved(paths, std::vector<std::string>{toRemove});
}

} // namespace

BOOST_AUTO_TEST_CASE( test_entrypoint_empty )
{
  Wt::Configuration configuration("", "", "", nullptr);

  configuration.addEntryPoint(
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/users", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/a", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/foo/bar", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    std::make_shared<Wt::EntryPoint>(
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
    std::make_shared<Wt::EntryPoint>(
      Wt::EntryPointType::Application, // type
      [](const Wt::WEnvironment&) { return nullptr; }, // ApplicationCreator
      "/${var1}/${var2}/${var3}", // path
      "" // favicon
    )
  );

  configuration.addEntryPoint(
    std::make_shared<Wt::EntryPoint>(
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

BOOST_AUTO_TEST_CASE( test_entry_point_removal_removes_correct_entry_point )
{
  std::vector<std::string> paths = {"/a", "/b", "/c", "/d"};
  std::string toRemove = "/c";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();


  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 3);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 3);

  BOOST_CHECK_EQUAL(seg1.children.size(), 3);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_removes_dynamic_segment )
{
  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}"};
  std::string toRemove = "/b/${var}";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 3);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_CHECK_EQUAL(seg1.children.size(), 2);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_removes_useless_parent_segment )
{
  std::vector<std::string> paths = {"/a", "/b/c"};
  std::string toRemove = "/b/c";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 1);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 1);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_keeps_parent_segment_with_static_children )
{
  std::vector<std::string> paths = {"/a", "/b/c", "/b/d"};
  std::string toRemove = "/b/c";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 3);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_CHECK_EQUAL(seg1.children.size(), 2);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_keeps_parent_segment_with_dynamic_child )
{
  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}"};
  std::string toRemove = "/b/c";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 1);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_CHECK_EQUAL(seg1.children.size(), 2);
  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      BOOST_TEST(child->dynamicChild.get());
      BOOST_CHECK_EQUAL(child->children.size(), 0);
    }

    BOOST_CHECK_EQUAL(child->children.size(), 0);
  }

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_preserves_root )
{
  std::vector<std::string> paths = {"/"};
  std::string toRemove = "/";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 0);

  BOOST_CHECK_EQUAL(seg1.segment, "");

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_only_removes_entry_point )
{
  std::vector<std::string> paths = {"/a", "/b/c", "/b/d"};
  std::string toRemove = "/b";

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 4);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 3);

  BOOST_CHECK_EQUAL(seg1.children.size(), 2);
  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      BOOST_CHECK_EQUAL(child->children.size(), 2);
      BOOST_TEST(!child->entryPoint);
    } else {
      BOOST_CHECK_EQUAL(child->segment, "a");
      BOOST_TEST(child->entryPoint);
    }
  }

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_multiple_static )
{
  std::vector<std::string> paths = {"/a", "/b", "/c", "/d"};
  std::vector<std::string> toRemove = {"/b", "/d"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_multiple_dynamic )
{
  std::vector<std::string> paths = {"/a", "/b/${x}", "/c/${y}", "/d"};
  std::vector<std::string> toRemove = {"/b/${x}", "/c/${y}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_root_and_static )
{
  std::vector<std::string> paths = {"/", "/a", "/b"};
  std::vector<std::string> toRemove = {"/", "/b"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 1);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 1);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_mixed_static_dynamic )
{
  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}", "/d"};
  std::vector<std::string> toRemove = {"/b/c", "/b/${var}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 2);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_all )
{
  std::vector<std::string> paths = {"/a", "/b", "/c"};
  std::vector<std::string> toRemove = {"/a", "/b", "/c"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 0);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_static_and_dynamic_with_overlap )
{
  std::vector<std::string> paths = {"/x", "/y/z", "/y/${foo}", "/y/w"};
  std::vector<std::string> toRemove = {"/y/z", "/y/${foo}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();

  BOOST_CHECK_EQUAL(seg1.segment, "");
  BOOST_TEST(!seg1.dynamicChild);
  BOOST_TEST(!seg1.entryPoint);
  BOOST_CHECK_EQUAL(seg1.children.size(), 2);

  for (const auto& child : seg1.children) {
    if (child->segment == "y") {
      BOOST_TEST(!child->entryPoint);
      BOOST_TEST(!child->dynamicChild);
      BOOST_REQUIRE_EQUAL(child->children.size(), 1);

      BOOST_CHECK_EQUAL(child->children[0]->segment, "w");
      BOOST_TEST(child->children[0]->entryPoint.get());
    } else {
      BOOST_CHECK_EQUAL(child->segment, "x");
      BOOST_CHECK_EQUAL(child->children.size(), 0);
      BOOST_TEST(!child->dynamicChild);
      BOOST_TEST(child->entryPoint.get());
    }
  }
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_all_static_same_segment )
{
  std::vector<std::string> paths = {"/foo/a", "/foo/b", "/foo/c", "/bar/x"};
  std::vector<std::string> toRemove = {"/foo/a", "/foo/b", "/foo/c"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 1);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_all_dynamic_each_segment )
{
  std::vector<std::string> paths = {"/foo/${a}", "/bar/${b}", "/baz/${c}", "/keep/static"};
  std::vector<std::string> toRemove = {"/foo/${a}", "/bar/${b}", "/baz/${c}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 1);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_all_static_and_dynamic_same_segment )
{
  std::vector<std::string> paths = {"/foo/a", "/foo/b", "/foo/${id}", "/other/keep"};
  std::vector<std::string> toRemove = {"/foo/a", "/foo/b", "/foo/${id}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  Wt::EntryPointManager& mgr2 = mgrs.second;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();
  const Wt::PathSegment& seg2 = mgr2.rootPathSegment();

  TreeInfo info = computeTreeInfo(&seg2);
  BOOST_REQUIRE_EQUAL(info.staticSegmentsCount, 2);
  BOOST_REQUIRE_EQUAL(info.dynamicSegmentsCount, 0);
  BOOST_REQUIRE_EQUAL(info.entryPointsCount, 1);

  BOOST_TEST(compareSegments(&seg1, &seg2));
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_all_from_multiple_segments )
{
  std::vector<std::string> paths = {"/foo/a", "/foo/${id}", "/bar/b", "/bar/${name}", "/keep/this"};
  std::vector<std::string> toRemove = {"/foo/a", "/foo/${id}", "/bar/b", "/bar/${name}"};

  std::pair<Wt::EntryPointManager, Wt::EntryPointManager> mgrs = buildManagersWithRemoved(paths, toRemove);
  Wt::EntryPointManager& mgr1 = mgrs.first;
  const Wt::PathSegment& seg1 = mgr1.rootPathSegment();

  BOOST_CHECK_EQUAL(seg1.segment, "");
  BOOST_TEST(!seg1.dynamicChild);
  BOOST_TEST(!seg1.entryPoint);
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 1);

  BOOST_CHECK_EQUAL(seg1.children[0]->segment, "keep");
  BOOST_TEST(!seg1.children[0]->dynamicChild);
  BOOST_TEST(!seg1.children[0]->entryPoint);
  BOOST_REQUIRE_EQUAL(seg1.children[0]->children.size(), 1);

  BOOST_CHECK_EQUAL(seg1.children[0]->children[0]->segment, "this");
  BOOST_TEST(!seg1.children[0]->children[0]->dynamicChild);
  BOOST_TEST(seg1.children[0]->children[0]->entryPoint.get());
  BOOST_CHECK_EQUAL(seg1.children[0]->children[0]->children.size(), 0);
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_by_resource )
{
  Wt::EntryPointManager mgr;
  const std::vector<unsigned char> data = {'d', 'a', 't', 'a'};
  auto resource = std::make_shared<Wt::WMemoryResource>("text/plain", data);
  resource->setInternalPath("/b/resource");

  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}"};
  addPathsToManager(mgr, paths);
  mgr.addEntryPoint(std::make_shared<const Wt::EntryPoint>(resource, "/b/resource"));

  // Verify the entry point is present
  const Wt::PathSegment& seg1 = mgr.rootPathSegment();
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  Wt::PathSegment* bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 2);

  bool found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      found = true;
      break;
    }
  }

  BOOST_REQUIRE(found);

  // Remove the entry point by resource
  mgr.removeResource(resource.get());

  // Verify the entry point was removed
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 1);

  found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      found = true;
      break;
    }
  }

  BOOST_TEST(!found);
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_by_resource_only_removes_if_correct_resource )
{
  Wt::EntryPointManager mgr;
  const std::vector<unsigned char> data = {'d', 'a', 't', 'a'};
  auto resource = std::make_shared<Wt::WMemoryResource>("text/plain", data);
  auto otherResource = std::make_shared<Wt::WMemoryResource>("text/plain", data);
  resource->setInternalPath("/b/resource");
  otherResource->setInternalPath("/b/resource");

  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}"};
  addPathsToManager(mgr, paths);
  mgr.addEntryPoint(std::make_shared<const Wt::EntryPoint>(resource, "/b/resource"));

  // Verify the entry point is present
  const Wt::PathSegment& seg1 = mgr.rootPathSegment();
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  Wt::PathSegment* bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 2);

  bool found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      found = true;
      break;
    }
  }

  BOOST_REQUIRE(found);

  // Attempt to remove the entry point by a different resource
  mgr.removeResource(otherResource.get());

  // Verify the entry point was not removed and the resource is still present
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 2);

  found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      BOOST_CHECK_EQUAL(child->entryPoint->resource(), resource.get());
      found = true;
      break;
    }
  }

  BOOST_TEST(found);
}

BOOST_AUTO_TEST_CASE( test_entry_point_removal_by_resource_only_remove_if_resource_exists )
{
  Wt::EntryPointManager mgr;
  const std::vector<unsigned char> data = {'d', 'a', 't', 'a'};
  auto resource = std::make_shared<Wt::WMemoryResource>("text/plain", data);
  resource->setInternalPath("/b/resource");

  std::vector<std::string> paths = {"/a", "/b/c", "/b/${var}", "/b/resource"};
  addPathsToManager(mgr, paths);

  // Verify the entry point is present
  const Wt::PathSegment& seg1 = mgr.rootPathSegment();
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  Wt::PathSegment* bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 2);

  bool found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      found = true;
      break;
    }
  }

  BOOST_REQUIRE(found);

  // Try to remove the entry point by resource
  mgr.removeResource(resource.get());

  // Verify the entry point was not removed
  BOOST_REQUIRE_EQUAL(seg1.children.size(), 2);
  bSeg = nullptr;

  for (const auto& child : seg1.children) {
    if (child->segment == "b") {
      bSeg = child.get();
      break;
    }
  }

  BOOST_REQUIRE(bSeg != nullptr);
  BOOST_REQUIRE_EQUAL(bSeg->children.size(), 2);

  found = false;
  for (const auto& child : bSeg->children) {
    if (child->segment == "resource") {
      found = true;
      break;
    }
  }

  BOOST_TEST(found);
}