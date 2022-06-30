// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFILEDROPCONTAINER_WIDGET_H_
#define WFILEDROPCONTAINER_WIDGET_H_

#include "Wt/WContainerWidget.h"
#include "Wt/WResource.h"

namespace Wt {

class WMemoryResource;
namespace Json {
  class Object;
}

/*! \brief Enumeration of file picker types
 *
 * The browser can open the file picker in two modes: one where only files
 * can be selected and another where only directories can be selected.
 */
enum class FilePickerType {
  None,              //!< No file picker.
  FileSelection,     //!< Only files can be selected.
  DirectorySelection //!< Only directories can be selected.
};

/*! \class WFileDropWidget Wt/WFileDropWidget.h Wt/WFileDropWidget.h
 *  \brief A widget that allows dropping files and directories for upload.
 *
 * This widget accepts files that are dropped into it. A signal is triggered
 * whenever one or more files or directories are dropped. The filename, type
 * and size of files is immediately available through the WFileDropWidget::File
 * interface. Similarly, information about directories is available through
 * the WFileDropWidget::Directory interface (which is a subclass of File).
 *
 * The file upload is done sequentially. All files before the currentIndex()
 * have either finished, failed or have been cancelled.
 *
 * The widget has the default style-class 'Wt-filedropzone'. The style-class
 * 'Wt-dropzone-hover' is added when files are hovered over the widget.
 *
 * Apart from dropping files, users can also use the browser-specific dialog
 * to select files or directories. Note that the dialog will support either
 * selecting files or directories, but not both at the same time.
 * The dialog can be opened by clicking the widget. The type of dialog that is
 * opened can be configured with setOnClickFilePicker(FilePickerType). The
 * dialog can also be opened programmatically in response to another event
 * (e.g. a user clicking a button outside this widget) using openFilePicker()
 * and openDirectoryPicker().
 */
class WT_API WFileDropWidget : public WContainerWidget {
public:
  /*! \class File
   *  \brief A nested class of WFileDropWidget representing a file
   *
   * The methods returning the filename, mime-type and size return valid
   * values if the upload of this file is not yet finished. The method
   * uploadedFile() is only available after the upload is finished.
   */
  class WT_API File : public WObject {
  public:
    /*! \brief Returns the client filename.
     */
    const std::string& clientFileName() const { return clientFileName_; }

    /*! \brief Returns the path of the file.
     *
     * This is only relevant if the user dropped a folder. The path will be
     * relative to the folder that was dropped.
     */
    const std::string& path() const { return path_; }

    /*! \brief Returns whether this is a directory.
     */
    virtual bool directory() const { return false; }

    /*! \brief Returns the mime-type of the file.
     */
    const std::string& mimeType() const { return type_; }

    /*! \brief Returns the size of the file.
     */
    ::uint64_t size() const { return size_; }

    /*! \brief Returns the uploaded file as a Http::UploadedFile.
     *
     * This method will throw an expection if the upload is not yet finished.
     *
     * \sa uploadFinished()
     */
    const Http::UploadedFile& uploadedFile() const;

    /*! \brief Returns true if the upload is finished.
     *
     * When this method returns true, the uploaded file is available on the
     * server.
     *
     * \sa uploadedFile()
     */
    bool uploadFinished() const;

    /*! \brief This signal allows you to track the upload progress of the file.
     *
     * The first argument is the number of bytes received so far,
     * and the second argument is the total number of bytes.
     */
    Signal< ::uint64_t, ::uint64_t >& dataReceived() { return dataReceived_; }

    /*! \brief This signal is triggered when the upload is finished.
     *
     * This is also signalled using the WFileDropWidget
     * \link WFileDropWidget::uploaded uploaded() \endlink signal.
     */
    Signal<>& uploaded() { return uploaded_; }

    void setFilterEnabled(bool enabled);
    bool filterEnabled() { return filterEnabled_; }

    bool isFiltered() const { return isFiltered_; }

    // Wt internal
    File(int id, const std::string& fileName, const std::string& path, const std::string& type,
         ::uint64_t size, ::uint64_t chunkSize);
    int uploadId() const { return id_; }
    void handleIncomingData(const Http::UploadedFile& file, bool last);
    void cancel();
    bool cancelled() const;
    void emitDataReceived(::uint64_t current, ::uint64_t total,
                          bool filterSupported);
    void setIsFiltered(bool filtered);

