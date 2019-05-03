#include "Wt/WFileDropWidget.h"

#include "Wt/Http/Response.h"
#include "Wt/Json/Array.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Object.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WMemoryResource.h"
#include "Wt/WServer.h"
#include "Wt/WSignal.h"

#include "FileUtils.h"
#include "WebUtils.h"
#include "WebSession.h"
#include <exception>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

#ifndef WT_DEBUG_JS
#include "js/WFileDropWidget.min.js"
#endif

namespace Wt {

class WFileDropWidget::WFileDropUploadResource final : public WResource {
public:
  WFileDropUploadResource(WFileDropWidget *fileDropWidget, File *file)
    : WResource(),
      parent_(fileDropWidget),
      app_(WApplication::instance()),
      currentFile_(file)
  {
    setUploadProgress(true);
  }

  virtual ~WFileDropUploadResource()
  {
    beingDeleted();
  }

  void setCurrentFile(File *file) { currentFile_ = file; }

protected:
  virtual void handleRequest(const Http::Request &request,
			     Http::Response &response) override
  {
    // In JWt we still have the update lock
#ifndef WT_TARGET_JAVA
    /**
     * Taking the update-lock (rather than posting to the event loop):
     *   - guarantee that the updates to WFileDropWidget happen immediately, 
     *     before any application-code is called by the finished upload.
     *   - only Wt-code is executed within this lock
     */
    WApplication::UpdateLock lock(WApplication::instance());
#endif // WT_TARGET_JAVA

    const std::string *fileId = request.getParameter("file-id");
    if (fileId == 0 || (*fileId).empty()) {
      response.setStatus(404);
      return;
    }
    int id = boost::lexical_cast<int>(*fileId);
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

    // check is js filter was used
    const std::string *filtFlag = request.getParameter("filtered");
    currentFile_->setIsFiltered((filtFlag != 0) && ((*filtFlag) == "true"));

    // add data to currentFile_
    const std::string *lastFlag = request.getParameter("last");
    bool isLast = (lastFlag == 0) || // if not present, assume not chunked
      (lastFlag != 0 && (*lastFlag) == "true");
    currentFile_->handleIncomingData(files[0], isLast);

    if (isLast) {
      parent_->proceedToNextFile();
    }

    response.setMimeType("text/plain"); // else firefox complains
  }

private:
  WFileDropWidget *parent_;
  WApplication *app_;
  File *currentFile_;
};

const std::string WFileDropWidget::WORKER_JS =
#include "js/WFileDropWidget_worker.min.js"
  ;

WFileDropWidget::File::File(int id, const std::string& fileName,
                            const std::string& type, ::uint64_t size,
                            ::uint64_t chunkSize)
  : id_(id),
    clientFileName_(fileName),
    type_(type),
    size_(size),
    uploadStarted_(false),
    uploadFinished_(false),
    cancelled_(false),
    filterEnabled_(true),
    isFiltered_(false),
    nbReceivedChunks_(0),
    chunkSize_(chunkSize)
{ }

const Http::UploadedFile& WFileDropWidget::File::uploadedFile() const {
  if (!uploadFinished_)
    throw WException("Can not access uploaded files before upload is done.");
  else
    return uploadedFile_;
}

void WFileDropWidget::File::handleIncomingData(const Http::UploadedFile& file, bool last)
{
  if (!uploadStarted_) {
    uploadedFile_ = file;
    uploadStarted_ = true;
  } else {
    // append data to spool-file
    Wt::FileUtils::appendFile(file.spoolFileName(),
			      uploadedFile_.spoolFileName());
  }
  nbReceivedChunks_++;
  
  if (last)
    uploadFinished_ = true;
}

void WFileDropWidget::File::cancel()
{
  cancelled_ = true;
}

bool WFileDropWidget::File::cancelled() const
{
  return cancelled_;
}

void WFileDropWidget::File::emitDataReceived(::uint64_t current, ::uint64_t total,
					     bool filterSupported) {
  if (!filterEnabled_ || !filterSupported || chunkSize_ == 0) {
    dataReceived_.emit(current, total);
  } else {
    ::uint64_t currentChunkSize = chunkSize_;
    unsigned nbChunks = (unsigned)(size_ / chunkSize_);
    if (nbReceivedChunks_ == nbChunks) // next chunk is the remainder
      currentChunkSize = size_ - (nbReceivedChunks_*chunkSize_);
    
    ::uint64_t progress = nbReceivedChunks_*chunkSize_
	+ ::uint64_t( (double(current)/double(total)) * currentChunkSize );
    dataReceived_.emit(progress, size_);
  }
}

void WFileDropWidget::File::setFilterEnabled(bool enabled) {
  filterEnabled_ = enabled;
}

void WFileDropWidget::File::setIsFiltered(bool filtered) {
  isFiltered_ = filtered;
}
  
WFileDropWidget::WFileDropWidget()
  : uploadWorkerResource_(nullptr),
    resource_(nullptr),
    currentFileIdx_(0),
    chunkSize_(0),
    filterSupported_(true),
    hoverStyleClass_("Wt-dropzone-hover"),
    acceptDrops_(true),
    acceptAttributes_(""),
    dropIndicationEnabled_(false),
    globalDropEnabled_(false),
    dropSignal_(this, "dropsignal"),
    requestSend_(this, "requestsend"),
    fileTooLarge_(this, "filetoolarge"),
    uploadFinished_(this, "uploadfinished"),
    doneSending_(this, "donesending"),
    jsFilterNotSupported_(this, "filternotsupported"),
    updatesEnabled_(false)
{
  WApplication *app = WApplication::instance();
  if (!app->environment().ajax())
    return;
  
  setup();
}

void WFileDropWidget::enableAjax()
{
  setup();
  repaint();
  
  WContainerWidget::enableAjax();
}

void WFileDropWidget::setup()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WFileDropWidget.js", "WFileDropWidget", wtjs1);

