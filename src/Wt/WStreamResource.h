// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTREAM_RESOURCE_H_
#define WSTREAM_RESOURCE_H_

#include <Wt/WResource.h>

#include <string>

namespace Wt {

/*! \class WStreamResource Wt/WStreamResource.h Wt/WStreamResource.h
 *  \brief An object which streams the data from a std::istream.
 *
 * This class can be useful base for implementing resources which streams
 * the data from std::istream derivatives.
 *
 * The utility method handleRequestPiecewise() makes use of continuations
 * to transmit data piecewise, without blocking a thread or requiring the
 * whole data to be read in memory. The size of the buffer can be changed
 * by using setBufferSize().
 *
 * \if cpp
 * Example for a custom stream resource implementation:
 * \code
class MyStreamResource : public Wt::WStreamResource
{
public:
  MyStreamResource(const std::string& fileName)
    : Wt::WStreamResource(),
      fileName_(fileName)
  {
    suggestFileName("data.txt");
  }

  ~MyStreamResource() {
    beingDeleted();
  }

  void handleRequest(const Wt::Http::Request& request,
                     Wt::Http::Response& response) {
    std::ifstream r(fileName_.c_str(), std::ios::in | std::ios::binary);
    handleRequestPiecewise(request, response, r);
  }

private:
  std::string fileName_;
};
 * \endcode
 * \endif
 *
 * \sa WFileResource
 */
class WT_API WStreamResource : public WResource
{
public:
  /*! \brief Default constructor.
   *
   * The mime type defaults to "text/plain".
   */
  WStreamResource();

  /*! \brief Creates a new resource with given mime-type.
   */
  WStreamResource(const std::string& mimeType);

  /*! \brief Destructor.
   *
   * It is up to the user to make sure that the resource is no longer
   * in use (by e.g. a WImage).
   */
  ~WStreamResource();

  /*! \brief Sets the mime-type.
   */
  void setMimeType(const std::string& mimeType);

  /*! \brief Returns the mime-type.
   */
  const std::string& mimeType() const { return mimeType_; }

  /*! \brief Configures the buffer size.
   *
   * This configures the size of the buffer used to transmit the data
   * piece by piece.
   */
  void setBufferSize(int size);

  /*! \brief Returns the buffer size.
   *
   * \sa setBufferSize()
   */
  int bufferSize() const { return bufferSize_; }

protected:
  /*! \brief Handles a request and streams the data from a std::istream.
   *
   * You can call this method from a custom handleRequest() implementations.
   */
  void handleRequestPiecewise(const Http::Request& request,
                              Http::Response& response, std::istream& input);

private:
  std::string mimeType_;
  int bufferSize_;
  std::streamsize beyondLastByte_;
};

}

#endif // WSTREAM_RESOURCE_H_
