/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WContainerWidget"
#include "Wt/WTemplate"
#include "Wt/WText"
#include "Wt/WFormWidget"
#include "Wt/WApplication"

#include "Wt/WSuggestionPopup"
#include "Wt/WStringListModel"

#include "JavaScriptLoader.h"
#ifndef WT_DEBUG_JS
#include "js/WSuggestionPopup.min.js"
#endif

namespace Wt {

#define TEMPLATE "${shadow-x1-x2}${contents}"

WSuggestionPopup::WSuggestionPopup(const Options& options,
				   WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new WTemplate(WString::fromUTF8(TEMPLATE))),
    model_(0),
    modelColumn_(0),
    filterLength_(0),
    matcherJS_(generateMatcherJS(options)),
    replacerJS_(generateReplacerJS(options)),
    filter_(impl_, "filter"),
    editKeyDown_(parent), // should be this, but IE hack...
    editKeyUp_(parent),
    delayHide_(parent)
{
  init();
}

WSuggestionPopup::WSuggestionPopup(const std::string& matcherJS,
				   const std::string& replacerJS,
				   WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new WTemplate(WString::fromUTF8(TEMPLATE))),
    model_(0),
    modelColumn_(0),
    filterLength_(0),
    matcherJS_(matcherJS),
    replacerJS_(replacerJS),
    filter_(impl_, "filter"),
    editKeyDown_(parent), // should be this, but IE hack...
    editKeyUp_(parent),
    delayHide_(parent)
{
  init();
}

void WSuggestionPopup::init()
{
  setImplementation(impl_);
  impl_->setStyleClass("Wt-suggest Wt-outset");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("contents", content_ = new WContainerWidget());
  content_->setStyleClass("content");

  setAttributeValue("style", "z-index: 10000");
  setPositionScheme(Absolute);

  setJavaScript(editKeyDown_, "editKeyDown");
  setJavaScript(editKeyUp_, "editKeyUp");
  setJavaScript(delayHide_, "delayHide");

  hide();

  setModel(new WStringListModel(this));

  filter_.connect(SLOT(this, WSuggestionPopup::doFilter));
}

void WSuggestionPopup::setMaximumSize(const WLength& width,
				      const WLength& height)
{
  content_->setMaximumSize(width, height);
}

void WSuggestionPopup::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WSuggestionPopup.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WSuggestionPopup", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  app->doJavaScript("new " WT_CLASS ".WSuggestionPopup("
		    + app->javaScriptClass() + "," + jsRef() + ","
		    + replacerJS_ + "," + matcherJS_ + ","
		    + boost::lexical_cast<std::string>(filterLength_) + ");");
}

void WSuggestionPopup::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WCompositeWidget::render(flags);
}

void WSuggestionPopup::setJavaScript(JSlot& slot, 
				     const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """jQuery.data(" + jsRef() + ", 'obj')." + methodName + "(obj, event);"
    "}";
  slot.setJavaScript(jsFunction);
}

void WSuggestionPopup::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  /* connect slots to new model */
  modelConnections_.push_back(model_->rowsInserted().connect
     (SLOT(this, WSuggestionPopup::modelRowsInserted)));
  modelConnections_.push_back(model_->rowsRemoved().connect
     (SLOT(this, WSuggestionPopup::modelRowsRemoved)));
  modelConnections_.push_back(model_->dataChanged().connect
     (SLOT(this, WSuggestionPopup::modelDataChanged)));
  modelConnections_.push_back(model_->layoutChanged().connect
     (SLOT(this, WSuggestionPopup::modelLayoutChanged)));
  modelConnections_.push_back(model_->modelReset().connect
     (SLOT(this, WSuggestionPopup::modelLayoutChanged)));

  setModelColumn(modelColumn_);
}

void WSuggestionPopup::setModelColumn(int modelColumn)
{
  modelColumn_ = modelColumn;

  content_->clear();

  modelRowsInserted(WModelIndex(), 0, model_->rowCount() - 1);
}

void WSuggestionPopup::modelRowsInserted(const WModelIndex& parent,
					 int start, int end)
{
  if (modelColumn_ >= model_->columnCount())
    return;

  if (parent.isValid())
    return;

  for (int i = start; i <= end; ++i) {
    WContainerWidget *line = new WContainerWidget();
    content_->insertWidget(i, line);

    boost::any d = model_->data(i, modelColumn_);
    WText *value = new WText(asString(d), PlainText);

    boost::any d2 = model_->data(i, modelColumn_, UserRole);
    if (d2.empty())
      d2 = d;

    line->addWidget(value);
    value->setAttributeValue("sug", asString(d2));
  }
}

void WSuggestionPopup::modelRowsRemoved(const WModelIndex& parent,
					int start, int end)
{
  if (parent.isValid())
    return;

  for (int i = start; i <= end; ++i)
    delete content_->widget(i);
}

void WSuggestionPopup::modelDataChanged(const WModelIndex& topLeft,
					const WModelIndex& bottomRight)
{
  if (topLeft.parent().isValid())
    return;

  if (modelColumn_ < topLeft.column() || modelColumn_ > bottomRight.column())
    return;

  for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
    WContainerWidget *w = dynamic_cast<WContainerWidget *>(content_->widget(i));
    WText *value = dynamic_cast<WText *>(w->widget(0));

    boost::any d = model_->data(i, modelColumn_);
    value->setText(asString(d));

    boost::any d2 = model_->data(i, modelColumn_, UserRole);
    if (d2.empty())
      d2 = d;

    value->setAttributeValue("sug", asString(d2));
  }
}

void WSuggestionPopup::modelLayoutChanged()
{
  content_->clear();

  setModelColumn(modelColumn_);
}

void WSuggestionPopup::forEdit(WFormWidget *edit)
{
  edit->keyPressed().connect(editKeyDown_);
  edit->keyWentDown().connect(editKeyDown_);
  edit->keyWentUp().connect(editKeyUp_);
  edit->blurred().connect(delayHide_);
}

void WSuggestionPopup::clearSuggestions()
{
  model_->removeRows(0, model_->rowCount());
}

void WSuggestionPopup::addSuggestion(const WString& suggestionText,
				     const WString& suggestionValue)
{
  int row = model_->rowCount();

  if (model_->insertRow(row)) {
    model_->setData(row, modelColumn_, boost::any(suggestionText), DisplayRole);
    model_->setData(row, modelColumn_, boost::any(suggestionValue), UserRole);
  }
}

namespace {
  std::string generateParseEditJS(const WSuggestionPopup::Options& options)
  {
    return std::string() +
      "var value = edit.value;"
      "var pos;"
      "if (edit.selectionStart)"
      """pos = edit.selectionStart;"
      "else "
      """pos = value.length;"
      "var ws='" + options.whitespace + "';"
      + (options.listSeparator != 0
	 ? (std::string("var start = value.lastIndexOf('")
	    + options.listSeparator + "', pos - 1) + 1;")
	 : ("var start = 0;")) +
      "while ((start < pos)"
      ""      "&& (ws.indexOf(value.charAt(start)) != -1))"
      """start++;"
      "var end = pos;";
  }
};