  private:
    int id_;
    std::string clientFileName_;
    std::string path_;
    std::string type_;
    ::uint64_t size_;
    Http::UploadedFile uploadedFile_;
    Signal< ::uint64_t, ::uint64_t > dataReceived_;
    Signal<> uploaded_;

    bool uploadStarted_;
    bool uploadFinished_;
    bool cancelled_;
    bool filterEnabled_;
    bool isFiltered_;
    unsigned nbReceivedChunks_;
    ::uint64_t chunkSize_;
  };

  /*! \class Directory
   *  \brief A nested class of WFileDropWidget representing a Directory.
   *
   * In true linux tradition, a Directory is a File. However, in this case it
   * was more a matter of compatibility. This class was added later on and by
   * having it inherit from File, the existing WFileDropWidget::drop() signal
   * can return both Files and Directories.
   */
  class WT_API Directory : public File {
  public:
    /*! \brief Returns the contents of the directory.
     */
    const std::vector<File*>& contents() const { return contents_; }

    /*! \brief Returns whether this is a directory.
     */
    bool directory() const override { return true; }

    // Wt internal
    Directory(const std::string& fileName, const std::string& path);
    void addFile(File *file);

  private:
    std::vector<File*> contents_;
  };

  /*! \brief Constructor
   */
  WFileDropWidget();

#ifndef WT_TARGET_JAVA
  /*! \brief Destructor
   */
  ~WFileDropWidget();
#endif

  /*! \brief Returns the vector of uploads managed by this widget.
   *
   * The files in this vector are handled sequentially by the widget. All
   * WFileDropWidget::File objects in this vector have either finished or
   * failed if they are before the currentIndex(), depending on the return
   * value of WFileDropWidget::File::uploadFinished(). The other files are
   * still being handled.
   *
   * \remark Since version 4.7.0, this method returns a copy of the vector
   * because we changed the internal vector to hold values of type
   * std::unique_ptr.
   *
   * \sa currentIndex()
   */
  std::vector<File*> uploads() const;

  /*! \brief Return the index of the file that is currently being handled.
   *
   * If nothing is to be done, this will return the size of the vector returned
   * by uploads().
   */
  int currentIndex() const { return currentFileIdx_; }

  /*! \brief Cancels the upload of a file.
   *
   * If you cancel a file that is still waiting to be uploaded, it will stay
   * in the uploads() vector, but it will be skipped.
   */
  void cancelUpload(File *file);

  /*! \brief Removes the file.
   *
   * This can be used to free resources of files that were already uploaded. A
   * file can only be removed if its index in uploads() is before the current
   * index. A directory can be removed as soon as the drop() signal is emitted.
   *
   * \note This method is only important if you intend to use this widget to
   * upload a lot of files. Otherwise, simply removing the widget will also clean
   * up all resources.
   *
   * \sa removeDirectories()
   */
  bool remove(File *file);

  /*! \brief Cleans up resources of WFileDropWidget::Directory objects
   *
   * This can be used to free resources. The drop() signal returns raw pointers
   * for objects that are managed by this widget. The Directory objects are no
   * longer needed after the drop() signal, so whenever you don't need them
   * anymore, it is safe to call this method. Note that no WFileDropWidget::File
   * objects are removed by this method since these objects can only be removed
   * after their upload has completed.
   *
   * \note This method is only important if you intend to use this widget to
   * upload a lot of files. Otherwise, simply removing the widget will also clean
   * up all resources.
   *
   * \sa remove(File*)
   */
  void cleanDirectoryResources();

  /*! \brief When set to false, the widget no longer accepts any files.
   */
  void setAcceptDrops(bool enable);

  /*! \brief Set the style class that is applied when a file is hovered over
   * the widget.
   *
   * \deprecated Override the css rule '.Wt-filedropzone.Wt-dropzone-hover' instead.
   */
  WT_DEPRECATED("Override the CSS rule '.Wt-filedropzone.Wt-dropzone-hover' instead.")
  void setHoverStyleClass(const std::string& className);

  /*! \brief Sets input accept attributes
   *
   * The accept attribute may be specified to provide user agents with a
   * hint of what file types will be accepted. Use html input accept attributes
   * as input.
   * This only affects the popup that is shown when users click on the widget.
   * A user can still drop any file type.
   */
  void setFilters(const std::string& acceptAttributes);

  /*! \brief Highlight widget if a file is dragged anywhere on the page
   *
   * As soon as a drag enters anywhere on the page the styleclass
   * 'Wt-dropzone-indication' is added to this widget. This can be useful to
   * point the user to the correct place to drop the file. Once the user drags
   * a file over the widget itself, the styleclass 'hover-style' is also
   * added.
   * This can be enabled for multiple dropwidgets if only one of them is
   * visible at the same time.
   *
   * \sa setGlobalDropEnabled()
   */
  void setDropIndicationEnabled(bool enable);

