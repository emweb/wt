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
#include "Wt/WProgressBar"
#include "Wt/WResource"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "DomElement.h"
#include "WebSession.h"
#include "WebRequest.h"
#include "Utils.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

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
    bool triggerUpdate = false;

    std::vector<Http::UploadedFile> files;
#ifdef WT_TARGET_JAVA
    static Http::UploadedFile* uploaded;
#endif
    Utils::find(request.uploadedFiles(), "data", files);

    if (!request.tooLarge())
      if (!files.empty() || request.getParameter("data"))
	triggerUpdate = true;

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

    if (triggerUpdate || request.tooLarge()) {
      o << "if (window.parent."
	<< WApplication::instance()->javaScriptClass() << ") ";

      if (triggerUpdate) {
	o << "window.parent."
	  << WApplication::instance()->javaScriptClass()
	  << "._p_.update(null, '"
	  << fileUpload_->uploaded().encodeCmd() << "', null, true);";
      } else if (request.tooLarge()) {
	o << "window.parent."
	  << WApplication::instance()->javaScriptClass()
	  << "._p_.update(null, '"
	  << fileUpload_->fileTooLargeImpl().encodeCmd() << "', null, true);";
      }
    }

    o << "}\n"
      "</script></head>"
      "<body onload=\"load();\""
      "style=\"margin:0;padding:0;\">";

    o << "</body></html>";

    if (request.tooLarge())
      fileUpload_->tooLargeSize_ = request.tooLarge();
    else
      if (!files.empty())
	fileUpload_->setFiles(files);
  }

private:
  WFileUpload *fileUpload_;
};

const char *WFileUpload::CHANGE_SIGNAL = "M_change";
const char *WFileUpload::UPLOADED_SIGNAL = "M_uploaded";
const char *WFileUpload::FILETOOLARGE_SIGNAL = "M_filetoolarge";

/*
 * Supporting the file API:
 * - still create the resource
 * - do not create the iframe
 * - JavaScript method to do the upload
 */

WFileUpload::WFileUpload(WContainerWidget *parent)
  : WWebWidget(parent),
    textSize_(20),
    doUpload_(false),
    enableAjax_(false),
    uploading_(false),
    multiple_(false),
    fileTooLarge_(this),
    dataReceived_(this),
    progressBar_(0),
    tooLargeSize_(0)
{
  setInline(true);
  fileTooLargeImpl().connect(this, &WFileUpload::handleFileTooLargeImpl);
  create();
}

void WFileUpload::create()
{
  // NOTE: this is broken on konqueror: you cannot target a form anymore
  bool methodIframe = WApplication::instance()->environment().ajax();

  if (methodIframe)
    fileUploadTarget_ = new WFileUploadResource(this);
  else
    fileUploadTarget_ = 0;

  setFormObject(!fileUploadTarget_);
}

WFileUpload::~WFileUpload()
{
  if (uploading_)
    WApplication::instance()->enableUpdates(false);
}

void WFileUpload::onData(::uint64_t current, ::uint64_t total)
{
  dataReceived_.emit(current, total);

  if (WebSession::Handler::instance()->request()->postDataExceeded()) {
    if (uploading_) {
      uploading_ = false;
      handleFileTooLargeImpl();

      WApplication *app = WApplication::instance();
      app->triggerUpdate();
      app->enableUpdates(false);
    }

    return;
  }

  if (progressBar_ && uploading_) {
    progressBar_->setRange(0, (double)total);
    progressBar_->setValue((double)current);

    WApplication *app = WApplication::instance();
    app->triggerUpdate();
  }

  if (current == total) {
    WApplication *app = WApplication::instance();
    uploading_ = false;
    app->enableUpdates(false);
  }
}

void WFileUpload::enableAjax()
{
  create();
  enableAjax_ = true;
  repaint();
  WWebWidget::enableAjax();
}

void WFileUpload::setProgressBar(WProgressBar *bar)
{
  delete progressBar_;
  progressBar_ = bar;

  if (progressBar_) {
    if (!progressBar_->parent()) {
      progressBar_->setParentWidget(this);
      progressBar_->hide();
    }
  }
}

EventSignal<>& WFileUpload::uploaded()
{
  return *voidEventSignal(UPLOADED_SIGNAL, true);
}

EventSignal<>& WFileUpload::changed()
{
  return *voidEventSignal(CHANGE_SIGNAL, true);
}

EventSignal<>& WFileUpload::fileTooLargeImpl()
{
  return *voidEventSignal(FILETOOLARGE_SIGNAL, true);
}

void WFileUpload::handleFileTooLargeImpl()
{
  fileTooLarge().emit(tooLargeSize_);
}

void WFileUpload::setFileTextSize(int chars)
{
  textSize_ = chars;
}

void WFileUpload::setMultiple(bool multiple)
{
  multiple_ = multiple;
}

std::string WFileUpload::spoolFileName() const
{
  if (!empty())
    return uploadedFiles_[0].spoolFileName();
  else
    return std::string();
}

