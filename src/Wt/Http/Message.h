// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_MESSAGE_H_
#define WT_HTTP_MESSAGE_H_

#include <Wt/WStringStream.h>

namespace Wt {
  namespace Http {

/*! \class Message Wt/Http/Message.h Wt/Http/Message.h
 *  \brief An HTTP client message (request or response).
 *
 * This class implements a message that is sent or received by the
 * HTTP Client.
 *
 * It is not to be confused with Request and Response, which are
 * involved in the web application server handling.
 *
 * \ingroup http
 */
class WT_API Message
{
public:
  /*! \class Header
   *  \brief An HTTP message header.
   * 
   * An HTTP message header is a name/value pair, as defined by RFC 822.
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

  /*! \brief Constructor.
   *
   * This creates an empty message, with an invalid status (-1), no headers
   * and an empty body.
   */
  Message();

  /*! \brief Constructor.
   *
   * This creates an empty message, with an invalid status (-1), an empty body
   * and the given headers.
   */
  Message(std::vector<Header> headers);

  /*! \brief Copy constructor.
   */
  Message(const Message& message);

  /*! \brief Sets the status code.
   *
   * \note This method is probably not useful to you, since for a request
   *       it is ignored, and for a response it is set by the client.
   */
  void setStatus(int status);

  /*! \brief Returns the status code.
   *
   * This returns the HTTP status code of a response message. Typical values
   * are 200 (OK) or 404 (Not found).
   */
  int status() const { return status_; }

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
   * was present. This is allowed by HTTP only for certain headers
   * (e.g. Set-Cookie).
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

  /*! \brief Concatenates body text.
   *
   * Adds the \p text to the message body.
   */
  void addBodyText(const std::string& text);

  /*! \brief Returns the body text.
   *
   * Returns the body text.
   */
  std::string body() const;

private:
  int status_;
  std::vector<Header> headers_;
  WStringStream body_;

  friend class Client;
};

  }
}

#endif // WT_HTTP_MESSAGE_H_
