// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFILE_RESOURCE_H_
#define WFILE_RESOURCE_H_

#include <Wt/WStreamResource.h>

namespace Wt {

/*! \class WFileResource Wt/WFileResource.h Wt/WFileResource.h
 *  \brief A resource which streams the data from a local file.
 *
 * To update the resource, either use setFileName() to point it to a
 * new file, or emit the WResource::dataChanged() signal when only the
 * file contents has changed, but not the filename.
 *
 * The resource makes use of continuations to transmit data piecewise,
 * without blocking a thread or requiring the entire file to be read
 * in memory. The size of the buffer can be changed using
 * setBufferSize().
 *
 * \if cpp
 * Usage examples:
 * \code
 * auto csvFile = std::make_unique<Wt::WFileResource>("text/csv", "/opt/files/afile.csv");
 * csvFile->suggestFileName("data.csv");
 * auto anchor = std::make_unique<Wt::WAnchor>(csvFile.get(), "CSV data");
 *
 * auto imageFile = std::make_unique<Wt::WFileResource>("image/png", "/opt/files/image.png");
 * imageFile->suggestFileName("data.png");
 * auto image = std::make_unique<Wt::WImage>(imageFile.get(), "PNG version");
 * \endcode
 * \endif
 *
 * \sa WStreamResource, WMemoryResource
 */
class WT_API WFileResource : public WStreamResource
{
public:
  /*! \brief Default constructor.
   *
   * You need to set a file name (and mime type) for the resource
   * using setFileName() and setMimeType().
   */
  WFileResource();

  /*! \brief Creates a new resource for a file.
   *
   * The mime type defaults to "text/plain".
   */
  WFileResource(const std::string& fileName);

  /*! \brief Creates a new resource with given mime-type for a file.
   */
  WFileResource(const std::string& mimeType, const std::string& fileName);

  /*! \brief Destructor.
   *
   * It is up to the user to make sure that the resource is no longer
   * in use (by e.g. a WImage).
   */
  ~WFileResource();

  /*! \brief Sets a (different) filename.
   *
   * Set the location of the file on the local filesystem which must be
   * streamed for this resource.
   */
  void setFileName(const std::string& fileName);

  /*! \brief Returns the filename.
   */
  const std::string& fileName() const { return fileName_; }

  /*! \brief Handles a request.
   *
   * You may want to specialize this function to compute the file on the fly.
   * However, you need to take into account the fact that the WFileResource
   * implementation may use continuations to split the download in smaller
   * chunks. Your implementation should thus look like:
   *
   * \if cpp
   * \code
   * void handleRequest(const Http::Request& request,
   *                    Http::Response& response) {
   *   if (!request.continuation()) {
   *     ... prepare data
   *     setFileName(myTmpFile);
   *   }
   *
   *   WFileResource::handleRequest(request, response);
   * }
   * \endcode
   * \endif
   */
  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response) override;

private:
  std::string fileName_;
};

}

#endif // WFILE_RESOURCE_H_
