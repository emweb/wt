// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef COMPOSER_H_
#define COMPOSER_H_

#include <Wt/WCompositeWidget.h>

#include "Contact.h"
#include "Attachment.h"

namespace Wt {
class WLineEdit;
class WPushButton;
class WTable;
class WText;
class WTextArea;
}

class AddresseeEdit;
class AttachmentEdit;
class ContactSuggestions;
class OptionList;
class Option;

using namespace Wt;

/**
 * @addtogroup composerexample
 */
//!@{

/*! \brief An E-mail composer widget.
 *
 * This widget is part of the %Wt composer example.
 */
class Composer : public WCompositeWidget
{
public:
  /*! \brief Construct a new Composer
   */
  Composer();

  /*! \brief Set message To: contacts
   */
  void setTo(const std::vector<Contact>& to);

  /*! \brief Set subject.
   */
  void setSubject(const WString& subject);

  /*! \brief Set the message.
   */
  void setMessage(const WString& message);

  /*! \brief Set the address book, for autocomplete suggestions. 
   */
  void setAddressBook(const std::vector<Contact>& addressBook);

  /*! \brief Get the To: contacts.
   */
  std::vector<Contact> to() const;

  /*! \brief Get the Cc: contacts.
   */
  std::vector<Contact> cc() const;

  /*! \brief Get the Bc: contacts.
   */
  std::vector<Contact> bcc() const;

  /*! \brief Get the subject.
   */
  const WString& subject() const;

  /*! \brief Get the list of attachments.
   *
   * The ownership of the attachment spool files is transferred
   * to the caller as well, be sure to delete them !
   */
  std::vector<Attachment> attachments() const;

  /*! \brief Get the message.
   */
  const WString& message() const;

public:
  /*! \brief The message is ready to be sent...
   */
  Wt::Signal<> send;

  /*! \brief The message must be discarded.
   */
  Wt::Signal<> discard;

private:
  WContainerWidget *layout_;

  WPushButton      *topSendButton_, *topSaveNowButton_, *topDiscardButton_;
  WPushButton      *botSendButton_, *botSaveNowButton_, *botDiscardButton_;
  WText            *statusMsg_;

  WTable           *edits_;

  //! To: Addressees edit.
  AddresseeEdit    *toEdit_;
  //! Cc: Addressees edit.
  AddresseeEdit    *ccEdit_;
  //! Bcc: Addressees edit.
  AddresseeEdit    *bccEdit_;

  //! The suggestions popup for the addressee edits.
  ContactSuggestions *contactSuggestions_;

  //! The subject line edit.
  WLineEdit        *subject_;

  //! OptionsList for editing Cc or Bcc
  OptionList       *options_;

  //! Option for editing Cc:
  Option           *addcc_;
  //! Option for editing Bcc:
  Option           *addbcc_;
  //! Option for attaching a file.
  Option           *attachFile_;
  //! Option for attaching another file.
  Option           *attachOtherFile_;

  //! Array which holds all the attachments, including one extra invisible one.
  std::vector<AttachmentEdit *> attachments_;

  //! WTextArea for the main message.
  WTextArea        *message_;

  //! state when waiting asyncrhonously for attachments to be uploaded
  bool saving_, sending_;

  //! number of attachments waiting to be uploaded during saving
  int attachmentsPending_;

  /*!\brief Add an attachment edit.
   */
  void attachMore();

  /*!\brief Remove the given attachment edit.
   */
  void removeAttachment(AttachmentEdit *attachment);

  /*! \brief Slot attached to the Send button.
   *
   * Tries to save the mail message, and if succesfull, sends it.
   */
  void sendIt();

  /*! \brief Slot attached to the Save now button.
   *
   * Tries to save the mail message, and gives feedback on failure
   * and on success.
   */
  void saveNow();

  /*! \brief Slot attached to the Discard button.
   *
   * Discards the current message: emits the discard event.
   */
  void discardIt();

  /*! \brief Slotcalled when an attachment has been uploaded.
   *
   * This used during while saving the email and waiting
   * for remaining attachments to be uploaded. It is connected
   * to the AttachmentEdit control signals that are emitted when
   * an attachment has been processed.
   */
  void attachmentDone();

private:
  // create the user-interface
  void createUi();

  /*! \brief All attachments have been processed, determine the result
   *         of saving the message.
   */
  void saved();

  /*! \brief Set the status, and apply the given style.
   */
  void setStatus(const WString& text, const WString& style);

  friend class AttachmentEdit;
};

//!@}

#endif // COMPOSER_H_
