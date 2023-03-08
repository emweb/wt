// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include "Wt/WApplication.h"
#include "Wt/WGlobal.h"

#include <deque>

namespace Wt {

class WT_API EntryPoint {
public:
  EntryPoint(EntryPointType type, ApplicationCreator appCallback,
             const std::string& path,
             const std::string& favicon);
  EntryPoint(WResource *resource, const std::string& path);
  EntryPoint(const std::shared_ptr<WResource>& resource, const std::string& path);
  ~EntryPoint();

  void setPath(const std::string& path);

  EntryPointType type() const { return type_; }
  WResource *resource() const { return resource_; }
  ApplicationCreator appCallback() const { return appCallback_; }
  const std::string& path() const { return path_; }
  const std::string& favicon() const { return favicon_; }

private:
  EntryPointType type_;
  WResource *resource_;
  std::shared_ptr<WResource> ownedResource_;
  ApplicationCreator appCallback_;
  std::string path_;
  std::string favicon_;
};

typedef std::deque<EntryPoint> EntryPointList;

struct WT_API EntryPointMatch {
  EntryPointMatch() noexcept
    : entryPoint(nullptr),
      extraStartIndex(0)
  { }

  EntryPointMatch(
      const EntryPoint *ep,
      std::size_t extraStartIndex) noexcept
    : entryPoint(ep),
      extraStartIndex(extraStartIndex)
  { }

  bool operator<(const EntryPointMatch& other) const noexcept;

  std::size_t depth() const noexcept;

  const EntryPoint *entryPoint;
  std::vector<std::pair<std::string, std::string> > urlParams;
  std::size_t extraStartIndex;
};

}

#endif // ENTRYPOINT_H
