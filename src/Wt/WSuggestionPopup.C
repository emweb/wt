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
#include "Wt/WLogger"

#include "Wt/WSuggestionPopup"
#include "Wt/WStringListModel"

#include "JavaScriptLoader.h"
#include "Utils.h"
#include "EscapeOStream.h"

#ifndef WT_DEBUG_JS
#include "js/WSuggestionPopup.min.js"
#endif

namespace {
  std::string instantiateStdMatcher(const Wt::WSuggestionPopup::Options&
				    options) {
    Wt::SStream s;

    s << "new " WT_CLASS ".WSuggestionPopupStdMatcher("
      << Wt::WWebWidget::jsStringLiteral(options.highlightBeginTag) << ", "
      << Wt::WWebWidget::jsStringLiteral(options.highlightEndTag) << ", ";

    if (options.listSeparator)
      s << Wt::WWebWidget::jsStringLiteral
	(std::string() + options.listSeparator);
    else
      s << "null";

    s << ", " << Wt::WWebWidget::jsStringLiteral(options.whitespace) << ", "
      << Wt::WWebWidget::jsStringLiteral(options.wordSeparators) << ", "
      << Wt::WWebWidget::jsStringLiteral(options.appendReplacedText) << ")";

    return s.str();
  }
}

namespace Wt {

#define TEMPLATE "${shadow-x1-x2}${contents}"

WSuggestionPopup::WSuggestionPopup(const Options& options,
				   WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new WTemplate(WString::fromUTF8(TEMPLATE))),
    model_(0),
    modelColumn_(0),
    filterLength_(0),
    filtering_(false),
    matcherJS_(generateMatcherJS(options)),
    replacerJS_(generateReplacerJS(options)),
    filterModel_(this),
    activated_(this),
    filter_(impl_, "filter"),
    jactivated_(impl_, "select"),
    global_(false)
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
    filtering_(false),
    defaultValue_(-1),
    matcherJS_(matcherJS),
    replacerJS_(replacerJS),
    filter_(impl_, "filter"),
    jactivated_(impl_, "select"),
    global_(false)
{
  init();
}

void WSuggestionPopup::init()
{
  setImplementation(impl_);
  impl_->setLoadLaterWhenInvisible(false);
  impl_->setStyleClass("Wt-suggest Wt-outset");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("contents", content_ = new WContainerWidget());
  content_->setStyleClass("content");

  setAttributeValue("style", "z-index: 10000");
  setPositionScheme(Absolute);

  hide();

  setModel(new WStringListModel(this));

  filter_.connect(this, &WSuggestionPopup::doFilter);
  jactivated_.connect(this, &WSuggestionPopup::doActivate);
}

void WSuggestionPopup::setMaximumSize(const WLength& width,
				      const WLength& height)
{
  WCompositeWidget::setMaximumSize(width, height);
  content_->setMaximumSize(width, height);
}

void WSuggestionPopup::setMinimumSize(const WLength& width,
				      const WLength& height)
{
  WCompositeWidget::setMinimumSize(width, height);
  content_->setMinimumSize(width, height);
}

void WSuggestionPopup::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WSuggestionPopup.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WSuggestionPopup", wtjs1);
    LOAD_JAVASCRIPT(app, THIS_JS, "WSuggestionPopupStdMatcher", wtjs2);
    app->setJavaScriptLoaded(THIS_JS);
  }

  app->doJavaScript("new " WT_CLASS ".WSuggestionPopup("
		    + app->javaScriptClass() + "," + jsRef() + ","
		    + replacerJS_ + "," + matcherJS_ + ","
		    + boost::lexical_cast<std::string>(filterLength_) + ","
		    + boost::lexical_cast<std::string>(defaultValue_) + ","
		    + (global_ ? "true" : "false") + ");");
}

void WSuggestionPopup::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WCompositeWidget::render(flags);
}