  /*! \brief Returns if the widget is highlighted for drags anywhere on the page
   *
   * \sa setDropIndicationEnabled()
   */
  bool dropIndicationEnabled() const;

  /*! \brief Allow dropping the files anywhere on the page
   *
   * If enabled, a drop anywhere on the page will be forwarded to this widget.
   *
   * \sa setDropIndicationEnabled()
   */
  void setGlobalDropEnabled(bool enable);

  /*! \brief Returns if all drops are forwarded to this widget.
   *
   * \sa setGlobalDropEnabled
   */
  bool globalDropEnabled() const;

  /*! \brief Supply a function to process file data before it is uploaded to the server.
   */
  void setJavaScriptFilter(const std::string& filterFn, ::uint64_t chunksize = 0, const std::vector<std::string>& imports = std::vector<std::string>());

  /*! \brief Allow users to drop directories.
   *
   * Dropping a directory will emit the drop() signal with a Directory object (which
   * inherits File). A directory can also be recognized by the File::directory()
   * method. After downcasting the object, the method Directory::contents() can be
   * used to iterate over the contents.
   *
   * Subdirectories are also included in the contents. The contents of subdirectories
   * itself is only included if recursive is true.
   *
   * Only File objects for which File::directory() is false are uploaded to the
   * server. The contents of a directory is 'flattened' into the uploads() vector.
   * The directory structure is still available through the File::path() method
   * that describes the file's path relative to the dropped directory.
   *
   * \sa openFilePicker(), openDirectoryPicker()
   */
  void setAcceptDirectories(bool enable, bool recursive = false);

  /*! \brief Returns if directories are accepted.
   *
   * Dropping a directory will upload all of its contents. This can be done either
   * non-recursively (default) or recursively. The directory structure is available
   * during the initial drop() signal or through the File::path() method.
   */
  bool acceptDirectories() const { return acceptDirectories_; }

  /*! \brief Returns if directory contents is uploaded recursively or not.
   */
  bool acceptDirectoriesRecursive() const { return acceptDirectoriesRecursive_; }

  /*! \brief Set the type of file picker that is opened when a user clicks the widget.
   *
   * The default is FilePickerType::FileSelection.
   *
   * When FilePickerType::None is passed, no file picker will be shown. Files or
   * directories can still be dropped in, if setAcceptDrops() is set to \p true (which
   * by default it is). Also note that in this case, the methods openFilePicker() and
   * openDirectoryPicker() can still be used to open a picker by redirecting clicks
   * from other buttons.
   */
  void setOnClickFilePicker(FilePickerType type);

  /*! \brief Returns the type of file picker that is opened when a user clicks the widget.
   */
  FilePickerType onClickFilePicker() const { return onClickFilePicker_; }

  /*! \brief Programmatically open the file picker.
   *
   * Users can click the widget to open a browser-specific dialog to select either files
   * or directories (see setOnClickFilePicker(FilePickerType)). This method allows
   * developers to also open the dialog by other means, e.g. buttons outside the widget
   * to open either the file- or directory picker.
   *
   * \sa openDirectoryPicker()
   */
  void openFilePicker();

  /*! \brief Programmatically open the directory picker.
   *
   * Users can click the widget to open a browser-specific dialog to select either files
   * or directories (see setOnClickFilePicker(FilePickerType)). This method allows
   * developers to also open the dialog by other means, e.g. buttons outside the widget
   * to open either the file- or directory picker.
   *
   * \warning Due to limitations in the directory picker api, empty directories will not be
   * returned when selected through the dialog.
   *
   * \sa openFilePicker()
   */
  void openDirectoryPicker();

  /*! \brief The signal triggers if one or more files are dropped.
   */
  Signal<std::vector<File*> >& drop() { return dropEvent_; }

  /*! \brief The signal triggers when the upload of a file is about to begin.
   *
   * After this signal is triggered, the upload automatically starts. The
   * upload can still fail if the file is too large or if there is a network
   * error.
   */
  Signal<File*>& newUpload() { return uploadStart_; }

  /*! \brief The signal is triggered if any file finished uploading.
   */
  Signal<File*>& uploaded() { return uploaded_; }

