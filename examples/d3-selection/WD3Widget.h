// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#pragma once

#include <Wt/WInteractWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WStringStream.h>

using namespace Wt;

// WD3Widget: A widget for embedding D3.js visualizations in Wt
class WD3Widget: public WInteractWidget
{
private:
  // D3ObjectRef: Reference to a D3 object managed in JS
  class D3ObjectRef: public WObject
  {
  public:
    // Constructor: saves a D3 object reference
    D3ObjectRef(const std::string& obj, WD3Widget* widget);
    // Destructor: removes the D3 object reference
    ~D3ObjectRef();

    D3ObjectRef(const D3ObjectRef&) = delete;
    D3ObjectRef& operator=(const D3ObjectRef&) = delete;
    D3ObjectRef(D3ObjectRef&&) = delete;
    D3ObjectRef& operator=(D3ObjectRef&&) = delete;

    // Returns the JavaScript reference string
    std::string jsRef() const { return jsRef_; }
    // Returns the D3 object id
    long long d3Id() const { return id_; }

    // Signal emitted when this object is clicked
    Signal<>& clicked();

  private:
    WD3Widget* widget_; // Parent widget
    bool connected_;    // Whether signal is connected
    std::string jsRef_; // JavaScript reference
    long long id_;      // D3 object id
    Signal<> clicked_;  // Clicked signal

    // Emits clicked_ if the id matches
    void emitIfTargeted(long long id);
  };

  // D3Object: Represents a D3 object and its JS state
  struct D3Object
  {
  public:
    // Constructor
    D3Object(const std::string& js = "", WD3Widget* widget = nullptr);

    // Copy/move constructors and assignment
    D3Object(const D3Object& other);
    D3Object& operator=(const D3Object& other);
    D3Object(D3Object&& other);
    D3Object& operator=(D3Object&& other);

    // Destructor
    ~D3Object();

    // Returns true if the object is valid
    bool isValid() const { return widget_; }

    operator bool() const { return isValid(); }

    // Persists the D3 object in JS
    void persist() const;
    // Pushes JS updates to the widget
    void pushUpdates() const;

    WD3Widget* widget_; // Parent widget
    mutable std::shared_ptr<D3ObjectRef> savedRef_; // Saved JS ref

    mutable std::string js_; // JS code
    mutable WStringStream jsUpdates_; // JS updates
  };

public:
  // JsObject: Wrapper for a JS value
  struct JsObject
  {
    // Constructor
    explicit JsObject(const std::string& obj = "");
  private:
    std::string js_; // JS code

    friend class WD3Widget;
  };

  // Selection: Represents a D3 selection and provides D3 API
  class Selection
  {
  public:
    // Constructors
    Selection();
    Selection(const std::string& js, WD3Widget* widget);

    // Returns true if the selection is valid
    bool isValid() const { return obj_.isValid(); }

    operator bool() const { return obj_; }

    // D3 selection API methods
    Selection& attr(const std::string& name, const std::string& value); // Set attribute
    Selection& attr(const std::string& name, const JsObject& value);

    Selection& classed(const std::string& name, bool value); // Set class
    Selection& classed(const std::string& name, const JsObject& value = JsObject());

    Selection& style(const std::string& name, const std::string& value, const std::string& priority = ""); // Set style
    Selection& style(const std::string& name, const JsObject& value, const std::string& priority = "");

    Selection& property(const std::string& name, const std::string& value); // Set property
    Selection& property(const std::string& name, const JsObject& value);

    Selection& text(const std::string& value); // Set text
    Selection& text(const JsObject& value);

    Selection append(const std::string& type); // Append element
    Selection append(const JsObject& type);

    Selection insert(const std::string& type, const std::string& before); // Insert element
    Selection insert(const JsObject& type, const std::string& before);
    Selection insert(const std::string& type, const JsObject& before = JsObject());
    Selection insert(const JsObject& type, const JsObject& before = JsObject());

    Selection remove(); // Remove element

    Selection clone(bool deep); // Clone selection

    Selection sort(const JsObject& compare = JsObject()); // Sort selection

    Selection& order(); // Order selection
    Selection& raise(); // Raise selection
    Selection& lower(); // Lower selection

    Selection data(const JsObject& data, const JsObject& key = JsObject()) const; // Bind data

    Selection join(const JsObject& enter,
                   const JsObject& update = JsObject(),
                   const JsObject& exit = JsObject()); // Join selections

    Selection join(const std::string& enter,
                   const JsObject& update = JsObject(),
                   const JsObject& exit = JsObject());

    Selection enter() const; // Enter selection
    Selection exit() const;  // Exit selection

    Selection& datum(const std::string& value); // Set datum
    Selection& datum(const JsObject& value);

    Selection merge(const Selection& other); // Merge selections

    // D3 selection traversal
    Selection select(const std::string& selector = "") const;
    Selection selectAll(const std::string& selector = "") const;
    Selection selectChild(const std::string& selector = "") const;
    Selection selectChildren(const std::string& selector = "") const;
    Selection filter(const std::string& filter) const;

    Selection& selection() { return *this; } // Return self

    Selection& each(const JsObject& callback); // For each

    Selection& on(const std::string& event, const JsObject& callback, const JsObject& options = JsObject()); // Event handler

    // Call a JS function with arguments on the selection
    template<typename... Args>
    Selection& call(const JsObject& callback, const Args&... args);

    // Call a JS function without arguments on the selection
    Selection& call(const JsObject& callback);

    Selection& persist(); // Persist selection

    void pushUpdates() const; // Push JS updates

    Signal<>& clicked(); // Clicked signal

  private:
    D3Object obj_; // Underlying D3 object

    void throwIfInvalid() const; // Throws if invalid
    Selection getModified(const std::string& modification); // Returns modified selection

    // Helper to push JS arguments
    template <typename T>
    void pushArgs(const T& t) { obj_.jsUpdates_ << "," << t; }

    void pushArgs(const std::string& s);
    void pushArgs(const JsObject& obj);
    void pushArgs(const bool& b);
    void pushArgs(const Selection& selection);

    template<typename T, typename... Args>
    void pushArgs(const T& t, Args... args) {
        pushArgs(t);
        pushArgs(args...);
    }
  };

  // Constructor
  WD3Widget();

  // Returns the root SVG selection
  Selection body() { return Selection(svgRef(), this); }

protected:
  // D3 initialization and update hooks
  virtual void initializeD3();
  virtual void updateD3();

  // Returns the DOM element type (DIV)
  DomElementType domElementType() const override { return DomElementType::DIV; }
  // Renders the widget and applies JS updates
  void render(WFlags<RenderFlag> flags) override;

private:
  long long lastObjectId_; // Last used D3 object id
  bool jsDefined_;         // Whether JS is defined
  JSignal<long long> selectionSignal_; // Selection signal

  WStringStream jsUpdates_; // JS update stream

  // Defines the required JavaScript for the widget
  void defineJavaScript();

  // Returns the SVG JS reference
  std::string svgRef() const { return jsRef() + ".d3WidgetObj.body()"; }

  // Returns a JS string literal or empty string
  static std::string jsStringLiteralOrEmpty(const std::string& str);
};

// Template implementation for call()
template<typename... Args>
WD3Widget::Selection& WD3Widget::Selection::call(const JsObject& callback, const Args&... args)
{
  throwIfInvalid();

  obj_.jsUpdates_ << ".call(" << callback.js_;

  pushArgs(args...);

  obj_.jsUpdates_ << ")";
  return *this;
}