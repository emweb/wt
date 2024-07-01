# MFA examples
---

This example illustrates the use of the `Wt::Auth::Mfa` namespace.

It allows for another factor when logging in, essentially increasing security.
This avoids a single failure of a re-used, weak or compromised password. This presents an additional type of evidence for authentication.
See [OWASP MFA](https://cheatsheetseries.owasp.org/cheatsheets/Multifactor_Authentication_Cheat_Sheet.html) for a more detailed description on types, recommendations, and more.

#The examples are:
---

## TOTP
---

This is Wt's default implementation, offering a TOTP additional factor, compliant with the [RFC-6238](https://datatracker.ietf.org/doc/html/rfc6238) specification.

## PIN
---

A partial custom implementation, showing how the [AbstractMfaProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1AbstractMfaProcess.html) can be used.
This still uses the [Identity](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Identity.html) system of Wt.
It will ask for a simple PIN (like phones or Windows do).

**This is not a valid MFA approach.** This relies on two identical factors, which isn't counted as proper MFA. The example is illustrative.

## Phone

A complete custom implementation, showing how the [AbstractMfaProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1AbstractMfaProcess.html) can be used.
It does not rely on Wt's [Identity](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Identity.html) to keep track of the user's data. A custom database entry is used for that.
This requires some more methods to be customized, for integrating correctly with the framework.
It will show a QR code, to be scanned with your phone. That take a very minimal "footprint" of your phone, which can be reused.

**The footprint is minimal, and absolutely not secure enough.** It however shows how to customize MFA completely, so it is again mainly illustrative.
