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

#include "Wt/WSuggestionPopup"
#include "Wt/WStringListModel"

namespace Wt {

WSuggestionPopup::WSuggestionPopup(const std::string& matcherJS,
				   const std::string& replacerJS,
				   WContainerWidget *parent)
  : WCompositeWidget(parent),
    model_(0),
    modelColumn_(0),
    matcherJS_(matcherJS),
    replacerJS_(replacerJS),
    editKeyDown_(parent), // should be this, but IE hack...
    editKeyUp_(parent),
    suggestionClicked_(parent),
    delayHide_(parent)
{
  const char *TEMPLATE =
      "${shadow-x1-x2}"
      "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));
  impl_->setStyleClass("Wt-suggest Wt-outset");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("contents", content_ = new WContainerWidget());

  setPopup(true);
  setPositionScheme(Absolute);

  editKeyDown_.setJavaScript
    ("function(edit, event) {"
     """var self = " + jsRef() + ";"
     """var sel = self.sel;"
     """if (sel != null) sel = " WT_CLASS ".getElement(sel);"
     """if (self.style.display != 'none' && sel != null) {"
     ""  "if ((event.keyCode == 13) || (event.keyCode == 9)) {"
     ""    "sel.firstChild.onclick();"
     ""    WT_CLASS ".cancelEvent(event);"
     ""    "return false;"
     ""  "} else if (event.keyCode == 40 || event.keyCode == 38) {"
     ""    "if (event.type.toUpperCase() == 'KEYDOWN')"
     ""      "self.kd = true;"
     // FIXME: cancel the event so that up and down is not handled by
     // the text area
     ""    "if (event.type.toUpperCase() == 'KEYPRESS'"
     ""      "&& self.kd == true) {"
     ""       WT_CLASS ".cancelEvent(event);"
     ""      "return false;"
     ""    "}"
     ""    "var n = sel;"
     ""    "for (var n = (event.keyCode == 40) ? n.nextSibling : "
     ""                                         "n.previousSibling; "
     ""         "n != null"
     ""         "&& n.nodeName.toUpperCase() == 'DIV' "
     ""         "&& n.style.display == 'none';"
     ""         "n = (event.keyCode == 40) ? n.nextSibling : n.previousSibling) { }"
     ""    "if (n != null && n.nodeName.toUpperCase() == 'DIV') {"
     ""      "sel.className = null;"
     ""      "n.className = 'sel';"
     ""      "self.sel = n.id;"
     ""    "}"
     ""    "return false;"
     ""  "}"
     """}"
     """return (event.keyCode != 13 && event.keyCode != 9);"
     "}");

  editKeyUp_.setJavaScript
    ("function(edit, event) {"
     """var self = " + jsRef() + ";"
     """var sel = self.sel;"
     """if (sel != null)"
     ""  "sel = " WT_CLASS ".getElement(sel);"
     ""
     """if (event.keyCode == 27"
     ""    "|| event.keyCode == 37"
     ""    "|| event.keyCode == 39) {"
     ""  "self.style.display = 'none';"
     ""  "if (event.keyCode == 27)"
     ""    "edit.blur();"
     """} else {"
     ""  "var text = edit.value;"
     ""  "var matcher = " + matcherJS_ + "(edit);"
     ""  "var first = null;"
     ""  "var sels = self.lastChild.childNodes;"
     ""  "for (var i = 0; i < sels.length; i++) {"
     ""    "var child = sels[i];"
     ""    "if (child.nodeName.toUpperCase() == 'DIV') {"
     ""      "if (child.orig == null)"
     ""        "child.orig = child.firstChild.innerHTML;"
     ""      "else "
     ""        "child.firstChild.innerHTML = child.orig;"
     ""      "var result = matcher(child.firstChild.innerHTML);"
     ""      "child.firstChild.innerHTML = result.suggestion;"
     ""      "if (result.match) {"
     ""        "child.style.display = 'block';"
     ""        "if (first == null) first = child;"
     ""      "} else "
     ""        "child.style.display = 'none';"
     ""      "child.className = null;"
     ""    "}"
     ""  "}"
     ""  "if (first == null) {"
     ""    "self.style.display = 'none';"
     ""  "} else {"
     ""    "if (self.style.display != 'block') {"
     ""      "self.style.display = 'block';"
     ""      WT_CLASS ".positionAtWidget(self.id, edit.id, "
     ""                                  WT_CLASS ".Vertical);"
     ""      "self.sel = null;"
     ""      "self.edit = edit.id;"
     ""      "sel = null;"
     ""    "}"
     ""    "if ((sel == null) || (sel.style.display == 'none')) {"
     ""      "self.sel = first.id;"
     ""      "first.className = 'sel';"
     ""    "} else {"
     ""      "sel.className = 'sel';"
     ""    "}"
     ""  "}"
     """}"
     "}");

  suggestionClicked_.setJavaScript
    ("function(suggestion, event) {"
     """var self = " + jsRef() + ";"
     """var edit = " WT_CLASS ".getElement(self.edit);"

     """var sText = suggestion.innerHTML;"
     """var sValue = suggestion.getAttribute('sug');"
     """var replacer = " + replacerJS_ + ";"
     """edit.focus();"

     """replacer(edit, sText, sValue);"

     """self.style.display = 'none';"
     "}");

  delayHide_.setJavaScript
    ("function(edit, event) {"
     """setTimeout(function() {"
     ""  "if (" + jsRef() + ") " + jsRef() + ".style.display = 'none';"
     """}, 300);"
      "}");

