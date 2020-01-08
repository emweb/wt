// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WJavaScriptObjectStorage.h"

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WWidget.h"

#include "Wt/Json/Object.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Value.h"

#include "web/WebUtils.h"

namespace Wt {

LOGGER("WJavaScriptObjectStorage");

WJavaScriptObjectStorage::WJavaScriptObjectStorage(WWidget *widget)
  : widget_(widget)
{ }

WJavaScriptObjectStorage::~WJavaScriptObjectStorage()
{
  for (std::size_t i = 0; i < jsValues_.size(); ++i) {
    delete jsValues_[i];
  }
}

int WJavaScriptObjectStorage::doAddObject(WJavaScriptExposableObject *o)
{
  jsValues_.push_back(o);
  dirty_.push_back(true);
  std::size_t index = jsValues_.size() - 1;
  o->clientBinding_ = new WJavaScriptExposableObject::JSInfo(
      this, jsRef() + ".jsValues[" + std::to_string(index) + "]");
  return (int)index;
}

void WJavaScriptObjectStorage::updateJs(WStringStream &js, bool all)
{
  for (std::size_t i = 0; i < jsValues_.size(); ++i) {
    if (dirty_[i] || all) {
      js << jsRef() + ".setJsValue(" + std::to_string(i) + ",";
      js << jsValues_[i]->jsValue() << ");";
      dirty_[i] = false;
    }
  }
}

std::size_t WJavaScriptObjectStorage::size() const
{
  return jsValues_.size();
}

void WJavaScriptObjectStorage::assignFromJSON(const std::string &json)
{
  try {
    Json::Value result;
    Json::parse(json, result);
    Json::Object &o = result;

    if (jsValues_.size() < o.size())
      throw WException("JSON array length is larger than number of jsValues");

    for (Json::Object::iterator i = o.begin();
         i != o.end(); ++i) {
      std::size_t idx = Utils::stoull(i->first);
      if (idx >= jsValues_.size())
        throw WException("JSON value index is outside of bounds");
      if (!dirty_[idx])
        jsValues_[idx]->assignFromJSON(i->second);
    }
  } catch (const Json::ParseError &e) {
    LOG_ERROR("Failed to parse JSON: " + std::string(e.what()));
  } catch (const WException &e) {
    LOG_ERROR("Failed to assign value from JSON: " + std::string(e.what()));
  } catch (const std::invalid_argument &e) {
    LOG_ERROR("Failed to assign value from JSON, couldn't cast index: " + std::string(e.what()));
  }
}

std::string WJavaScriptObjectStorage::jsRef() const
{
  return widget_->jsRef() + ".wtJSObj";
}

}
