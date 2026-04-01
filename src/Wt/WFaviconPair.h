// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFAVICON_PAIR_H_
#define WFAVICON_PAIR_H_

#include "Wt/WFavicon.h"
#include "Wt/WResource.h"

namespace Wt {

/*! \class WFaviconPair Wt/WFaviconPair.h Wt/WFaviconPair.h
 *  \brief A WFavicon switching between two favicons.
 *
 * This is a WFavicon that uses a different WFavicon for its
 * default and updated state.
 *
 * \sa WApplication::setFavicon()
 */
class WT_API WFaviconPair : public WFavicon
{
public:
  /*! \brief Constructs a WFaviconPair.
   *
   * Constructs a WFaviconPair from two WFavicon.
   */
  WFaviconPair(std::unique_ptr<WFavicon> defaultFavicon,
               std::unique_ptr<WFavicon> updatedFavicon);

  /*! \brief Sets the default favicon.
   */
  void setDefaultFavicon(std::unique_ptr<WFavicon> defaultFavicon);

  /*! \brief Returns the default favicon.
   *
   * \sa setDefaultFavicon()
   */
  WFavicon *defaultFavicon() const { return defaultFavicon_.get(); }

  /*! \brief Sets the updated favicon.
   */
  void setUpdatedFavicon(std::unique_ptr<WFavicon> updatedFavicon);

  /*! \brief Returns the updated favicon.
   *
   * \sa setUpdatedFavicon()
   */
  WFavicon* updatedFavicon() const { return updatedFavicon_.get(); }

  /*! \brief Returns the current favicon used.
   *
   * \sa setUpdatedFavicon(), setDefaultFavicon()
   */
  WFavicon* currentFavicon() const { return isUpdated() ? updatedFavicon() : defaultFavicon(); }

  std::string url() const override;

private:
  std::unique_ptr<WFavicon> defaultFavicon_, updatedFavicon_;
};

}
#endif //WFAVICON_PAIR_H_