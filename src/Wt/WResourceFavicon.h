// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRESOURCE_FAVICON_H_
#define WRESOURCE_FAVICON_H_

#include "Wt/WFavicon.h"
#include "Wt/WResource.h"

namespace Wt {

/*! \class WResourceFavicon Wt/WResourceFavicon.h Wt/WResourceFavicon.h
 *  \brief A WFavicon using a resource.
 *
 * This is a WFavicon that uses a resource for its favicon.
 *
 * \sa WApplication::setFavicon()
 */
class WT_API WResourceFavicon : public WFavicon
{
public:
  /*! \brief Constructs a WResourceFavicon.
   *
   * Constructs a WResourceFavicon from a WResource.
   */
  WResourceFavicon(const std::shared_ptr<WResource>& resource);

  /*! \brief Constructs a WResourceFavicon from a file.
   *
   * This is a constructor that automatically creates a
   * WResourceFavicon of WFileResource using the given filename
   * and MIME type.
   *
   * By default, the MIME type is application/octet-stream, which
   * lets the browser guess the MIME type.
   */
  WResourceFavicon(const std::string& filename,
                   const std::string& mimeType = "application/octet-stream");

  //! Sets the default favicon.
  void setResource(const std::shared_ptr<WResource>& resource);

  /*! \brief Returns the resource.
   *
   * \sa setResource()
   */
  WResource* resource() const { return resource_.get(); }

  std::string url() const override;

private:
  std::shared_ptr<WResource> resource_;
};

}
#endif //WRESOURCE_FAVICON_H_