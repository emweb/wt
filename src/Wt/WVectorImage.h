// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVECTOR_IMAGE_H_
#define WVECTOR_IMAGE_H_

#include <Wt/WPaintDevice.h>

namespace Wt {

/*! \class WVectorImage Wt/WVectorImage.h Wt/WVectorImage.h
 *  \brief An abstract paint device for rendering using native vector graphics.
 *
 * \ingroup painting
 */
class WT_API WVectorImage : public WPaintDevice
{
public:
  ~WVectorImage();

  /*! \brief Internal method.
   */
  virtual std::string rendered() = 0;
};

}

#endif // WVECTOR_IMAGE_H_
