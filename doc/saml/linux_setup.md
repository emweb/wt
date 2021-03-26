# Installing Wt with SAML support on Linux {#saml_linux}

## Dependencies

### Debian or Ubuntu

In order to build Wt with SAML support on Debian or Ubuntu, you can simply install the following packages:

```shell
apt install libsaml-dev opensaml-schemas xmltooling-schemas
```

### From source

The following dependencies are usually available as a package on most distributions:

- [Xerces-C++](http://xerces.apache.org/xerces-c/download.cgi)
- [Apache Santuario / Apache XML Security for C++](https://santuario.apache.org/cindex.html)
- [libcurl](https://curl.se/)
- [Boost](https://www.boost.org/) (also a Wt dependency)
- [OpenSSL](https://www.openssl.org/) (also an optional Wt dependency)

You can download sources for Log4Shib, XMLTooling, and OpenSAML from [Shibboleth](https://shibboleth.net/downloads).

- [Log4Shib](https://shibboleth.net/downloads/log4shib/latest/)
- [XMLTooling and OpenSAML](https://shibboleth.net/downloads/c++-opensaml/latest/)

Installation is a simple `./configure`/`make install`. If you're installing into a prefix, you can use `PKG_CONFIG_PATH`:

```shell
PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig ./configure --prefix=$PREFIX
make
make install
```

## Configuring Wt

You need to set `ENABLE_SAML` to `ON` when configuring Wt. You can use `SAML_ROOT` to set a different prefix:

```shell
cmake path/to/sources -DENABLE_SAML=ON -DSAML_ROOT=$PREFIX
```
