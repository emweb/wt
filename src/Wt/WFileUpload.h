// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFILEUPLOAD_H_
#define WFILEUPLOAD_H_

#include <Wt/WJavaScriptSlot.h>
#include <Wt/WWebWidget.h>

namespace Wt {

/*! \class WFileUpload Wt/WFileUpload.h Wt/WFileUpload.h
 *  \brief A widget that allows a file to be uploaded.
 *
 * This widget is displayed as a box in which a filename can be
 * entered and a browse button.
 *
 * Depending on availability of JavaScript, the behaviour of the widget
 * is different, but the API is designed in a way which facilitates
 * a portable use.
 *
 * When JavaScript is available, the file will not be uploaded until
 * upload() is called. This will start an asynchronous upload (and
 * thus return immediately). \if cpp While the file is being uploaded,
 * the dataReceived() signal is emitted when data is being received
 * and if the connector library provides support (see also
 * WResource::setUploadProgress() for a more detailed
 * discussion). Although you can modify the GUI from this signal, you
 * still need to have a mechanism in place to update the client
 * interface (using a WTimer or using \link
 * WApplication::enableUpdates() server-push\endlink). When the file
 * has been uploaded, the uploaded() signal is emitted, or if the file
 * was too large, the fileTooLarge() signal is emitted. You may
 * configure a progress bar that is used to show the upload progress
 * using setProgressBar(). \endif
 *
 * When no JavaScript is available, the file will be uploaded with the
 * next click event. Thus, upload() has no effect -- the file will
 * already be uploaded, and the corresponding signals will already be
 * emitted. To test if upload() will start an upload, you may check
 * using the canUpload() call.
 *
 * Thus, to properly use the widget, one needs to follow these
 * rules:
 * <ul>
 *   <li>Be prepared to handle the uploaded() or fileTooLarge() signals
 *       also when upload() was not called.</li>
 *   <li>Check using canUpload() if upload() will schedule a new
 *       upload. if (!canUpload()) then upload() will not have any
 *       effect. if (canUpload()), upload() will start a new file upload,
 *       which completes succesfully using an uploaded() signal or a
 *       fileTooLarge() signals gets emitted.
 *   </li>
 * </ul>
 *
 * The %WFileUpload widget must be hidden or deleted when a file is
 * received. In addition it is wise to prevent the user from uploading
 * the file twice as in the example below.
 *
 * The uploaded file is automatically spooled to a local temporary
 * file which will be deleted together with the WFileUpload widget,
 * unless stealSpooledFile() is called.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WFileUpload *upload = addWidget(std::make_unique<Wt::WFileUpload>());
 * upload->setFileTextSize(40);
 *
 * // Provide a button
 * Wt::WPushButton *uploadButton =
 *   addWidget(std::make_unique<Wt::WPushButton>("Send"));

 * // Upload when the button is clicked.
 * uploadButton->clicked().connect(upload, &Wt::WFileUpload::upload);
 * uploadButton->clicked().connect(uploadButton, &Wt::WPushButton::disable);
 *
 * // Upload automatically when the user entered a file.
 * upload->changed().connect(upload, &WFileUpload::upload);
 * upload->changed().connect(uploadButton, &Wt::WPushButton::disable);
 *
 * // React to a succesfull upload.
 * upload->uploaded().connect(this, &MyWidget::fileUploaded);
 *
 * // React to a fileupload problem.
 * upload->fileTooLarge().connect(this, &MyWidget::fileTooLarge);
 * \endcode
 * \endif
 *
 * %WFileUpload is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The file upload itself corresponds to a <tt>&lt;input
 * type="file"&gt;</tt> tag, but may be wrapped in a
 * <tt>&lt;form&gt;</tt> tag for an Ajax session to implement the
 * asynchronous upload action. This widget does not provide styling,
 * and styling through CSS is not well supported across browsers.
 */
class WT_API WFileUpload : public WWebWidget
{
public:
  /*! \brief Creates a file upload widget
   */
  WFileUpload();

  ~WFileUpload();

  /*! \brief Sets whether the file upload accepts multiple files.
   *
   * In browsers which support the "multiple" attribute for the file
   * upload (to be part of HTML5) control, this will allow the user to
   * select multiple files at once.
   *
   * All uploaded files are available from uploadedFiles(). The
   * single-file API will return only information on the first
   * uploaded file.
   *
   * The default value is \c false.
   */
  void setMultiple(bool multiple);