WT_USTRING WFileUpload::clientFileName() const
{
  if (!empty())
    return WT_USTRING::fromUTF8(uploadedFiles_[0].clientFileName());
  else
    return WT_USTRING();
}

WT_USTRING WFileUpload::contentDescription() const
{
  if (!empty())
    return WT_USTRING::fromUTF8(uploadedFiles_[0].contentType());
  else
    return WT_USTRING();
}

void WFileUpload::stealSpooledFile()
{
  if (!empty())
    uploadedFiles_[0].stealSpoolFile();
}

bool WFileUpload::emptyFileName() const
{
  return empty();
}

bool WFileUpload::empty() const
{
  return uploadedFiles_.empty();
}

void WFileUpload::updateDom(DomElement& element, bool all)
{
  if (fileUploadTarget_ && doUpload_) {
    element.callMethod("submit()");
    doUpload_ = false;

    fileUploadTarget_->setUploadProgress(true);
    fileUploadTarget_->dataReceived().connect(this, &WFileUpload::onData);

    if (progressBar_)
      if (progressBar_->parent() == this) {
	DomElement *inputE = DomElement::getForUpdate("in" + id(),
						      DomElement_INPUT);
	inputE->setProperty(PropertyStyleDisplay, "none");
	element.addChild(inputE);
      }
  }

  if (element.type() != DomElement_INPUT
      && progressBar_ && !progressBar_->isRendered())
    element.addChild(((WWebWidget *)progressBar_)
		     ->createDomElement(WApplication::instance()));

  WWebWidget::updateDom(element, all);
}

void WFileUpload::propagateRenderOk(bool deep)
{
  // no need for anything not updated in updateDom()
  WWebWidget::propagateRenderOk(deep);
}

DomElementType WFileUpload::domElementType() const
{
  return fileUploadTarget_ ? DomElement_FORM : DomElement_INPUT;
}

void WFileUpload::getDomChanges(std::vector<DomElement *>& result,
				WApplication *app)
{
  if (enableAjax_) {
    DomElement *plainE = DomElement::getForUpdate(this, DomElement_INPUT);
    DomElement *ajaxE = createDomElement(app);
    plainE->replaceWith(ajaxE);
    result.push_back(plainE);
  } else
    WWebWidget::getDomChanges(result, app);
}

DomElement *WFileUpload::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  if (result->type() == DomElement_FORM)
    result->setId(id());
  else
    result->setName(id());

  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);

  if (fileUploadTarget_) {
    DomElement *i = DomElement::createNew(DomElement_IFRAME);
    i->setProperty(PropertyClass, "Wt-resource");
    i->setProperty(PropertySrc, fileUploadTarget_->url());
    i->setName("if" + id());

    DomElement *form = result;

    form->setAttribute("method", "post");
    form->setAttribute("action", fileUploadTarget_->generateUrl());
    form->setAttribute("enctype", "multipart/form-data");
    form->setProperty(PropertyStyle, "margin:0;padding:0;display:inline");
    form->setProperty(PropertyTarget, "if" + id());

    /*
     * wrap iframe in an extra span to work around bug in IE which does
     * not set the name use DOM methods
     */
    DomElement *d = DomElement::createNew(DomElement_SPAN);
    d->addChild(i);

    form->addChild(d);

    DomElement *input = DomElement::createNew(DomElement_INPUT);
    input->setAttribute("type", "file");
    if (multiple_)
      input->setAttribute("multiple", "multiple");
    input->setAttribute("name", "data");
    input->setAttribute("size", boost::lexical_cast<std::string>(textSize_));
    input->setId("in" + id());

    if (change)
      updateSignalConnection(*input, *change, "change", true);

    form->addChild(input);

  } else {
    result->setAttribute("type", "file");
    if (multiple_)
      result->setAttribute("multiple", "multiple");
    result->setAttribute("size", boost::lexical_cast<std::string>(textSize_));

    if (change)
      updateSignalConnection(*result, *change, "change", true);
  }

  updateDom(*result, true);

  enableAjax_ = false;

  return result;
}

void WFileUpload::setFormData(const FormData& formData)
{
  setFiles(formData.files);

  if (!formData.files.empty())
    uploaded().emit();
}

void WFileUpload::setFiles(const std::vector<Http::UploadedFile>& files)
{
  uploadedFiles_.clear();

  for (unsigned i = 0; i < files.size(); ++i)
    if (!files[i].clientFileName().empty())
      uploadedFiles_.push_back(files[i]);
}

void WFileUpload::setRequestTooLarge(int size)
{
  fileTooLarge().emit(size);
}

void WFileUpload::upload()
{
  if (fileUploadTarget_ && !uploading_) {
    doUpload_ = true;
    repaint(RepaintPropertyIEMobile);

    if (progressBar_) {
      if (progressBar_->parent() != this)
	hide();
      else
	progressBar_->show();
    }

    WApplication::instance()->enableUpdates();

    uploading_ = true;
  }
}

}
