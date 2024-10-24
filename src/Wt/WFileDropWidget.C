#include "Wt/WFileDropWidget.h"

#include "Wt/Http/Response.h"
#include "Wt/Json/Array.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Object.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WMemoryResource.h"
#include "Wt/WServer.h"
#include "Wt/WSignal.h"

#include "FileUtils.h"
#include "WebUtils.h"
#include "WebSession.h"
#include <exception>
#include <boost/lexical_cast.hpp>
#include <fstream>

#ifndef WT_DEBUG_JS
#include "js/WFileDropWidget.min.js"
#endif

namespace {
  std::vector<Wt::WFileDropWidget::File*> flattenUploadsVector(Wt::WFileDropWidget::Directory *dir) {
    std::vector<Wt::WFileDropWidget::File*> retVal;
    for (Wt::WFileDropWidget::File *entry : dir->contents()) {
      if (!entry->directory()) {
        retVal.push_back(entry);
      } else {
        auto subdirEntries = flattenUploadsVector(dynamic_cast<Wt::WFileDropWidget::Directory*>(entry));
#ifndef WT_TARGET_JAVA
        retVal.insert(retVal.end(), subdirEntries.begin(), subdirEntries.end());
#else
        for (auto subdirEntry : subdirEntries) {
          retVal.push_back(subdirEntry);
        }
#endif
      }
    }
    return retVal;
  }
}

namespace Wt {

LOGGER("WFileDropWidget");

class WFileDropWidget::WFileDropUploadResource final : public WResource {
public:
  WFileDropUploadResource(WFileDropWidget *fileDropWidget, File *file)
    : WResource(),
      parent_(fileDropWidget),
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
  File *currentFile_;
};

const std::string WFileDropWidget::WORKER_JS =
#include "js/WFileDropWidget_worker.min.js"
  ;

WFileDropWidget::File::File(int id, const std::string& fileName, const std::string& path,
                             const std::string& type, ::uint64_t size, ::uint64_t chunkSize)
  : id_(id),
    clientFileName_(fileName),
    path_(path),
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

WFileDropWidget::Directory::Directory(const std::string& fileName, const std::string& path)
  : File(-1, fileName, path, "", 0, 0)
{ }

void WFileDropWidget::Directory::addFile(WFileDropWidget::File *file) {
  contents_.push_back(file);
}

const Http::UploadedFile& WFileDropWidget::File::uploadedFile() const {
  if (directory()) {
    throw WException("Directory: no file to upload.");
  } else if (!uploadFinished_) {
    throw WException("Can not access uploaded files before upload is done.");
  } else {
    return uploadedFile_;
  }
}

bool WFileDropWidget::File::uploadFinished() const
{
  return !directory() ? uploadFinished_ : true;
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
  if (directory()) {
    throw Wt::WException("Directory: cannot directly cancel, you must iterate over the contents.");
  }
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
    acceptDirectories_(false),
    acceptDirectoriesRecursive_(false),
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

#ifndef WT_TARGET_JAVA
/* Destructor must be defined in implementation file, otherwise it is generated
 * inline and a client will get a compiler error because WFileDropUploadResource
 * is an incomplete type.
 */
WFileDropWidget::~WFileDropWidget() = default;
#endif

std::vector<WFileDropWidget::File*> WFileDropWidget::uploads() const
{
  std::vector<WFileDropWidget::File*> copy;
  for (auto& upload : uploads_) {
    copy.push_back(upload.get());
  }
  return copy;
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
    Json::Object jsonObj = (Json::Object)dropped[i];
    File* dropObject = addDropObject(jsonObj);
    drops.push_back(dropObject);
  }

  // Convert the drop to a list of Files that need to be uploaded.
  std::vector<File*> newUploads;
  for (File *drop : drops) {
    if (!drop->directory()) {
      newUploads.push_back(drop);
    } else {
      std::vector<File*> dirUploads = flattenUploadsVector(dynamic_cast<Directory*>(drop));
#ifndef WT_TARGET_JAVA
      newUploads.insert(newUploads.end(), dirUploads.begin(), dirUploads.end());
#else
      for (auto dirEntry : dirUploads) {
        newUploads.push_back(dirEntry);
      }
#endif
    }
  }

  WStringStream ss;
  ss << "[";
  for (std::size_t i=0; i < newUploads.size(); ++i) {
    ss << "{\"id\":" << newUploads[i]->uploadId() << "}";
    if (i != newUploads.size()-1) {
      ss << ",";
    }
  }
  ss << "]";

  dropEvent_.emit(drops);
  doJavaScript(jsRef() + ".markForSending(" + ss.str() + ");");
}

WFileDropWidget::File* WFileDropWidget::addDropObject(const Json::Object& object) {
  int id = -1;
  ::uint64_t size = 0;
  std::string name, type, path;
  bool isDirectory = false;
  Json::Array contents;
  for (Json::Object::const_iterator it = object.begin(); it != object.end();
       it++) {
#ifndef WT_TARGET_JAVA
    if (it->first == "id")
      id = it->second;
    else if (it->first == "filename")
      name = (std::string)it->second;
    else if (it->first == "path")
      path = (std::string)it->second;
    else if (it->first == "type")
      type = (std::string)it->second;
    else if (it->first == "size")
      size = (long long)it->second;
    else if (it->first == "contents") {
      isDirectory = true;
      contents = (Json::Array)it->second;
    }
#else
    if (it->first == "id")
      id = it->second.getAsInt();
    else if (it->first == "filename")
      name = it->second.getAsString();
    else if (it->first == "path")
      path = it->second.getAsString();
    else if (it->first == "type")
      type = it->second.getAsString();
    else if (it->first == "size")
      size = it->second.getAsLong();
    else if (it->first == "contents") {
      isDirectory = true;
      for (Json::Value& contentsItem : it->second.getAsJsonArray()) {
        contents.push_back(contentsItem);
      }
    }
#endif
    else
      throw std::exception();
  }

  WFileDropWidget::File* retVal;
  if (isDirectory) {
    auto dir = std::make_unique<Directory>(name, path);
#ifndef WT_TARGET_JAVA
    for (Json::Object& dirItem : contents) {
#else
    for (Json::Value& dirItemValue : contents) {
      Json::Object dirItem = dirItemValue.getAsJsonObject();
#endif
      dir->addFile(addDropObject(dirItem));
    }
    retVal = dir.get();
    directories_.push_back(std::move(dir));
  } else {
    auto file = std::make_unique<File>(id, name, path, type, size, chunkSize_);
    retVal = file.get();
    uploads_.push_back(std::move(file));
  }
  return retVal;
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
      resource_ = uploadResource();
      resource_->dataReceived().connect(this, &WFileDropWidget::onData);
      resource_->dataExceeded().connect(this, &WFileDropWidget::onDataExceeded);
      doJavaScript(jsRef() + ".send('" + resource_->url() + "', "
                   + (currentFile()->filterEnabled() ? "true" : "false")
                   + ");");
      uploadStart_.emit(currentFile());
      break;
    } else {
      // If a previous upload was not cancelled, it must have failed
      if (!uploads_[i]->cancelled())
        uploadFailed_.emit(uploads_[i].get());
    }
  }

