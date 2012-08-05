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

#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WCheckBox>
#include <Wt/WCssDecorationStyle>
#include <Wt/WFileResource>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WText>

#include "Attachment.h"
#include "AttachmentEdit.h"
#include "Composer.h"
#include "Option.h"

AttachmentEdit::UploadInfo::UploadInfo(const Http::UploadedFile& f,
				       WContainerWidget *parent)
  : WContainerWidget(parent),
    info_(f)
{
  /*
   * Include the file ?
   */
  keep_ = new WCheckBox(this);
  keep_->setChecked();

  /*
   * Give information on the file uploaded.
   */
  std::streamsize fsize = 0;
  {
    std::ifstream theFile(info_.spoolFileName().c_str());
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

  std::wstring fn = static_cast<std::wstring>
    (escapeText(WString::fromUTF8(info_.clientFileName())));

  downloadLink_
    = new WAnchor("", fn + L" (<i>" + WString::fromUTF8(info_.contentType())
		  + L"</i>) " + size, this);

  WFileResource *res = new WFileResource(info_.contentType(),
					 info_.spoolFileName(),
					 this);
  res->suggestFileName(info_.clientFileName());
  downloadLink_->setLink(res);
}

AttachmentEdit::AttachmentEdit(Composer *composer, WContainerWidget *parent)
  : WContainerWidget(parent),
    composer_(composer),
    uploadDone_(this),
    uploadFailed_(false)
{
  /*
   * The file upload itself.
   */
  upload_ = new WFileUpload(this);
  upload_->setMultiple(true);
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
  std::vector<Http::UploadedFile> files = upload_->uploadedFiles();

  if (!files.empty()) {
    /*
     * Delete this widgets since we have a succesfull upload.
     */
    delete upload_;
    upload_ = 0;
    delete remove_;
    remove_ = 0;
    delete error_;
    error_ = 0;

    for (unsigned i = 0; i < files.size(); ++i)
      uploadInfo_.push_back(new UploadInfo(files[i], this));
  } else {
    error_->setText(tr("msg.file-empty"));
    uploadFailed_ = true;
  }

  /*
   * Signal to the Composer that a new asynchronous file upload was processed.
   */
  uploadDone_.emit();
}

void AttachmentEdit::remove()
{
  composer_->removeAttachment(this);
}

void AttachmentEdit::fileTooLarge(::int64_t size)
{
  error_->setText(tr("msg.file-too-large")
		  .arg(size / 1024)
		  .arg(WApplication::instance()->maximumRequestSize() / 1024));
  uploadFailed_ = true;

  /*
   * Signal to the Composer that a new asyncrhonous file upload was processed.
   */
  uploadDone_.emit();
}

std::vector<Attachment> AttachmentEdit::attachments()
{
  std::vector<Attachment> result;

  for (unsigned i = 0; i < uploadInfo_.size(); ++i) {
    if (uploadInfo_[i]->keep_->isChecked()) {
      Http::UploadedFile& f = uploadInfo_[i]->info_;
      f.stealSpoolFile();
      result.push_back(Attachment
		       (WString::fromUTF8(f.clientFileName()),
			WString::fromUTF8(f.contentType()),
			f.spoolFileName()));
    }
  }

  return result;
}