  /*! \brief Returns whether multiple files can be uploaded.
   *
   * \sa setMultiple()
   */
  bool multiple() const { return flags_.test(BIT_MULTIPLE); }

  /*! \brief Sets the size of the file input.
   */
  void setFileTextSize(int chars);

  /*! \brief Returns the size of the file input.
   */
  int fileTextSize() const { return textSize_; }

  /*! \brief Returns the spooled location of the uploaded file.
   *
   * Returns the temporary filename in which the uploaded file was
   * spooled. The file is guaranteed to exist as long as the
   * WFileUpload widget is not deleted, or a new file is not uploaded.
   *
   * When multiple files were uploaded, this returns the information
   * from the first file.
   *
   * \sa stealSpooledFile()
   * \sa uploaded
   */
  std::string spoolFileName() const;

  /*! \brief Returns the client filename.
   *
   * When multiple files were uploaded, this returns the information
   * from the first file.
   *
   * \note Depending on the browser this is an absolute path
   *       or only the file name.
   */
  WT_USTRING clientFileName() const;

  /*! \brief Returns the client content description.
   *
   * When multiple files were uploaded, this returns the information
   * from the first file.
   */
  WT_USTRING contentDescription() const;

  /*! \brief Steals the spooled file.
   *
   * By stealing the file, the spooled file will no longer be deleted
   * together with this widget, which means you need to take care of
   * managing that.
   *
   * When multiple files were uploaded, this returns the information
   * from the first file.
   */
  void stealSpooledFile();

  /*! \brief Returns whether one or more files have been uploaded.
   */
  bool empty() const;

  /*! \brief Returns the uploaded files.
   */
  const std::vector<Http::UploadedFile>& uploadedFiles() const
    { return uploadedFiles_; }

  /*! \brief Returns whether upload() will start a new file upload.
   *
   * A call to upload() will only start a new file upload if there is
   * no JavaScript support. Otherwise, the most recent file will
   * already be uploaded.
   */
  bool canUpload() const { return fileUploadTarget_ != nullptr; }

  /*! \brief Use the click signal of another widget to open the file picker.
   *
   * This hides the default WFileUpload widget and uses the click-signal of the argument to
   * open the file picker. The upload logic is still handled by WFileUpload behind the scenes.
   * This action cannot be undone.
   * 
   * WFileUpload does not take ownership of the widget, nor does it display it. You must still 
   * place it in the widget tree yourself.
   */
  void setDisplayWidget(WInteractWidget *widget);
  
  /*! \brief %Signal emitted when a new file was uploaded.
   *
   * This signal is emitted when file upload has been completed.  It
   * is good practice to hide or delete the WFileUpload widget when a
   * file has been uploaded succesfully.
   *
   * \sa upload()
   * \sa fileTooLarge()
   */
  EventSignal<>& uploaded();

  /*! \brief %Signal emitted when the user tried to upload a too large file.
   *
   * The parameter is the (approximate) size of the file (in bytes) the user
   * tried to upload.
   *
   * The maximum file size is determined by the maximum request size,
   * which may be configured in the configuration file (<max-request-size>).
   *
   * \sa uploaded()
   * \sa WApplication::requestTooLarge()
   */
  JSignal< ::int64_t>& fileTooLarge() { return fileTooLarge_; }

  /*! \brief %Signal emitted when the user selected a new file.
   *
   * One could react on the user selecting a (new) file, by uploading
   * the file immediately.
   *
   * Caveat: this signal is not emitted with konqueror and possibly
   * other browsers. Thus, in the above scenario you should still provide
   * an alternative way to call the upload() method.
   */
  EventSignal<>& changed();

  /*! \brief Starts the file upload.
   *
   * The uploaded() signal is emitted when a file is uploaded, or the
   * fileTooLarge() signal is emitted when the file size exceeded the
   * maximum request size.
   *
   * \sa uploaded()
   * \sa canUpload()
   */
  void upload();

