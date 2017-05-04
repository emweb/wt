OpenID Connect feature example
------------------------------

This is an example that illustrates the use of the `Wt::Auth::OidcService` API
for OpenID Connect-based authentication, which can also be used directly without
the rest of the Wt::Auth framework. And the various classes required to
implement an OpenID Connect identity provider.

How to run
----------

See the README in the parent directory.

Additional arguments: `-c wt_config.xml`

The configuration specifies where the example is deployed and it's an example
for how you could configure the `Wt::Auth::OidcService`.

What it illustrates
-------------------

- the use of the `Wt::Auth::OidcService` (see `Oidc.C`).

- how to implement an OpenID Connect provider.

  + the use of the `Wt::Auth::OAuthAuthorizationEndpointProcess` to implement an
    OpenID connect authorization endpoint (see `OAuthAuthorizationEndpoint`).

  + the use of the `Wt::Auth::OAuthTokenEndpoint` and
    `Wt::Auth::OidcUserInfoEndpoint` (see `Oidc.C`).

  + how to implement the `Wt::Auth::AbstractUserDatabase` using `Wt::Dbo` to
    support OpenID Connect (see `model/OidcUserDatabase`, `model/IssuedToken`,
    `model/Session`, `model/OAuthClient`, and `model/User`).