  std::string maxFileSize = std::to_string(WApplication::instance()->maximumRequestSize());
  setJavaScriptMember(" WFileDropWidget", "new " WT_CLASS ".WFileDropWidget("
                      + app->javaScriptClass() + "," + jsRef() + ","
                      + maxFileSize + ");");


  dropSignal_.connect(this, &WFileDropWidget::handleDrop);
  requestSend_.connect(this, &WFileDropWidget::handleSendRequest);
  fileTooLarge_.connect(this, &WFileDropWidget::handleTooLarge);
  uploadFinished_.connect(this, &WFileDropWidget::emitUploaded);
  doneSending_.connect(this, &WFileDropWidget::stopReceiving);
  jsFilterNotSupported_.connect(this, &WFileDropWidget::disableJavaScriptFilter);

  addStyleClass("Wt-filedropzone");
}

void WFileDropWidget::handleDrop(const std::string& newDrops)
{
#ifndef WT_TARGET_JAVA
  Json::Value dropdata;
#else
  Json::Value& dropdata;
#endif
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
#ifndef WT_TARGET_JAVA
      if (it->first == "id")
	id = it->second;
      else if (it->first == "filename")
	name = (std::string)it->second;
      else if (it->first == "type")
	type = (std::string)it->second;
      else if (it->first == "size")
	size = (long long)it->second;
#else
      if (it->first == "id")
	id = it->second.getAsInt();
      else if (it->first == "filename")
	name = it->second.getAsString();
      else if (it->first == "type")
	type = it->second.getAsString();
      else if (it->first == "size")
	size = it->second.getAsLong();
#endif
      else
	throw std::exception();
    }
    
    File *file = new File(id, name, type, size, chunkSize_);
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
      delete resource_;
      resource_ = new WFileDropUploadResource(this, uploads_[currentFileIdx_]);
      resource_->dataReceived().connect(this, &WFileDropWidget::onData);
      resource_->dataExceeded().connect(this, &WFileDropWidget::onDataExceeded);
      doJavaScript(jsRef() + ".send('" + resource_->url() + "', "
		   + (uploads_[i]->filterEnabled() ? "true" : "false")
		   + ");");
      uploadStart_.emit(uploads_[currentFileIdx_]);
      break;
    } else {
      // If a previous upload was not cancelled, it must have failed
      if (!uploads_[i]->cancelled())
	uploadFailed_.emit(uploads_[i]);
    }
  }

  if (!fileFound)
    doJavaScript(jsRef() + ".cancelUpload(" + std::to_string(id) + ");");
  else {
    updatesEnabled_ = true;
    WApplication::instance()->enableUpdates(true);
  }
}

