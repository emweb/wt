// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDOC_ROOT_DATA_INFO_H_
#define WDOC_ROOT_DATA_INFO_H_

#include "Wt/WAbstractDataInfo.h"

namespace Wt {

/*! \class WDocRootDataInfo Wt/WDocRootDataInfo Wt/WDocRootDataInfo
 *  \brief A class that stores information of a file in the docroot
 *
 *  This class stores the uri and the file path of a file inside of the
 *  document root.
 *
 *  \sa WDocRootDataInfo
 */
class WT_API WDocRootDataInfo : public WAbstractDataInfo
{
public:
  /*! \brief Creates a WDocRootDataInfo.
   *
   * Creates a WDocRootDataInfo with the given \p path. The path must be relative
   * to the WApplication's \p docroot.
   */
  WDocRootDataInfo(const std::string& path);

  //! Set the path, which should be relative to the WApplication's \p docroot.
  void setRelativePath(const std::string& path);

  /*! \brief Returns a path to a file containing the data.
   *
   * Throws if the \p path provided to the constructor or set with
   * setRelativePath() was an empty string.
   *
   * \sa hasFilePath()
   */
  std::string filePath() const override;

  /*! \brief Returns the URI of the data.
   *
   * Throws if the \p path provided to the constructor or set with
   * setRelativePath() was an empty string.
   *
   * \sa hasUri()
   */
  std::string uri() const override;

  /*! \brief Returns whether this contains a file path.
   *
   * This returns whether the \p path provided to the constructor
   * or set with setRelativePath() was a non-empty string.
   *
   * \sa filePath()
   */
  bool hasFilePath() const override { return !relPath_.empty(); }

  /*! \brief Returns whether this contains a uri.
   *
   * This returns whether the \p path provided to the constructor
   * or set with setRelativePath() was a non-empty string.
   *
   * \sa uri()
   */
  bool hasUri() const override { return !relPath_.empty(); }

private:
  std::string relPath_;
};

}



#endif // WDOC_ROOT_DATA_INFO_H_