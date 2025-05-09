# Minimum dependency versions

//! [MinDep]
We base the minimum versions of dependencies on a roughly 5-year sliding
window and a selection of commonly used operating systems. This means
that %Wt will be guaranteed to build using:
//! [MinDep]

- Debian oldstable
- The previous LTS version of Ubuntu
- The previous release of Red Hat Enterprise Linux (and its derivatives)
- The previous release of FreeBSD
- What was the latest version of Visual Studio 5 years ago

//! [MinDep2]
Note that this doesn't mean that you can't compile %Wt on older versions
of these operating systems. We will just not go out of our way to
maintain compatibility with them.
<br><br>
You may be able to compile %Wt on older operating systems if you
use a different toolset. For example, Red Hat supplies toolsets that
allow you to use a more recent compiler on older RHEL releases.
//! [MinDep2]

## Example: September 2022

At the time of writing, in September 2022, this means that we'll look at:

//! [UnixVersion]
- Debian 10
- Ubuntu 20.04
- Red Hat Enterprise Linux 8
- FreeBSD 12
//! [UnixVersion]
//! [WindowsVersion]
- Visual Studio 2017
//! [WindowsVersion]

## Wt's current core dependencies

//! [CoreDep]
<h3><a name="coredep">%Core dependencies</a></h3>
These dependencies are required to build %Wt. %Wt relies on functionality of these libraries to implement functionality or to actually be able to build.
Boost is the only actual library that %Wt depends on. There are a few configuration options when building with Boost that can be configured, and some are
required. The options are the following:
- <strong>BOOST_ROOT/BOOST_PREFIX</strong>: either of these options can (or rather, should) be used if Boost is not installed in a default location. It points to the Boost installation path.
- <strong>Boost_ADDITIONAL_VERSIONS</strong>: a list defining the viable Boost versions. You may need to append your version here. This will mainly be the case for custom installations that do not use the expected versioning system.
- <strong>Boost_USE_STATIC_LIBS</strong>: can be defined on Windows, indicating whether Boost ought to be linked statically or dynamically.

Not all components of Boost are required to build %Wt, and some are only
required under certain conditions.
The current list consists of:
<table>
  <tr>
    <td>Boost component</td>
    <td>Requirement conditions</td>
  </tr>
  <tr>
    <td>Program Options</td>
    <td>Always required</td>
  </tr>
  <tr>
    <td>System</td>
    <td>Always required</td>
  </tr>
  <tr>
    <td>Thread</td>
    <td>Always required</td>
  </tr>
  <tr>
    <td>Filesystem</td>
    <td>
      Required if the configuration flag
      <strong>WT_CPP17_FILESYSTEM_IMPLEMENTATION</strong> is set to
      <strong>boost</strong>. This is flag can be set to
      <strong>std</strong>, which will make %Wt use
      <code>std::filesystem</code> (only available for C++17 or higher)
      instead of <code>boost::filesystem</code>.
    </td>
  </tr>
  <tr>
    <td>Unit Test Framework</td>
    <td>
      Required to compile the tests that comes with %Wt. You can set
      the configuration flag <strong>BUILD_TESTS</strong> to
      <strong>OFF</strong> if you do not want to compile those tests.
    </td>
  </tr>
</table>

More information on including Boost can be found <a href="https://cmake.org/cmake/help/latest/module/FindBoost.html" target="_blank">here</a>.
//! [CoreDep]

//! [Unix]
- <a href="https://cmake.org/" target="_blank">CMake 3.13</a> or higher
- <a href="https://www.boost.org/" target="_blank">Boost 1.66</a> or higher
- <a href="https://gcc.gnu.org/" target="_blank">GCC 8</a> or higher
- <a href="https://clang.llvm.org/" target="_blanks">Clang 7</a> or higher
//! [Unix]
//! [Windows]
- <a href="https://cmake.org/" target="_blank">CMake 3.13</a> or higher
- <a href="https://www.boost.org/" target="_blank">Boost 1.66</a> or higher
- <a href="https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-150" target="_blank">MSVC toolset 14.1</a> or higher
//! [Windows]

## Wt's current optional dependencies

