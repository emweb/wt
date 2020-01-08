/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Message.h"
#include "Wt/WException.h"
#include "Wt/WStringStream.h"
#include "base64.h"

#ifndef WT_WIN32
#include <unistd.h>
#endif
#ifdef WT_WIN32
#include <process.h>
#endif

/*
 * References:
 *  - RFC2045 -- MIME
 *  - RFC2821 -- SMTP
 *  - ...
 */

namespace Wt {
  namespace Mail {

Message::Header::Header()
{ }

Message::Header::Header(const std::string& name, const std::string& value)
  : name_(name),
    value_(value)
{ }

Message::Header::Header(const Header& other)
  : name_(other.name_),
    value_(other.value_)
{ }

void Message::Header::setName(const std::string& name)
{
  name_ = name;
}

void Message::Header::setValue(const std::string& value)
{
  value_ = value;
}

Message::Message()
{ }

void Message::setFrom(const Mailbox& from)
{
  from_ = from;
}

void Message::setReplyTo(const Mailbox& replyTo)
{
  replyTo_ = replyTo;
}

void Message::setSubject(const WString& subject)
{
  subject_ = subject;
}

void Message::setBody(const WString& text)
{
  body_ = text;
}

void Message::addRecipient(RecipientType type, const Mailbox& recipient)
{
  recipients_.push_back(Recipient());
  recipients_.back().mailbox = recipient;
  recipients_.back().type = type;
}

void Message::addHtmlBody(const WString& text)
{
  htmlBody_ = text;
}

void Message::addAttachment(const std::string& mimeType,
			    const std::string& fileName,
			    std::istream *data)
{
  Attachment attachment;
  attachment.mimeType = mimeType;
  attachment.fileName = fileName;
  attachment.data = data;

  attachments_.push_back(attachment);
}

void Message::setHeader(const std::string& name, const std::string& value)
{
  for (unsigned i = 0; i < headers_.size(); ++i) {
    if (headers_[i].name() == name) {
      headers_[i].setValue(value);
      return;
    }
  }

  addHeader(name, value);
}

void Message::setDate(const Wt::WLocalDateTime& date)
{
  date_ = date;
}

void Message::addHeader(const std::string& name, const std::string& value)
{
  headers_.push_back(Header(name, value));
}

const std::string *Message::getHeader(const std::string& name) const
{
  for (unsigned i = 0; i < headers_.size(); ++i)
    if (headers_[i].name() == name)
      return &headers_[i].value();

  return nullptr;
}

std::string Message::generateBoundary()
{
  std::string result;

  result.reserve(32);
  result = "--=_"; // recommended as per RFC 2045
  srand(getpid() + rand());

  for (unsigned j = 0; j < 50; ++j) {
    unsigned i = rand() % 67;
    char c;
    if (i < 26)
      c = 'a' + i;
    else if (i < 52)
      c = 'A' + (i - 26);
    else if (i < 62)
      c = '0' + (i - 52);
    else {
      /*
       * We had to trim this down from what is allowed by the RFC because
       * of Outlook ...
       */
      const char *specials = "()+-.";
      c = specials[i - 62];
    }

    result += c;
  }

  return result;
}

void Message::write(std::ostream& out) const
{
  out << "MIME-Version: 1.0\r\n"; // to support encodings

  bool mimeMultiPartAlternative = !htmlBody_.empty();
  bool mimeMultiPartMixed = !attachments_.empty();

  std::string altBoundary, mixedBoundary;

  if (mimeMultiPartMixed)
    mixedBoundary = generateBoundary();

  if (mimeMultiPartAlternative)
    altBoundary = generateBoundary();

  from_.write("From", out);

  if (!date_.isNull())
    out << "Date: " << date_.toString("ddd, dd MMM yyyy HH:mm:ss Z") << "\r\n";

  if (!replyTo_.empty())
    replyTo_.write("Reply-To", out);

  if (!subject_.empty()) {
    out << "Subject: ";
    encodeWord(subject_, out, false);
    out << "\r\n";
  }

  for (unsigned i = 0; i < recipients_.size(); ++i) {
    static const char *recipients[] = {
      "To", "Cc"
    };

    const Recipient& r = recipients_[i];
    if (r.type != RecipientType::Bcc)
      r.mailbox.write(recipients[static_cast<unsigned int>(r.type)], out);
  }

  for (unsigned i = 0; i < headers_.size(); ++i) {
    const Header& h = headers_[i];
    out << h.name() << ": ";
    encodeWord(h.value(), out, false);
    out << "\r\n";
  }

  if (mimeMultiPartMixed) {
    out << "Content-Type: multipart/mixed; boundary=\""
	<< mixedBoundary << "\"\r\n\r\n"
	<< "--" << mixedBoundary << "\r\n";
  }

  if (mimeMultiPartAlternative) {
    out << "Content-Type: multipart/alternative; boundary=\""
	<< altBoundary << "\"\r\n\r\n"
	<< "--" << altBoundary << "\r\n";
  }

  out << "Content-Type: text/plain; charset=UTF-8\r\n"
      << "Content-Transfer-Encoding: quoted-printable\r\n"
      << "\r\n";

  encodeQuotedPrintable(body_, out);

  if (mimeMultiPartAlternative) {
    out << "--" << altBoundary << "\r\n";
    out << "Content-Type: text/html; charset=UTF-8\r\n"
	<< "Content-Transfer-Encoding: quoted-printable\r\n"
	 << "\r\n";
    encodeQuotedPrintable(htmlBody_, out);
    out << "--" << altBoundary << "--\r\n";
  }

  for (unsigned i = 0; i < attachments_.size(); ++i) {
    out << "--" << mixedBoundary << "\r\n";

    encodeAttachment(attachments_[i], out);
  }

  if (mimeMultiPartMixed)
    out << "--" << mixedBoundary << "--\r\n";
}

void Message::encodeAttachment(const Attachment& attachment, std::ostream& out)
{
  out << "Content-Type: ";

  std::string contentType = attachment.mimeType;
  if (!attachment.fileName.empty())
    contentType += "; name=\"" + attachment.fileName + "\"";

  encodeWord(WString::fromUTF8(contentType), out, false);
  out << "\r\n";

  if (!attachment.fileName.empty()) {
    out << "Content-Disposition: ";
    encodeWord(WString::fromUTF8("attachment; filename=\""
				 + attachment.fileName + "\""), out, false);
    out << "\r\n";
  }

  out << "Content-Transfer-Encoding: base64\r\n"
      << "\r\n";

  std::istreambuf_iterator<char> eos;
  std::istreambuf_iterator<char> iit(attachment.data->rdbuf());

  base64::encode(iit, eos, std::ostreambuf_iterator<char>(out));
  out << "\r\n";
}

void Message::encodeQuotedPrintable(const WString& text, std::ostream& out)
{
  std::string msg = text.toUTF8();

  WStringStream line;
  for (unsigned i = 0; i < msg.length(); ++i) {
    unsigned char d = msg[i];
    unsigned char peek = 0;
    if (i + 1 < msg.length())
      peek = msg[i + 1];

    /* skip '\r' if followed by '\n' */
    if (d == '\r' && peek == '\n') {
      ++i;
      d = msg[i];
    }

    if (d >= 33 && d <= 126 && d != 61) {
      line << (char)d;
    } else if (peek != '\n' && (d == '\t' || d == ' ')) {
      line << (char)d;
    } else if (d != '\n') {
      encodeChar(line, d);
    }

    bool eol = false;

    if (d == '\n')
      eol = true;
    else if (line.length() >= 72) {
      line << '=';
      eol = true;
    }

    if (eol) {
      if (line.c_str()[0] == '.')
	out << '.';
      out << line.c_str() << "\r\n";
      line.clear();
    }
  }

  if (!line.empty()) {
    if (line.c_str()[0] == '.')
      out << '.';
    out << line.c_str() << "\r\n";
  }
}

void Message::encodeChar(WStringStream& s, unsigned char c)
{
  s << '=';
  const char *hex = "0123456789ABCDEF";
  s << hex[(c & 0xF0) >> 4];
  s << hex[c & 0x0F];
}

void Message::encodeWord(const WString& text, std::ostream& out,
			 bool quoteIfNeeded)
{
  std::string msg = text.toUTF8();

  bool needQuote = false;
  bool needQEncode = false;

  for (unsigned i = 0; i < msg.size(); ++i) {
    unsigned char c = msg[i];

    if (c > 127)
      needQEncode = true;

    if (quoteIfNeeded && !needQuote) {
      /*
       * Note: not using std::isalnum becaues we need to be sure this
       * uses the C locale
       */
      bool alpha = (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
      bool digit = (c >= 48 && c <= 57);
      if (!alpha && !digit && c != '.' && c != '_' && c != '-')
	needQuote = true;
    }

    if (c == '\r' || c == '\n')
      throw WException("Illegal header value.");
  }

  if (needQEncode) {
    WStringStream line;

    for (unsigned i = 0; i < msg.length(); ++i) {
      if (line.empty())
	line << "=?UTF-8?q?";

      unsigned char d = msg[i];

      if (d >= 33 && d <= 126
	  && d != '='
	  && d != '?'
	  && d != '_')
	line << (char)d;
      else if (d == ' ')
	line << '_';
      else
	encodeChar(line, d);

      if (line.length() >= 72) {
	line << "?=";
	if (i != msg.length() - 1)
	  line << "\r\n ";
	out << line.c_str();
	line.clear();
      }
    }

    if (!line.empty())
      out << line.c_str() << "?=";
  } else if (quoteIfNeeded && needQuote) {
    out << '"' << msg << '"';
  } else {
    out << msg;
  }
}

  }
}
