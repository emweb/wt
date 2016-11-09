Auth1 feature example
---------------------

This example builds further on auth1, but shows how to customize the
authentication framework, by expanding the registration process to also
include other User information.

How to run
----------

See the README in the parent directory.

Additional arguments: `-c wt_config.xml`

The configuration file specifies the configuration for using Google and/or Facebook as
an authentication provider (you need to register with Google and/or Facebook for this to
work).

What it illustrates
-------------------

- specializing AuthWidget and RegistrationWidget to capture additional
  information of a User during registration
