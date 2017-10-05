// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WSTRING_STREAM_H_
#define WT_WSTRING_STREAM_H_

#include <Wt/WDllDefs.h>
#include <iostream>
#include <string>
#include <vector>

#ifdef WT_ASIO_IS_BOOST_ASIO
namespace boost {
#endif
namespace asio {
  class const_buffer;
}
#ifdef WT_ASIO_IS_BOOST_ASIO
}
#endif

namespace Wt {

/*! \class WStringStream Wt/WStringStream.h Wt/WStringStream.h
 *
 * This is an efficient std::stringstream replacement. It is in
 * particular more efficient when a relatively short string is being
 * composed from many different pieces (avoiding any memory allocation
 * all-together).
 *
 * Compared to std::stringstream, it also avoids overhead by not
 * supporting the formatting options of the latter, and by not making
 * use of the std::locale, which apparently hampers std::ostream
 * performance (%Wt internally uses UTF-8 encoding throughout).
 */
class WT_API WStringStream
{
public:
  /*! \brief An implementation of an output generator for appending data.
   *
   * \sa back_inserter()
   */
  struct iterator {
    struct char_proxy {
      char_proxy& operator= (char c);

    private:
      char_proxy(WStringStream& stream);
      WStringStream& stream_;

      friend struct iterator;
    };

    iterator();

    char_proxy operator * ();

    iterator& operator ++ ();
    iterator  operator ++ (int);

  private:
    WStringStream *stream_;
    iterator(WStringStream& stream);

    friend class WStringStream;
  };

  /*! \brief Default constructor.
   *
   * Creates a string stream.
   */
  WStringStream();

  /*! \brief Assignment operator.
   */
  WStringStream& operator=(const WStringStream& other);

  /*! \brief Constructor with std::ostream sink.
   *
   * Creates a string stream which flushes contents to an
   * std::ostream, instead of relying on internal buffering. The
   * output may still be internally buffered (for performance
   * reasons), and this buffer is only flushed to the underlying ostream
   * when you delete the string stream.
   */
  WStringStream(std::ostream& sink);

  /*! \brief Destructor.
   */
  ~WStringStream();

  /*! \brief Appends a string.
   *
   * Appends \p length bytes from the given string.
   */
  void append(const char *s, int length);

#ifndef WT_TARGET_JAVA
  /*
   * Should not be implemented but is needed to support the specialization
   * for string literals !
   */
  template <typename T>
    inline WStringStream& operator<< (T t) {
    WStringStream please_cast_to_a_supported_type = t;
    please_cast_to_a_supported_type << 'a'; // silence compiler for normal case
    return *this;
  }

  template <std::size_t N>
    WStringStream& operator<< (const char (&s)[N]) {
    append(s, N-1); return *this; 
  }
#endif // WT_TARGET_JAVA

  /*! \brief Appends a character.
   */
  WStringStream& operator<< (char);

  /*! \brief Appends a C string.
   */
  WStringStream& operator<< (char *s);

  /*! \brief Appends a C++ string.
   */
  WStringStream& operator<< (const std::string& s);

  /*! \brief Appends a boolean.
   *
   * This is written to the stream as <tt>true</tt> or <tt>false</tt>.
   */
  WStringStream& operator<< (bool);

  /*! \brief Appends an integer number.
   */
  WStringStream& operator<< (int);

  /*! \brief Appends an integer number.
   */
  WStringStream& operator<< (long long);

  /*! \brief Appends a double.
   */
  WStringStream& operator<< (double);

  /*! \brief Iterator for appending.
   */
  iterator back_inserter();

  /*! \brief Returns the contents as a null-terminated C string.
   *
   * The behaviour is only defined for a string stream with internal
   * buffering.
   *
   * \note This is only supported when the length of the total string
   *       is less than 1024 bytes. Returns 0 if the operation could not
   *       be completed.
   */
  const char *c_str();

  /*! \brief Returns the contents as a C++ string.
   *
   * The behaviour is only defined for a string stream with internal
   * buffering.
   */
  std::string str() const;

#ifdef WT_ASIO_IS_BOOST_ASIO
  void asioBuffers(std::vector<boost::asio::const_buffer>& result) const;
#else
  void asioBuffers(std::vector<asio::const_buffer>& result) const;
#endif

  /*! \brief Returns whether the contents is empty.
   *
   * The behaviour is only defined for a string stream with internal
   * buffering.
   */
  bool empty() const;

  /*! \brief Returns the total length.
   *
   * The behaviour is only defined for a string stream with internal
   * buffering.
   */
  std::size_t length() const;

  /*! \brief Clears the contents.
   *
   * The behaviour is only defined for a string stream with internal
   * buffering.
   */
  void clear();

  // no-op for C++, but needed for Java
  void spool(std::ostream& ) { }

private:
  WStringStream(const WStringStream& other);

  enum {S_LEN = 1024};
  enum {D_LEN = 2048};

  std::ostream *sink_;

  char static_buf_[S_LEN + 1];

  char *buf_;
  int buf_i_;

  int buf_len() const 
    { return buf_ == static_buf_ ? static_cast<int>(S_LEN)
	: static_cast<int>(D_LEN); }

  std::vector<std::pair<char *, int> > bufs_;

  void flushSink();
  void pushBuf();
};

}

#endif // WT_WSTRING_STREAM_H_