//! [OptDep]
<h3><a name="optdep">Optional dependencies</a></h3>
The following dependencies are optional. They are used for specific functionality that %Wt offers, and are often hidden behind certain compilation flags.
The current list consists of:
<table>
  <tr>
    <td>Dependency</td>
    <td>Configuration flag</td>
    <td>Default value</td>
    <td>Details</td>
  </tr>
  <tr>
    <td><a href="http://www.openssl.org" target="_blank">OpenSSL</a></td>
    <td>CONNECTOR_HTTP</td>
    <td>ON</td>
    <td>Used to support the HTTPS protocol in the web client, and the HTTPS protocol in the built-in wthttpd connector. This will only be used of the wthttpd connector is actually build. This is managed by the build flag <strong>CONNECTOR_HTTP</strong>, which can be <strong>ON</strong> or <strong>OFF</strong>. If OpenSSL is not installed in a default location, its prefix needs to be specified with <strong>SSL_PREFIX</strong> (as a path).</td>
  </tr>
  <tr>
    <td><a href="http://libharu.org/" target="_blank">Haru Free PDF Library</a></td>
    <td>ENABLE_HARU</td>
    <td>ON</td>
    <td>Used to provide support for painting to PDF (<a href="classWt_1_1WPdfImage.html" target="_blank">WPdfImage</a>). This can be managed by using the <strong>ENABLE_HARU</strong> flag (<strong>ON/OFF</strong>). If Haru is not installed in a default location, its prefix needs to be specified with <strong>HARU_PREFIX</strong> (as a path). The library can be configured to link statically with <strong>HARU_DYNAMIC</strong> (where <strong>OFF</strong> means static link).<br><br><strong>%Wt relies on an older verion of HARU, below 2.4.0. Something does not quite compile right with higher versions. We may look at including a more recent version (see: <a href="https://redmine.emweb.be/issues/11704" target="_blank">#11704</a>). Additionally, there seems to be a rendering bug with higher versions (2.4.0-2.4.3), where floating point values are not correctly rendered (see: <a href="https://redmine.emweb.be/issues/11833" target="_blank">#11833</a>).</strong></td>
  </tr>
  <tr>
    <td><a href="http://www.graphicsmagick.org/" target="_blank">GraphicsMagick</a></td>
    <td>WT_WRASTERIMAGE_IMPLEMENTATION</td>
    <td>none</td>
    <td>Used for supporting painting to raster images (PNG, GIF, ...) (<a href="classWt_1_1WRasterImage.html" target="_blank">WRasterImage</a>), which are rendered server-side. This configures in what way images can be rendered in %Wt. This is managed by the <strong>WT_WRASTERIMAGE_IMPLEMENTATION</strong> flag. This flag is a string, and can take the values <strong>GraphicsMagick/Direct2D/none</strong>. Where <strong>GraphicsMagick</strong> will require a graphicsmagick installation. If this is not found, it needs to be specified using <strong>GM_PREFIX</strong> (as a path). Generally speaking <strong>GraphicsMagick</strong> is a Linux way to handle images, whereas <strong>Direct2D</strong> is the Windows way.</td>
  </tr>
  <tr>
    <td><a href="http://www.pango.org/" target="_blank">Pango</a></td>
    <td>ENABLED_PANGO</td>
    <td>ON</td>
    <td>Used for improved font support in the <a href="classWt_1_1WPdfImage.html" target="_blank">WPdfImage</a> and <a href="classWt_1_1WRasterImage.html" target="_blank">WRasterImage</a> paint devices. It can be configured by <strong>ENABLED_PANGO</strong> (<strong>ON/OFF</strong>). This will allow for more fonts to be used by %Wt.</td>
  </tr>
  <tr>
    <td><a href="http://www.postgresql.org/" target="_blank">PostgreSQL</a></td>
    <td>ENABLE_POSTGRES</td>
    <td>ON</td>
    <td>Used for the PostgreSQL backend for Wt::Dbo (<a href="classWt_1_1Dbo_1_1backend_1_1Postgres.html" target="_blank">Dbo::backend::Postgres</a>). This will allow %Wt to use PostgreSQL as its Dbo backend. This can be configured using <strong>ENABLE_POSTGRES</strong> (<strong>ON/OFF</strong>). If PostgreSQL is not installed in a default location, its prefix needs to be specified with <strong>POSTGRES_PREFIX</strong> (as a path).</td>
  </tr>
  <tr>
    <td><a href="http://www.firebirdsql.org/" target="_blank">Firebird</a></td>
    <td>ENABLE_FIREBIRD</td>
    <td>ON</td>
    <td>Used for the Firebird backend for Wt::Dbo (<a href="classWt_1_1Dbo_1_1backend_1_1Firebird.html" target="_blank">Dbo::backend::Firebird</a>). This will allow %Wt to use Firebird as its Dbo backend. This can be configured using <strong>ENABLE_FIREBIRD</strong> (<strong>ON/OFF</strong>). If Firebird is not installed in a default location, its prefix needs to be specified with <strong>FIREBIRD_PREFIX</strong> (as a path). By default %Wt includes a IBPP implementation (a library for accessing Firebird databases). If one wished to use a custom IBPP implementation, this will require <strong>IBPP_SRC_DIRECTORY</strong>, which can be found <a href="https://ibpp.sourceforge.io/" target="_blank">here</a>, together with <strong>USE_SYSTEM_IBPP</strong> being set to <strong>ON</strong>.</td>
  </tr>
  <tr>
    <td><a href="https://www.mysql.com/products/connector/" target="_blank">the C API for MySQL (mysqlclient)</a><br>or the<br><a href="https://mariadb.com/kb/en/the-mariadb-library/about-mariadb-connector-c">MariaDB connector library</a></td>
    <td>ENABLE_MYSQL</td>
    <td>ON</td>
    <td>Used for the MySQL/MariaDB backend for Wt::Dbo (<a href="classWt_1_1Dbo_1_1backend_1_1MySQL.html" target="_blank">Dbo::backend::MySQL</a>). This will allow %Wt to use MySQL/MariaDB as its Dbo backend. This can be configured using <strong>ENABLE_MYSQL</strong> (<strong>ON/OFF</strong>). If MySQL / MariaDB is not installed in a default location, its prefix needs to be specified with <strong>MYSQL_PREFIX</strong> (as a path).</td>
  </tr>
  <tr>
    <td><a href="http://www.unixodbc.org" target="_blank">unixODBC</a></td>
    <td>ENABLE_MSSQLSERVER</td>
    <td>ON</td>
    <td>Used for the SQL Server backend for Wt::Dbo (<a href="classWt_1_1Dbo_1_1backend_1_1MSSQLServer.html" target="_blank">Dbo::backend::MSSQLServer</a>). This will allow %Wt to use MSSQL as its Dbo backend. This can be configured using <strong>ENABLE_MSSQLSERVER</strong> (<strong>ON/OFF</strong>). As the name indicates, this is a Windows backend. But it can also be installed on Linux systems (see: <a href="https://learn.microsoft.com/en-us/sql/linux/sql-server-linux-setup" target="_blank">Microsoft's documentation</a> for more information).</td>
  </tr>
  <tr>
    <td><a href="http://www.nongnu.org/libunwind/" target="_blank">libunwind</a></td>
    <td>ENABLE_UNWIND</td>
    <td>OFF</td>
    <td>Used for the saving of backtraces in exceptions (useful for debugging). This can be configured by <strong>ENABLE_UNWIND</strong> (<strong>ON/OFF</strong>). If libunwind is not installed in a default location, its prefix needs to be specified with <strong>UNWIND_PREFIX</strong> (as a path).</td>
  </tr>
  <tr>
    <td><a href="https://zlib.net/" target="_blank">zlib</a></td>
    <td>HTTP_WITH_ZLIB/td>
    <td>Depends</td>
    <td>Used for the compression of data over HTTP or using WebSockets. This will only affect the <strong>wthttpd</strong> connector. This can be configured by <strong>HTTP_WITH_ZLIB</strong> (<strong>ON/OFF</strong>). If zlib is not installed in a default location, its prefix needs to be specified with <strong>ZLIB_PREFIX</strong> (as a path).</td>
  </tr>
  <tr>
    <td><a href="http://think-async.com/Asio/" target="_blank">standalone Asio</a></td>
    <td>WT_ASIO_IMPLEMENTATION/td>
    <td>boost</td>
    <td>It is possible to use the standalone version of Asio (Asio 1.12.0 or higher) instead of the one in the Boost library. This can be configured by <strong>WT_ASIO_IMPLEMENTATION</strong> (<strong>boost/standalone</strong>). If standalone Asio is not installed in a default location, its prefix needs to be specified with <strong>ASIO_PREFIX</strong> (as a path).</td>
  </tr>
</table>
//! [OptDep]
