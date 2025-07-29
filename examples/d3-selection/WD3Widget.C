/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WJavaScriptPreamble.h>

#include "WD3Widget.h"

using namespace Wt;

WD3Widget::D3ObjectRef::D3ObjectRef(const std::string& obj, WD3Widget* widget)
  : widget_(widget),
    connected_(false)
{
  // Assign a unique id to the D3 object
  id_ = widget->lastObjectId_++;
  // Instruct the JS side to save the D3 object reference
  widget_->jsUpdates_ << widget_->jsRef() << ".d3WidgetObj.saveD3Obj(" << id_ << "," << obj <<");";
  // Build a JS reference string for later use
  jsRef_ = widget_->jsRef() + ".d3WidgetObj.getD3Obj(" + std::to_string(id_) + ")";
}

WD3Widget::D3ObjectRef::~D3ObjectRef()
{
  // Instruct the JS side to remove the D3 object reference
  widget_->jsUpdates_ << widget_->jsRef() << ".d3WidgetObj.removeD3Obj(" << id_ << ");";
}

Signal<>& WD3Widget::D3ObjectRef::clicked()
{
  // Connect the selectionSignal_ to this object's emitIfTargeted if not
  // already connected
  if (!connected_) {
    widget_->selectionSignal_.connect(this, &D3ObjectRef::emitIfTargeted);
    connected_ = true;
  }
  // Return the clicked_ signal, which can be used to listen for click
  // events on this D3 object
  return clicked_;
}

void WD3Widget::D3ObjectRef::emitIfTargeted(long long id)
{
  // Emit the clicked_ signal if the provided id matches this object's id
  if (id == id_) {
    clicked_.emit();
  }
}

WD3Widget::JsObject::JsObject(const std::string& obj)
  : js_(obj)
{ }

WD3Widget::D3Object::D3Object(const std::string& js, WD3Widget* widget)
  : widget_(widget),
    savedRef_(nullptr),
    js_(js)
{ }

WD3Widget::D3Object::D3Object(const D3Object& other)
  : widget_(other.widget_),
    savedRef_(other.savedRef_)
{
  // Ensure the other object is persisted before copying its JS reference
  other.persist();

  widget_ = other.widget_;
  savedRef_ = other.savedRef_;
  js_ = other.js_;
}

WD3Widget::D3Object& WD3Widget::D3Object::operator=(const D3Object& other)
{
  // Ensure the other object is persisted before copying its JS reference
  other.persist();

  widget_ = other.widget_;
  savedRef_ = other.savedRef_;
  js_ = other.js_;
  return *this;
}

WD3Widget::D3Object::D3Object(D3Object&& other)
  : widget_(std::move(other.widget_)),
    savedRef_(std::move(other.savedRef_)),
    js_(std::move(other.js_))
{
  jsUpdates_ = std::move(other.jsUpdates_);

  // Reset other's jsUpdates_ to avoid pushing updates at deletion
  other.jsUpdates_ = WStringStream();
}

WD3Widget::D3Object& WD3Widget::D3Object::operator=(D3Object&& other)
{
  widget_ = std::move(other.widget_);
  savedRef_ = std::move(other.savedRef_);
  js_ = std::move(other.js_);
  jsUpdates_ = std::move(other.jsUpdates_);

  // Reset other's jsUpdates_ to avoid pushing updates at deletion
  other.jsUpdates_ = WStringStream();

  return *this;
}

WD3Widget::D3Object::~D3Object()
{
  pushUpdates();
}

void WD3Widget::D3Object::persist() const
{
  // Push any pending JS updates
  pushUpdates();
  // If the object is valid and not already persisted, create a new
  // D3ObjectRef and update js_ to the new JS reference
  if (isValid() && (!savedRef_ || savedRef_->jsRef() != js_)) {
    savedRef_ = std::make_shared<D3ObjectRef>(js_, widget_);
    js_ = savedRef_->jsRef();
  }
}

void WD3Widget::D3Object::pushUpdates() const
{
  // If there are pending JS updates, append them to the widget's
  // jsUpdates_ stream
  if (isValid() && !jsUpdates_.empty()) {
    widget_->jsUpdates_ << js_ << jsUpdates_.str() << ";";
    jsUpdates_.clear();
  }
}

WD3Widget::Selection::Selection()
{ }

WD3Widget::Selection::Selection(const std::string& js, WD3Widget* widget)
  : obj_(js, widget)
{ }

WD3Widget::Selection& WD3Widget::Selection::attr(const std::string& name, const std::string& value)
{
  return attr(name, JsObject(jsStringLiteral(value)));
}

WD3Widget::Selection& WD3Widget::Selection::attr(const std::string& name, const JsObject& value)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".attr(" << jsStringLiteral(name) << "," << value.js_ << ")";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::classed(const std::string& name, bool value)
{
  return classed(name, JsObject(value ? "true" : "false"));
}

