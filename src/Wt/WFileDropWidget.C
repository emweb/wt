#include "Wt/WFileDropWidget.h"

#include "Wt/Http/Response.h"
#include "Wt/Json/Array.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Object.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WServer.h"
#include "Wt/WSignal.h"

#include "WebUtils.h"
#include "WebSession.h"
#include <exception>
#include <boost/bind.hpp>

#ifndef WT_DEBUG_JS
#include "js/WFileDropWidget.min.js"
#endif

#define NestedResource Wt::WFileDropWidget::WFileDropUploadResource

namespace Wt {

WFileDropWidget::File::File(int id, const std::string& fileName,
                            const std::string& type, ::uint64_t size)
  : id_(id),
    clientFileName_(fileName),
    type_(type),
    size_(size),
    uploadedFile_("","",""),
    uploadFinished_(false),
    cancelled_(false)
{ }
  
const Http::UploadedFile& WFileDropWidget::File::uploadedFile() const {
  if (!uploadFinished_)
    throw std::exception();
  else
    return uploadedFile_;
}

void WFileDropWidget::File::setUploadedFile(const Http::UploadedFile& file)
{
  uploadFinished_ = true;
  uploadedFile_ = file;
}

void WFileDropWidget::File::cancel()
{
  cancelled_ = true;
}

bool WFileDropWidget::File::cancelled() const
{
  return cancelled_;
}
  
NestedResource::WFileDropUploadResource(WFileDropWidget *fileDropWidget)
  : WResource(),
    parent_(fileDropWidget),
    app_(WApplication::instance())
{
  setUploadProgress(true);
}

void NestedResource::handleRequest(const Http::Request& request,
				   Http::Response& response)
{
  WApplication::UpdateLock lock(WApplication::instance());
  const std::string *fileId = request.getParameter("file-id");
  if (fileId == 0 || (*fileId).empty()) {
    response.setStatus(404);
    return;
  }
  int id = Utils::stoi(*fileId);
  bool validId = parent_->incomingIdCheck(id);
  if (!validId) {
    response.setStatus(404);
    return;
  }
  
  std::vector<Http::UploadedFile> files;
  Utils::find(request.uploadedFiles(), "data", files);
  if (files.empty()) {
    response.setStatus(404);
    return;
  }

  parent_->setUploadedFile(files[0]);
  // WServer::instance()->post(app_->sessionId(),
  // 			    boost::bind(&WFileDropWidget::setUploadedFile,
  // 					parent_, files[0]));
}


WFileDropWidget::WFileDropWidget()
  : resource_(new WFileDropUploadResource(this)),
    currentFileIdx_(0),
    dropSignal_(this, "dropsignal"),
    requestSend_(this, "requestsend"),
    fileTooLarge_(this, "filetoolarge"),
    uploadFinished_(this, "uploadfinished"),
    doneSending_(this, "donesending")
{
  WApplication *app = WApplication::instance();
  if (!app->environment().ajax())
    return;
  
  LOAD_JAVASCRIPT(app, "js/WFileDropWidget.js", "WFileDropWidget", wtjs1);

  std::string maxFileSize = std::to_string(WApplication::instance()->maximumRequestSize());
  setJavaScriptMember(" WFileDropWidget", "new " WT_CLASS ".WFileDropWidget("
		      + app->javaScriptClass() + "," + jsRef() + ",'"
		      + resource_->generateUrl() + "', "
		      + maxFileSize + ");");


  dropSignal_.connect(this, &WFileDropWidget::handleDrop);
  requestSend_.connect(this, &WFileDropWidget::handleSendRequest);
  fileTooLarge_.connect(this, &WFileDropWidget::handleTooLarge);
  uploadFinished_.connect(this, &WFileDropWidget::emitUploaded);
  doneSending_.connect(this, &WFileDropWidget::stopReceiving);

  resource_->dataReceived().connect(this, &WFileDropWidget::onData);
  
  addStyleClass("Wt-filedropzone");
}

void WFileDropWidget::handleDrop(const std::string& newDrops)
{
  Json::Value dropdata;
  Json::parse(newDrops, dropdata);

  std::vector<File*> drops;
  
  Json::Array dropped = (Json::Array)dropdata;
  for (std::size_t i = 0; i < dropped.size(); ++i) {
    Json::Object upload = (Json::Object)dropped[i];
    int id = -1;
    ::uint64_t size = 0;
    std::string name, type;
    for (Json::Object::iterator it = upload.begin(); it != upload.end();
	 it++) {
      if (it->first == "id")
	id = it->second;
      else if (it->first == "filename")
	name = (std::string)it->second;
      else if (it->first == "type")
	type = (std::string)it->second;
      else if (it->first == "size")
	size = (long long)it->second;
      else
	throw std::exception();
    }
    
    File *file = new File(id, name, type, size);
    drops.push_back(file);
    uploads_.push_back(file);
  }
  dropEvent_.emit(drops);
  doJavaScript(jsRef() + ".markForSending(" + newDrops + ");");
}

void WFileDropWidget::handleSendRequest(int id)
{
  /* When something invalid is dropped, the upload can fail (eg. a folder).
   * We simply proceed to the next and consider this upload as 'cancelled'
   * since it is past the currentFileIdx.
   * A cancelled upload will also be skipped in this way.
   */
  bool fileFound = false;
  for (unsigned i=currentFileIdx_; i < uploads_.size(); i++) {
    if (uploads_[i]->uploadId() == id) {
      fileFound = true;
      currentFileIdx_ = i;
      doJavaScript(jsRef() + ".send();");
      uploadStart_.emit(uploads_[currentFileIdx_]);
      break;
    } else {
      if (!uploads_[i]->cancelled())
	uploadFailed_.emit(uploads_[i]);
    }
  }

  if (!fileFound)
    doJavaScript(jsRef() + ".cancelUpload(" + std::to_string(id) + ");");
  else {
    WApplication::instance()->enableUpdates(true);
  }
}

void WFileDropWidget::handleTooLarge(::uint64_t size)
{
  tooLarge_.emit(uploads_[currentFileIdx_], size);
  currentFileIdx_++;
}
  
void WFileDropWidget::stopReceiving()
{
  if (currentFileIdx_ < uploads_.size()) {
    for (unsigned i=currentFileIdx_; i < uploads_.size(); i++)
      if (!uploads_[i]->cancelled())
	uploadFailed_.emit(uploads_[i]);
    // std::cerr << "ERROR: file upload was still listening, "
    // 	      << "cancelling expected uploads"
    // 	      << std::endl;
    currentFileIdx_ = uploads_.size();
    WApplication::instance()->enableUpdates(false);
  }
}

// Note: args by value, since this is handled after handleRequest is finished
void WFileDropWidget::setUploadedFile(Http::UploadedFile file)
{
  File *f = uploads_[currentFileIdx_];
  currentFileIdx_++;
  
  f->setUploadedFile(file);
  // f->uploaded().emit();
  // uploaded().emit(f);
  // WApplication::instance()->triggerUpdate();

  if (currentFileIdx_ == uploads_.size()) {
    WApplication::instance()->enableUpdates(false);
  }
}

void WFileDropWidget::emitUploaded(int id)
{
  for (unsigned i=0; i < currentFileIdx_; i++) {
    File *f = uploads_[i];
    if (f->uploadId() == id) {
      f->uploaded().emit();
      uploaded().emit(f);
    }
  }
}

bool WFileDropWidget::incomingIdCheck(int id)
{
  if (uploads_[currentFileIdx_]->uploadId() == id)
    return true;
  else {
    return false;
  }
}

void WFileDropWidget::cancelUpload(File *file)
{
  file->cancel();
  int i = file->uploadId();
  doJavaScript(jsRef() + ".cancelUpload(" + std::to_string(i) + ");");
}
  
bool WFileDropWidget::remove(File *file)
{
  std::vector<File*>::iterator last = uploads_.begin() + currentFileIdx_;
  std::vector<File*>::iterator it = std::find(uploads_.begin(), last, file);

  if (it != last) {
    uploads_.erase(it);
    currentFileIdx_--;
    return true;
  } else {
    return false;
  }
}

void WFileDropWidget::onData(::uint64_t current, ::uint64_t total)
{
  WebSession::Handler *h = WebSession::Handler::instance();

  ::int64_t dataExceeded = h->request()->postDataExceeded();
  h->setRequest(0, 0); // so that triggerUpdate() will work
  
  File *file = uploads_[currentFileIdx_];
  file->dataReceived().emit(current, total);

  WApplication::instance()->triggerUpdate();
}

void WFileDropWidget::setHoverStyleClass(const std::string& className)
{
  doJavaScript(jsRef() + ".configureHoverClass('" + className + "');");
}

void WFileDropWidget::setAcceptDrops(bool enable)
{
  doJavaScript(jsRef() + ".setAcceptDrops(" + (enable ? "true" : "false")
	       + ");");
}
  
}
