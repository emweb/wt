/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WPainter.h"

#ifdef WT_HAS_WPDFIMAGE
#include "Wt/WPdfImage.h"
#endif // WT_HAS_WPDFIMAGE

#include "ServerSideFontMetrics.h"

namespace Wt {

ServerSideFontMetrics::ServerSideFontMetrics()
{
#ifdef WT_HAS_WPDFIMAGE
  img_.reset(new WPdfImage(100, 100));
  painter_.reset(new WPainter(img_.get()));
#endif // WT_HAS_WPDFIMAGE
}

ServerSideFontMetrics::~ServerSideFontMetrics()
{ }

WFontMetrics ServerSideFontMetrics::fontMetrics(const WFont& font)
{
#ifdef WT_HAS_WPDFIMAGE
  painter_->setFont(font);
  return painter_->device()->fontMetrics();
#else
  throw WException("ServerSideFontMetrics not available");
#endif // WT_HAS_WPDFIMAGE
}

WTextItem
ServerSideFontMetrics::measureText(const WFont& font,
				   const WString& text, double maxWidth,
				   bool wordWrap)
{
#ifdef WT_HAS_WPDFIMAGE
  painter_->setFont(font);
  WTextItem t = painter_->device()->measureText(text, maxWidth, wordWrap);
  const double REL_ERROR = 1.02;
  return WTextItem(t.text(), t.width() * REL_ERROR,
		   t.nextWidth() > 0 ? t.nextWidth() * REL_ERROR : t.nextWidth());
#else
  throw WException("ServerSideFontMetrics not available");
#endif // WT_HAS_WPDFIMAGE
} 

bool ServerSideFontMetrics::available()
{
#ifdef WT_HAS_WPDFIMAGE
  return true;
#else
  return false;
#endif // WT_HAS_WPDFIMAGE
}

}