std::string WSuggestionPopup::generateMatcherJS(const Options& options)
{
  return std::string() +
    "function (edit) {"
    + generateParseEditJS(options) +
    """value = edit.value.substring(start, end);"

    """return function(suggestion) {"
    ""  "if (!suggestion)"
    ""    "return value;"

    ""  "var sep='" + options.wordSeparators + "',"
    ""    "matched = false,"
    ""    "i = 0,"
    ""    "sugup = suggestion.toUpperCase(),"
    ""    "val = value.toUpperCase(),"
    ""    "inserted = 0;"
    
    ""  "if (val.length) {"
    ""    "while ((i != -1) && (i < sugup.length)) {"
    ""      "var matchpos = sugup.indexOf(val, i);"
    ""        "if (matchpos != -1) {"
    ""          "if ((matchpos == 0)"
    ""              "|| (sep.indexOf(sugup.charAt(matchpos - 1)) != -1)) {"
    + (!options.highlightEndTag.empty()
       ? ("suggestion = suggestion.substring(0, matchpos + inserted)"
	  " + '" + options.highlightBeginTag + "'"
	  " + suggestion.substring(matchpos + inserted,"
	  "     matchpos + inserted + val.length)"
	  " + '" + options.highlightEndTag + "'"
	  " + suggestion.substring(matchpos + inserted + val.length,"
	  "     suggestion.length);"
	  " inserted += "
	  + boost::lexical_cast<std::string>(options.highlightBeginTag.length()
					     + options.highlightEndTag.length())
	  + ";")
       : "") +
    ""            "matched = true;"
    ""          "}"
    ""        "i = matchpos + 1;"
    ""      "} else "
    ""        "i = matchpos;"
    ""    "}"
    ""  "}"

    ""  "return { match: matched,"
    ""           "suggestion: suggestion }"
    """}"
    "}";
}

std::string WSuggestionPopup::generateReplacerJS(const Options& options)
{
  return std::string() +
    "function (edit, suggestionText, suggestionValue) {"
    + generateParseEditJS(options) +
    "edit.value = edit.value.substring(0, start) +"
    "  suggestionValue "
    + (!options.appendReplacedText.empty()
       ? "+ '" + options.appendReplacedText + "'"
       : "") +
    " + edit.value.substring(end, edit.value.length);"
    " if (edit.selectionStart) {"
    "   edit.selectionStart = start + suggestionValue.length"
    + (!options.appendReplacedText.empty()
       ? "+ " + boost::lexical_cast<std::string>(2)
       : "") + ";"
    "   edit.selectionEnd = start + suggestionValue.length"
    + (!options.appendReplacedText.empty()
       ? "+ " + boost::lexical_cast<std::string>(2)
       : "") + ";"
    " }"
    "}";
}

void WSuggestionPopup::setFilterLength(int length)
{
  filterLength_ = length;
}

void WSuggestionPopup::doFilter(std::string input)
{
  filterModel_.emit(WT_USTRING::fromUTF8(input));

  WApplication *app = WApplication::instance();
  app->doJavaScript("jQuery.data(" + jsRef() + ", 'obj').filtered("
		    + WWebWidget::jsStringLiteral(input) + ")");
}

}
