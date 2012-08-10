/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PdfUtils.h"

#include <string>

namespace Wt {
  namespace Pdf {
#ifdef WT_TARGET_JAVA	
    class PdfUtils {
    };
#endif //WT_TARGET_JAVA

    std::string toBase14Font(const WFont& font)
    {
      const char *base = 0;
      const char *italic = 0;
      const char *bold = 0;

      switch (font.genericFamily()) {
      case WFont::Default:
      case WFont::Serif:
      case WFont::Fantasy: // Not really !
      case WFont::Cursive: // Not really !
	base = "Times";
	italic = "Italic";
	bold = "Bold";
	break;
      case WFont::SansSerif:
	base = "Helvetica";
	italic = "Oblique";
	bold = "Bold";
	break;
      case WFont::Monospace:
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
	case WFont::NormalStyle:
	  italic = 0;
	  break;
	default:
	  break;
	}

      if (font.weightValue() <= 400)
	bold = 0;

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
