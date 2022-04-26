# Installing Wt with SAML support on Windows {#saml_windows}

## Dependencies

These instructions were derived from the Windows build instruction for Shibboleth SP:
https://wiki.shibboleth.net/confluence/display/SP3/WindowsBuild

These steps describe how I (Roel Standaert) built it on Windows with the Visual Studio 2017 toolset:

### Get dependencies

Create your `ROOT_DIR` to put everything in, e.g.:

```
C:\Users\roel\libraries\saml
```

Git, if installed into its default location is fine for the `SED` dependency.

Get [Perl](https://strawberryperl.com/)

Netwide Assembler (for OpenSSL): https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe

```cmd
git clone https://git.shibboleth.net/git/cpp-msbuild
cd cpp-msbuild
git checkout 5fca76fcfe36aa3a37220a2e5f033677b8688479
cd ..
git clone https://git.shibboleth.net/git/cpp-xmltooling -b 3.2.0
git clone https://git.shibboleth.net/git/cpp-opensaml -b 3.2.0
```

Get log4shib:
```cmd
git clone https://git.shibboleth.net/git/cpp-log4shib
cd cpp-log4shib
REM master at time of writing (2.0.0 release has issues):
git checkout ed0436600d5315e8795a35079548abe157700756
cd ..
```

Get [zlib](https://zlib.net/zlib1211.zip), put `zlib-1.2.11` folder in `ROOT_DIR`.

Get [OpenSSL](https://www.openssl.org/source/openssl-1.1.1j.tar.gz), and unpack it in `ROOT_DIR`.

Get [curl](https://github.com/curl/curl/releases/download/curl-7_75_0/curl-7.75.0.zip), and unpack it in `ROOT_DIR`.

Get [Xerces C++](https://downloads.apache.org//xerces/c/3/sources/xerces-c-3.2.3.zip), and unpack it in `ROOT_DIR`

Get [XmlSecurity](https://www.apache.org/dyn/closer.lua/santuario/c-library/xml-security-c-2.0.2.zip), and unpack it in `ROOT_DIR`

### Dependency configuration

Had to uncomment in `cpp-msbuild/env.make`:

```Makefile
# !if "$(SED)" == ""
# SED=sed
# !endif
```

Change `ROOT_DIR`, e.g.:

```bat
set ROOT_DIR=C:\Users\roel\libraries\saml\
```

Note the trailing slash!

#### Modify `cpp-msbuild/dependencies/config.bat`

Under `:path_done`:

Add Perl and NASM:

```bat
REM Set Perl and nasm
set PERL=c:\Strawberry\perl\bin\perl.exe
set PATH=%PATH%;C:\Program Files\NASM
```

Change OpenSSL:

```bat
set OPENSSL_DIR=openssl-1.1.1j
REM j is the tenth letter of the alphabet, hence 10
set OPENSSL_MM_VERSION=1.1.1.10
set OPENSSL_FILE_VERSION=1_1_1_10
```

Change Curl:

```bat
Set LIBCURL_VERSION=7.75.0
set LIBCURL_DIR=curl-%LIBCURL_VERSION%
set LIBCURL_FILE_VERSION=
```

XmlSecurity is already fine

Zlib is already fine

Change Log4shib:

```bat
set LOG4SHIB_DIR=cpp-log4shib
set LOG4SHIB_MM_VERSION=2.0.0
set LOG4SHIB_FILE_VERSION=2_0
```

Xerces is already fine

This corresponds to Visual Studio 2017:

```bat
SET MsVCVersionNum=15
```

#### OpenSSL modifications

Add the OpenSSL subversion in `Configurations/10-main.conf`:

```perl
        multilib         => "_1_10-x64",
```

Duplicate the whole setting to a new target "VC-WIN64AD", this time add D to the `multilib`:

```perl
    "VC-WIN64AD" => {
        inherit_from     => [ "VC-WIN64-common", asm("x86_64_asm"),
                              sub { $disabled{shared} ? () : "x86_64_uplink" } ],
        AS               => sub { vc_win64a_info()->{AS} },
        ASFLAGS          => sub { vc_win64a_info()->{ASFLAGS} },
        asoutflag        => sub { vc_win64a_info()->{asoutflag} },
        asflags          => sub { vc_win64a_info()->{asflags} },
        sys_id           => "WIN64A",
        bn_asm_src       => sub { return undef unless @_;
                                  my $r=join(" ",@_); $r=~s|asm/x86_64-gcc|bn_asm|; $r; },
        perlasm_scheme   => "auto",
        multilib         => "_1_10D-x64",
    }
```

We won't do the 32 bit build.

### Building dependencies

Launch "x64 Native Tools Command Prompt for VS 2017" and go to the `ROOT_DIR`.

```cmd
cd cpp-msbuild\dependencies
.\config.bat
```

```cmd
nmake /f dependency.make openssl
REM got winsock2.h include error
REM opened sln file for log4shib in Visual Studio and
REM retargeted to one of the Windows SDK versions I had
nmake /f dependency.make log4shib
nmake /f dependency.make zlib
nmake /f dependency.make xerces
nmake /f dependency.make curl
REM needed MFC installed for afxres.h
nmake /f dependency.make xsec
```

### Building OpenSAML

Create `BuildPath.props` in `ROOT_DIR`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
</Project>
```

Update `cpp-msbuild/Versions.props`:

I left the GUIDs intact for now, but maybe they should be changed?

Curl (7.74.0 to 7.75.0):

```xml
  <PropertyGroup Label="CurlGuidsBad" Condition="'$(LIBCURL_FILE_VERSION)' != '9'">
    <CurlFileVersion>BAD_LIBCURL_FILE_VERSION</CurlFileVersion>
    <!-- Changing LIBCURL_FILE_VERSION version requires changing the GUIDs since it is garnish on the name -->
  </PropertyGroup>
  <PropertyGroup Label="CurlGuids" Condition="'$(LIBCURL_FILE_VERSION)' == '9'">
    <CurlVersion Condition="'$(LIBCURL_DIR)'!=''">$(LIBCURL_VERSION)</CurlVersion>
    <CurlVersion Condition="'$(LIBCURL_DIR)'==''">7.75.0</CurlVersion>
    <curl Condition="'$(LIBCURL_DIR)'!=''">$(LIBCURL_DIR)</curl>
    <curl Condition="'$(LIBCURL_DIR)'==''">curl-$(CurlVersion)</curl>

    <CurlFileVersion>$(LIBCURL_FILE_VERSION)</CurlFileVersion>
    <LibCurlGuid32>{C5DA388E-C39C-4164-98EF-CB5660F9ABB4}</LibCurlGuid32>
    <LibCurlGuid64>{38B92986-C5BA-4CB1-BFFD-E031FF22BE4C}</LibCurlGuid64>
    <LibCurlGuid32d>{42951E00-7863-41B6-A770-BB3711588BDD}</LibCurlGuid32d>
    <LibCurlGuid64d>{8C610CA1-DDA8-45AA-A9C4-914420824D84}</LibCurlGuid64d>
  </PropertyGroup>
```

OpenSSL (1.1.1i to 1.1.1j):

```xml
  <PropertyGroup Label="OpenSSLGeneral">
    <!-- Just for now -->
    <OPENSSL_FILE_VERSION Condition="'$(OPENSSL_FILE_VERSION)' == ''">1_1_1_10</OPENSSL_FILE_VERSION>

    <openssl Condition="'$(OPENSSL_DIR)'==''">openssl-1.1.1j</openssl>
    <openssl Condition="'$(OPENSSL_DIR)'!=''">$(OPENSSL_DIR)</openssl>
  </PropertyGroup>

  <PropertyGroup Label="OpenSSLGuidsBad" Condition="'$(OPENSSL_FILE_VERSION)' != '1_1_1_10'">
    <OpenSSLFileVersion>BAD_OPENSSL_FILE_VERSION</OpenSSLFileVersion>
    <!-- Changing OPENSSL_FILE_VERSION version requires changing the GUIDs since it is garnish on the name -->
  </PropertyGroup>
  <PropertyGroup Label="OpenSSLGuids" Condition="'$(OPENSSL_FILE_VERSION)' == '1_1_1_10'">
    <OpenSSLFileVersion>$(OPENSSL_FILE_VERSION)</OpenSSLFileVersion>
    <!-- Change the 8 components below iff OpenSSLFileVersion changes -->
    <LibEay32Component>{E5CDA78B-1024-4DF7-9F11-8532B6F55E21}</LibEay32Component>
    <SSlEay32Component>{6AD1215B-E983-4823-A363-6127420408E4}</SSlEay32Component>
    <LibEay32Componentd>{E3B837AE-B103-45F7-AC98-72BE66B76746}</LibEay32Componentd>
    <SSlEay32Componentd>{C7454E2E-DAEA-487B-AB1C-9339E5E390C0}</SSlEay32Componentd>
    <LibEay64Component>{6E57A83F-E67E-4E8D-B24B-ACDDFC7FDA5E}</LibEay64Component>
    <SSlEay64Component>{73702736-2A6F-43DC-BB08-663641C1798C}</SSlEay64Component>
    <LibEay64Componentd>{DBA7EF27-C817-43EC-B2E6-A4E945355F0A}</LibEay64Componentd>
    <SSlEay64Componentd>{96350A7E-15DA-4CF4-B90D-F3E670AF9D36}</SSlEay64Componentd>
  </PropertyGroup>
```

Fixing up log4shib path:

```xml
  <PropertyGroup Label="Log4ShibGeneral">
    <!-- Just for now -->
    <LOG4SHIB_FILE_VERSION Condition="'$(LOG4SHIB_FILE_VERSION)' == ''">2_0</LOG4SHIB_FILE_VERSION>
    <Log4ShibVersion Condition="'$(LOG4SHIB_MM_VERSION)' == ''">2.0.0</Log4ShibVersion>
    <Log4ShibVersion Condition="'$(LOG4SHIB_MM_VERSION)' != ''">$(LOG4SHIB_MM_VERSION)</Log4ShibVersion>
  </PropertyGroup>

  <PropertyGroup Label="Log4ShibGuidsBad" Condition="'$(LOG4SHIB_FILE_VERSION)' != '2_0'">
    <Log4ShibFileVersion>BAD_LOG4SHIB_FILE_VERSION</Log4ShibFileVersion>
    <!-- Changing LOG4SHIB_FILE_VERSION version requires changing the GUIDs since it is garnish on the name -->
  </PropertyGroup>
  <PropertyGroup Label="Log4ShibGuids" Condition="'$(LOG4SHIB_FILE_VERSION)' == '2_0'">
    <Log4ShibFileVersion>$(LOG4SHIB_FILE_VERSION)</Log4ShibFileVersion>
    <!-- Change the 4 components below if the Log4ShibFileVersion name changes -->
    <Log4ShibComponent32>{4A96D4F3-51DD-4FBD-9478-8EC2197FAF70}</Log4ShibComponent32>
    <Log4ShibComponent32d>{08A406FB-D426-40A6-B5C1-9FFBC9A085B2}</Log4ShibComponent32d>
    <Log4ShibComponent64>{AED2A19A-D47E-41B4-A4FB-5F23601F989D}</Log4ShibComponent64>
    <Log4ShibComponent64d>{A30DBD49-6717-44CB-9A38-349D4F8DF149}</Log4ShibComponent64d>
    <log4shib>cpp-log4shib</log4shib>
  </PropertyGroup>
  ```

Other versions seem fine...

Using `boost_1_74_0`: Download zipfile and unpack it in the `ROOT_DIR`. Header only is fine?

Need to retarget xmltooling and opensaml!

Add env variable `DEBUG_INSTALLER`, set it to `YES`

```cmd
nmake /f build.make saml64
```

### Collecting all headers and libraries

This is how I went about adding all libraries and headers to an install dir. Some parts I did in `git bash` because
I'm not very familiar with `cmd.exe`. This may not be necessary if you go through the painstaking process of setting
all of the correct include and lib directories when configuring Wt.

```cmd
mkdir install-dir
mkdir install-dir\lib
mkdir install-dir\bin
mkdir install-dir\include
copy %ROOT_DIR%\cpp-opensaml\Build\VC15\x64\Debug\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-opensaml\Build\VC15\x64\Debug\*.dll  %ROOT_DIR%\install-dir\bin\
copy %ROOT_DIR%\cpp-opensaml\Build\VC15\x64\Release\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-opensaml\Build\VC15\x64\Release\*.dll  %ROOT_DIR%\install-dir\bin\
```

```sh
cd cpp-opensaml/saml
for f in $(find . -name '*.h'); do dir=$(dirname $f); mkdir -p ../../install-dir/include/saml/$dir; cp $f ../../install-dir/include/saml/$dir/; done
cd ../..
```

```cmd
copy %ROOT_DIR%\cpp-xmltooling\Build\VC15\x64\Debug\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-xmltooling\Build\VC15\x64\Debug\*.dll  %ROOT_DIR%\install-dir\bin\
copy %ROOT_DIR%\cpp-xmltooling\Build\VC15\x64\Release\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-xmltooling\Build\VC15\x64\Release\*.dll  %ROOT_DIR%\install-dir\bin\
```

```sh
cd cpp-xmltooling/xmltooling
for f in $(find . -name '*.h'); do dir=$(dirname $f); mkdir -p ../../install-dir/include/xmltooling/$dir; cp $f ../../install-dir/include/xmltooling/$dir/; done
cd ../..
```

```cmd
copy %ROOT_DIR%\cpp-log4shib\msvc15\x64\Debug\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-log4shib\msvc15\x64\Debug\*.dll  %ROOT_DIR%\install-dir\bin\
copy %ROOT_DIR%\cpp-log4shib\msvc15\x64\Release\*.lib  %ROOT_DIR%\install-dir\lib\
copy %ROOT_DIR%\cpp-log4shib\msvc15\x64\Release\*.dll  %ROOT_DIR%\install-dir\bin\
```

```sh
cd cpp-log4shib/include
cp -r log4shib ../../install-dir/include
cd ../..
```

```cmd
copy %ROOT_DIR%\xml-security-c-2.0.2\Build\x64\VC15\"Debug Minimal"\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\xml-security-c-2.0.2\Build\x64\VC15\"Debug Minimal"\*.dll %ROOT_DIR%\install-dir\bin
copy %ROOT_DIR%\xml-security-c-2.0.2\Build\x64\VC15\"Release Minimal"\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\xml-security-c-2.0.2\Build\x64\VC15\"Release Minimal"\*.dll %ROOT_DIR%\install-dir\bin
```

```sh
cd xml-security-c-2.0.2/xsec
for f in $(find . -name '*.h' -or -name '*.hpp'); do dir=$(dirname $f); mkdir -p ../../install-dir/include/xsec/$dir; cp $f ../../install-dir/include/xsec/$dir/; done
cd ../..
```

```cmd
copy %ROOT_DIR%\xerces-c-3.2.3\Install64\VC15\lib\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\xerces-c-3.2.3\Install64\VC15\bin\*.dll %ROOT_DIR%\install-dir\bin
```

```sh
cp -r xerces-c-3.2.3/Install64/VC15/include/xercesc install-dir/include/
cp -r xerces-c-3.2.3/Install64/VC15/cmake install-dir/lib/
```

```cmd
copy %ROOT_DIR%\zlib-1.2.11\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\zlib-1.2.11\*.dll %ROOT_DIR%\install-dir\bin
copy %ROOT_DIR%\zlib-1.2.11\*.h %ROOT_DIR%\install-dir\include
```

```cmd
copy %ROOT_DIR%\curl-7.75.0\builds\libcurl-vc15-x64-debug-dll-ssl-dll-zlib-dll-ipv6-sspi\lib\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\curl-7.75.0\builds\libcurl-vc15-x64-debug-dll-ssl-dll-zlib-dll-ipv6-sspi\bin\*.dll %ROOT_DIR%\install-dir\bin
copy %ROOT_DIR%\curl-7.75.0\builds\libcurl-vc15-x64-release-dll-ssl-dll-zlib-dll-ipv6-sspi\lib\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\curl-7.75.0\builds\libcurl-vc15-x64-release-dll-ssl-dll-zlib-dll-ipv6-sspi\bin\*.dll %ROOT_DIR%\install-dir\bin
```

```cmd
copy %ROOT_DIR%\openssl-1.1.1j\*.lib %ROOT_DIR%\install-dir\lib
copy %ROOT_DIR%\openssl-1.1.1j\*.dll %ROOT_DIR%\install-dir\bin
copy %ROOT_DIR%\openssl-1.1.1j\x64Debug\bin\*.dll %ROOT_DIR%\install-dir\bin
```

```sh
cp -r openssl-1.1.1j/include/* install-dir/include/
```

## Configuring Wt

You need to set `ENABLE_SAML` to `ON` when configuring Wt. You can use `SAML_ROOT` to set the SAML prefix:

```shell
cmake path\to\sources -DENABLE_SAML=ON -DSAML_ROOT=path\to\saml\prefix
```
