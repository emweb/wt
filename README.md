What is Wt ?
------------

Wt is a C++ library for developing web applications. It consists of:

- libwt, a widget/rendering library
- libwthttp, an (async I/O) HTTP/WebSockets server
- libwtfcgi, a FastCGI connector library (Unix)
- libwtisapi, an ISAPI connector library (Windows)
- libwttest, a test connector environment

It also contains a C++ ORM, which can be used in a web application
(obviously), but can also be used on its own:

- libwtdbo, a C++ ORM
- libwtdbopostgres, PostgreSQL backend
- libwtdbosqlite3, Sqlite3 backend
- libwtdbomysql, MySQL and MariaDB backend
- libwtdbomssqlserver, Microsoft SQL Server backend
- libwtdbofirebird, Firebird backend

For more information, see [the homepage](http://www.webtoolkit.eu/wt
"Wt homepage").

Dependencies
------------

To build Wt from source you will need at least
[CMake](https://cmake.org/) (>= 3.1), and
[boost](http://www.boost.org) (version >= 1.50).

Optionally, you may want to add:

- [OpenSSL](https://www.openssl.org) for SSL and WebSockets support in
  the built-in httpd, the HTTP(S) client, and additional cryptographic
  hashes in the authentication module
- [Haru PDF library](http://libharu.org) which is used for painting to PDF
- [GraphicsMagick](http://www.graphicsmagick.org/) which is used for painting
  to PNG, GIF (on Windows, Direct2D can be used instead)
- [PostgreSQL](https://www.postgresql.org/) for the PostgreSQL Dbo backend
- [MySQL](https://www.mysql.com) or [MariaDB](https://mariadb.org/) for the MySQL Dbo backend
- An [ODBC driver](https://docs.microsoft.com/en-us/sql/connect/odbc/download-odbc-driver-for-sql-server)
  for the Microsoft SQL Server Dbo backend, and [unixODBC](http://www.unixodbc.org/) on Unix-like platforms
- [Firebird](http://www.firebirdsql.org/) for the Firebird Dbo backend
- [Pango](http://www.pango.org/) for improved font support in PDF and raster
  image painting. On Windows, DirectWrite can be used instead.
- [ZLib](https://zlib.net/) for compression in the built-in httpd.

For the FastCGI connector, you also need:

- [FastCGI development kit](http://www.fastcgi.com/): you need the
  C/C++ library (libfcgi++)

Building
--------

Generic instructions for [Unix-like
platforms](https://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html)
or [Windows
platforms](https://www.webtoolkit.eu/wt/doc/reference/html/InstallationWindows.html).

Bug Reporting
-------------
Bugs can be reported here
http://redmine.webtoolkit.eu/projects/wt/issues/new

Demos, examples
---------------

[The homepage](https://www.webtoolkit.eu/wt), itself a Wt application,
contains also [various examples](https://www.webtoolkit.eu/wt/documentation/examples).
