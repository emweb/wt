// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "Wt/WApplication.h"
#include "Wt/WGlobal.h"

#include <deque>

#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H
namespace Wt {

class WT_API EntryPoint {
public:
  EntryPoint(EntryPointType type, ApplicationCreator appCallback,
	     const std::string& path, 
             const std::string& favicon);
  EntryPoint(WResource *resource, const std::string& path);
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
  ApplicationCreator appCallback_;
  std::string path_;
  std::string favicon_;
};

typedef std::deque<EntryPoint> EntryPointList;

struct WT_API EntryPointMatch {
  EntryPointMatch()
    : entryPoint(nullptr),
      extra(0)
  { }

  EntryPointMatch(
      const EntryPoint *ep,
      std::size_t x)
    : entryPoint(ep),
      extra(x)
  { }

  const EntryPoint *entryPoint;
  std::vector<std::pair<std::string, std::string> > urlParams;
  std::size_t extra;
};

}

#endif // ENTRYPOINT_H
