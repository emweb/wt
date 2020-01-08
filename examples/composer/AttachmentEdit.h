// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ATTACHMENT_EDIT_H_
#define ATTACHMENT_EDIT_H_

#include <Wt/WContainerWidget.h>

namespace Wt {
  class WFileUpload;
  class WText;
  class WCheckBox;
}

class Attachment;
class Composer;
class Option;

using namespace Wt;

/**
 * @addtogroup composerexample
 */
/*@{*/

/*! \brief An edit field for an email attachment.
 *
 * This widget manages one attachment edit: it shows a file upload
 * control, handles the upload, and gives feed-back on the file
 * uploaded.
 *
 * This widget is part of the %Wt composer example.
 */
class AttachmentEdit : public WContainerWidget
{
public:
  /*! \brief Creates an attachment edit field.
   */
  AttachmentEdit(Composer *composer);

  /*! \brief Updates the file now.
   *
   * Returns whether a new file will be uploaded. If so, the uploadDone
   * signal will be signalled when the file is uploaded (or failed to
   * upload).
   */
  bool uploadNow();

  /*! \brief Returns whether the upload failed.
   */
  bool uploadFailed() const { return uploadFailed_; }

  /*! \brief Returns the attachment.
   */
  std::vector<Attachment> attachments();

  /*! \brief Signal emitted when new attachment(s) have been uploaded (or failed
   *         to upload.
   */
  Signal<>& uploadDone() { return uploadDone_; }

private:
  Composer    *composer_;

  Signal<> uploadDone_;

  //! The WFileUpload control.
  WFileUpload *upload_;

  class UploadInfo : public WContainerWidget
  {
  public:
    UploadInfo(const Http::UploadedFile& f);

    Http::UploadedFile info_;

    //! Anchor referencing the file.
    WAnchor   *downloadLink_;

    //! The check box to keep or discard the uploaded file.
    WCheckBox *keep_;
  };

  std::vector<UploadInfo *> uploadInfo_;

  //! The text box to display an error (empty or too big file)
  WText *error_;

  //! The option to cancel the file upload
  Option *remove_;

  //! The state of the last upload process.
  bool uploadFailed_;

  //! Slot triggered when the WFileUpload completed an upload.
  void uploaded();

  //! Slot triggered when the WFileUpload received an oversized file.
  void fileTooLarge(::int64_t size);

  //! Slot triggered when the users wishes to remove this attachment edit.
  void remove();
};

/*@}*/

#endif // ATTACHMENT_EDIT_H_