void WSuggestionPopup::connectObjJS(EventSignalBase& s, 
				     const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = jQuery.data(" + jsRef() + ", 'obj');"
    """if (o) o." + methodName + "(obj, event);"
    "}";
  s.connect(jsFunction);
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
     (this, &WSuggestionPopup::modelRowsInserted));
  modelConnections_.push_back(model_->rowsRemoved().connect
     (this, &WSuggestionPopup::modelRowsRemoved));
  modelConnections_.push_back(model_->dataChanged().connect
     (this, &WSuggestionPopup::modelDataChanged));
  modelConnections_.push_back(model_->layoutChanged().connect
     (this, &WSuggestionPopup::modelLayoutChanged));
  modelConnections_.push_back(model_->modelReset().connect
     (this, &WSuggestionPopup::modelLayoutChanged));

  setModelColumn(modelColumn_);
}

void WSuggestionPopup::setModelColumn(int modelColumn)
{
  modelColumn_ = modelColumn;

  content_->clear();
  modelRowsInserted(WModelIndex(), 0, model_->rowCount() - 1);
}

void WSuggestionPopup::setDefaultIndex(int row)
{
  if (defaultValue_ != row) {
    defaultValue_ = row;

    if (isRendered()) {
      WApplication *app = WApplication::instance();
      app->doJavaScript("jQuery.data(" + jsRef() + ", 'obj').defaultValue = "
			+ boost::lexical_cast<std::string>(defaultValue_)
			+ ';');      
    }
  }
}

void WSuggestionPopup::modelRowsInserted(const WModelIndex& parent,
					 int start, int end)
{
  if (filterLength_ > 0 && !filtering_)
    return;

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
    if (start < content_->count())
      delete content_->widget(start);
    else
      break;
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
  modelRowsInserted(WModelIndex(), 0, model_->rowCount() - 1);
}

void WSuggestionPopup::forEdit(WFormWidget *edit, WFlags<PopupTrigger> triggers)
{
#ifdef WT_CNOR // ??
  EventSignalBase& b = edit->keyPressed();
  EventSignalBase& c = edit->clicked();
#endif

  connectObjJS(edit->keyPressed(), "editKeyDown");
  connectObjJS(edit->keyWentDown(), "editKeyDown");
  connectObjJS(edit->keyWentUp(), "editKeyUp");
  connectObjJS(edit->blurred(), "delayHide");

  if (triggers & Editing)
    edit->addStyleClass("Wt-suggest-onedit");

  if (triggers & DropDownIcon) {
    edit->addStyleClass("Wt-suggest-dropdown");
    connectObjJS(edit->clicked(), "editClick");
    connectObjJS(edit->mouseMoved(), "editMouseMove");
  }

  edits_.push_back(edit);
}

void WSuggestionPopup::removeEdit(WFormWidget *edit)
{
  if (Utils::erase(edits_, edit)) {
    edit->removeStyleClass("Wt-suggest-onedit");
    edit->removeStyleClass("Wt-suggest-dropdown");
  }
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

void WSuggestionPopup::setFilterLength(int length)
{
  filterLength_ = length;
}

void WSuggestionPopup::doFilter(std::string input)
{
  filtering_ = true;
  filterModel_.emit(WT_USTRING::fromUTF8(input));
  filtering_ = false;

  WApplication *app = WApplication::instance();
  app->doJavaScript("jQuery.data(" + jsRef() + ", 'obj').filtered("
		    + WWebWidget::jsStringLiteral(input) + ")");
}

void WSuggestionPopup::doActivate(std::string itemId, std::string editId)
{
  WFormWidget *edit = 0;

  for (unsigned i = 0; i < edits_.size(); ++i)
    if (edits_[i]->id() == editId) {
      edit = edits_[i];
      break;
    }

  if (edit == 0)
    wApp->log("error") << "WSuggestionPopup activate from bogus editor";

  for (int i = 0; i < content_->count(); ++i)
    if (content_->widget(i)->id() == itemId) {
      activated_.emit(i, edit);
      return;
    }

  wApp->log("error") << "WSuggestionPopup activate for bogus item";
}

std::string WSuggestionPopup::generateMatcherJS(const Options& options)
{
  return instantiateStdMatcher(options) + ".match";
}

std::string WSuggestionPopup::generateReplacerJS(const Options& options)
{
  return instantiateStdMatcher(options) + ".replace";
}

}
