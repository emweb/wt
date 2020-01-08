/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#ifndef WIN32
#include <unistd.h>
#endif

#include <iostream>

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WCheckBox.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WFileResource.h>
#include <Wt/WLink.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WText.h>

#include "Attachment.h"
#include "AttachmentEdit.h"
#include "Composer.h"
#include "Option.h"

AttachmentEdit::UploadInfo::UploadInfo(const Http::UploadedFile& f)
  : WContainerWidget(),
    info_(f)
{
  /*
   * Include the file ?
   */
  keep_ = this->addWidget(cpp14::make_unique<WCheckBox>());
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
  std::u32string size;
  if (fsize < 1024)
    size = WString(std::to_string(fsize)) + U" bytes";
  else
    size = WString(std::to_string((int)(fsize / 1024)))
      + U"kb";

  std::u32string fn = static_cast<std::u32string>
    (escapeText(WString(info_.clientFileName())));

  downloadLink_
    = this->addWidget(cpp14::make_unique<WAnchor>("", fn + U" (<i>" + WString(info_.contentType())
                  + U"</i>) " + size));

  auto res = std::make_shared<WFileResource>(info_.contentType(),info_.spoolFileName());
  res->suggestFileName(info_.clientFileName());
  downloadLink_->setLink(WLink(res));
}

AttachmentEdit::AttachmentEdit(Composer *composer)
  : WContainerWidget(),
    composer_(composer),
    uploadDone_(),
    uploadFailed_(false)
{
  /*
   * The file upload itself.
   */
  upload_ = this->addWidget(cpp14::make_unique<WFileUpload>());
  upload_->setMultiple(true);
  upload_->setFileTextSize(40);

  /*
   * A progress bar
   */
  std::unique_ptr<WProgressBar> progress = cpp14::make_unique<WProgressBar>();
  progress->setFormat(WString::Empty);
  progress->setVerticalAlignment(AlignmentFlag::Middle);
  upload_->setProgressBar(std::move(progress));

  /*
   * The 'remove' option.
   */
  remove_ = this->addWidget(cpp14::make_unique<Option>(tr("msg.remove")));
  upload_->decorationStyle().font().setSize(FontSize::Smaller);
  upload_->setVerticalAlignment(AlignmentFlag::Middle);
  remove_->setMargin(5, Side::Left);
  remove_->item()->clicked().connect(this, &WWidget::hide);
  remove_->item()->clicked().connect(this, &AttachmentEdit::remove);

  // The error message.
  error_ = this->addWidget(cpp14::make_unique<WText>(""));
  error_->setStyleClass("error");
  error_->setMargin(WLength(5), Side::Left);

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
    upload_ = 0;
    this->removeWidget(remove_);
    remove_ = 0;
    this->removeWidget(error_);
    error_ = 0;

    for (unsigned i = 0; i < files.size(); ++i) {
      UploadInfo *info = this->addWidget(cpp14::make_unique<UploadInfo>(files[i]));
      uploadInfo_.push_back(info);
    }
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
		       (WString(f.clientFileName()),
			WString(f.contentType()),
			f.spoolFileName()));
    }
  }

  return result;
}
