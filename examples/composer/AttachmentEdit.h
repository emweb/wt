// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ATTACHMENT_EDIT_H_
#define ATTACHMENT_EDIT_H_

#include <Wt/WContainerWidget>

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
 * This widget managements one attachment edit: it shows a file upload
 * control, handles the upload, and gives feed-back on the file
 * uploaded.
 *
 * This widget is part of the %Wt composer example.
 */
class AttachmentEdit : public WContainerWidget
{
public:
  /*! \brief Create an attachment edit field.
   */
  AttachmentEdit(Composer *composer, WContainerWidget *parent = 0);
  ~AttachmentEdit();

  /*! \brief Update the file now.
   *
   * Returns whether a new file will be uploaded. If so, the uploadDone
   * signal will be signalled when the file is uploaded (or failed to
   * upload).
   */
  bool uploadNow();

  /*! \brief Return whether the upload failed.
   */
  bool uploadFailed() const { return uploadFailed_; }

  /*! \brief Return whether this attachment must be included in the message.
   */
  bool include() const;

  /*! \brief Return the attachment.
   */
  Attachment attachment();

  /*! \brief Signal emitted when a new attachment has been uploaded (or failed
   *         to upload.
   */
  Signal<void>& uploadDone() { return uploadDone_; }

private:
  Composer    *composer_;

  Signal<void> uploadDone_;

  //! The WFileUpload control.
  WFileUpload *upload_;

  //! The text describing the uploaded file.
  WText       *uploaded_;

  //! The check box to keep or discard the uploaded file.
  WCheckBox   *keep_;

  //! The option to remove the file
  Option      *remove_;

  //! The text box to display an error (empty or too big file)
  WText       *error_;

  //! The state of the last upload process.
  bool         uploadFailed_;

  //! The filename of the uploaded file.
  std::wstring  fileName_;

  //! The filename of the local spool file.
  std::string  spoolFileName_;

  //! The content description that was sent along with the file.
  std::wstring  contentDescription_;

  //! Whether the spool file is "taken" and is no longer managed by the edit.
  bool         taken_;

private slots:
  //! Slot triggered when the WFileUpload completed an upload.
  void uploaded();

  //! Slot triggered when the WFileUpload received an oversized file.
  void fileTooLarge(int size);

  //! Slot triggered when the users wishes to remove this attachment edit.
  void remove();

};

/*@}*/

#endif // ATTACHMENT_EDIT_H_
