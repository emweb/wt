/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include "AddresseeEdit.h"
#include "AttachmentEdit.h"
#include "Composer.h"
#include "ContactSuggestions.h"
#include "Label.h"
#include "Option.h"
#include "OptionList.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WStringUtil.h>
#include <algorithm>

Composer::Composer()
  : WCompositeWidget(),
    saving_(false),
    sending_(false)
{
  std::unique_ptr<WContainerWidget> layout
      = cpp14::make_unique<WContainerWidget>();
  layout_ = layout.get();
  setImplementation(std::move(layout));

  createUi();
}

void Composer::setTo(const std::vector<Contact>& to)
{
  toEdit_->setAddressees(to);
}

void Composer::setSubject(const WString& subject)
{
  subject_->setText(subject);
}

void Composer::setMessage(const WString& message)
{
  message_->setText(message);
}

std::vector<Contact> Composer::to() const
{
  return toEdit_->addressees();
}

std::vector<Contact> Composer::cc() const
{
  return ccEdit_->addressees();
}
 
std::vector<Contact> Composer::bcc() const
{
  return bccEdit_->addressees();
}

void Composer::setAddressBook(const std::vector<Contact>& contacts)
{
  contactSuggestions_->setAddressBook(contacts);
}

const WString& Composer::subject() const
{
  return subject_->text();
}

std::vector<Attachment> Composer::attachments() const
{
  std::vector<Attachment> attachments;

  for (unsigned i = 0; i < attachments_.size() - 1; ++i) {
    std::vector<Attachment> toadd = attachments_[i]->attachments();

    attachments.insert(attachments.end(), toadd.begin(), toadd.end());
  }

  return attachments;
}

const WString& Composer::message() const
{
  return message_->text();
}

