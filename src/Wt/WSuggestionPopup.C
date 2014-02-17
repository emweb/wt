/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WAnchor"
#include "Wt/WContainerWidget"
#include "Wt/WFormWidget"
#include "Wt/WLogger"
#include "Wt/WSuggestionPopup"
#include "Wt/WStringStream" 
#include "Wt/WStringListModel"
#include "Wt/WTemplate"
#include "Wt/WText"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WSuggestionPopup.min.js"
#endif

namespace {
  std::string instantiateStdMatcher(const Wt::WSuggestionPopup::Options&
				    options) {
    Wt::WStringStream s;
    
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
      << Wt::WWebWidget::jsStringLiteral(options.wordStartRegexp) << ", "
      << Wt::WWebWidget::jsStringLiteral(options.appendReplacedText) << ")";

    return s.str();
  }
}

namespace Wt {

LOGGER("WSuggestionPopup");

#ifdef WT_TARGET_JAVA
WSuggestionPopup::Options::Options() 
  : listSeparator(0)
{ }
#endif

WSuggestionPopup::WSuggestionPopup(const Options& options, WObject *parent)
  : WPopupWidget(new WContainerWidget(), parent),
    model_(0),
    modelColumn_(0),
    filterLength_(0),
    filtering_(false),
    defaultValue_(-1),
    isDropDownIconUnfiltered_(false),
    matcherJS_(generateMatcherJS(options)),
    replacerJS_(generateReplacerJS(options)),
    filterModel_(this),
    activated_(this),
    filter_(implementation(), "filter"),
    jactivated_(implementation(), "select")
{
  init();
}

WSuggestionPopup::WSuggestionPopup(const std::string& matcherJS,
				   const std::string& replacerJS,
				   WObject *parent)
  : WPopupWidget(new WContainerWidget(), parent),
    model_(0),
    modelColumn_(0),
    filterLength_(0),
    filtering_(false),
    defaultValue_(-1),
    isDropDownIconUnfiltered_(false),
    matcherJS_(matcherJS),
    replacerJS_(replacerJS),
    filter_(implementation(), "filter"),
    jactivated_(implementation(), "select")
{
  init();
}

void WSuggestionPopup::init()
{
  impl_ = dynamic_cast<WContainerWidget *>(implementation());

  impl_->setList(true);
  impl_->setLoadLaterWhenInvisible(false);

  /*
   * We use display: none because logically, the popup is visible and
   * propagates signals
   */
  setAttributeValue("style", "z-index: 10000; display: none; overflow: auto");

  setModel(new WStringListModel(this));

  impl_->escapePressed().connect(this, &WWidget::hide);

  filter_.connect(this, &WSuggestionPopup::doFilter);
  jactivated_.connect(this, &WSuggestionPopup::doActivate);
}

void WSuggestionPopup::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WSuggestionPopup.js";
  LOAD_JAVASCRIPT(app, THIS_JS, "WSuggestionPopup", wtjs1);
  LOAD_JAVASCRIPT(app, THIS_JS, "WSuggestionPopupStdMatcher", wtjs2);

  std::string ddUnfiltered = isDropDownIconUnfiltered_ ? "true" : "false";
  setJavaScriptMember(" WSuggestionPopup",
		      "new " WT_CLASS ".WSuggestionPopup("
		      + app->javaScriptClass() + "," + jsRef() + ","
		      + replacerJS_ + "," + matcherJS_ + ","
		      + boost::lexical_cast<std::string>
		        (std::max(0, filterLength_)) + ","
		      + boost::lexical_cast<std::string>(partialResults()) + ","
                      + boost::lexical_cast<std::string>(defaultValue_) + ","
                      + ddUnfiltered + ");");
}

void WSuggestionPopup::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WPopupWidget::render(flags);
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

  impl_->clear();
  modelRowsInserted(WModelIndex(), 0, model_->rowCount() - 1);
}

void WSuggestionPopup::setDefaultIndex(int row)
{
  if (defaultValue_ != row) {
    defaultValue_ = row;

    if (isRendered())
      doJavaScript("jQuery.data(" + jsRef() + ", 'obj').defaultValue = "
		   + boost::lexical_cast<std::string>(defaultValue_)
		   + ';');      
  }
}