  /*! \brief The signal triggers when a file is too large for upload.
   *
   * This signal is triggered when the widget attempts to upload the file.
   *
   * The second argument is the size of the file in bytes.
   */
  Signal<File*, ::uint64_t>& tooLarge() { return tooLarge_; }

  /*! \brief The signal triggers when an upload failed.
   *
   * This signal will trigger when the widget skips over one of the files
   * in the list for an unknown reason (e.g. happens when you drop a folder).
   */
  Signal<File*>& uploadFailed() { return uploadFailed_; }

  /*! \brief Indicate that the next file can be handled
   *
   * Internally indicate handling of the next file. Any resource handling the
   * upload needs to call this when the file is handled.
   */
  void proceedToNextFile();

protected:
  virtual std::string renderRemoveJs(bool recursive) override;
  virtual void enableAjax() override;
  virtual void updateDom(DomElement& element, bool all) override;
  class WFileDropUploadResource;

  /*! \brief Resource to upload data to
   *
   * This returns a resource to upload data to.
   * By default this returns a resource where the file contents can be POSTed.
   * This can be overridden to allow for custom upload mechanisms.
   *
   * This can be used to implement upload protocols that are different from the
   * normal upload flow. The request may include extra information in their
   * payload, or be located on a public fixed URL and require custom handling
   * of the request.
   *
   * On the client side, the JS function wtCustomSend(isValid, url, upload, APP)
   * can implement a custom upload mechanism, with:
   * - isValid: whether a valid file is uploaded
   * - url: the upload location
   * - upload: a file object with:
   *   - id: generated upload identifier
   *   - filename: upload file name
   *   - type: file type
   *   - size: file size
   * 
   * To use this function, define the JS boolean \c wtUseCustomSend, which is
   * \c false by default. Example:
   * \code
   * Wt::WApplication::instance().setJavaScriptMember("wtUseCustomSend", "true");
   * Wt::WApplication::instance().setJavaScriptMember("wtCustomSend",
   *   "function(isValid, url, upload) { * ... * };");
   * \endcode
   */
  virtual std::unique_ptr<WResource> uploadResource();
  virtual JSignal<int>& requestSend() { return requestSend_; };
  virtual File* currentFile() { return uploads_[currentFileIdx_].get(); };

private:
  void setup();
  void handleDrop(const std::string& newDrops);
  void handleTooLarge(::uint64_t size);
  void handleSendRequest(int id);
  void emitUploaded(int id);
  void stopReceiving();
  void onData(::uint64_t current, ::uint64_t total);
  void onDataExceeded(::uint64_t dataExceeded);
  void createWorkerResource();
  void disableJavaScriptFilter();
  File* addDropObject(const Wt::Json::Object& object);

  // Functions for handling incoming requests
  bool incomingIdCheck(int id);

  WMemoryResource *uploadWorkerResource_;
  std::unique_ptr<WResource> resource_;
  unsigned currentFileIdx_;

  static const std::string WORKER_JS;
  std::string jsFilterFn_;
  std::vector<std::string> jsFilterImports_;
  ::uint64_t chunkSize_;
  bool filterSupported_;

  std::string hoverStyleClass_;
  bool acceptDrops_;
  std::string acceptAttributes_;
  bool dropIndicationEnabled_;
  bool globalDropEnabled_;

  bool acceptDirectories_;
  bool acceptDirectoriesRecursive_;

  FilePickerType onClickFilePicker_ = FilePickerType::FileSelection;

  JSignal<std::string> dropSignal_;
  JSignal<int> requestSend_;
  JSignal< ::uint64_t > fileTooLarge_;
  JSignal<int> uploadFinished_;
  JSignal<> doneSending_;
  JSignal<> jsFilterNotSupported_;

  Signal<std::vector<File*> > dropEvent_;
  Signal<File*> uploadStart_;
  Signal<File*> uploaded_;
  Signal< File*, ::uint64_t > tooLarge_;
  Signal<File*> uploadFailed_;

  std::vector<std::unique_ptr<File> > uploads_;
  std::vector<std::unique_ptr<File> > directories_;

  static const int BIT_HOVERSTYLE_CHANGED        = 0;
  static const int BIT_ACCEPTDROPS_CHANGED       = 1;
  static const int BIT_FILTERS_CHANGED           = 2;
  static const int BIT_DRAGOPTIONS_CHANGED       = 3;
  static const int BIT_JSFILTER_CHANGED          = 4;
  static const int BIT_ONCLICKFILEPICKER_CHANGED = 5;
  std::bitset<6> updateFlags_;
  bool updatesEnabled_; // track if this widget enabled updates.

};

}

#endif
