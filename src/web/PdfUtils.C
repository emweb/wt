/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PdfUtils.h"

#include <string>

namespace Wt {
  namespace Pdf {
#ifdef WT_TARGET_JAVA	
    class PdfUtils {
    private:
      PdfUtils() { }
    };
#endif //WT_TARGET_JAVA

    std::string toBase14Font(const WFont& font)
    {
      const char *base = nullptr;
      const char *italic = nullptr;
      const char *bold = nullptr;

      switch (font.genericFamily()) {
      case FontFamily::Default:
      case FontFamily::Serif:
      case FontFamily::Fantasy: // Not really !
      case FontFamily::Cursive: // Not really !
	base = "Times";
	italic = "Italic";
	bold = "Bold";
	break;
      case FontFamily::SansSerif:
	base = "Helvetica";
	italic = "Oblique";
	bold = "Bold";
	break;
      case FontFamily::Monospace:
	base = "Courier";
	italic = "Oblique";
	bold = "Bold";
	break;
      }

      if (font.specificFamilies() == "Symbol")
	base = "Symbol";
      else if (font.specificFamilies() == "ZapfDingbats")
	base = "ZapfDingbats";

      if (italic)
	switch (font.style()) {
	case FontStyle::Normal:
	  italic = nullptr;
	  break;
	default:
	  break;
	}

      if (font.weightValue() <= 400)
	bold = nullptr;

      std::string name = base;
      if (bold) {
	name += std::string("-") + bold;
	if (italic)
	  name += italic;
      } else if (italic)
	name += std::string("-") + italic;

      if (name == "Times")
	name = "Times-Roman";

      return name;
    }
  }
}
