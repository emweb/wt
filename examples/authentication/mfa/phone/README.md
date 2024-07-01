# Phone example
---

This example illustrates the use of the `Wt::Auth::Mfa` namespace.

It allows for another factor when logging in, essentially increasing security.
This avoids a single failure of a re-used, weak or compromised password. This presents an additional type of evidence for authentication.
See [OWASP MFA](https://cheatsheetseries.owasp.org/cheatsheets/Multifactor_Authentication_Cheat_Sheet.html) for a more detailed description on types, recommendations, and more.

This example will require the user to have a smartphone that can scan QR codes. The phone should be able to reach the device on which the example is hosted.
The QR code will redirect the phone's browser to a certain URL. Here a minimal "fingerprint" of the device is taken, essentially remembering the access method.
Later, this same device can be used to pass the MFA step. This requires the same browser to be used.

**The fingerprint is very minimal, and DOES NOT constitute a safe and valid MFA approach.** This example is illustrative.

Two default users of are provided to easily start using the example. The username:password combo's are:

- admin:admin
- user:user

Only the former user (admin) is set up to require the MFA step.

# How to run
---

See the README in the parent directory.
Important here is that the `resources-dir` is provided. Otherwise an error will be shown.


# What it illustrates
---

- how to create a custom [AbstractMfaProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1AbstractMfaProcess.html) implementation.
- how Wt can go outside the [Identities](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Identity.html) attached to a user. This allows for user data to be managed in a more custom manner, and for entries to be encrypted or at least obfuscated if desired.
- how this custom implementation can be used in the existing flow.
