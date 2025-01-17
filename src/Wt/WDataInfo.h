// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATA_INFO_H_
#define WDATA_INFO_H_

#include "Wt/WAbstractDataInfo.h"

namespace Wt {

/*! \class WDataInfo Wt/WDataInfo Wt/WDataInfo
 *  \brief A class that stores informations about data
 *
 *  This is a barebone version of WAbstractDataInfo. It simply stores
 *  the information given to it.
 *
 *  \sa WDocRootDataInfo
 */
class WT_API WDataInfo : public WAbstractDataInfo
{
public:
  /*! \brief Creates a WDataInfo.
   *
   * Creates a WDataInfo with the given \p uri and \p filePath.
   */
  WDataInfo(const std::string& uri, const std::string& filePath);

  //! Sets the file path.
  void setFilePath(const std::string& filePath);

  /*! \brief Returns a path to a file containing the data.
   *
   * Throws if the file path is set to an empty string.
   *
   * \sa hasFilePath()
   */
  std::string filePath() const override;

  //! Sets the URI.
  void setUri(const std::string& uri);

  /*! \brief Returns the URI of the data.
   *
   * Throws if the URI is set to an empty string.
   *
   * \sa hasUri()
   */
  std::string uri() const override;

  /*! \brief Returns whether this contains a file path.
   *
   * This returns whether filePath() returns a non-empty string.
   *
   * \sa filePath()
   */
  bool hasFilePath() const override { return !filePath_.empty(); }

  /*! \brief Returns whether this contains a uri.
   *
   * This returns whether uri() returns a non-empty string.
   *
   * \sa uri()
   */
  bool hasUri() const override { return !uri_.empty(); }

private:
  std::string uri_, filePath_;
};

}



#endif // WDATA_INFO_H_