# TOTP example
---

This example illustrates the use of the `Wt::Auth::Mfa` namespace.

It allows for another factor when logging in, essentially increasing security.
This avoids a single failure of a re-used, weak or compromised password. This presents an additional type of evidence for authentication.
See [OWASP MFA](https://cheatsheetseries.owasp.org/cheatsheets/Multifactor_Authentication_Cheat_Sheet.html) for a more detailed description on types, recommendations, and more.

Two default users of are provided to easily start using the example. The username:password combo's are:

- admin:admin
- user:user

# How to run
---

See the README in the parent directory.
Important here is that the `resources-dir` is provided. Otherwise an error will be shown.


# What it illustrates
---

- how to configure the default [TotpProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1TotpProcess.html) implementation.
- the use of the [TotpProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1TotpProcess.html) as the additional factor.
