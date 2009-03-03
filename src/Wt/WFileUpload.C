/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WText"
#include "Wt/WFileUpload"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"

#include "DomElement.h"
#include "CgiParser.h"

namespace Wt {

WFileUpload::WFileUpload(WContainerWidget *parent)
  : WWebWidget(parent),
    uploaded(this),
    fileTooLarge(this),
    changed(this),
    textSize_(20),
    textSizeChanged_(false),
    isStolen_(false),
    doUpload_(false)
{
  // NOTE: this is broken on konqueror: you cannot target a form anymore
  methodIframe_ = WApplication::instance()->environment().ajax();

  setInline(true);

  if (!methodIframe_)
    setFormObject(true);
}

WFileUpload::~WFileUpload()
{
  if (!isStolen_)
    unlink(spoolFileName_.c_str());
}

void WFileUpload::setFileTextSize(int chars)
{
  textSize_ = chars;
  textSizeChanged_ = true;

  repaint(RepaintPropertyAttribute);
}

void WFileUpload::stealSpooledFile()
{
  isStolen_ = true;
}

void WFileUpload::updateDom(DomElement& element, bool all)
{
  if (methodIframe_ && doUpload_) {
    element.callMethod("submit()");
    doUpload_ = false;
  }

  WWebWidget::updateDom(element, all);
}

DomElementType WFileUpload::domElementType() const
{
  return methodIframe_ ? DomElement_FORM : DomElement_INPUT;
}

DomElement *WFileUpload::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  result->setId(this, true);

  if (methodIframe_) {
    DomElement *form = result;

    form->setAttribute("method", "post");
    form->setAttribute("action", generateUrl());
    form->setAttribute("enctype", "multipart/form-data");
    form->setAttribute("style", "margin:0;padding:0;display:inline");
    form->setProperty(PropertyTarget, "if" + formName());

    DomElement *i = DomElement::createNew(DomElement_IFRAME);
    i->setAttribute("class", "Wt-resource");
    i->setAttribute("src", generateUrl());
    i->setId("if" + formName(), true);

    /*
     * wrap iframe in an extra span to work around bug in IE which does
     * not set the name use DOM methods
     */
    DomElement *d = DomElement::createNew(DomElement_SPAN);
    d->addChild(i);

    form->addChild(d);

    DomElement *input = DomElement::createNew(DomElement_INPUT);
    input->setAttribute("type", "file");
    input->setAttribute("name", "data");
    input->setAttribute("size", boost::lexical_cast<std::string>(textSize_));
    input->setId("in" + formName());

    updateSignalConnection(*input, changed, "change", true);

    form->addChild(input);

  } else {
    result->setAttribute("type", "file");
    result->setAttribute("size", boost::lexical_cast<std::string>(textSize_));

    updateSignalConnection(*result, changed, "change", true);
  }

  updateDom(*result, true);

  return result;
}

void WFileUpload::setFormData(CgiEntry *entry)
{
  if (!entry->clientFilename().empty()) {
    spoolFileName_ = entry->value();
    clientFileName_ = WString(entry->clientFilename(), UTF8);
    contentDescription_ = WString(entry->contentType(), UTF8);
    entry->stealFile();
    isStolen_ = false;
  }

  resourceTriggerUpdate_ = uploaded.encodeCmd();
}

void WFileUpload::formDataSet()
{
  //uploaded.emit();
}

void WFileUpload::requestTooLarge(int size)
{
  fileTooLarge.emit(size);
}

void WFileUpload::upload()
{
  if (methodIframe_) {
    doUpload_ = true;
    repaint(RepaintPropertyIEMobile);
  }
}

void WFileUpload::setNoFormData()
{
}

void WFileUpload::htmlText(std::ostream&)
{
}

}