WD3Widget::Selection& WD3Widget::Selection::classed(const std::string& name, const JsObject& value)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".classed(" << jsStringLiteral(name);

  if (!value.js_.empty()) {
    obj_.jsUpdates_ << "," << value.js_;
  }

  obj_.jsUpdates_ << ")";

  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::style(const std::string& name, const std::string& value, const std::string& priority)
{
  return style(name, JsObject(jsStringLiteral(value)), priority);
}

WD3Widget::Selection& WD3Widget::Selection::style(const std::string& name, const JsObject& value, const std::string& priority)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".style(" << jsStringLiteral(name) << "," << value.js_;

  if (!priority.empty()) {
    obj_.jsUpdates_ << "," << jsStringLiteral(priority);
  }

  obj_.jsUpdates_ << ")";

  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::property(const std::string& name, const std::string& value)
{
  return property(name, JsObject(jsStringLiteral(value)));
}

WD3Widget::Selection& WD3Widget::Selection::property(const std::string& name, const JsObject& value)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".property(" << jsStringLiteral(name) << "," << value.js_ << ")";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::text(const std::string& value)
{
  return text(JsObject(jsStringLiteral(value)));
}

WD3Widget::Selection& WD3Widget::Selection::text(const JsObject& value)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".text(" << value.js_ << ")";
  return *this;
}

WD3Widget::Selection WD3Widget::Selection::append(const std::string& type)
{
  return append(JsObject(jsStringLiteral(type)));
}

WD3Widget::Selection WD3Widget::Selection::append(const JsObject& type)
{
  return getModified(".append(" + type.js_ + ")");
}

WD3Widget::Selection WD3Widget::Selection::insert(const std::string& type, const std::string& before)
{
  return insert(JsObject(jsStringLiteral(type)), JsObject(jsStringLiteral(before)));
}

WD3Widget::Selection WD3Widget::Selection::insert(const JsObject& type, const std::string& before)
{
  return insert(type, JsObject(jsStringLiteral(before)));
}

WD3Widget::Selection WD3Widget::Selection::insert(const std::string& type, const JsObject& before)
{
  return insert(JsObject(jsStringLiteral(type)), before);
}

WD3Widget::Selection WD3Widget::Selection::insert(const JsObject& type, const JsObject& before)
{
  WStringStream ss;
  ss << ".insert(" << type.js_;
  if (!before.js_.empty()) {
    ss << "," << before.js_;
  }
  ss << ")";

  return getModified(ss.str());
}

WD3Widget::Selection WD3Widget::Selection::remove()
{
  return getModified(".remove()");
}

WD3Widget::Selection WD3Widget::Selection::sort(const JsObject& compare)
{
  return getModified(".sort(" + compare.js_ + ")");
}

WD3Widget::Selection& WD3Widget::Selection::order()
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".order()";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::raise()
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".raise()";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::lower()
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".lower()";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::datum(const std::string& value)
{
  return datum(JsObject(jsStringLiteral(value)));
}

WD3Widget::Selection& WD3Widget::Selection::datum(const JsObject& value)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".datum(" << value.js_ << ")";
  return *this;
}

WD3Widget::Selection WD3Widget::Selection::merge(const WD3Widget::Selection& other)
{
  other.throwIfInvalid();

  return getModified(".merge(" + other.obj_.js_ + ")");
}

WD3Widget::Selection& WD3Widget::Selection::persist()
{
  throwIfInvalid();

  obj_.persist();

  return *this;
}

WD3Widget::Selection WD3Widget::Selection::clone(bool deep)
{
  Wt::WStringStream ss;
  ss << ".clone(" << (deep ? "true" : "false") << ")";
  return getModified(ss.str());
}

WD3Widget::Selection WD3Widget::Selection::data(const JsObject& data, const JsObject& key) const
{
  throwIfInvalid();

  Selection result = *this;

  Wt::WStringStream ss;
  ss << ".data(" << data.js_;

  if (!key.js_.empty()) {
    ss << "," << key.js_;
  }
  ss << ")";

  result.obj_.js_ = obj_.js_ + ss.str();
  return result;
}

WD3Widget::Selection WD3Widget::Selection::join(const std::string& enter, const JsObject& update, const JsObject& exit)
{
  return join(JsObject(jsStringLiteral(enter)), update, exit);
}

WD3Widget::Selection WD3Widget::Selection::join(const JsObject& enter, const JsObject& update, const JsObject& exit)
{
  throwIfInvalid();

  Wt::WStringStream ss;
  ss << ".join(" << enter.js_;
  if (!update.js_.empty()) {
    ss << "," << update.js_;

    if (!exit.js_.empty()) {
      ss << "," << exit.js_;
    }
  }
  ss << ")";

  return getModified(ss.str());
}

