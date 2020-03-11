#include "Logger.h"

namespace {

const Wt::WLogSink *customLogger_ = nullptr;

}

namespace Wt {
namespace Dbo {

void setCustomLogger(const WLogSink &customLogger)
{
  customLogger_ = &customLogger;
}

namespace Utils {

// defined in EscapeOStream.C
extern std::string& replace(std::string& s, char c, const std::string& r);

}

} // Dbo
} // Wt

#define WT_DBO_LOGGER
#include "../WLogger.C"
#undef WT_DBO_LOGGER
