// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_MAIL_MESSAGE_H_
#define WT_MAIL_MESSAGE_H_

#include <Wt/WLocalDateTime.h>
#include <Wt/Mail/Mailbox.h>

namespace Wt {

  class WStringStream;

  namespace Mail {

/*! \brief Enumeration for a recipient type.
 *
 * \sa Message::addRecipient()
 */
enum class RecipientType {
  To, //!< To: recipient
  Cc, //!< Cc: recipient
  Bcc //!< Bcc: recipient (is omitted from the recipients in the message itself)
};

/*! \class Message Wt/Mail/Message.h Wt/Mail/Message.h
 *  \brief A mail message.
 *
 * This class represents a MIME-compliant mail message.
 *
 * The message can have a plain text body and an optional HTML body,
 * which when present is encoded as an MIME multipart/alternative. It
 * is recommended to send the same contents both in a plain text and
 * an HTML variant.
 *
 * Recipient names, names, and body text may contain unicode text.
 *
 * \sa Client::send()
 *
 * \ingroup mail
 */
class WT_API Message
{
public:
  /*! \class Header
   *  \brief An SMTP message header.
   * 
   * An SMTP message header is a name/value pair, as defined by RFC 822.
   */
  class WT_API Header 
  {
  public:
    /*! \brief Default constructor.
     */
    Header();

    /*! \brief Constructs a header with a given name and value.
     */
    Header(const std::string& name, const std::string& value);

    /*! \brief Copy constructor.
     */
    Header(const Header& other);

    /*! \brief Sets the header name.
     */
    void setName(const std::string& name);

    /*! \brief Returns the header name.
     *
     * \sa setName()
     */
    const std::string& name() const { return name_; }

    /*! \brief Sets the header value.
     */
    void setValue(const std::string& value);

    /*! \brief Returns the header value.
     *
     * \sa setValue()
     */
    const std::string& value() const { return value_; }

  private:
    std::string name_, value_;
  };

  /*! \class Recipient
   *  \brief A struct representing a recipient
   */
  struct Recipient {
    RecipientType type; //!< The type of recipient
    Mailbox mailbox; //!< The mailbox
  };

  /*! \brief Default constructor.
   *
   * Creates an empty message. You need to add at least a sender and a recipient
   * to create a valid email message.
   */
  Message();

  /*! \brief Sets the sender mailbox.
   */
  void setFrom(const Mailbox& from);

  /*! \brief Returns the sender mailbox.
   *
   * \sa setFrom()
   */
  const Mailbox& from() const { return from_; }

  /*! \brief Sets the reply-to mailbox.
   */
  void setReplyTo(const Mailbox& replyTo);

  /*! \brief Returns the reply-to mailbox.
   *
   * \sa setReplyTo()
   */
  const Mailbox& replyTo() const { return replyTo_; }

  /*! \brief Sets a subject.
   */
  void setSubject(const WString& subject);

  /*! \brief Returns the subject.
   *
   * \sa setSubject()
   */
  const WString& subject() const { return subject_; }

  /*! \brief Sets a date.
   *
   * According to RFC 2822, the date should express local time.
   */
  void setDate(const WLocalDateTime& date);

  /*! \brief Returns the date.
   */
  WLocalDateTime date() const { return date_; }

  /*! \brief Sets the plain text body.
   *
   * This is the plain text mail contents.
   *
   * \sa addHtmlBody()
   */
  void setBody(const WString& text);

  /*! \brief Returns the plain text body.
   *
   * \sa setBody()
   */
  const WString& body() const { return body_; }

  /*! \brief Adds a recipient.
   *
   * A mail can have multiple recipients.
   */
  void addRecipient(RecipientType type, const Mailbox& recipient);

  /*! \brief Returns the recipients.
   *
   * \sa addRecipient()
   */
  const std::vector<Recipient>& recipients() const { return recipients_; }

  /*! \brief Sets a header value.
   *
   * If a header with that value was already defined, it is replaced with
   * the new value. Otherwise, the header is added.
   *
   * \sa addHeader()
   */
  void setHeader(const std::string& name, const std::string& value);

  /*! \brief Adds a header value.
   *
   * A header is added, even if a header with the same name already
   * was present.
   *
   * \sa setHeader()
   */
  void addHeader(const std::string& name, const std::string& value);

  /*! \brief Returns the headers.
   */
  const std::vector<Header>& headers() const { return headers_; }

  /*! \brief Returns a header value.
   *
   * Returns 0 if no header with that name is found.
   */
  const std::string *getHeader(const std::string& name) const;

  /*! \brief Adds an HTML body.
   *
   * The \p text should be an HTML version of the plain text body.
   */
  void addHtmlBody(const WString& text);

  /*! \brief Returns the HTML body.
   *
   * \sa setHtmlBody()
   */
  const WString& htmlBody() const { return htmlBody_; }

  /*! \brief Adds an attachment.
   *
   * Ownership of the \p data stream is not transferred; you should keep this
   * object valid until the message has been sent using Client::send() or
   * written using write().
   */
  void addAttachment(const std::string& mimeType, const std::string& fileName,
		     std::istream *data);

  /*! \brief Writes the message to the stream.
   *
   * This writes the message as a MIME 1.0 message to the output stream.
   */
  void write(std::ostream& out) const;

private:
  struct Attachment {
    std::string mimeType;
    std::string fileName;
    std::istream *data;
  };

  Mailbox from_, replyTo_;
  std::vector<Recipient> recipients_;
  std::vector<Header> headers_;
  std::vector<Attachment> attachments_;

  WString subject_, body_, htmlBody_;
  WLocalDateTime date_;

  static std::string generateBoundary();
  static void encodeAttachment(const Attachment& attachment,
			       std::ostream& out);

  static void encodeChar(WStringStream& s, unsigned char c);
  static void encodeWord(const WString& text, std::ostream& out,
			 bool quoteIfNeeded);
  static void encodeQuotedPrintable(const WString& text, std::ostream& out);

  friend class Mailbox;
};

  }
}

#endif // WT_MAIL_MESSAGE_H_