WD3Widget::Selection WD3Widget::Selection::enter() const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".enter()";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::exit() const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".exit()";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::select(const std::string& selector) const
{
  throwIfInvalid();
  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".select(" + jsStringLiteralOrEmpty(selector) + ")";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::selectAll(const std::string& selector) const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".selectAll(" + jsStringLiteralOrEmpty(selector) + ")";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::selectChild(const std::string& selector) const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".selectChild(" + jsStringLiteralOrEmpty(selector) + ")";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::selectChildren(const std::string& selector) const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".selectChildren(" + jsStringLiteralOrEmpty(selector) + ")";
  return result;
}

WD3Widget::Selection WD3Widget::Selection::filter(const std::string& filter) const
{
  throwIfInvalid();

  Selection result = *this;
  result.obj_.js_ = obj_.js_ + ".filter(" + jsStringLiteralOrEmpty(filter) + ")";
  return result;
}

WD3Widget::Selection& WD3Widget::Selection::each(const JsObject& callback)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".each(" << callback.js_ << ")";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::on(const std::string& event, const JsObject& callback, const JsObject& options)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".on(" << jsStringLiteral(event) << "," << callback.js_;
  if (!options.js_.empty()) {
    obj_.jsUpdates_ << "," << options.js_;
  }
  obj_.jsUpdates_ << ")";
  return *this;
}

WD3Widget::Selection& WD3Widget::Selection::call(const JsObject& callback)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".call(" << callback.js_ << ")";
  return *this;
}

void WD3Widget::Selection::pushUpdates() const
{
  obj_.pushUpdates();
}

void WD3Widget::Selection::throwIfInvalid() const
{
  if (!isValid()) {
    throw WException("Trying to use an invalid WD3Widget::Selection object.");
  }
}

WD3Widget::Selection WD3Widget::Selection::getModified(const std::string& modification)
{
  throwIfInvalid();

  Selection result = *this;
  /*
   * Append the modification to obj_.js_ (not obj_.jsUpdates_) to
   * create a new JS object reference (e.g., a new selection), not just
   * queue a method call. This enables further chaining or event
   * binding.
   *
   * Calling persist() materializes the JS object and resets obj_.js_
   * to the new reference, so the modification is not applied twice.
   */
  result.obj_.js_ = obj_.js_ + modification;
  return result.persist();
}

void WD3Widget::Selection::pushArgs(const std::string& s)
{
  obj_.jsUpdates_ << "," << jsStringLiteral(s);
}

void WD3Widget::Selection::pushArgs(const JsObject& obj)
{
  obj_.jsUpdates_ << "," << obj.js_;
}

void WD3Widget::Selection::pushArgs(const bool& b)
{
  obj_.jsUpdates_ << "," << (b ? "true" : "false");
}

void WD3Widget::Selection::pushArgs(const WD3Widget::Selection& selection)
{
  selection.throwIfInvalid();
  obj_.jsUpdates_ << "," << selection.obj_.js_;
}

Signal<>& WD3Widget::Selection::clicked()
{
  throwIfInvalid();
  // Persist the selection and retrieve the clicked signal from the
  // saved D3ObjectRef
  persist();
  Signal<>& res = obj_.savedRef_->clicked();

  // Make the click handler emit the selectionSignal_
  WStringStream ss;
  ss << "function(e) {"
     << obj_.widget_->selectionSignal_.createCall({"" + std::to_string(obj_.savedRef_->d3Id())})
     << "}";

  on("click", JsObject(ss.str()));
  return res;
}

WD3Widget::WD3Widget()
  : lastObjectId_(0),
    jsDefined_(false),
    selectionSignal_(this, "selectionSignal")
{ }

void WD3Widget::initializeD3()
{ }

void WD3Widget::updateD3()
{ }

void WD3Widget::render(WFlags<RenderFlag> flags)
{
  // Ensure the required JavaScript is defined
  defineJavaScript();

  // Call initializeD3() on full render
  if (flags.test(RenderFlag::Full)) {
    initializeD3();
  }

  updateD3();

  // Execute the JS updates if any
  if (!jsUpdates_.empty()) {
    doJavaScript(jsUpdates_.str());
    jsUpdates_.clear();
  }

  WInteractWidget::render(flags);
}

void WD3Widget::defineJavaScript()
{
  // Only define JS once
  if (jsDefined_) {
    return;
  }
  WApplication* app = WApplication::instance();

  // Load D3.js and WD3Widget.js scripts if not already loaded
  std::string d3Filename = "d3.v7.min.js";
  app->require(d3Filename);
  app->require("WD3Widget.js", "WD3Widget");

  // Instantiate the WD3Widget JS object for this widget
  WStringStream ss;
  ss << "new WD3Widget("
     << app->javaScriptClass() << ","
     << jsRef() << ","
     << width().toPixels() << ","
     << height().toPixels() << ");";

  setJavaScriptMember("d3WidgetObj", ss.str());

  jsDefined_ = true;
}

std::string WD3Widget::jsStringLiteralOrEmpty(const std::string& str)
{
  return str.empty() ? "" : jsStringLiteral(str);
}