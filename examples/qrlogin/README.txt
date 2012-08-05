QRLogin example
===============

This is an example that illustrates how Wt::Auth can be extended with
additional authentication mechanisms.

In this case, we add the ability to login by authenticating on a
mobile phone after scanning a QR code. This means that if on the phone
you have a remember-me cookie stored, you can sign in to a public
computer without needing password authentication.

How to run
----------

   ln -s ../../resources .
   ../../build/examples/qrlogin/qrlogin.wt -c wt_config.xml

The configuration file configures progressive bootstrap, which is needed
to apply viewport indications for mobile browsers, and also enables
websockets, just for fun.

What it illustrates
-------------------

- server push
- the use of WResource and Http::Client to implement inter-session
  communication in case not all sessions run within the same process
- using Wt::Dbo for a single table
- how to customize and extend Wt::Auth