  hide();

  setModel(new WStringListModel(this));
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
    value->clicked().connect(suggestionClicked_);
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
      "if (edit.selectionStart) { pos = edit.selectionStart; }"
      "  else { pos = value.length; }"
      "var ws = '" + options.whitespace + "';"
      + (options.listSeparator != 0
	 ? (std::string("var start = value.lastIndexOf('")
	    + options.listSeparator + "', pos - 1) + 1;")
	 : ("var start = 0;")) +
      "while ((start < pos)"
      "  && (ws.indexOf(value.charAt(start)) != -1))"
      "  start++;"
      "var end = pos;";
  }
};

std::string WSuggestionPopup::generateMatcherJS(const Options& options)
{
  return std::string() +
    "function (edit) {"
    + generateParseEditJS(options) +
    "value = edit.value.substring(start, end).toUpperCase();"
    ""
    "return function(suggestion) {"
    "var sep='" + options.wordSeparators + "';"
    "var matched = false;"
    "var i = 0;"
    "var sugup = suggestion.toUpperCase();"
    "var inserted = 0;"
    "if (value.length != 0) {"
    "while ((i != -1) && (i < sugup.length)) {"
    "  var matchpos = sugup.indexOf(value, i);"
    "  if (matchpos != -1) {"
    "    if ((matchpos == 0)"
    "       || (sep.indexOf(sugup.charAt(matchpos - 1)) != -1)) {"
    + (!options.highlightEndTag.empty()
       ? ("suggestion = suggestion.substring(0, matchpos + inserted)"
	  " + '" + options.highlightBeginTag + "'"
	  " + suggestion.substring(matchpos + inserted,"
	  "     matchpos + inserted + value.length)"
	  " + '" + options.highlightEndTag + "'"
	  " + suggestion.substring(matchpos + inserted + value.length,"
	  "     suggestion.length);"
	  " inserted += "
	  + boost::lexical_cast<std::string>(options.highlightBeginTag.length()
					     + options.highlightEndTag.length())
	  + ";")
       : "") +
    "      matched = true;"
    "    }"
    "    i = matchpos + 1;"
    "  } else "
    "    i = matchpos;"
    "}"
    "}"
    "return { match: matched,"
    "         suggestion: suggestion }"
    "}"
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

}
