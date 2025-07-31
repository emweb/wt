// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef ENTRYPOINT_MANAGER_H
#define ENTRYPOINT_MANAGER_H

#include "EntryPoint.h"

#include <boost/algorithm/string.hpp>

namespace Wt {

/// A segment in the deployment path of an entry point,
/// used for routing.
struct WT_API PathSegment {
  PathSegment()
    : parent(nullptr),
      entryPoint(nullptr)
  { }

  PathSegment(const std::string& s,
              PathSegment* p)
    : parent(p),
      entryPoint(nullptr),
      segment(s)
  { }

  PathSegment* parent;
  std::shared_ptr<const EntryPoint> entryPoint;
  std::vector<std::unique_ptr<PathSegment>> children; // Static path segments
  std::unique_ptr<PathSegment> dynamicChild; // Dynamic path segment, lowest priority
  std::string segment;
};

class WT_API EntryPointManager
{
public:
  void addEntryPoint(const std::shared_ptr<const EntryPoint>& entryPoint);
  bool tryAddResource(const std::shared_ptr<const EntryPoint>& entryPoint); // Returns bool indicating success:
                                                     // false if entry point existed already
  void removeEntryPoint(const std::string& path);
  // Returns matching entry point and match length
  EntryPointMatch matchEntryPoint(const std::string& scriptName,
                                  const std::string& path,
                                  bool matchAfterSlash) const;

  const EntryPointList& entryPoints() const { return entryPoints_; }
  const PathSegment& rootPathSegment() const { return rootPathSegment_; }

private:
  EntryPointList entryPoints_;
  PathSegment rootPathSegment_; /// The toplevel path segment ('/') for routing,
                                /// root of the routing tree.

  // Add the given entryPoint to the routing tree
  // NOTE: Server may not be running, or WRITE_LOCK should
  // be grabbed before registerEntryPoint is invoked.
  // This is to be used by the other entry point functions
  // (addEntryPoint, tryAddResource, removeEntryPoint,...)
  void registerEntryPoint(const std::shared_ptr<const EntryPoint>& entryPoint);
};

} // namespace Wt

#endif // ENTRYPOINT_MANAGER_H