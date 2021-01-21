// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMEMORY_RESOURCE_H_
#define WMEMORY_RESOURCE_H_

#include <mutex>
#include <string>
#include <thread>

#include <Wt/WResource.h>

namespace Wt {

/*! \class WMemoryResource Wt/WMemoryResource.h Wt/WMemoryResource.h
 *  \brief A resource which streams data from memory
 *
 * Use this resource if you want to serve resource data from memory. This
 * is suitable for relatively small resources, which still require some
 * computation.
 *
 * If creating the data requires computation which you would like to
 * post-pone until the resource is served, then you may want to
 * directly reimplement WResource instead and compute the data on the
 * fly while streaming.
 *
 * Usage examples:
 * \code
 * auto imageResource = std::make_shared<Wt::WMemoryResource>("image/gif");
 *
 * static const unsigned char gifData[]
 *    = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00,
 *        0x80, 0x00, 0x00, 0xdb, 0xdf, 0xef, 0x00, 0x00, 0x00, 0x21,
 *        0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
 *        0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
 *        0x01, 0x00, 0x3b };
 *
 * imageResource->setData(gifData, 43);
 * auto image = std::make_unique<Wt::WImage>(Wt::WLink(imageResource), "1 transparent pixel");
 * \endcode
 *
 * \sa WFileResource.
 */
class WT_API WMemoryResource : public WResource
{
public:
  /*! \brief Creates a new resource.
   *
   * You must call setMimeType() and setData() before using the resource.
   */
  WMemoryResource();

  /*! \brief Creates a new resource with given mime-type.
   *
   * You must call setData() before using the resource.
   */
  WMemoryResource(const std::string& mimeType);

  /*! \brief Creates a new resource with given mime-type and data
   */
  WMemoryResource(const std::string& mimeType,
		  const std::vector<unsigned char>& data);

  ~WMemoryResource();

  /*! \brief Sets new data for the resource to serve.
   */
  void setData(const std::vector<unsigned char> &data);

  /*! \brief Sets new data for the resource to serve.
   *
   * Sets the data from using the first \p count bytes from the
   * C-style \p data array.
   */
  void setData(const unsigned char *data, int count);

  /*! \brief Returns the data this resource will serve.
   */
  const std::vector<unsigned char> data() const;

  /*! \brief Returns the mime-type.
   */
  const std::string mimeType() const { return mimeType_; }

  /*! \brief Sets the mime-type.
   */
  void setMimeType(const std::string& mimeType);

  virtual void handleRequest(const Http::Request& request,
                             Http::Response& response) override;

private:
  typedef std::shared_ptr< const std::vector<unsigned char> > DataPtr;

  std::string mimeType_;
  DataPtr data_;

  std::shared_ptr<std::mutex> dataMutex_;

  void create();
};

}

#endif // WMEMORY_RESOURCE_H_