void WSuggestionPopup::modelRowsInserted(const WModelIndex& parent,
					 int start, int end)
{
  if (filterLength_ != 0 && !filtering_)
    return;

  if (modelColumn_ >= model_->columnCount())
    return;

  if (parent.isValid())
    return;

  for (int i = start; i <= end; ++i) {
    WContainerWidget *line = new WContainerWidget();
    impl_->insertWidget(i, line);

    WModelIndex index = model_->index(i, modelColumn_);

    boost::any d = index.data();

    TextFormat format = index.flags() & ItemIsXHTMLText ? XHTMLText : PlainText;
    WAnchor *anchor = new WAnchor(line);
    WText *value = new WText(asString(d), format, anchor);

    boost::any d2 = index.data(UserRole);
    if (d2.empty())
      d2 = d;

    value->setAttributeValue("sug", asString(d2));

    boost::any styleclass = index.data(StyleClassRole);
    if (!styleclass.empty()) {
      value->setAttributeValue("class", asString(styleclass));
    }
  }
}

void WSuggestionPopup::modelRowsRemoved(const WModelIndex& parent,
					int start, int end)
{
  if (parent.isValid())
    return;

  for (int i = start; i <= end; ++i)
    if (start < impl_->count())
      delete impl_->widget(start);
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
    WContainerWidget *w = dynamic_cast<WContainerWidget *>(impl_->widget(i));
    WAnchor *anchor = dynamic_cast<WAnchor *>(w->widget(0));
    WText *value = dynamic_cast<WText *>(anchor->widget(0));

    WModelIndex index = model_->index(i, modelColumn_);

    boost::any d = index.data();
    value->setText(asString(d));

    TextFormat format = index.flags() & ItemIsXHTMLText ? XHTMLText : PlainText;
    value->setTextFormat(format);

    boost::any d2 = model_->data(i, modelColumn_, UserRole);
    if (d2.empty())
      d2 = d;

    value->setAttributeValue("sug", asString(d2));
  }
}

void WSuggestionPopup::modelLayoutChanged()
{
  impl_->clear();
  modelRowsInserted(WModelIndex(), 0, model_->rowCount() - 1);
}

void WSuggestionPopup::forEdit(WFormWidget *edit, WFlags<PopupTrigger> triggers)
{
  EventSignalBase& b = edit->keyPressed();

  connectObjJS(b, "editKeyDown");
  connectObjJS(edit->keyWentDown(), "editKeyDown");
  connectObjJS(edit->keyWentUp(), "editKeyUp");
  connectObjJS(edit->blurred(), "delayHide");

  if (triggers & Editing)
    edit->addStyleClass("Wt-suggest-onedit");

  if (triggers & DropDownIcon) {
    edit->addStyleClass("Wt-suggest-dropdown");
    EventSignalBase& c = edit->clicked();
    connectObjJS(c, "editClick");
    connectObjJS(edit->mouseMoved(), "editMouseMove");
  }

  edits_.push_back(edit);
}

void WSuggestionPopup::setDropDownIconUnfiltered(bool isUnfiltered)
{
  isDropDownIconUnfiltered_ = isUnfiltered;
}

void WSuggestionPopup::showAt(WFormWidget *edit)
{
  doJavaScript("jQuery.data(" + jsRef() + ", 'obj').showAt("
	       + edit->jsRef() + ");");
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
    if (!suggestionValue.empty())
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

  /*
   * We do not use this->doJavaScript() because that would be rendered
   * before the updated children (suggestions) are rendered.
   */
  WApplication::instance()->
    doJavaScript("jQuery.data(" + jsRef() + ", 'obj').filtered("
		 + WWebWidget::jsStringLiteral(input) + ","
		 + (partialResults() ? "1" : "0") + ");");
}

bool WSuggestionPopup::partialResults() const
{
  if (filterLength_ < 0)
    return true;
  else if (model_->rowCount() > 0) {
    WModelIndex index = model_->index(model_->rowCount() - 1, modelColumn_);
    boost::any styleclass = index.data(StyleClassRole);
    return Wt::asString(styleclass) == "Wt-more-data";
  } else
    return false;
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
    LOG_ERROR("activate from bogus editor");

  for (int i = 0; i < impl_->count(); ++i)
    if (impl_->widget(i)->id() == itemId) {
      activated_.emit(i, edit);
      return;
    }

  LOG_ERROR("activate for bogus item");
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
