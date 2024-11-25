// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_MFA_TOTPQRCODE_H_
#define WT_AUTH_MFA_TOTPQRCODE_H_

#include "Wt/WQrCode.h"

#include "thirdparty/qrcodegen/QrCode.hpp"

namespace Wt {
  namespace Auth {
    namespace Mfa {
  /*! \class TotpQrCode Wt/Auth/Mfa/TotpQrCode.h
   *  \brief A QR code generator for TOTP secret keys.
   *
   * This class can be used to generate a QR code from a TOTP secret
   * key. The QR code will then be painted to the screen.
   *
   * This allows for some authenticator apps to more conveniently add
   * the TOTP secret key. This QR code embeds more than just the secret.
   *
   * \sa formatKey for a full list of what items are required.
   * \sa generateSecretKey()
   */
  class WT_API TotpQrCode : public WQrCode
  {
  public:

    /*! \brief Constructor
     *
     * This takes the arguments:
     *  - \p key: the secret TOTP key.
     *  - \p seviceName: the name of the application.
     *  - \p userName: the identifier of the client used to log in.
     *  - \p codeDigits: the length of the expected TOTP code accepted.
     */
    TotpQrCode(const std::string& key, const std::string& serviceName, const std::string& userName, int codeDigits);

    /*! \brief Format the key and other information to a correct QR code
     *
     * To generate a correct QR code, it needs to follow a specific
     * format. The rules of this format can be consulted on the site:
     * https://github.com/google/google-authenticator/wiki/Key-Uri-Format.
     *
     * This stipulates that a valid string must contain a label,
     * followed by some (optional) parameters.
     *
     * The label is formatted such that:
     * <code>serviceName:userName</code>.
     *
     * The used parameters are:
     *  - secret: the generated TOTP secret key
     *  - issuer: same as the serviceName
     *  - userName: the name of the user for whom the QR code is created
     *  - algorithm: always SHA1 (the default)
     *  - digits: the number of digits the generated code contains
     *  - period: the size of the time frame/window
     */
    virtual std::string formatKey(const std::string& key, const std::string& serviceName, const std::string& userName, int codeDigits) const;
  };
    }
  }
}
#endif // WT_AUTH_MFA_TOTPQRCODE_H_
