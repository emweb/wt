/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#ifndef WIN32
#include <unistd.h>
#endif
#include <boost/lexical_cast.hpp>

#include <iostream>

#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WText>

#include "Attachment.h"
#include "AttachmentEdit.h"
#include "Composer.h"
#include "Option.h"

AttachmentEdit::AttachmentEdit(Composer *composer, WContainerWidget *parent)
  : WContainerWidget(parent),
    composer_(composer),
    uploadDone_(this),
    uploadFailed_(false),
    taken_(false)
{
  /*
   * The file upload itself.
   */
  upload_ = new WFileUpload(this);
  upload_->setFileTextSize(40);

  /*
   * A progress bar
   */
  WProgressBar *progress = new WProgressBar();
  progress->setFormat(WString::Empty);
  progress->setVerticalAlignment(AlignMiddle);
  upload_->setProgressBar(progress);

  /*
   * The 'remove' option.
   */
  remove_ = new Option(tr("msg.remove"), this);
  upload_->decorationStyle().font().setSize(WFont::Smaller);
  upload_->setVerticalAlignment(AlignMiddle);
  remove_->setMargin(5, Left);
  remove_->item()->clicked().connect(this, &WWidget::hide);
  remove_->item()->clicked().connect(this, &AttachmentEdit::remove);

  /*
   * Fields that will display the feedback.
   */

  // The check box to include or exclude the attachment.
  keep_ = new WCheckBox(this);
  keep_->hide();

  // The uploaded file information.
  uploaded_ = new WText("", this);
  uploaded_->setStyleClass("option");
  uploaded_->hide();

  // The error message.
  error_ = new WText("", this);
  error_->setStyleClass("error");
  error_->setMargin(WLength(5), Left);

  /*
   * React to events.
   */

  // Try to catch the fileupload change signal to trigger an upload.
  // We could do like google and at a delay with a WTimer as well...
  upload_->changed().connect(upload_, &WFileUpload::upload);

  // React to a succesfull upload.
  upload_->uploaded().connect(this, &AttachmentEdit::uploaded);

  // React to a fileupload problem.
  upload_->fileTooLarge().connect(this, &AttachmentEdit::fileTooLarge);

  /*
   * Connect the uploadDone signal to the Composer's attachmentDone,
   * so that the Composer can keep track of attachment upload progress,
   * if it wishes.
   */
  uploadDone_.connect(composer, &Composer::attachmentDone);
}

AttachmentEdit::~AttachmentEdit()
{
  // delete the local attachment file copy, if it was not taken from us.
  if (!taken_)
    unlink(spoolFileName_.c_str());
}

bool AttachmentEdit::uploadNow()
{
  /*
   * See if this attachment still needs to be uploaded,
   * and return if a new asynchronous upload is started.
   */
  if (upload_) {
    if (upload_->canUpload()) {
      upload_->upload();
      return true;
    } else
      return false;
  } else
    return false;
}

void AttachmentEdit::uploaded()
{
  if (!upload_->emptyFileName()) {
    fileName_ = upload_->clientFileName();
    spoolFileName_ = upload_->spoolFileName();
    upload_->stealSpooledFile();
    contentDescription_ = upload_->contentDescription();

    /*
     * Delete this widgets since we have a succesfull upload.
     */
    delete upload_;
    upload_ = 0;
    delete remove_;
    remove_ = 0;

    error_->setText("");

    /*
     * Include the file ?
     */
    keep_->show();
    keep_->setChecked();

    /*
     * Give information on the file uploaded.
     */
    std::streamsize fsize = 0;
    {
      std::ifstream theFile(spoolFileName_.c_str());
      theFile.seekg(0, std::ios_base::end);
      fsize = theFile.tellg();
      theFile.seekg(0);
    }
    std::wstring size;
    if (fsize < 1024)
      size = boost::lexical_cast<std::wstring>(fsize) + L" bytes";
    else
      size = boost::lexical_cast<std::wstring>((int)(fsize / 1024))
	+ L"kb";

    uploaded_->setText(static_cast<std::wstring>(escapeText(fileName_))
		       + L" (<i>" + contentDescription_ + L"</i>) " + size);
    uploaded_->show();

    uploadFailed_ = false;
  } else {
    error_->setText(tr("msg.file-empty"));
    uploadFailed_ = true;
  }

  /*
   * Signal to the Composer that a new asyncrhonous file upload was processed.
   */
  uploadDone_.emit();
}

void AttachmentEdit::remove()
{
  composer_->removeAttachment(this);
}

void AttachmentEdit::fileTooLarge(int size)
{
  error_->setText(tr("msg.file-too-large"));
  uploadFailed_ = true;

  /*
   * Signal to the Composer that a new asyncrhonous file upload was processed.
   */
  uploadDone_.emit();
}

bool AttachmentEdit::include() const
{
  return keep_->isChecked();
}

Attachment AttachmentEdit::attachment()
{
  taken_ = true;
  return Attachment(fileName_, contentDescription_, spoolFileName_);
}
