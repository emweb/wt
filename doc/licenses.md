# Wt License information

This file includes an exhaustive list of the licenses used by Wt, source code from external sources included in Wt
and any external dependencies. For common licenses, a link to SPDX is included. Non-common licenses are listed in the
appendix.

## Wt license

Wt itself is available under two licenses:

- The [GNU General Public License v2.0](https://spdx.org/licenses/GPL-2.0-only.html), with
OpenSSL exception. Other versions of the GPL do not apply.
- a [commercial license](https://www.webtoolkit.eu/wt/license/Wt%20License%20Agreement.pdf), which does not require you to distribute the source code of your application. Request a quotation [online](https://www.webtoolkit.eu/wt/download) or contact sales@emweb.be for more information.

Wt is Copyright &copy; [Emweb](https://www.emweb.be) bv.

### Exceptions

The `wtwithqt` example is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).

## Source code included in Wt

### C++

| Library | Comment | License | Authors |
| ------- | --------- | ------- | ------- |
| RapidXML | Used for XML parsing and validation | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | Copyright &copy; 2006, 2007 Marcin Kalicinski |
| CGI parser | Wt's CGI parser is based on a Perl implementation, rewritten in C++ | See appendix | Copyright &copy; 1993-1999 Steven E. Brenner |
| `alpha_composite.h` | Header from GraphicsMagick, required when GraphicsMagick is used | http://www.graphicsmagick.org/Copyright.html | Copyright &copy; 2003, 2005, 2008 GraphicsMagick Group<br>Copyright &copy; 2002 ImageMagick Studio |
| base64 encoding/decoding | | [zlib License](https://spdx.org/licenses/Zlib.html) | Copyright &copy; 2002 Ryan Petrie |
| bcrypt and blowfish | Used in Wt::Auth for password hashing | Public domain, see appendix | |
| passwdqc | Used in Wt::Auth for password strength validation | See appendix | Copyright &copy; 2000-2002 by Solar Designer<br>Copyright &copy; 2008,2009 by Dmitry V. Levin |
| md5 | | [zlib License](https://spdx.org/licenses/Zlib.html) | Copyright &copy; 1999, 2000, 2002 Aladdin Enterprises |
| sha1 | | Freeware Public License (FPL), see appendix | Copyright &copy; 1998, 2009 Paul E. Jone |
| date | | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright &copy; 2015, 2016, 2017 Howard Hinnant<br>Copyright &copy; 2015 Ville Voutilainen<br>Copyright &copy; 2016, 2017, 2019 Jiangang Zhuang<br>Copyright &copy; 2016 Adrian Colomitchi<br>Copyright &copy; 2016 Alexander Kormanovsky<br>Copyright &copy; 2017, 2018, 2019 Tomasz Kamiński<br>Copyright &copy; 2017 Aaron Bishop<br>Copyright &copy; 2017 Florian Dang<br>Copyright &copy; 2017 Nicolas Veloz Savino<br>Copyright &copy; 2017 Paul Thompson<br>Copyright &copy; 2019 Asad. Gharighi |
| any | Used to provide `std::any` implementation when C++17 is not available | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | Copyright &copy; 2016 Denilson das Mercês Amorim |
| signals | Wt's signal/slot implementation is adapted from this | [Creative Commons Zero v1.0 Universal](https://spdx.org/licenses/CC0-1.0.html) | Tim Janik |
| GLEW | Used for server side rendering of `WGLWidget` | [BSD 3-Clause "New" or "Revised" License](https://spdx.org/licenses/BSD-3-Clause.html) | Copyright &copy; 2002-2008, Milan Ikits <milan ikits[]ieee org><br>Copyright &copy; 2002-2008, Marcelo E. Magallon <mmagallo[]debian org><br>Copyright &copy; 2002, Lev Povalahev |
| http client example | `Wt::Http::Client` is based on a Boost Asio example | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | Copyright &copy; 2003-2008 Christopher M. Kohlhoff |

### JavaScript

| Library | Comment | License | Authors |
| ------- | ------- | ------- | ------- |
| jQuery v1.4b1pre | When no external jQuery is used, Wt uses this old version of jQuery | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright 2010, John Resig |
| jQuery Caret Range plugin | Detects the range of highlighted text | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright &copy; 2009 Matt Zabriski |
| Yahoo! History Framework | | [BSD 3-Clause "New" or "Revised" License](https://spdx.org/licenses/BSD-3-Clause.html) | Copyright &copy; 2008, Yahoo! Inc. |
| array remove function | | [MIT License](https://spdx.org/licenses/MIT.html) | John Resig |
| jPlayer | | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright &copy; 2009 - 2014 Happyworm Ltd |
| `glMatrix.js` | Wt uses a modified version of `glMatrix.js` for `WGLWidget` | [zlib License](https://spdx.org/licenses/Zlib.html) | Copyright &copy; 2010 Brandon Jones |
| ResizeSensor | | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright &copy; 2013 Marc J. Schmidt |

### Other

| Library | Comment | License | Authors |
| ------- | ------- | ------- | ------- |
| Font Awesome | | [SIL OFL 1.1](http://scripts.sil.org/OFL) and [MIT License](https://spdx.org/licenses/MIT.html) | Font Awesome by Dave Gandy - http://fontawesome.io |
| Twitter Bootstrap 2 | `WBootstrapTheme`, when version 2 is used| [Apache License 2.0](https://spdx.org/licenses/Apache-2.0.html) | Copyright 2012 Twitter, Inc |
| Twitter Bootstrap 3 | `WBootstrapTheme`, when version 3 is used| [MIT License](https://spdx.org/licenses/MIT.html) | Copyright 2011-2019 Twitter, Inc. |
| jQuery mobile framework | CSS transitions | [MIT License](https://spdx.org/licenses/MIT.html) | Copyright &copy; jQuery Project |
| FindSqlite3 | CMake script to find Sqlite3| [BSD 3-Clause "New" or "Revised" License](https://spdx.org/licenses/BSD-3-Clause.html) | Copyright &copy; 2010 Pau Garcia i Quiles |

## Source code included in Wt::Dbo

| Library | Comment | License | Authors |
| ------- | ------- | ------- | ------- |
| SQLite | Wt::Dbo Sqlite backend | [Public Domain](https://www.sqlite.org/copyright.html) | https://www.sqlite.org |
| IBPP | Wt::Dbo Firebird backend | IBPP License, see appendix | &copy; Copyright 2000-2006 T.I.P. Group S.A. and the IBPP Team |

## Source code included in wthttp connector

| Library | Comment | License | Authors |
| ------- | ------- | ------- | ------- |
| http server example | The `wthttp` connector is based on Boost Asio example code | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | Copyright &copy; 2003-2006 Christopher M. Kohlhoff |

## Source code included in examples

| Library | Use in Wt | License | Authors |
| ------- | --------- | ------- | ------- |
| prettify.js | Used for syntax highlighting in the `codeview` example | [Apache License 2.0](https://spdx.org/licenses/Apache-2.0.html) | Copyright &copy; 2006 Google Inc. |

## External dependencies

Wt's only essential external dependency, apart from system- and compiler-specific libraries, is Boost (https://www.boost.org).

The table below lists all of Wt's dependencies that are not system- or compiler-specific, depending on how Wt is configured.
Note that this list is provided for reference. Since this is external software and not distributed as part of the
Wt source code, their licenses may change.

| Library | Use in Wt | License | URL |
| ------- | --------- | ------- | ---- |
| Boost   | Required | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | https://www.boost.org |
| Standalone Asio    | Alternative to Boost::Asio | [Boost Software License 1.0](https://spdx.org/licenses/BSL-1.0.html) | https://think-async.com/Asio/ |
| OpenSSL | TLS support | [OpenSSL License](https://spdx.org/licenses/OpenSSL.html) | https://www.openssl.org |
| zlib | `wthttp` connector: gzip compression support | [zlib License](https://spdx.org/licenses/Zlib.html) | https://zlib.net/ |
| FastCGI | `wtfcgi` connector | [FastCGI Open Market License](https://fastcgi-archives.github.io/LICENCE.html) | https://fastcgi-archives.github.io/ (archived copy) |
| libHaru | PDF rendering, font metrics | [zlib License](https://spdx.org/licenses/Zlib.html) | http://libharu.org/ |
| Pango   | Improved font selection and rendering (DirectWrite may be used on Windows instead) | [GNU Library General Public License v2 or later](https://spdx.org/licenses/LGPL-2.0-or-later.html) | https://pango.gnome.org/ |
| GraphicsMagick | `WRasterImage` support (Direct2D may be used on Windows instead) | [MIT License](https://spdx.org/licenses/MIT.html) | http://www.graphicsmagick.org/ |
| GLEW | Server side rendering for `WGLWidget` (alternative to built-in version) | [BSD 3-Clause "New" or "Revised" License](https://spdx.org/licenses/BSD-3-Clause.html) | http://glew.sourceforge.net/ |
| SQLite | Wt::Dbo Sqlite backend (alternative to built-in version) | [Public Domain](https://www.sqlite.org/copyright.html) | https://www.sqlite.org |
| PostgreSQL | Wt::Dbo PostgreSQL backend | [PostgreSQL License](https://spdx.org/licenses/PostgreSQL.html) | https://www.postgresql.org/ |
| MariaDB Connector/C | Wt::Dbo MySQL backend | [GNU Lesser General Public License v2.1 or later](https://spdx.org/licenses/LGPL-2.1-or-later.html) | https://downloads.mariadb.org/connector-c/ |
| MySQL Connector/C | Wt::Dbo MySQL backend (alternative) | [GNU General Public License v2.0](https://spdx.org/licenses/GPL-2.0-only.html) or proprietary | https://www.mysql.com/products/connector/ |
| UnixODBC | Wt::Dbo SQL Server backend on non-Windows | [GNU Lesser General Public License v2.1 or later](https://spdx.org/licenses/LGPL-2.1-or-later.html) | http://www.unixodbc.org/ |
| Qt | `wtwithqt` example | Multiple licenses | https://www.qt.io/ |
| libunwind | Stacktrace in exception messages | [MIT License](https://spdx.org/licenses/MIT.html) | https://www.nongnu.org/libunwind/ |
| TinyMCE | Client side, `WTextEdit` widget | [GNU Lesser General Public License v2.1](https://spdx.org/licenses/LGPL-2.1-only.html) | https://www.tiny.cloud/ |
| Leaflet | Client side, `WLeafletMap` widget | [BSD 2-Clause "Simplified" License](https://spdx.org/licenses/BSD-2-Clause.html) | https://leafletjs.com/ |

## Appendix: special licenses

### CGI Parser

```
Copyright (c) 1993-1999 Steven E. Brenner
Unpublished work.
Permission granted to use and modify this library so long as the
copyright above is maintained, modifications are documented, and
credit is given for any use of the library.
```
### Bcrypt and Blowfish

```
Written by Solar Designer <solar at openwall.com> in 1998-2011.
No copyright is claimed, and the software is hereby placed in the public
domain.  In case this attempt to disclaim copyright and place the software
in the public domain is deemed null and void, then the software is
Copyright (c) 1998-2011 Solar Designer and it is hereby released to the
general public under the following terms:

Redistribution and use in source and binary forms, with or without
modification, are permitted.

There's ABSOLUTELY NO WARRANTY, express or implied.
```

### passwdqc

```
Copyright (c) 2000-2002 by Solar Designer
Copyright (c) 2008,2009 by Dmitry V. Levin
Redistribution and use in source and binary forms, with or without
modification, are permitted.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

### Sha1

```
Copyright (C) 1998, 2009
Paul E. Jones <paulej@packetizer.com>
All Rights Reserved

Freeware Public License (FPL)

This software is licensed as "freeware."  Permission to distribute
this software in source and binary forms, including incorporation
into other products, is hereby granted without a fee.  THIS SOFTWARE
IS PROVIDED 'AS IS' AND WITHOUT ANY EXPRESSED OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  THE AUTHOR SHALL NOT BE HELD
LIABLE FOR ANY DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE, EITHER
DIRECTLY OR INDIRECTLY, INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA
OR DATA BEING RENDERED INACCURATE.
 ```

### IBPP

```
IBPP License v1.1
-----------------

(C) Copyright 2000-2006 T.I.P. Group S.A. and the IBPP Team (www.ibpp.org)

Permission is hereby granted, free of charge, to any person or organization
("You") obtaining a copy of this software and associated documentation files
covered by this license (the "Software") to use the Software as part of another
work; to modify it for that purpose; to publish or distribute it, modified or
not, for that same purpose; to permit persons to whom the other work using the
Software is furnished to do so; subject to the following conditions: the above
copyright notice and this complete and unmodified permission notice shall be
included in all copies or substantial portions of the Software; You will not
misrepresent modified versions of the Software as being the original.

The Software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
the authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use of other dealings in
the Software.
```
