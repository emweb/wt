#pragma once

#include <Wt/WDllDefs.h>

#include <chrono>
#include <string>

namespace Wt {
  namespace Auth {
    /*! \brief Namespace for Multi-Factor Authentication
     */
    namespace Mfa {
#ifdef WT_TARGET_JAVA
    /*! \class Totp Wt/Auth/Mfa/Totp.h
     *  \brief Utility class containing functions for TOTP functionality.
     */
    class Totp {
    private:
      Totp() { }
    public:
#endif // WT_TARGET_JAVA
      /*! \brief Generate a secret key, for Multi-Factor Authentication.
       *
       * This will generate a base32-encoded string, of \p length. This
       * will only contain characters from [A-Z2-7]. The generated
       * string is created securely, and sufficiently random for
       * cryptographic purposes.
       *
       * This string returned by this function can be used for a user as
       * their shared secret to generate and verify
       * [TOTP](https://datatracker.ietf.org/doc/html/rfc6238) codes.
       *
       * Secret keys with length between 16 and 256 are allowed. By default
       * the length will be 32.
       *
       * \throw WException if the \p length specified isn't within the
       * range [16, 256].
       *
       * \sa WRandom
       */
#ifndef WT_TARGET_JAVA
      WT_NODISCARD WT_API extern std::string generateSecretKey(int length = 32);
#else
      WT_NODISCARD WT_API static std::string generateSecretKey(int length = 32);
#endif // WT_TARGET_JAVA

      /*! \brief Generates a TOTP (Time-Based One-Time Password) code
       *
       * This code is generated from a secret \p key, at the specified
       * \p time. The code will be of length \p codeDigits.
       *
       * The \p key should be a base32-encoded string, with a length
       * between 16 and 256. The \p codeDigits parameter should be at
       * least 6 characters, and at most be 16 characters long. Supplying
       * a \p codeDigits outside of this boundary will result in an
       * exception being thrown.
       *
       * The specified time will be the time the code is generated. This
       * ensures that the TOTP algorithm generates a different code for
       * each time window, where the width of a window is 30 seconds.
       *
       * The \p startTime is optional and is used to define an offset.
       * This offset will be subtracted from the actual \p time. It can
       * be used to define a starting point.
       *
       * \throw WException if the \p codeDigits specified isn't within the
       * range [6, 16].
       */
#ifndef WT_TARGET_JAVA
      WT_NODISCARD WT_API extern std::string generateCode(const std::string& key, int codeDigits, std::chrono::seconds time,
                                                          std::chrono::seconds startTime = std::chrono::seconds(0));
#else
      WT_NODISCARD WT_API static std::string generateCode(const std::string& key, int codeDigits, std::chrono::seconds time,
                                                          std::chrono::seconds startTime = std::chrono::seconds(0));
#endif // WT_TARGET_JAVA

      /*! \brief Validate the given \p code with the given time frame
       *
       * Here the \p key is the secret key attached to the User, the
       * \p code is the TOTP code the user has entered, which is expected
       * to be of length \p codeDigits. This length is configured in
       * AuthService::setMfaCodeLength().
       *
       * The \p time specifies the time window for which the code is valid.
       * When this function executes, the code will be generated for the
       * time frame the passed \p time falls in, and in the previous
       * window. Each window has a width of 30 seconds. Meaning that at
       * most a user has 1 minute to enter the code (if they submit it
       * immediately at the start of the first time frame). Or at least 30
       * seconds (if they submit it at the end of the first time frame).
       *
       * Time frames start either immediately on the minute, or halfway.
       * This means that for the times:
       *  - 12:52:12, the start time frame will be 12:52:00
       *  - 12:52:48, the start time frame will be 12:52:30
       *
       * The \p startTime is optional and is used to define an offset.
       * This offset will be subtracted from the actual \p time. It can
       * be used to define a starting point.
       */
#ifndef WT_TARGET_JAVA
      WT_NODISCARD WT_API extern bool validateCode(const std::string& key, const std::string& code, int codeDigits, std::chrono::seconds time,
                                                   std::chrono::seconds startTime = std::chrono::seconds(0));
#else
      WT_NODISCARD WT_API static bool validateCode(const std::string& key, const std::string& code, int codeDigits, std::chrono::seconds time,
                                                   std::chrono::seconds startTime = std::chrono::seconds(0));
    };
#endif // WT_TARGET_JAVA
    }
  }
}

