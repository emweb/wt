/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/TextEdit"
#include "Wt/WTextArea"

#include "DomElement.h"

namespace Wt {

  namespace Ext {

TextEdit::TextEdit(WContainerWidget *parent)
  : FormField(parent),
    linkDefaultLocation_("http://"),
    alignments_(true),
    colors_(true),
    font_(true),
    format_(true),
    links_(true),
    lists_(true),
    sourceEdit_(true)
{
  //extjs: textedit doesn't stand a parent with display: none
  setHideWithOffsets(true);

  textArea_ = new WTextArea();
  addOrphan(textArea_);
}

TextEdit::TextEdit(const WT_USTRING& text, WContainerWidget *parent)
  : FormField(parent),
    linkDefaultLocation_("http://"),
    alignments_(true),
    colors_(true),
    font_(true),
    format_(true),
    links_(true),
    lists_(true),
    sourceEdit_(true)
{
  //extjs: textedit doesn't stand a parent with display: none
  setHideWithOffsets(true);

  textArea_ = new WTextArea();
  addOrphan(textArea_);

  textArea_->setText(text);
}

WFormWidget *TextEdit::formWidget() const
{
  return textArea_;
}

void TextEdit::setLinkDefault(const WString& text, const WString& location)
{
  linkDefaultText_ = text;
  linkDefaultLocation_ = location;
}

void TextEdit::setEnableAlignments(bool enable)
{
  alignments_ = enable;
}

void TextEdit::setEnableColors(bool enable)
{
  colors_ = enable;
}

void TextEdit::setEnableFont(bool enable)
{
  font_ = enable;
}

void TextEdit::setEnableFormat(bool enable)
{
  format_ = enable;
}

void TextEdit::setEnableLinks(bool enable)
{
  links_ = enable;
}

void TextEdit::setEnableLists(bool enable)
{
  lists_ = enable;
}

void TextEdit::setEnableSourceEdit(bool enable)
{
  sourceEdit_ = enable;
}

void TextEdit::setText(const WT_USTRING& text)
{
  textArea_->setText(text);
  if (isRendered())
    addUpdateJS(elVar() + ".setValue(" + WWebWidget::jsStringLiteral(text)
		+ ");");
}

const WT_USTRING& TextEdit::text() const
{
  return textArea_->text();
}

WValidator::State TextEdit::validate()
{
  if (validator()) {
    return validator()->validate(textArea_->text()).state();
  } else
    return WValidator::Valid;
}

void TextEdit::resize(const WLength& width, const WLength& height)
{
  textArea_->resize(width, height);
}

void TextEdit::useAsTableViewEditor()
{
  textArea_->setFormObject(false);
}

std::string TextEdit::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar() << " = new Ext.form.HtmlEditor(" << configStruct() << ");";

  applyToWidget(textArea_, result, inContainer);

  return result.str();
}

void TextEdit::createConfig(std::ostream& config)
{
  if (!linkDefaultText_.empty())
    config << ",createLinkText:" << linkDefaultText_.jsStringLiteral();
  config << ",defaultLinkValue:" << linkDefaultLocation_.jsStringLiteral();
  if (alignments_ != true)
    config << ",enableAlignments:false";
  if (colors_ != true)
    config << ",enableColors:false";
  if (font_ != true)
    config << ",enableFont:false";
  if (format_ != true)
    config << ",enableFormat:false";
  if (colors_ != true)
    config << ",enableLinks:false";
  if (font_ != true)
    config << ",enableLists:false";
  if (format_ != true)
    config << ",enableSourceEdit:false";

  FormField::createConfig(config);
}

  }
}
