// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WJAVASCRIPT_OBJECT_STORE_H_
#define WJAVASCRIPT_OBJECT_STORE_H_

#include "Wt/WJavaScriptExposableObject.h"
#include "Wt/WJavaScriptHandle.h"

#include <string>

namespace Wt {

class WStringStream;
class WWidget;

class WJavaScriptObjectStorage {
public:
  WJavaScriptObjectStorage(WWidget *widget);

  ~WJavaScriptObjectStorage();

  // NOTE: transfers ownership
  // T extends WJavaScriptExposableObject
  template<typename T>
  WJavaScriptHandle<T> addObject(T *o)
  {
    int index = doAddObject(o);
    return WJavaScriptHandle<T>(index, o);
  }

  void updateJs(WStringStream &js, bool all);

  std::size_t size() const;

  void assignFromJSON(const std::string &json);

  std::string jsRef() const;

private:
  template<typename T>
  friend class WJavaScriptHandle;

  int doAddObject(WJavaScriptExposableObject *o);

  std::vector<WJavaScriptExposableObject *> jsValues_;
  std::vector<bool> dirty_;
  WWidget *widget_;
};

}

#endif // WJAVASCRIPT_OBJECT_STORE_H_