void WFileDropWidget::handleTooLarge(::uint64_t size)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // this signal a few times, causing currentFileIdx_
    // to go out of bounds
    return;
  }
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
    if (updatesEnabled_) {
      WApplication::instance()->enableUpdates(false);
      updatesEnabled_ = false;
    }
  }
}

// Note: args by value, since this is handled after handleRequest is finished
void WFileDropWidget::proceedToNextFile()
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // the filetoolarge signal too many times, causing currentFileIdx_
    // to go out of bounds
    return;
  }

  currentFileIdx_++;
  if (currentFileIdx_ == uploads_.size()) {
    if (updatesEnabled_) {
      WApplication::instance()->enableUpdates(false);
      updatesEnabled_ = false;
    }
  }
}

void WFileDropWidget::emitUploaded(int id)
{
  for (unsigned i=0; i < currentFileIdx_ && i < uploads_.size(); i++) {
    File *f = uploads_[i];
    if (f->uploadId() == id) {
      f->uploaded().emit();
      uploaded().emit(f);
    }
  }
}

bool WFileDropWidget::incomingIdCheck(int id)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // the filetoolarge signal too many times, causing currentFileIdx_
    // to go out of bounds
    return false;
  }
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
  for (unsigned i=0; i < currentFileIdx_ && i < uploads_.size(); i++) {
    if (uploads_[i] == file) {
      uploads_.erase(uploads_.begin()+i);
      currentFileIdx_--;
      return true;
    }
  }
  return false;
}

void WFileDropWidget::onData(::uint64_t current, ::uint64_t total)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // the filetoolarge signal too many times, causing currentFileIdx_
    // to go out of bounds
    return;
  }
  File *file = uploads_[currentFileIdx_];
  file->emitDataReceived(current, total, filterSupported_);

  WApplication::instance()->triggerUpdate();
}

void WFileDropWidget::onDataExceeded(::uint64_t dataExceeded)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // the filetoolarge signal too many times, causing currentFileIdx_
    // to go out of bounds
    return;
  }
  tooLarge_.emit(uploads_[currentFileIdx_], dataExceeded);

  WApplication *app = WApplication::instance();
  app->triggerUpdate();
}

void WFileDropWidget::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();
  if (app->environment().ajax()) {
    if (updateFlags_.test(BIT_HOVERSTYLE_CHANGED) || all)
      doJavaScript(jsRef() + ".configureHoverClass('" + hoverStyleClass_
		   + "');");
    if (updateFlags_.test(BIT_ACCEPTDROPS_CHANGED) || all)
      doJavaScript(jsRef() + ".setAcceptDrops("
		   + (acceptDrops_ ? "true" : "false") + ");");
    if (updateFlags_.test(BIT_FILTERS_CHANGED) || all)
      doJavaScript(jsRef() + ".setFilters("
		   + jsStringLiteral(acceptAttributes_) + ");");
    if (updateFlags_.test(BIT_DRAGOPTIONS_CHANGED) || all) {
      doJavaScript(jsRef() + ".setDropIndication("
		   + (dropIndicationEnabled_ ? "true" : "false") + ");");
      doJavaScript(jsRef() + ".setDropForward("
		   + (globalDropEnabled_ ? "true" : "false") + ");");
    }
    if (updateFlags_.test(BIT_JSFILTER_CHANGED) || all) {
      createWorkerResource();
      
      doJavaScript(jsRef() + ".setUploadWorker(\""
		   + (uploadWorkerResource_ ?
		      uploadWorkerResource_->url() : "")
		   + "\");");
      doJavaScript(jsRef() + ".setChunkSize("
		   + boost::lexical_cast<std::string>(chunkSize_) + ");");
    }
      
    updateFlags_.reset();
  }
  
  WContainerWidget::updateDom(element, all);
}

