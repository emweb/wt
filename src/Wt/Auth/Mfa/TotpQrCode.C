#include "TotpQrCode.h"

#include "Wt/WBrush.h"
#include "Wt/WPaintedWidget.h"
#include "Wt/WPainter.h"

namespace Wt {
  namespace Auth {
    namespace Mfa {
  TotpQrCode::TotpQrCode(const std::string& key, const std::string& serviceName, const std::string& userName, int codeDigits)
    : code_(qrcodegen::QrCode::encodeText(formatKey(key, serviceName, userName, codeDigits).c_str(), ErrorLevelCorrection))
  {
    auto size = code().getSize() * SQUARE_SIZE;
    resize(size, size);
  }

  void TotpQrCode::paintEvent(WPaintDevice* paintDevice)
  {
    WPainter painter(paintDevice);
    auto brush = WBrush(StandardColor::Black);

    for (auto line = 0; line < code().getSize(); ++line) {
      for (auto column = 0; column < code().getSize(); ++column) {
        if (code().getModule(column, line)) {
          painter.fillRect(line * SQUARE_SIZE, column * SQUARE_SIZE,
                           SQUARE_SIZE, SQUARE_SIZE,
                           brush);
        }
      }
    }
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

