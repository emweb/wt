// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WJavaScriptObjectStorage>
#include <Wt/WStringStream>

namespace Wt {

WJavaScriptObjectStorage::WJavaScriptObjectStorage(const std::string &jsRef)
  : jsRef(jsRef)
{ }

WJavaScriptObjectStorage::~WJavaScriptObjectStorage()
{
  for (std::size_t i = 0; i < jsValues.size(); ++i) {
    delete jsValues[i];
  }
}

int WJavaScriptObjectStorage::doAddObject(WJavaScriptExposableObject *o)
{
  jsValues.push_back(o);
  dirty.push_back(true);
  std::size_t index = jsValues.size() - 1;
  o->clientBinding_ = new WJavaScriptExposableObject::JSInfo(
      this, jsRef + ".jsValues[" + boost::lexical_cast<std::string>(index) + "]");
  return (int)index;
}

void WJavaScriptObjectStorage::updateJs(WStringStream &js)
{
  for (std::size_t i = 0; i < jsValues.size(); ++i) {
    if (dirty[i]) {
      js << jsValues[i]->jsRef() << "=" << jsValues[i]->jsValue() << ";";
      dirty[i] = false;
    }
  }
}

std::size_t WJavaScriptObjectStorage::size() const
{
  return jsValues.size();
}

}
