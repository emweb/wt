// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WURL_FAVICON_H_
#define WURL_FAVICON_H_

#include "Wt/WFavicon.h"

namespace Wt {

/*! \class WUrlFavicon Wt/WUrlFavicon.h Wt/WUrlFavicon.h
 *  \brief A WFavicon located at a given URL.
 *
 * \sa WApplication::setFavicon()
 */
class WT_API WUrlFavicon : public WFavicon
{
public:
  /*! \brief Constructs a WUrlFavicon.
   *
   * Constructs a WUrlFavicon from the given \p url.
   */
  WUrlFavicon(const std::string& url);

  //! \brief Sets the URL.
  void setUrl(const std::string& url);

  std::string url() const override { return url_; };

private:
  std::string url_;
};

}
#endif //WURL_FAVICON_H_