/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WFileUpload"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WProgressBar"
#include "Wt/WResource"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "DomElement.h"
#include "WebSession.h"
#include "WebRequest.h"
#include "WebUtils.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

namespace Wt {

LOGGER("WFileUpload");

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

    o << "<!DOCTYPE html>"
      "<html>\n"
      "<head><script type=\"text/javascript\">\n"
      "function load() { ";

    if (triggerUpdate || request.tooLarge()) {
      o << "if (window.parent."
	<< WApplication::instance()->javaScriptClass() << ") ";

      if (triggerUpdate) {
	LOG_DEBUG("Resource handleRequest(): signaling uploaded");

	o << "window.parent."
	  << WApplication::instance()->javaScriptClass()
	  << "._p_.update(null, '"
	  << fileUpload_->uploaded().encodeCmd() << "', null, true);";
      } else if (request.tooLarge()) {
	LOG_DEBUG("Resource handleRequest(): signaling file-too-large");

	o << "window.parent."
	  << WApplication::instance()->javaScriptClass()
	  << "._p_.update(null, '"
	  << fileUpload_->fileTooLargeImpl().encodeCmd() << "', null, true);";
      }
    } else {
      LOG_DEBUG("Resource handleRequest(): no signal");
    }

    o << "}\n"
      "</script></head>"
      "<body onload=\"load();\"></body></html>";

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