  if (!fileFound)
    doJavaScript(jsRef() + ".cancelUpload(" + std::to_string(id) + ");");
  else {
    updatesEnabled_ = true;
    WApplication::instance()->enableUpdates(true);
  }
}

std::unique_ptr<WResource> WFileDropWidget::uploadResource()
{
  return std::make_unique<WFileDropUploadResource>(this, currentFile());
}

void WFileDropWidget::handleTooLarge(::uint64_t size)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // this signal a few times, causing currentFileIdx_
    // to go out of bounds
    return;
  }
  tooLarge_.emit(currentFile(), size);
  currentFileIdx_++;
}

void WFileDropWidget::stopReceiving()
{
  if (currentFileIdx_ < uploads_.size()) {
    for (unsigned i=currentFileIdx_; i < uploads_.size(); i++)
      if (!uploads_[i]->cancelled())
        uploadFailed_.emit(uploads_[i].get());
    // std::cerr << "ERROR: file upload was still listening, "
    //               << "cancelling expected uploads"
    //               << std::endl;
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
    auto f = uploads_[i].get();
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
  if (currentFile()->uploadId() == id)
    return true;
  else {
    return false;
  }
}

void WFileDropWidget::cancelUpload(File *file)
{
  if (file->directory()) {
    Directory *dir = dynamic_cast<Directory*>(file);
    for (File *f : dir->contents()) {
      cancelUpload(f);
    }
  } else {
    file->cancel();
    int i = file->uploadId();
    doJavaScript(jsRef() + ".cancelUpload(" + std::to_string(i) + ");");
  }
}

bool WFileDropWidget::remove(File *file)
{
  if (file->directory()) {
    for (unsigned i=0; i < directories_.size(); i++) {
      if (directories_[i].get() == file) {
        directories_.erase(directories_.begin()+i);
        return true;
      }
    }
  } else {
    for (unsigned i=0; i < currentFileIdx_ && i < uploads_.size(); i++) {
      if (uploads_[i].get() == file) {
        uploads_.erase(uploads_.begin()+i);
        currentFileIdx_--;
        return true;
      }
    }
  }
  return false;
}

void WFileDropWidget::cleanDirectoryResources()
{
  directories_.clear();
}

void WFileDropWidget::onData(::uint64_t current, ::uint64_t total)
{
  if (currentFileIdx_ >= uploads_.size()) {
    // This shouldn't happen, but a mischievous client might emit
    // the filetoolarge signal too many times, causing currentFileIdx_
    // to go out of bounds
    return;
  }
  currentFile()->emitDataReceived(current, total, filterSupported_);

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
  tooLarge_.emit(currentFile(), dataExceeded);

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
      doJavaScript(jsRef() + ".setAcceptDirectories("
                   + (acceptDirectories_ ? "true" : "false") + ", "
                   + (acceptDirectoriesRecursive_ ? "true" : "false") + ");");
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
    if (updateFlags_.test(BIT_ONCLICKFILEPICKER_CHANGED) || all) {
      std::string type = "file-selection";
      if (onClickFilePicker_ == FilePickerType::None) {
        type = "none";
      } else if (onClickFilePicker_ == FilePickerType::DirectorySelection) {
        type = "directory-selection";
      } else if (onClickFilePicker_ != FilePickerType::FileSelection) {
        LOG_WARN("Unknown FilePickerType, falling back to FileSelection.");
      }
      doJavaScript(jsRef() + ".setOnClickFilePicker(\"" + type + "\");");
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

void WFileDropWidget::setAcceptDirectories(bool enable, bool recursive) {
  acceptDirectories_ = enable;
  acceptDirectoriesRecursive_ = recursive;
  updateFlags_.set(BIT_ACCEPTDROPS_CHANGED);

  if (!acceptDirectories_ && onClickFilePicker_ == FilePickerType::DirectorySelection) {
    LOG_WARN("setAcceptDirectories: Reverting the onClickFilePicker to FileSelection since this widget no longer accepts directories.");
    setOnClickFilePicker(FilePickerType::FileSelection);
  }

  repaint();
}

void WFileDropWidget::setOnClickFilePicker(FilePickerType type) {
  if (onClickFilePicker_ == type)
    return;

  if (type == FilePickerType::DirectorySelection && !acceptDirectories_) {
    LOG_ERROR("setOnClickFilePicker: Cannot configure directory filepicker because this widget does not accept directories.");
    return;
  }
  onClickFilePicker_ = type;

  updateFlags_.set(BIT_ONCLICKFILEPICKER_CHANGED);
  repaint();
}

void WFileDropWidget::openFilePicker() {
  if (!acceptDrops_) {
    LOG_WARN("Not opening file picker since acceptDrop() is false.");
    return;
  }

  wApp->doJavaScript(jsRef() + ".serverFileInput.click()");
}

void WFileDropWidget::openDirectoryPicker() {
  if (!acceptDrops_) {
    LOG_WARN("Not opening directory picker since acceptDrop() is false.");
    return;
  }
  if (!acceptDirectories_) {
    LOG_WARN("Not opening directory picker since acceptDirectories() is false.");
    return;
  }

  wApp->doJavaScript(jsRef() + ".serverDirInput.click()");
}

void WFileDropWidget::createWorkerResource() {
  if (uploadWorkerResource_ != 0) {
    delete uploadWorkerResource_;
    uploadWorkerResource_ = 0;
  }

  if (jsFilterFn_.empty())
    return;

#ifndef WT_TARGET_JAVA
  uploadWorkerResource_ = addChild(std::make_unique<WMemoryResource>("text/javascript"));
#else // WT_TARGET_JAVA
  uploadWorkerResource_ = new WMemoryResource("text/javascript");
#endif // WT_TARGET_JAVA

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