  /*! \brief Sets a progress bar to indicate upload progress.
   *
   * When the file is being uploaded, upload progress is indicated
   * using the provided progress bar. Both the progress bar range and
   * values are configured when the upload starts.
   *
   * The file upload itself is hidden as soon as the upload starts.
   *
   * The default progress bar is 0 (no upload progress is indicated).
   *
   * \if java
   * To update the progess bar server push is used, you should only
   * use this functionality when using a Servlet 3.0 compatible servlet 
   * container.
   * \endif
   *
   * \sa dataReceived()
   */
  void setProgressBar(WProgressBar *progressBar);

  /*! \brief Sets a progress bar to indicate upload progress.
   *
   * When the file is being uploaded, upload progress is indicated
   * using the provided progress bar. Both the progress bar range and
   * values are configured when the upload starts.
   *
   * The bar becomes part of the file upload, and replaces the file
   * prompt when the upload is started.
   *
   * The default progress bar is 0 (no upload progress is indicated).
   *
   * \if java
   * To update the progess bar server push is used, you should only
   * use this functionality when using a Servlet 3.0 compatible servlet 
   * container.
   * \endif
   *
   * \sa dataReceived()
   */
  void setProgressBar(std::unique_ptr<WProgressBar> progressBar);

  /*! \brief Returns the progress bar.
   *
   * \sa setProgressBar()
   */
  WProgressBar *progressBar() const { return progressBar_; }

  /*! \brief %Signal emitted while a file is being uploaded.
   *
   * When supported by the connector library, you can track the
   * progress of the file upload by listening to this signal.
   *
   * The first argument is the number of bytes received so far,
   * and the second argument is the total number of bytes.
   */
  Signal< ::uint64_t, ::uint64_t>& dataReceived() { return dataReceived_; }

  virtual void enableAjax() override;

  ///
  /// \brief Sets input accept attributes
  ///
  /// The accept attribute may be specified to provide user agents with a hint
  /// of what file types will be accepted.
  /// Use html input accept attributes as input.
  ///
  /// \code
  /// WFileUpload *fu = new WFileUpload(root());
  /// fu->setFilters("image/*");
  /// \endcode
  ///
  void setFilters(const std::string& acceptAttributes);

private:
  static const char *CHANGE_SIGNAL;
  static const char *UPLOADED_SIGNAL;

  static const int BIT_DO_UPLOAD        	= 0;
  static const int BIT_ENABLE_AJAX      	= 1;
  static const int BIT_UPLOADING        	= 2;
  static const int BIT_MULTIPLE         	= 3;
  static const int BIT_ENABLED_CHANGED  	= 4;
  static const int BIT_ACCEPT_ATTRIBUTE_CHANGED = 5;
  static const int BIT_USE_DISPLAY_WIDGET       = 6;

  std::bitset<7> flags_;

  int textSize_;

  std::vector<Http::UploadedFile> uploadedFiles_;

  JSignal< ::int64_t> fileTooLarge_;

  Signal< ::uint64_t, ::uint64_t> dataReceived_;

  Core::observing_ptr<WInteractWidget> displayWidget_;
  JSlot displayWidgetRedirect_;

  std::unique_ptr<WResource> fileUploadTarget_;
  std::unique_ptr<WProgressBar> containedProgressBar_;
  WProgressBar *progressBar_;

  std::string acceptAttributes_;
  void create();

  void onData(::uint64_t current, ::uint64_t total);
  void onDataExceeded(::uint64_t dataExceeded);

  std::string displayWidgetClickJS();

  virtual void setRequestTooLarge(::int64_t size) override;

protected:
  virtual void           updateDom(DomElement& element, bool all) override;
  virtual DomElement    *createDomElement(WApplication *app) override;
  virtual DomElementType domElementType() const override;
  virtual void           propagateRenderOk(bool deep) override;
  virtual void           getDomChanges(std::vector<DomElement *>& result,
                                       WApplication *app) override;
  virtual void propagateSetEnabled(bool enabled) override;
  virtual std::string renderRemoveJs(bool recursive) override;

private:
  void handleFileTooLarge(::int64_t fileSize);

  void onUploaded();

  virtual void setFormData(const FormData& formData) override;
  void setFiles(const std::vector<Http::UploadedFile>& files);

  friend class WFileUploadResource;
};

}

#endif // WFILEUPLOAD_H_
