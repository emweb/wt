# PIN example
---

This example illustrates the use of the `Wt::Auth::Mfa` namespace.

It allows for another factor when logging in, essentially increasing security.
This avoids a single failure of a re-used, weak or compromised password. This presents an additional type of evidence for authentication.
See [OWASP MFA](https://cheatsheetseries.owasp.org/cheatsheets/Multifactor_Authentication_Cheat_Sheet.html) for a more detailed description on types, recommendations, and more.

**Do note that this is not a good MFA approach together with username/password.** MFA requires that the two factors are of different "types". In this case, they both are
of knowledge you posses. This doesn't constitute valid MFA. The example is purely illustrative, showing developers how they can adapt the default implementation to fit their needs.

Two default users of are provided to easily start using the example. The username:password combo's are:

- admin:admin
- user:user

# How to run
---

See the README in the parent directory.
Important here is that the `resources-dir` is provided. Otherwise an error will be shown.


# What it illustrates
---

- how to create a custom [AbstractMfaProcess](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Mfa_1_1AbstractMfaProcess.html) implementation.
- how Wt uses the [Identities](https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1Auth_1_1Identity.html) attached to a user to allow for easy (unencrypted!) storage of the data.
- how this custom implementation can be used in the existing flow.