  if (methodIframe) {
    fileUploadTarget_ = new WFileUploadResource(this);
    fileUploadTarget_->setUploadProgress(true);
    fileUploadTarget_->dataReceived().connect(this, &WFileUpload::onData);

    setJavaScriptMember(WT_RESIZE_JS,
			"function(self, w, h) {"
			"""if (w >= 0) "
			""  "$(self).find('input').width(w);"
			"}");
  } else
    fileUploadTarget_ = 0;

  setFormObject(!fileUploadTarget_);

  uploaded().connect(this, &WFileUpload::onUploaded);
  fileTooLarge().connect(this, &WFileUpload::onUploaded);
}

WFileUpload::~WFileUpload()
{
  if (flags_.test(BIT_UPLOADING))
    WApplication::instance()->enableUpdates(false);
}

void WFileUpload::onUploaded()
{
  if (flags_.test(BIT_UPLOADING)) {
    WApplication::instance()->enableUpdates(false);
    flags_.reset(BIT_UPLOADING);
  }
}

void WFileUpload::onData(::uint64_t current, ::uint64_t total)
{
  dataReceived_.emit(current, total);

  WebSession::Handler *h = WebSession::Handler::instance();

  ::int64_t dataExceeded = h->request()->postDataExceeded();
  h->setRequest(0, 0); // so that triggerUpdate() will work

  if (dataExceeded) {
    if (flags_.test(BIT_UPLOADING)) {
      flags_.reset(BIT_UPLOADING);
      tooLargeSize_ = dataExceeded;
      handleFileTooLargeImpl();

      WApplication *app = WApplication::instance();
      app->triggerUpdate();
      app->enableUpdates(false);
    }

    return;
  }

  if (progressBar_ && flags_.test(BIT_UPLOADING)) {
    progressBar_->setRange(0, (double)total);
    progressBar_->setValue((double)current);

    WApplication *app = WApplication::instance();
    app->triggerUpdate();
  }
}

void WFileUpload::enableAjax()
{
  create();
  flags_.set(BIT_ENABLE_AJAX);
  repaint();
  WWebWidget::enableAjax();
}

void WFileUpload::setFilters(const std::string& acceptAttributes)
{
  acceptAttributes_ = acceptAttributes;
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
  flags_.set(BIT_MULTIPLE, multiple);
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
  bool containsProgress = progressBar_ && progressBar_->parent() == this;
  DomElement *inputE = 0;

  if (element.type() != DomElement_INPUT
      && flags_.test(BIT_DO_UPLOAD)
      && containsProgress && !progressBar_->isRendered())
    element.addChild(progressBar_->createSDomElement(WApplication::instance()));

  // upload() + disable() does not work. -- fix after this release,
  // change order of javaScript_ and properties rendering in DomElement

  if (fileUploadTarget_ && flags_.test(BIT_DO_UPLOAD)) {
    // Reset the action and generate a new URL for the target,
    // because the session id may have changed in the meantime
    element.setAttribute("action", fileUploadTarget_->generateUrl());
    element.callMethod("submit()");
    flags_.reset(BIT_DO_UPLOAD);

    if (containsProgress) {
      inputE = DomElement::getForUpdate("in" + id(), DomElement_INPUT);
      inputE->setProperty(PropertyStyleDisplay, "none");
    }
  }

  if (flags_.test(BIT_ENABLED_CHANGED)) {
    if (!inputE)
      inputE = DomElement::getForUpdate("in" + id(), DomElement_INPUT);

    if (isEnabled())
      inputE->callMethod("disabled=false");
    else
      inputE->callMethod("disabled=true");

    inputE->setAttribute("accept", acceptAttributes_);

    flags_.reset(BIT_ENABLED_CHANGED);
  }

  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);
  if (change && change->needsUpdate(all)) {
    if (!inputE)
      inputE = DomElement::getForUpdate("in" + id(), DomElement_INPUT);

    updateSignalConnection(*inputE, *change, "change", all);
  }

  if (inputE)
    element.addChild(inputE);

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
  if (flags_.test(BIT_ENABLE_AJAX)) {
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
    if (app->environment().agentIsIE()) {
      // http://msdn.microsoft.com/en-us/library/ms536474%28v=vs.85%29.aspx
      // HTA's (started by mshta.exe) have a different security model than
      // a normal web app, and therefore a HTA browser does not allow
      // interaction from iframes to the parent window unless this
      // attribute is set. If omitted, this causes the 'uploaded()'
      // signal to be blocked when a Wt app is executed as a HTA.
      i->setAttribute("APPLICATION", "yes");
    }

    DomElement *form = result;

    form->setAttribute("method", "post");
    form->setAttribute("action", fileUploadTarget_->url());
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
    if (flags_.test(BIT_MULTIPLE))
      input->setAttribute("multiple", "multiple");
    input->setAttribute("name", "data");
    input->setAttribute("size", boost::lexical_cast<std::string>(textSize_));
    input->setAttribute("accept", acceptAttributes_);
    input->setId("in" + id());

    if (!isEnabled())
      input->setProperty(Wt::PropertyDisabled, "true");

    if (change)
      updateSignalConnection(*input, *change, "change", true);

    form->addChild(input);

  } else {
    result->setAttribute("type", "file");
    if (flags_.test(BIT_MULTIPLE))
      result->setAttribute("multiple", "multiple");
    result->setAttribute("size", boost::lexical_cast<std::string>(textSize_));

    if (!isEnabled())
      result->setProperty(Wt::PropertyDisabled, "true");

    if (change)
      updateSignalConnection(*result, *change, "change", true);
  }

  updateDom(*result, true);

  flags_.reset(BIT_ENABLE_AJAX);

  return result;
}

void WFileUpload::setFormData(const FormData& formData)
{
  setFiles(formData.files);

  LOG_DEBUG("setFormData() : " << formData.files.size() << " file(s)");

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

void WFileUpload::setRequestTooLarge(::int64_t size)
{
  fileTooLarge().emit(size);
}

void WFileUpload::upload()
{
  if (fileUploadTarget_ && !flags_.test(BIT_UPLOADING)) {
    flags_.set(BIT_DO_UPLOAD);
    repaint();

    if (progressBar_) {
      if (progressBar_->parent() != this)
	hide();
      else
	progressBar_->show();
    }

    WApplication::instance()->enableUpdates();

    flags_.set(BIT_UPLOADING);
  }
}

void WFileUpload::propagateSetEnabled(bool enabled)
{
  flags_.set(BIT_ENABLED_CHANGED);
  repaint();

  WWebWidget::propagateSetEnabled(enabled);
}

}
