/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WLogger.h"

#include "EntryPointManager.h"

namespace {
  static const std::string EMPTY_STR;
  static std::string FORWARD_SLASH = "/";
}

namespace Wt {

LOGGER("EntryPointManager");

void EntryPointManager::addEntryPoint(const std::shared_ptr<const EntryPoint>& ep)
{
  entryPoints_.push_back(ep);

  registerEntryPoint(entryPoints_.back());
}

bool EntryPointManager::tryAddResource(const std::shared_ptr<const EntryPoint>& ep)
{
  for (std::size_t i = 0; i < entryPoints_.size(); ++i) {
    if (entryPoints_[i]->path() == ep->path()) {
      return false;
    }
  }

  entryPoints_.push_back(ep);

  registerEntryPoint(entryPoints_.back());

  return true;
}

void EntryPointManager::removeEntryPoint(const std::string& path)
{
  for (unsigned i = 0; i < entryPoints_.size(); ++i) {
    const std::shared_ptr<EntryPoint>& ep = entryPoints_[i];
    if (ep->path() == path) {
      rootPathSegment_.children.clear();
      entryPoints_.erase(entryPoints_.begin() + i);
      for (std::size_t j = 0; j < entryPoints_.size(); ++j) {
        registerEntryPoint(entryPoints_[j]);
      }
      break;
    }
  }
}

EntryPointMatch EntryPointManager::matchEntryPoint(const std::string& scriptName,
                                               const std::string& path,
                                               bool matchAfterSlash) const
{
  if (!scriptName.empty()) {
    LOG_DEBUG("matchEntryPoint: matching entry point, scriptName: '" << scriptName << "', path: '" << path << '\'');
    auto scriptNameMatch = matchEntryPoint(EMPTY_STR, scriptName + path, matchAfterSlash);
    if (scriptNameMatch.extraStartIndex < scriptName.size()) {
      // Discard script name match if the tail of the match is longer than the actual path
      scriptNameMatch = EntryPointMatch();
    }
    auto pathOnlyMatch = matchEntryPoint(EMPTY_STR, path, matchAfterSlash);
    if (scriptNameMatch < pathOnlyMatch) {
      if (scriptNameMatch.entryPoint) {
        // Fix extraStartIndex, so it's an index of path instead of scriptName + path
        scriptNameMatch.extraStartIndex -= scriptName.size();
      }
      return scriptNameMatch;
    } else {
      return pathOnlyMatch;
    }
  }

  LOG_DEBUG("matchEntryPoint: matching entry point, path: '" << path << '\'');

  const PathSegment* pathSegment = &rootPathSegment_;

  // Flag marking whether any of the matched segments is dynamic
  bool dynamic = false;

  typedef boost::split_iterator<std::string::const_iterator> spliterator;

  spliterator it;
  if (!path.empty()) {
    it = spliterator(path.begin() + 1, path.end(),
                     boost::first_finder(FORWARD_SLASH, boost::is_equal()));
    // Move down the routing tree, segment per segment
    for (;it != spliterator(); ++it) {
      // Find exact path match for segment
      const auto& children = pathSegment->children;
      const PathSegment* childSegment = nullptr;
      auto c = std::find_if(children.begin(), children.end(), [&it](const std::unique_ptr<PathSegment>& c) {
        return c->segment == *it;
      });
      if (c != children.end())
        childSegment = c->get();

      // No exact match, see if there is a dynamic segment
      if (!childSegment &&
          !it->empty() &&
          pathSegment->dynamicChild) {
        childSegment = pathSegment->dynamicChild.get();
        dynamic = true;
      }

      // No match, deepest match reached
      if (!childSegment)
        break;

      // Move to the next segment
      pathSegment = childSegment;
    }
  } else {
    matchAfterSlash = true;
  }

  // Move up from the found segment, until we find one that corresponds to an entrypoint
  std::shared_ptr<const EntryPoint> match = nullptr;
  for (; pathSegment != nullptr; pathSegment = pathSegment->parent) {
    // If matchAfterSlash is true,
    // then the path /head/tail
    // may match the entry point /head/
    if (matchAfterSlash && (path.empty() || it != spliterator())) {
      const auto& children = pathSegment->children;
      if (!children.empty() && children.front()->segment.empty()) {
        match = children.front()->entryPoint;
        break;
      }
    }
    // If the current segment has an entrypoint, we're done
    if (pathSegment->entryPoint) {
      match = pathSegment->entryPoint;
      break;
    }
  }

  if (match && dynamic) {
    // Process path parameters
    EntryPointMatch result;
    result.entryPoint = match;
    // Iterate concurrently over the path (it1),
    // and the matched endpoint's path (it2)
    spliterator it1 = spliterator(path.begin() + 1, path.end(),
                                  boost::first_finder(FORWARD_SLASH, boost::is_equal()));
    for (spliterator it2 = spliterator(match->path().begin() + 1, match->path().end(),
                                       boost::first_finder(FORWARD_SLASH, boost::is_equal()));
         it1 != spliterator() && it2 != spliterator(); ++it1, ++it2) {
      // Check dynamic segment (e.g. "${var}")
      if (boost::starts_with(*it2, "${") &&
          boost::ends_with(*it2, "}")) {
        auto range = boost::iterator_range<std::string::const_iterator>(it2->begin() + 2, it2->end() - 1);
        result.urlParams.push_back(std::make_pair(boost::copy_range<std::string>(range),
                                                  boost::copy_range<std::string>(*it1)));
      }
    }
    if (it1 == spliterator())
      result.extraStartIndex = path.size(); // no extra path
    else
      result.extraStartIndex = std::distance(path.begin(), it1->begin()) - 1; // there's more

    LOG_DEBUG("matchEntryPoint: path '" << path << "' matches dynamic entry point: '" << match->path() << '\'');
    return result;
  } else if (match) {
    LOG_DEBUG("matchEntryPoint: path '" << path << "' matches entry point: '" << match->path() << '\'');
    return EntryPointMatch(match, path.empty() ? 0 : match->path().size()); // simple match
  } else {
    LOG_DEBUG("matchEntryPoint: no entry point match found for path: '" << path << '\'');
    return EntryPointMatch(); // no match
  }
}

void EntryPointManager::registerEntryPoint(const std::shared_ptr<const EntryPoint>& ep)
{
  const std::string& path = ep->path();

  assert(path.empty() || path[0] == '/');

  // The PathSegment in the routing tree where this entrypoint will end up
  PathSegment* pathSegment = &rootPathSegment_;

  if (path.empty()) {
    pathSegment->entryPoint = ep;
    return;
  }

  typedef boost::split_iterator<std::string::const_iterator> spliterator;
  for (spliterator it = spliterator(path.begin() + 1, path.end(),
                                    boost::first_finder(FORWARD_SLASH, boost::is_equal()));
       it != spliterator(); ++it) {
    PathSegment* childSegment = nullptr;
    if (boost::starts_with(*it, "${") &&
        boost::ends_with(*it, "}")) {
      // This is a dynamic segment, e.g. ${var}
      if (!pathSegment->dynamicChild) {
        pathSegment->dynamicChild = std::unique_ptr<PathSegment>(new PathSegment("", pathSegment));
      }
      childSegment = pathSegment->dynamicChild.get();
    } else {
      // This is a normal segment
      auto& children = pathSegment->children;
      auto c = std::find_if(children.begin(), children.end(), [&it](const std::unique_ptr<PathSegment>& c) {
        return c->segment == *it;
      });
      if (c != children.end())
        childSegment = c->get();

      if (!childSegment) {
        if (it->empty()) {
          // Empty part (entry point with trailing slash)
          // Put it in front by convention
          children.insert(children.begin(), std::make_unique<PathSegment>("", pathSegment));
          childSegment = children.front().get();
        } else {
          children.push_back(std::make_unique<PathSegment>(boost::copy_range<std::string>(*it), pathSegment));
          childSegment = children.back().get();
        }
      }
    }
    pathSegment = childSegment;
  }

  pathSegment->entryPoint = ep;
}

} // namespace Wt