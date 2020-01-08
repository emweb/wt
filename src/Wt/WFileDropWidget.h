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
  
/*! \class WFileDropWidget Wt/WFileDropWidget.h Wt/WFileDropWidget.h
 *  \brief A widget that allows dropping files for upload.
 *
 * This widget accepts files that are dropped into it. A signal is triggered
 * whenever one or more files are dropped. The filename, type and size of
 * these files is immediately available through the WFileDropWidget::File
 * interface.
 * 
 * The file upload is done sequentially. All files before the currentIndex()
 * have either finished, failed or have been cancelled.
 *
 * The widget has the default style-class 'Wt-filedropzone'. The style-class
 * 'Wt-dropzone-hover' is added when files are hovered over the widget.
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
    bool uploadFinished() const { return uploadFinished_; }

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
    File(int id, const std::string& fileName, const std::string& type, ::uint64_t size, ::uint64_t chunkSize);
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


  /*! \brief Constructor
   */
  WFileDropWidget();

  /*! \brief Returns the vector of uploads managed by this widget.
   *
   * The files in this vector are handled sequentially by the widget. All 
   * WFileDropWidget::File objects in this vector have either finished or
   * failed if they are before the currentIndex(), depending on the return
   * value of WFileDropWidget::File::uploadFinished(). The other files are
   * still being handled.
   *
   * \sa currentIndex()
   */
  const std::vector<File*>& uploads() const { return uploads_; }

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
   * index.
   */
  bool remove(File *file);

  /*! \brief When set to false, the widget no longer accepts any files.
   */
  void setAcceptDrops(bool enable);
  
  /*! \brief Set the style class that is applied when a file is hovered over 
   * the widget.
   *
   * \deprecated Override the css rule '.Wt-filedropzone.Wt-dropzone-hover' instead.
   */
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

protected:
  virtual std::string renderRemoveJs(bool recursive) override;
  virtual void enableAjax() override;
  virtual void updateDom(DomElement& element, bool all) override;

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

  // Functions for handling incoming requests
  void proceedToNextFile();
  bool incomingIdCheck(int id);

  WMemoryResource *uploadWorkerResource_;
  class WFileDropUploadResource;
  WFileDropUploadResource *resource_;
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
  
  std::vector<File*> uploads_;

  static const int BIT_HOVERSTYLE_CHANGED  = 0;
  static const int BIT_ACCEPTDROPS_CHANGED = 1;
  static const int BIT_FILTERS_CHANGED     = 2;
  static const int BIT_DRAGOPTIONS_CHANGED = 3;
  static const int BIT_JSFILTER_CHANGED    = 4;
  std::bitset<5> updateFlags_;
  bool updatesEnabled_; // track if this widget enabled updates.

  friend class WFileDropUploadResource;
};
  
}

#endif
