#include "DateTimeRegexFormatter.h"

#include "Wt/WException.h"

#include <sstream>

namespace Wt {
  namespace Utils {
    void fatalFormatRegExpError(const WString& format, int c, const char* cs)
    {
      std::stringstream s;
      s << "Date/Time formatting to regex: (for \"" << format.toUTF8()
        << "\"): cannot handle " << c << " consecutive " << cs;
      throw WException(s.str());
    }

#ifndef WT_TARGET_JAVA
    bool hasAP(const std::string& format)
    {
      bool inQuote = false;
      for (auto i = format.begin(); i != format.end(); i++) {
        switch (*i) {
        case '\'':
          inQuote = !inQuote;
          break;

        case 'A':
        case 'a':
          if (!inQuote) {
            return true;
          }
          WT_FALLTHROUGH
        default:
          break;
        }
      }

      return false;
    }
#endif

  }
}
