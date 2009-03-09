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
#include "Wt/WResource"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "DomElement.h"
#include "WebSession.h"

namespace Wt {

class WFileUploadResource : public WResource {
public:
  WFileUploadResource(WFileUpload *fileUpload)
    : WResource(fileUpload),
      fileUpload_(fileUpload)
  { }

protected:
  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) {
#ifdef WT_THREADED
    WebSession::Handler::instance()->lock().lock();
#endif // WT_THREADED

    bool triggerUpdate = false;

    const Http::UploadedFile *p = 0;

    if (!request.tooLarge()) {
      Http::UploadedFileMap::const_iterator i
	= request.uploadedFiles().find("data");
      if (i != request.uploadedFiles().end()) {
	p = &i->second;
	triggerUpdate = true;
      }
    }

    response.setMimeType("text/html; charset=utf-8");
    response.addHeader("Expires", "Sun, 14 Jun 2020 00:00:00 GMT");
    response.addHeader("Cache-Control", "max-age=315360000");

#ifndef WT_TARGET_JAVA
    std::ostream& o = response.out();
#else
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA

    o << "<!DOCTYPE html PUBLIC "
      "\"-//W3C//DTD HTML 4.01 Transitional//EN\" "
      "\"http://www.w3.org/TR/html4/loose.dtd\">"
      "<html lang=\"en\" dir=\"ltr\">\n"
      "<head><title></title>\n"
      "<script type=\"text/javascript\">\n"
      "function load() { ";

    if (triggerUpdate)
      o << "window.parent."
	<< WApplication::instance()->javaScriptClass()
	<< "._p_.update(null, '"
	<< fileUpload_->uploaded().encodeCmd() << "', null, true);";

    o << "}\n"
      "</script></head>"
      "<body onload=\"load();\""
      "style=\"margin:0;padding:0;\">";

    o << "</body></html>";

    if (request.tooLarge())
      fileUpload_->requestTooLarge(request.tooLarge());
    else
      if (p)
	fileUpload_->setFormData(*p);

#ifdef WT_THREADED
    WebSession::Handler::instance()->lock().unlock();
#endif // WT_THREADED
  }

private:
  WFileUpload *fileUpload_;
};

const char *WFileUpload::CHANGE_SIGNAL = "M_change";
const char *WFileUpload::UPLOADED_SIGNAL = "M_uploaded";

WFileUpload::WFileUpload(WContainerWidget *parent)
  : WWebWidget(parent),
    textSize_(20),
    isStolen_(false),
    doUpload_(false),
    fileTooLarge_(this)
{
  // NOTE: this is broken on konqueror: you cannot target a form anymore
  bool methodIframe = WApplication::instance()->environment().ajax();

  if (methodIframe)
    fileUploadTarget_ = new WFileUploadResource(this);
  else
    fileUploadTarget_ = 0;

  setInline(true);

  if (!fileUploadTarget_)
    setFormObject(true);
}

WFileUpload::~WFileUpload()
{
  if (!isStolen_)
    unlink(spoolFileName_.c_str());
}

EventSignal<void>& WFileUpload::uploaded()
{
  return *voidEventSignal(UPLOADED_SIGNAL, true);
}

EventSignal<void>& WFileUpload::changed()
{
  return *voidEventSignal(CHANGE_SIGNAL, true);
}

void WFileUpload::setFileTextSize(int chars)
{
  textSize_ = chars;
}

void WFileUpload::stealSpooledFile()
{
  isStolen_ = true;
}

void WFileUpload::updateDom(DomElement& element, bool all)
{
  if (fileUploadTarget_ && doUpload_) {
    element.callMethod("submit()");
    doUpload_ = false;
  }

  WWebWidget::updateDom(element, all);
}

DomElementType WFileUpload::domElementType() const
{
  return fileUploadTarget_ ? DomElement_FORM : DomElement_INPUT;
}

DomElement *WFileUpload::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  result->setId(this, true);

  EventSignal<void> *change = voidEventSignal(CHANGE_SIGNAL, false);

  if (fileUploadTarget_) {
    DomElement *form = result;

    form->setAttribute("method", "post");
    form->setAttribute("action", fileUploadTarget_->generateUrl());
    form->setAttribute("enctype", "multipart/form-data");
    form->setAttribute("style", "margin:0;padding:0;display:inline");
    form->setProperty(PropertyTarget, "if" + formName());

    DomElement *i = DomElement::createNew(DomElement_IFRAME);
    i->setAttribute("class", "Wt-resource");
    i->setAttribute("src", fileUploadTarget_->generateUrl());
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

    if (change)
      updateSignalConnection(*input, *change, "change", true);

    form->addChild(input);

  } else {
    result->setAttribute("type", "file");
    result->setAttribute("size", boost::lexical_cast<std::string>(textSize_));

    if (change)
      updateSignalConnection(*result, *change, "change", true);
  }

  updateDom(*result, true);

  return result;
}

void WFileUpload::setFormData(const FormData& formData)
{
  if (formData.file)
    setFormData(*formData.file);
}

void WFileUpload::setFormData(const Http::UploadedFile& file)
{
  spoolFileName_ = file.spoolFileName();
  clientFileName_ = WString::fromUTF8(file.clientFileName());
  contentDescription_ = WString::fromUTF8(file.contentType());
  file.stealSpoolFile();
  isStolen_ = false;

  //uploaded().emit();
}

void WFileUpload::requestTooLarge(int size)
{
  fileTooLarge().emit(size);
}

void WFileUpload::upload()
{
  if (fileUploadTarget_) {
    doUpload_ = true;
    repaint(RepaintPropertyIEMobile);
  }
}

}
