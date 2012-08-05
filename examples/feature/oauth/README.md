Auth1 feature example
---------------------

This is an example that illustrates the use of the
`Wt::Auth::OAuthService` API for OAuth-2.0-based authentication (aka
OpenID Connect), which can also be used directly without the rest of
the Wt::Auth framework.

How to run
----------

See the README in the parent directory.

Additional arguments: `-c wt_config.xml`

The configuration file specifies the configuration for using Google as
an authentication provider (you need to register with Google for this to
work).

What it illustrates
-------------------

- the use of many aspects of the `Wt::Auth::OAuthService` (outside the
  authentication framework).
