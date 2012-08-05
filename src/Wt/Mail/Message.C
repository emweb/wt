/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Message"
#include "Wt/WException"
#include "Wt/WStringStream"
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef WIN32
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

void Message::addAttachment(const std::string& mimeType /* ... */)
{

}

void Message::write(std::ostream& out) const
{
  bool mimeMultiPartAlternative = !htmlBody_.empty();
  std::string boundary;

  out << "MIME-Version: 1.0\r\n"; // to support encodings

  if (mimeMultiPartAlternative) {
    boundary.reserve(32);
    boundary = "--=_"; // recommended as per RFC 2045
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

      boundary += c;
    }
  }

  from_.write("From", out);

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
    if (r.type != Bcc)
      r.mailbox.write(recipients[r.type], out);
  }

  if (mimeMultiPartAlternative) {
    out << "Content-Type: multipart/alternative; boundary=\""
	<< boundary << "\"\r\n"
	<< "--" << boundary << "\r\n";
  }
  out << "Content-Type: text/plain; charset=UTF-8\r\n"
      << "Content-Transfer-Encoding: quoted-printable\r\n"
      << "\r\n";

  encodeQuotedPrintable(body_, out);

  if (mimeMultiPartAlternative) {
    out << "--" << boundary << "\r\n";
    out << "Content-Type: text/html; charset=UTF-8\r\n"
	<< "Content-Transfer-Encoding: quoted-printable\r\n"
	 << "\r\n";
    encodeQuotedPrintable(htmlBody_, out);
    out << "--" << boundary << "--\r\n";
  }
}

void Message::encodeQuotedPrintable(const WString& text, std::ostream& out)
{
  std::string msg = text.toUTF8();

  WStringStream line;
  bool lastIsSpace = false;
  for (unsigned i = 0; i < msg.length(); ++i) {
    unsigned char d = msg[i];

    /* skip '\r' if followed by '\n' */
    if (d == '\r' && i < msg.length() - 1 && msg[i + 1] == '\n') {
      ++i;
      d = msg[i];
    }

    if (d >= 33 && d <= 126 && d != 61) {
      line << (char)d;
      lastIsSpace = false;
    } else if (d == '\t' || d == ' ') {
      line << (char)d;
      lastIsSpace = true;
    } else if (d != '\n') {
      encodeChar(line, d);
      lastIsSpace = false;
    }

    bool eol = false;

    if (d == '\n') {
      if (lastIsSpace)
	line << '=';
      eol = true;
    } else if (line.length() >= 72) {
      line << '=';
      eol = true;
    }

    if (eol) {
      if (line.c_str()[0] == '.')
	out << '.';
      out << line.c_str() << "\r\n";
      line.clear();
      lastIsSpace = false;
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
