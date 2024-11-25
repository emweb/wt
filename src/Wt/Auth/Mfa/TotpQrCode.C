#include "TotpQrCode.h"

#include "Wt/WBrush.h"
#include "Wt/WPaintedWidget.h"
#include "Wt/WPainter.h"

namespace Wt {
  namespace Auth {
    namespace Mfa {
  TotpQrCode::TotpQrCode(const std::string& key, const std::string& serviceName, const std::string& userName, int codeDigits)
    : WQrCode()
  {
    setMessage(formatKey(key, serviceName, userName, codeDigits));
    setErrorCorrectionLevel(WQrCode::ErrorCorrectionLevel::LOW);
  }

  std::string TotpQrCode::formatKey(const std::string& key, const std::string& serviceName, const std::string& userName, int codeDigits) const
  {
    std::string path = Wt::WString("otpauth://totp/{1}:{2}?secret={3}&issuer={1}&algorithm=SHA1&digits={4}&period=30")
                       .arg(serviceName)
                       .arg(userName)
                       .arg(key)
                       .arg(codeDigits)
                       .toUTF8();
    return path;
  }
    }
  }
}