std::string WFileDropWidget::renderRemoveJs(bool recursive) {
  if (isRendered()) {
    std::string result = jsRef() + ".destructor();";

    if (!recursive)
	result += WT_CLASS ".remove('" + id() + "');";

    return result;
  } else {
    return WContainerWidget::renderRemoveJs(recursive);
  }
}

void WFileDropWidget::setHoverStyleClass(const std::string& className)
{
  if (className == hoverStyleClass_)
    return;

  hoverStyleClass_ = className;
  
  updateFlags_.set(BIT_HOVERSTYLE_CHANGED);
  repaint();
}

void WFileDropWidget::setAcceptDrops(bool enable)
{
  if (enable == acceptDrops_)
    return;

  acceptDrops_ = enable;

  updateFlags_.set(BIT_ACCEPTDROPS_CHANGED);
  repaint();
}

void WFileDropWidget::setFilters(const std::string& acceptAttributes)
{
  if (acceptAttributes == acceptAttributes_)
    return;

  acceptAttributes_ = acceptAttributes;
  
  updateFlags_.set(BIT_FILTERS_CHANGED);
  repaint();
}

void WFileDropWidget::setDropIndicationEnabled(bool enable) {
  if (enable == dropIndicationEnabled_)
    return;

  dropIndicationEnabled_ = enable;

  updateFlags_.set(BIT_DRAGOPTIONS_CHANGED);
  repaint();
}

bool WFileDropWidget::dropIndicationEnabled() const {
  return dropIndicationEnabled_;
}

void WFileDropWidget::setGlobalDropEnabled(bool enable) {
  if (enable == globalDropEnabled_)
    return;
  
  globalDropEnabled_ = enable;
  updateFlags_.set(BIT_DRAGOPTIONS_CHANGED);
  repaint();
}

bool WFileDropWidget::globalDropEnabled() const {
  return globalDropEnabled_;
}

void WFileDropWidget::setJavaScriptFilter(const std::string& filterFn,
					  ::uint64_t chunksize,
					  const std::vector<std::string>& imports) {
  if (jsFilterFn_ == filterFn && chunksize == chunkSize_)
    return;

  jsFilterFn_ = filterFn;
  jsFilterImports_ = imports;
  chunkSize_ = chunksize;

  updateFlags_.set(BIT_JSFILTER_CHANGED);
  repaint();
}

void WFileDropWidget::createWorkerResource() {
  if (uploadWorkerResource_ != 0) {
    delete uploadWorkerResource_;
    uploadWorkerResource_ = 0;
  }

  if (jsFilterFn_.empty())
    return;
  
  uploadWorkerResource_ = addChild(cpp14::make_unique<WMemoryResource>("text/javascript"));

  std::stringstream ss;
  ss << "importScripts(";
  for (unsigned i=0; i < jsFilterImports_.size(); i++) {
    ss << "\"" << jsFilterImports_[i] << "\"";
    if (i < jsFilterImports_.size()-1)
      ss << ", ";
  }
  ss << ");" << std::endl;
  ss << jsFilterFn_ << std::endl;

  ss << WORKER_JS;
  
#ifndef WT_TARGET_JAVA
  std::string js = ss.str();
  uploadWorkerResource_->setData((const unsigned char*)js.c_str(), js.length());
#else
  try {
    uploadWorkerResource_->setData(ss.str().getBytes("utf8"));
  } catch (UnsupportedEncodingException e) {
  }
#endif
}

  
void WFileDropWidget::disableJavaScriptFilter() {
  filterSupported_ = false;
}
  
}
