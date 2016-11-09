Client SSL authentication feature example
-----------------------------------------

This is an example that illustrates the use of the `WSslInfo` and
'WSslCertificate' to authenticate clients based on SSL certificates.
The example also demonstrates how this SSL based authentication can
be combined with Wt's authentication framework.

How to run
----------

See the README in the parent directory.

SSL certificates should be obtained (purchased) from Certificate Authorities,
but for testing you can use OpenSSL to generate your own test certificates.
This example requires CA public keys, server public and private keys, and
client public and private keys.

You will need to configure your webserver to work with these keys. A sample
configuration for the built-in Wt httpd is:

docroot=.
http-address=0.0.0.0
http-port=8080
https-address=0.0.0.0
https-port=4430
ssl-certificate=projects/ssl/myCA/certs/server.crt
ssl-private-key=projects/ssl/myCA/private/server.key
ssl-tmp-dh=projects/ssl/dh2048.pem
ssl-client-verification=required
ssl-verify-depth=15
ssl-ca-certificates=projects/ssl/myCA/certs/myca.crt

What it illustrates
-------------------

- the use of client certificates for authentication

