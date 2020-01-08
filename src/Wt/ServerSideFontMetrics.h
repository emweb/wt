// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SERVER_SIDE_FONT_METRICS_H_
#define SERVER_SIDE_FONT_METRICS_H_

#include <Wt/WGlobal.h>

namespace Wt {

/*
 * A private utility class that provides server-side font metrics.
 */
class ServerSideFontMetrics
{
public:
  ServerSideFontMetrics();
  ~ServerSideFontMetrics();

  WFontMetrics fontMetrics(const WFont& font);

  WTextItem measureText(const WFont& font,
			const WString& text, double maxWidth, bool wordWrap);

  static bool available();

private:
#ifdef WT_HAS_WPDFIMAGE
  std::unique_ptr<WPdfImage> img_;
  std::unique_ptr<WPainter> painter_;
#endif
};

}

#endif // SERVER_SIDE_FONT_METRICS_H_
