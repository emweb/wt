// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef W_ABSTRACT_DATA_INFO_H_
#define W_ABSTRACT_DATA_INFO_H_

#include "Wt/WDllDefs.h"
#include <string>

namespace Wt {

/*! \class WAbstractDataInfo Wt/WAbstractDataInfo Wt/WAbstractDataInfo
 *  \brief An abstract base class storing information of a resource
 *
 *  This is an abstract class which is meant to store/compute
 *  information about a resource/file. Its primary use is to map URIs
 *  to file paths. This is to avoid confusion when rendering out these
 *  resources, so that depending on the context the resources is
 *  created under, locating the file correctly happens.
 */
class WT_API WAbstractDataInfo
{
public:
  /*! \brief Returns a path to a file containing the data.
   *
   * This returns a path to a file containing the data. This should
   * point to a path that exists on the system.
   *
   * By default this will throw an exception.
   *
   * \warning If you reimplement this function, you must also
   *          reimplement hasFilePath()
   *
   * \sa hasFilePath()
   */
  virtual std::string filePath() const;

  /*! \brief Returns the uri of the data.
   *
   * This returns the uri of the data.
   *
   * By default this will throw an exception.
   *
   * \warning If you reimplement this function, you must also
   *          reimplement hasUri()
   *
   * \sa hasUri()
   */
  virtual std::string uri() const;

  /*! \brief Returns the name of the Data.
   *
   * This returns the name of the data. This is mainly use for error
   * reporting.
   *
   * By default this will return uri() if hasUri() is \p true. In case
   * it is \p false, it will return filePath() if hasFilePath() is \p
   * true, and it will return an empty string otherwise.
   */
  virtual std::string name() const;

  /*! \brief Returns whether this contains a path to a file.
   *
   * This returns whether filePath returns a path to a file containing
   * the data.
   *
   * By default this returns \p false.
   *
   * \sa filePath()
   */
  virtual bool hasFilePath() const { return false; }

  /*! \brief Returns whether this contains a URI.
   *
   * This returns whether uri() returns a URI of the data.
   *
   * By default this returns \p false.
   *
   * \sa uri()
   */
  virtual bool hasUri() const { return false; }
};

}



#endif // W_ABSTRACT_DATA_INFO_H_