void Composer::createUi()
{
  setStyleClass("darker");

  // horizontal layout container, used for top and bottom buttons.
  WContainerWidget *horiz;

  /*
   * Top buttons
   */
  horiz = layout_->addWidget(cpp14::make_unique<WContainerWidget>());
  horiz->setPadding(5);
  topSendButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.send")));
  topSendButton_->setStyleClass("default"); // default action
  topSaveNowButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.savenow")));
  topDiscardButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.discard")));

  // Text widget which shows status messages, next to the top buttons.
  statusMsg_ = horiz->addWidget(cpp14::make_unique<WText>());
  statusMsg_->setMargin(15, Side::Left);

  /*
   * To, Cc, Bcc, Subject, Attachments
   *
   * They are organized in a two-column table: left column for
   * labels, and right column for the edit.
   */
  edits_ = layout_->addWidget(cpp14::make_unique<WTable>());
  edits_->setStyleClass("lighter");
  edits_->resize(WLength(100, LengthUnit::Percentage), WLength::Auto);
  edits_->elementAt(0, 0)->resize(WLength(1, LengthUnit::Percentage),
				  WLength::Auto);

  /*
   * To, Cc, Bcc
   */
  toEdit_ = edits_->elementAt(0,1)->addWidget(cpp14::make_unique<AddresseeEdit>(tr("msg.to"), edits_->elementAt(0, 0)));
  // add some space above To:
  edits_->elementAt(0, 1)->setMargin(5, Side::Top);
  ccEdit_ = edits_->elementAt(1,1)->addWidget(cpp14::make_unique<AddresseeEdit>(tr("msg.cc"), edits_->elementAt(1, 0)));
  bccEdit_ = edits_->elementAt(2,1)->addWidget(cpp14::make_unique<AddresseeEdit>(tr("msg.bcc"), edits_->elementAt(2, 0)));

  ccEdit_->hide();
  bccEdit_->hide();

  /*
   * Addressbook suggestions popup
   */
  contactSuggestions_ = layout_->addWidget(cpp14::make_unique<ContactSuggestions>());

  contactSuggestions_->forEdit(toEdit_);
  contactSuggestions_->forEdit(ccEdit_);
  contactSuggestions_->forEdit(bccEdit_);

  /*
   * We use an OptionList widget to show the expand options for
   * ccEdit_ and bccEdit_ nicely next to each other, separated
   * by pipe characters.
   */
  options_ = edits_->elementAt(3, 1)->addWidget(cpp14::make_unique<OptionList>());
  std::unique_ptr<Option> addcc(new Option(tr("msg.addcc")));
  addcc_ = addcc.get();
  std::unique_ptr<Option> addbcc(new Option(tr("msg.addbcc")));
  addbcc_ = addbcc.get();

  options_->add(std::move(addcc));
  options_->add(std::move(addbcc));

  /*
   * Subject
   */
  edits_->elementAt(4, 0)->addWidget(cpp14::make_unique<Label>(tr("msg.subject"), edits_->elementAt(4, 0)));
  subject_ = edits_->elementAt(4, 1)->addWidget(cpp14::make_unique<WLineEdit>());
  subject_->resize(WLength(99, LengthUnit::Percentage), WLength::Auto);

  /*
   * Attachments
   */
  edits_->elementAt(5, 0)->addWidget(cpp14::make_unique<WImage>("icons/paperclip.png"));
  edits_->elementAt(5, 0)->setContentAlignment(AlignmentFlag::Right | AlignmentFlag::Top);
  edits_->elementAt(5, 0)->setPadding(3);
  
  // Attachment edits: we always have the next attachmentedit ready
  // but hidden. This improves the response time, since the show()
  // and hide() slots are stateless.
  AttachmentEdit *attachmentEdit = edits_->elementAt(5, 1)->addWidget(cpp14::make_unique<AttachmentEdit>(this));
  attachments_.push_back(attachmentEdit);
  attachments_.back()->hide();

  /*
   * Two options for attaching files. The first does not say 'another'.
   */
  attachFile_ = edits_->elementAt(5, 1)->addWidget(cpp14::make_unique<Option>(tr("msg.attachfile")));
  attachOtherFile_ = edits_->elementAt(5, 1)->addWidget(cpp14::make_unique<Option>(tr("msg.attachanother")));
  attachOtherFile_->hide();

  /*
   * Message
   */
  message_ = layout_->addWidget(cpp14::make_unique<WTextArea>());
  message_->setColumns(80);
  message_->setRows(10); // should be 20, but let's keep it smaller
  message_->setMargin(10);

  /*
   * Bottom buttons
   */
  horiz = layout_->addWidget(cpp14::make_unique<WContainerWidget>());
  horiz->setPadding(5);
  botSendButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.send")));
  botSendButton_->setStyleClass("default");
  botSaveNowButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.savenow")));
  botDiscardButton_ = horiz->addWidget(cpp14::make_unique<WPushButton>(tr("msg.discard")));

  /*
   * Button events.
   */
  topSendButton_->clicked().connect(this, &Composer::sendIt);
  botSendButton_->clicked().connect(this, &Composer::sendIt);
  topSaveNowButton_->clicked().connect(this, &Composer::saveNow);
  botSaveNowButton_->clicked().connect(this, &Composer::saveNow);
  topDiscardButton_->clicked().connect(this, &Composer::discardIt);
  botDiscardButton_->clicked().connect(this, &Composer::discardIt);

  /*
   * Option events to show the cc or Bcc edit.
   *
   * Clicking on the option should both show the corresponding edit, and
   * hide the option itself.
   */
  addcc_->item()->clicked().connect(ccEdit_, &WWidget::show);
  addcc_->item()->clicked().connect(addcc_, &WWidget::hide);
  addcc_->item()->clicked().connect(options_, &OptionList::update);
  addcc_->item()->clicked().connect(ccEdit_, &WWidget::setFocus);

  addbcc_->item()->clicked().connect(bccEdit_, &WWidget::show);
  addbcc_->item()->clicked().connect(addbcc_, &WWidget::hide);
  addbcc_->item()->clicked().connect(options_, &OptionList::update);
  addbcc_->item()->clicked().connect(bccEdit_, &WWidget::setFocus);

  /*
   * Option event to attach the first attachment.
   *
   * We show the first attachment, and call attachMore() to prepare the
   * next attachment edit that will be hidden.
   *
   * In addition, we need to show the 'attach More' option, and hide the
   * 'attach' option.
   */
  attachFile_->item()->clicked().connect(attachments_.back(), &WWidget::show);
  attachFile_->item()->clicked().connect(attachOtherFile_, &WWidget::show);
  attachFile_->item()->clicked().connect(attachFile_, &WWidget::hide);
  attachFile_->item()->clicked().connect(this, &Composer::attachMore);
  attachOtherFile_->item()->clicked().connect(this, &Composer::attachMore);
}

