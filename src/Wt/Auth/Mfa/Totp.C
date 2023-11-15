#include "Totp.h"

#include "Wt/WException.h"
#include <Wt/WRandom.h>
#include <Wt/Utils.h>

#include <cstring>
#include <cmath>
#ifdef WT_TARGET_JAVA
#include <ostream>
#endif // WT_TARGET_JAVA

namespace {
  constexpr int MINIMUM_SECRET_LENGTH = 16;
  constexpr int MAXIMUM_SECRET_LENGTH = 256;
  constexpr int PERIOD_IN_SECONDS = 30;
  constexpr int MINIMUM_CODE_LENGTH = 6;
  constexpr int MAXIMUM_CODE_LENGTH = 16;

  std::string toBigEndianHexString(uint64_t value)
  {
#ifndef WT_TARGET_JAVA
    // Check if little endian
    int x = 1;
    if (*(char*)(&x) == 1) {
      value = (value >> 56) |
          ((value<<40) & 0x00FF000000000000) |
          ((value<<24) & 0x0000FF0000000000) |
          ((value<<8) & 0x000000FF00000000) |
          ((value>>8) & 0x00000000FF000000) |
          ((value>>24) & 0x0000000000FF0000) |
          ((value>>40) & 0x000000000000FF00) |
          (value << 56);
    }

    std::string result(sizeof(uint64_t), '\0');
    std::memcpy(&result[0], &value, sizeof(uint64_t));
    return result;
#else
    return "";
#endif // WT_TARGET_JAVA
  }
}

namespace Wt {
  namespace Auth {
    namespace Mfa {
#ifndef WT_TARGET_JAVA
  std::string generateSecretKey(int length)
#else
  std::string Totp::generateSecretKey(int length)
#endif // WT_TARGET_JAVA
  {
    if (length < MINIMUM_SECRET_LENGTH || length > MAXIMUM_SECRET_LENGTH) {
      throw Wt::WException("Wt::Auth::Mfa::generateSecretKey: the key length should be between " + std::to_string(MINIMUM_SECRET_LENGTH) + " and " + std::to_string(MAXIMUM_SECRET_LENGTH));
    }

    // Random string [a-zA-Z0-9], of length.
    auto generatedId = WRandom::generateId(length);
    // Encoded to base32 [A-Z2-7], of increased length (factor of 1.6).
    auto encoded = Utils::base32Encode(generatedId);

    return encoded.substr(0, length);
  }

#ifndef WT_TARGET_JAVA
  std::string generateCode(const std::string& key, int codeDigits, std::chrono::seconds time, std::chrono::seconds startTime)
#else
  std::string Totp::generateCode(const std::string& key, int codeDigits, std::chrono::seconds time, std::chrono::seconds startTime)
#endif // WT_TARGET_JAVA
  {
    if (codeDigits > MAXIMUM_CODE_LENGTH) {
      throw Wt::WException("Wt::Auth::Mfa::generateCode: codeDigits cannot be greater than " + std::to_string(MAXIMUM_CODE_LENGTH));
    }

    if (codeDigits < MINIMUM_CODE_LENGTH) {
      throw Wt::WException("Wt::Auth::Mfa::generateCode: codeDigits cannot be lesser than " + std::to_string(MINIMUM_CODE_LENGTH));
    }

    // Define period of validity (30s)
    auto timeSteps = (time.count() - startTime.count()) / PERIOD_IN_SECONDS;
#ifndef WT_TARGET_JAVA
    auto timeHex = toBigEndianHexString(timeSteps);

    std::string hash = Wt::Utils::hmac_sha1(timeHex, Wt::Utils::base32Decode(key));
#else
    std::string hash;
    try {
      hash = Wt::Utils::hmac_sha1(timeSteps, Wt::Utils::base32Decode(key));
    } catch (std::ios_base::failure e) {
      // This is an empty catch that will never occur in C++ code.
      // This has been placed here for the Java (JWt) translation.
    }
#endif // WT_TARGET_JAVA

    int offset = hash[hash.length() - 1] & 0xf;
    int binary =
         ((hash[offset] & 0x7f) << 24) |
         ((hash[offset + 1] & 0xff) << 16) |
         ((hash[offset + 2] & 0xff) << 8) |
         (hash[offset + 3] & 0xff);

    // Set code length
    int otp = binary % static_cast<int>(std::pow(10, codeDigits));

    // Pad code
    auto result = std::to_string(otp);
    while (result.length() < static_cast<size_t>(codeDigits)) {
        result = "0" + result;
    }

    return result;
  }

#ifndef WT_TARGET_JAVA
  bool validateCode(const std::string& key, const std::string& code, int codeDigits, std::chrono::seconds time, std::chrono::seconds startTime)
#else
  bool Totp::validateCode(const std::string& key, const std::string& code, int codeDigits, std::chrono::seconds time, std::chrono::seconds startTime)
#endif // WT_TARGET_JAVA
  {
    if (code.length() != static_cast<std::size_t>(codeDigits)) {
      return false;
    }

    return generateCode(key, codeDigits, time, startTime) == code
           || generateCode(key, codeDigits, time - std::chrono::seconds(PERIOD_IN_SECONDS), startTime) == code;
  }
    }
  }
}
