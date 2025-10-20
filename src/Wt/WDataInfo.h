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
  //! Creates an empty WDataInfo.
  WDataInfo();

  /*! \brief Creates a WDataInfo.
   *
   * Creates a WDataInfo with the given \p url and \p filePath.
   */
  WDataInfo(const std::string& url, const std::string& filePath);

  //! Sets the file path.
  void setFilePath(const std::string& filePath);

  /*! \brief Returns a path to a file containing the data.
   *
   * Throws if the file path is set to an empty string.
   *
   * \sa hasFilePath()
   */
  std::string filePath() const override;

  //! Sets the URL.
  void setUrl(const std::string& url);

  /*! \brief Returns the URL of the data.
   *
   * Throws if the URL is set to an empty string.
   *
   * \sa hasUrl()
   */
  std::string url() const override;


  //! Sets the data formated as data URI.
  void setDataUri(const std::string& dataUri);

  /*! \brief Returns the data in data URI format.
   *
   * Throws if the data URI is set to an empty string.
   *
   * \sa hasDataUri()
   */
  std::string dataUri() const override;

  /*! \brief Returns whether this contains a file path.
   *
   * This returns whether filePath() returns a non-empty string.
   *
   * \sa filePath()
   */
  bool hasFilePath() const override { return !filePath_.empty(); }

  /*! \brief Returns whether this contains a url.
   *
   * This returns whether url() returns a non-empty string.
   *
   * \sa url()
   */
  bool hasUrl() const override { return !url_.empty(); }

  /*! \brief Returns whether this can return the data in data URI format.
   *
   * This returns whether dataUri() returns a non-empty string.
   *
   * \sa dataUri()
   */
  bool hasDataUri() const override { return !dataUri_.empty(); }

private:
  std::string url_, filePath_, dataUri_;
};

}



#endif // WDATA_INFO_H_