void Composer::attachMore()
{
  /*
   * Create and append the next AttachmentEdit, that will be hidden.
   */
  std::unique_ptr<AttachmentEdit> edit
        = cpp14::make_unique<AttachmentEdit>(this);
  AttachmentEdit *editPtr = edit.get();
  edits_->elementAt(5, 1)->insertBefore(std::move(edit), attachOtherFile_);
  attachments_.push_back(editPtr);
  attachments_.back()->hide();

  // Connect the attachOtherFile_ option to show this attachment.
  attachOtherFile_->item()->clicked()
    .connect(attachments_.back(), &WWidget::show);
}

void Composer::removeAttachment(AttachmentEdit *attachment)
{
  /*
   * Remove the given attachment from the attachments list.
   */
  std::vector<AttachmentEdit *>::iterator i
      = std::find(attachments_.begin(), attachments_.end(), attachment);

  if (i != attachments_.end()) {
    attachments_.erase(i);
    attachment->removeFromParent();

    if (attachments_.size() == 1) {
      /*
       * This was the last visible attachment, thus, we should switch
       * the option control again.
       */
      attachOtherFile_->hide();
      attachFile_->show();
      attachFile_->item()->clicked()
	.connect(attachments_.back(), &WWidget::show);
    }
  }
}

void Composer::sendIt()
{
  if (!sending_) {
    sending_ = true;

    /*
     * First save -- this will check for the sending_ state
     * signal if successfull.
     */
    saveNow();
  }
}

void Composer::saveNow()
{
  if (!saving_) {
    saving_ = true;

    /*
     * Check if any attachments still need to be uploaded.
     * This may be the case when fileupload change events could not
     * be caught (for example in Konqueror).
     */
    attachmentsPending_ = 0;

    for (unsigned i = 0; i < attachments_.size() - 1; ++i) {
      if (attachments_[i]->uploadNow()) {
	++attachmentsPending_;

	// this will trigger attachmentDone() when done, see
	// the AttachmentEdit constructor.
      }
    }

    std::cerr << "Attachments pending: " << attachmentsPending_ << std::endl;
    if (attachmentsPending_)
      setStatus(tr("msg.uploading"), "status");
    else
      saved();
  }
}

void Composer::attachmentDone()
{
  if (saving_) {
    --attachmentsPending_;
    std::cerr << "Attachments still: " << attachmentsPending_ << std::endl;

    if (attachmentsPending_ == 0)
      saved();
  }
}

void Composer::setStatus(const WString& text, const WString& style)
{
  statusMsg_->setText(text);
  statusMsg_->setStyleClass(style);
}

void Composer::saved()
{
  /*
   * All attachments have been processed.
   */

  bool attachmentsFailed = false;
  for (unsigned i = 0; i < attachments_.size() - 1; ++i)
    if (attachments_[i]->uploadFailed()) {
      attachmentsFailed = true;
      break;
    }

  if (attachmentsFailed) {
    setStatus(tr("msg.attachment.failed"), "error");
  } else {
    setStatus(tr("msg.ok"), "status");
    WString timeStr = Wt::WLocalDateTime::currentDateTime().toString("HH:mm");
    statusMsg_->setText(Wt::utf8("Draft saved at {1}").arg(timeStr));

    if (sending_) {
      send.emit();
      return;
    }
  }

  saving_ = false;
  sending_ = false;
}

void Composer::discardIt()
{ 
  discard.emit();
}
