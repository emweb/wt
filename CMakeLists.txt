CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE true)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0002 OLD)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  if(POLICY CMP0022)
    # Stops CMake from warning about CMP0022, even though we don't actually use
    # the LINK_INTERFACE_LIBRARIES or INTERFACE_LINK_LIBRARIES (only indirectly
    # through PUBLIC/PRIVATE in TARGET_LINK_LIBRARIES)
    CMAKE_POLICY(SET CMP0022 NEW)
  endif(POLICY CMP0022)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(WT)

SET(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${WT_SOURCE_DIR}
  ${WT_SOURCE_DIR}/cmake
)

SET(VERSION_SERIES 4)
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 2)

SET(WT_SOVERSION 45)
SET(WTHTTP_SOVERSION 45)
SET(WTFCGI_SOVERSION 45)
SET(WTISAPI_SOVERSION 21)
SET(WTDBO_SOVERSION 45)
SET(WTDBOSQLITE3_SOVERSION 45)
SET(WTDBOPOSTGRES_SOVERSION 45)
SET(WTDBOFIREBIRD_SOVERSION 45)
SET(WTDBOMYSQL_SOVERSION 45)
SET(WTDBOMSSQLSERVER_SOVERSION 45)
SET(WTTEST_SOVERSION 15)

IF(NOT SHARED_LIBS)
  IF(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" OFF)
  ELSE(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" ON)
  ENDIF(WIN32)
ENDIF(NOT SHARED_LIBS)

IF(ANDROID)
  IF(NOT DEFINED ANDROID_SDK_DIR)
    MESSAGE(FATAL_ERROR 
    "Your configuration is missing the ANDROID_SDK_DIR variable. ")
  ENDIF(NOT DEFINED ANDROID_SDK_DIR)
  IF(NOT DEFINED ANDROID_NDK_TOOLS_DIR)
    MESSAGE(FATAL_ERROR 
    "Your configuration is missing the ANDROID_NDK_TOOLS_DIR variable. ")
  ENDIF(NOT DEFINED ANDROID_NDK_TOOLS_DIR)
  IF(NOT DEFINED ANDROID_STAGING_DIR)	
    MESSAGE(FATAL_ERROR 
    "Your configuration is missing the ANDROID_STAGING_DIR variable. ")
  ENDIF(NOT DEFINED ANDROID_STAGING_DIR)
  IF(NOT DEFINED ANDROID_SDK_TARGET_ID)	
    MESSAGE(FATAL_ERROR 
    "Your configuration is missing the ANDROID_SDK_TARGET_ID variable. ")
  ENDIF(NOT DEFINED ANDROID_SDK_TARGET_ID)

  SET(SHARED_LIBS OFF)
  add_definitions(-DANDROID) # TODO(Roel): WT_NO_STD_WSTRING/WT_NO_STD_LOCALE?
ENDIF(ANDROID)

#
# Ubuntu patched this compiler to hell
# gcc (Ubuntu/Linaro 4.4.4-14ubuntu5) 4.4.5
#
EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
  ARGS --version
  OUTPUT_VARIABLE GCC_COMPILER_VERSION
)

IF(GCC_COMPILER_VERSION MATCHES ".*4\\.4\\.4\\-14ubuntu5.*")
  MESSAGE(" ")
  MESSAGE("    !!!!! WARNING Your compiler is BUGGY. !!!!! ")
  MESSAGE(" ")
  MESSAGE("    If possible, upgrade your compiler to e.g. g++ 4.5:")
  MESSAGE(" ")
  MESSAGE("       $ sudo apt-get install g++-4.5")
  MESSAGE(" ")
  MESSAGE("    And build using that compiler cmake -DCMAKE_CXX_COMPILER=g++-4.5")
  MESSAGE(" ")
  MESSAGE("    We will now disable all assertions as a work around, by")
  MESSAGE("    building using -DNDEBUG. You will need to define this")
  MESSAGE("    also for programs built using Wt")
  MESSAGE(" ")
  ADD_DEFINITIONS(-DNDEBUG)
ENDIF(GCC_COMPILER_VERSION MATCHES ".*4\\.4\\.4\\-14ubuntu5.*")

#
# Various things that must be configured by the user or packager ...
#
OPTION(BUILD_EXAMPLES "Build examples" ON)
OPTION(INSTALL_DOCUMENTATION "Install documentation (tutorials and reference)" OFF)
OPTION(INSTALL_EXAMPLES "Install examples (binaries and source)" OFF)
OPTION(INSTALL_RESOURCES "Install resources directory" ON)
OPTION(ENABLE_SSL "Enable cryptography functions, using OpenSSL" ON)
OPTION(ENABLE_HARU "Enable Haru Free PDF Library, which is used to provide support for painting to PDF (WPdfImage)" ON)
OPTION(ENABLE_PANGO "Enable Pango Library, which is used for improved font support (WPdfImage and WRasterImage)" ON)
OPTION(ENABLE_EXT "Build Wt Ext library with JavaScript-only widgets (http://extjs.com/) DEPRECATED" OFF)
OPTION(ENABLE_SQLITE "Build SQLite3 backend for Wt::Dbo" ON)
OPTION(ENABLE_POSTGRES "Build PostgreSQL backend for Wt::Dbo" ON)
OPTION(ENABLE_FIREBIRD "Build FirebirdSQL backend for Wt::Dbo" ON)
OPTION(ENABLE_MYSQL "Build mariadb/mysql backend for Wt::Dbo" ON)
OPTION(ENABLE_MSSQLSERVER "Build Microsoft SQL Server backend for Wt::Dbo" ON)
OPTION(ENABLE_QT4 "Build Qt4 interworking library (libwtwithqt" ON)
OPTION(ENABLE_LIBWTTEST "Build Wt::Test" ON)
OPTION(ENABLE_LIBWTDBO "Build Wt::Dbo" ON)
OPTION(WT_NO_STD_LOCALE "Build Wt to run on a system without std::locale support" OFF)
OPTION(WT_NO_STD_WSTRING "Build Wt to run on a system without std::wstring support" OFF)
OPTION(ENABLE_OPENGL "Build Wt with support for server-side opengl rendering" ON)
OPTION(WT_WARN_HEADER_MISSING_H "Output a warning if a Wt header without .h extension is included (deprecated since Wt 4.0.0)" ON)
OPTION(INSTALL_EXTENSIONLESS_HEADERS "Extensionless headers were deprecated in Wt 4.0.0. Disabling this option will cause extensionless headers not to be installed" ON)
OPTION(ENABLE_UNWIND "Build Wt with stacktrace support using libunwind" OFF)

IF(NOT CMAKE_CXX_STANDARD)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF(NOT CMAKE_CXX_STANDARD)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF(MSVC)

IF(APPLE)
  OPTION(USE_BOOST_FRAMEWORK "Uses a Boost framework" OFF)
ENDIF(APPLE)

IF(NOT DOCUMENTATION_DESTINATION)
  SET(DOCUMENTATION_DESTINATION share/doc/Wt)
ENDIF(NOT DOCUMENTATION_DESTINATION)
IF(NOT EXAMPLES_DESTINATION)
  SET(EXAMPLES_DESTINATION lib/Wt/examples)
ENDIF(NOT EXAMPLES_DESTINATION)

IF(WIN32)
  # required for all that uses boost.asio
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)

  IF(SHARED_LIBS)
    # See http://svn.boost.org/trac/boost/ticket/3465
    SET(WT_NO_BOOST_INTRUSIVE true)
  ENDIF(SHARED_LIBS)
ENDIF(WIN32)

# Fixup Windows declspec stuff
IF(NOT SHARED_LIBS)
  SET(WT_STATIC true)
  SET(WTHTTP_STATIC true)
  SET(WT_EXT_STATIC true)
  SET(WTDBO_STATIC true)
  SET(WTDBOSQLITE3_STATIC true)
  SET(WTDBOPOSTGRES_STATIC true)
  SET(WTDBOFIREBIRD_STATIC true)
  SET(WTDBOMYSQL_STATIC true)
  SET(WTDBOMSSQLSERVER_STATIC true)
ENDIF(NOT SHARED_LIBS)

IF(NOT MULTI_THREADED)
  OPTION(MULTI_THREADED "Build multi-threaded httpd deamon (if possible)" ON)
ENDIF(NOT MULTI_THREADED)

SET(BUILD_SHARED_LIBS ${SHARED_LIBS})

# Default is to use cmake's boost discovery. The default will use wt's own
# boost detection mechanism if one of the following is true:
# - cmake version is too old (prior to 2.6 series)
# - BOOST_COMPILER or BOOST_VERSION was defined (compatibility with existing
#   build scripts)
# - USE_BOOST_FRAMEWORK: we've adapted the vintage boost cmake find
#   script to locate a boost.framework
#
IF(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION OR USE_BOOST_FRAMEWORK)
  SET(DEFAULT_WT_BOOST_DISCOVERY TRUE)
ELSE(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION OR USE_BOOST_FRAMEWORK)
  SET(DEFAULT_WT_BOOST_DISCOVERY FALSE)
ENDIF(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION OR USE_BOOST_FRAMEWORK)

# There's no decent boost discovery prior to cmake 2.6
IF(CMAKE_MAJOR_VERSION LESS 2)
  SET(DEFAULT_WT_BOOST_DISCOVERY TRUE)
ELSEIF(CMAKE_MAJOR_VERSION EQUAL 2)
    IF(CMAKE_MINOR_VERSION LESS 6)
      SET(DEFAULT_WT_BOOST_DISCOVERY TRUE)
    ELSEIF(CMAKE_MINOR_VERSION EQUAL 6)
      IF(CMAKE_TINY_VERSION LESS 2)
        # Pau says: 2.6.0 and 2.6.1 are not very good
        SET(DEFAULT_WT_BOOST_DISCOVERY TRUE)
      ENDIF(CMAKE_TINY_VERSION LESS 2)
    ENDIF(CMAKE_MINOR_VERSION LESS 6)
ENDIF(CMAKE_MAJOR_VERSION LESS 2)

SET(WT_BOOST_DISCOVERY ${DEFAULT_WT_BOOST_DISCOVERY} CACHE BOOL "Use Wt's boost discovery method rather than the cmake 2.6+ method")

SET(DEBUG_LIB_POSTFIX "d" CACHE STRING "String appended to debug libraries")

SET(LIB_INSTALL_DIR "lib" CACHE STRING
  "Name for library directory within ${CMAKE_INSTALL_PREFIX}")

SET(CMAKE_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake" CACHE STRING
  "Name for CMake package configuration directory within ${CMAKE_INSTALL_PREFIX}")

IF(WIN32)

  SET(RUNDIR "c:/witty") # Does not apply to win32

  IF(NOT DEFINED CONFIGDIR)
    SET(CONFIGDIR ${RUNDIR} CACHE STRING "Path for the configuration files")
  ENDIF(NOT DEFINED CONFIGDIR)

  SET(USERLIB_PREFIX_DEFAULT "c:/libraries")
  IF(MSVC)
    SET(BUILD_PARALLEL "/MP" CACHE STRING "MSVC option for parallel builds (/MP or /MPx)")
  ENDIF(MSVC)

ELSE(WIN32)

  IF("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
    SET(CONFIG_PREFIX "")
  ELSEIF("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    SET(CONFIG_PREFIX "")
  ELSE()
    SET(CONFIG_PREFIX ${CMAKE_INSTALL_PREFIX})
  ENDIF()

  SET(RUNDIR "${CONFIG_PREFIX}/var/run/wt" CACHE PATH
    "Default path for wt session management (only used by FCGI connector)")

  IF(NOT DEFINED CONFIGDIR)
    SET(CONFIGDIR "${CONFIG_PREFIX}/etc/wt" CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )

  # If the user specifies the standard CMAKE_PREFIX_PATH to find packages,
  # honour it.
  IF(CMAKE_PREFIX_PATH)
    SET(USERLIB_PREFIX_DEFAULT ${CMAKE_PREFIX_PATH})
  ELSE()
    SET(USERLIB_PREFIX_DEFAULT "/usr")
  ENDIF()

ENDIF(WIN32)

IF(DEFINED USERLIB_ROOT) # Deprecated <= 3.1.3
  SET(USERLIB_PREFIX ${USERLIB_ROOT} CACHE PATH
    "Installation prefix of dependency libraries (by USERLIB_ROOT)")
ELSE(DEFINED USERLIB_ROOT)
  SET(USERLIB_PREFIX ${USERLIB_PREFIX_DEFAULT} CACHE PATH
    "Installation prefix of dependency libraries")
ENDIF(DEFINED USERLIB_ROOT)

IF(WIN32)
  SET(BOOST_PREFIX_DEFAULT "c:/Program Files/Boost")
  OPTION(BOOST_DYNAMIC "Link to boost DLLs (OFF means static link)" OFF)
  OPTION(HARU_DYNAMIC "Link to libharu DLLs (OFF means static link)" OFF)
ELSE(WIN32)
  SET(BOOST_PREFIX_DEFAULT ${USERLIB_PREFIX})
ENDIF(WIN32)

IF(DEFINED BOOST_DIR) # Deprecated <= 3.1.3
  SET(BOOST_PREFIX ${BOOST_DIR} CACHE PATH
    "Installation prefix of boost libraries (by BOOST_DIR)")
ELSE(DEFINED BOOST_DIR)
  SET(BOOST_PREFIX ${BOOST_PREFIX_DEFAULT} CACHE PATH
    "Installation prefix of boost libraries")
ENDIF(DEFINED BOOST_DIR)

SET(FCGI_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of fcgi library (overrides USERLIB_PREFIX)")
SET(POSTGRES_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of postgresql library (overrides USERLIB_PREFIX)")
SET(ODBC_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of ODBC (unixODBC), required for MS SQL Server, not necessary on Windows (overrides USERLIB_PREFIX)")
SET(FIREBIRD_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of FirebirdSql library. (overrides USERLIB_PREFIX)")
SET(IBPP_SRC_DIRECTORY CACHE PATH
  "Path to the SOURCE directory of the IBPP library (C++ Firebird client library, see ibpp.org), Wt includes an ibpp distribution which is used by default. To use a custom IBPP library configure this variable and enable the USE_SYSTEM_IBPP option.")
SET(MYSQL_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of mariadb client libraries (overrides USERLIB_PREFIX)")
SET(SQLITE3_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of sqlite3 library (overrides USERLIB_PREFIX)")
SET(HARU_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of libharu library (overrides USERLIB_PREFIX)")
SET(SSL_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of SSL library (overrides USERLIB_PREFIX)")
SET(ZLIB_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of zlib library (overrides USERLIB_PREFIX)")
SET(GM_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of GraphicsMagick library (overrides USERLIB_PREFIX)")
SET(SKIA_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Prefix of skia library (overrides USERLIB_PREFIX)")
SET(ASIO_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Prefix of Asio (overrides USERLIB_PREFIX), only used when WT_ASIO_IMPLEMENTATION is standalone")
SET(UNWIND_PREFIX ${UNWIND_PREFIX} CACHE PATH
  "Prefix of unwind library (overrides USERLIB_PREFIX)")

OPTION(DEBUG "Support for debugging, must be enabled also in wt_config.xml" OFF)

IF(CYGWIN)
  OPTION(BUILD_TESTS "Build Wt tests" OFF)
ELSE(CYGWIN)
  OPTION(BUILD_TESTS "Build Wt tests" ON)
ENDIF(CYGWIN)

ADD_DEFINITIONS(-DWT_WITH_OLD_INTERNALPATH_API)
IF(CYGWIN)
  ADD_DEFINITIONS(-D__USE_W32_SOCKETS)
ENDIF(CYGWIN)

MARK_AS_ADVANCED( CONFIGDIR )

SET(CONFIGURATION "${CONFIGDIR}/wt_config.xml" CACHE PATH "Path to the wt configuration file")
SET(WTHTTP_CONFIGURATION "${CONFIGDIR}/wthttpd" CACHE PATH "Path for the wthttpd configuration file")

SET(WEBUSER apache CACHE STRING "Webserver username (e.g. apache or www)")
SET(WEBGROUP apache CACHE STRING "Webserver groupname (e.g. apache or www or users)")
IF(WIN32)
  SET(CONNECTOR_FCGI FALSE)
  IF(NOT MINGW)
    OPTION(CONNECTOR_ISAPI "Compile in ISAPI connector (libwtisapi) ?" ON)
  ENDIF(NOT MINGW)
ELSE(WIN32)
  OPTION(CONNECTOR_FCGI "Compile in FCGI connector (libwtfcgi) ?" ON)
  SET(CONNECTOR_ISAPI OFF)
ENDIF(WIN32)
OPTION(CONNECTOR_HTTP "Compile in stand-alone httpd connector (libwthttp) ?" ON)
SET(EXAMPLES_CONNECTOR wthttp CACHE STRING "Connector used for examples")

include (CheckSymbolExists)

INCLUDE(cmake/WtFindBoost.txt)
INCLUDE(cmake/WtFindFcgi.txt)
INCLUDE(cmake/WtFindZlib.txt)
INCLUDE(cmake/WtFindPng.txt)
INCLUDE(cmake/WtFindSsl.txt)
INCLUDE(cmake/WtFindMysql.txt)
INCLUDE(cmake/WtFindPostgresql.txt)
INCLUDE(cmake/WtFindOdbc.txt)
INCLUDE(cmake/WtFindAsciidoc.txt)
INCLUDE(cmake/WtFindHaru.txt)
INCLUDE(cmake/WtFindGm.txt)
INCLUDE(cmake/WtFindGL.txt)
INCLUDE(cmake/WtFindSkia.txt)

IF (ENABLE_UNWIND)
  INCLUDE(cmake/WtFindUnwind.txt)
ENDIF (ENABLE_UNWIND)

IF (ENABLE_PANGO)
  INCLUDE(cmake/WtFindPangoFt2.txt)
ENDIF (ENABLE_PANGO)

INCLUDE(cmake/WtFindSystemLibraries.txt)

IF(ENABLE_QT4)
  FIND_PACKAGE(Qt4)

  IF(QT_FOUND)
    INCLUDE(${QT_USE_FILE})
  ENDIF(QT_FOUND)
ENDIF(ENABLE_QT4)

INCLUDE(FindThreads)

IF(NOT BOOST_WT_FOUND)
  SET(ERR
     "Could not find a boost installation in " ${BOOST_PREFIX} ".\n\n"
      "There are two methods in Wt to find boost:\n\n"
      "1. Find boost through cmake (recommended)\n"
      "This requires cmake 2.6, and is in "
      "that case the default (unless BOOST_COMPILER or BOOST_VERSION is "
      "defined). This method requires a multi-threaded boost installation.\n"
      "You may need to add your boost version number to "
      "Boost_ADDITIONAL_VERSIONS, and/or set BOOST_PREFIX to the location "
      "where boost is installed.\n\n"
      "2. Use the Wt-proprietary method to find boost.\n"
      "This requires you to define three variables:\n"
      "BOOST_PREFIX: set to the directory where you installed boost\n"
      "BOOST_COMPILER: set to the compiler signature as you find them in "
      "library names (e.g.: gcc42)\n"
      "BOOST_VERSION: set to the boost version, again as you find them in "
      "the library names (e.g.: 1_38)\n\n"
      "Note 1: WT_BOOST_DISCOVERY is the variable that selects the boost "
      "discovery method. When set to false, method 1 is used (default if "
      "cmake version is recent enough and BOOST_COMPILER and BOOST_VERSION "
      "are not defined). When set to true, method 2 is used.\n"
      "Note 2: the code to discover boost is located in cmake/WtFindBoost.txt\n"
      "Note 3: on windows, set BOOST_PREFIX to the full path, eg "
      "c:/Program Files/boost/boost_1_65_1")
  MESSAGE(FATAL_ERROR ${ERR})
ENDIF(NOT BOOST_WT_FOUND)

IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    MESSAGE("** Enabling multi threading.")
    SET(MULTI_THREADED_BUILD true)
    SET(WT_THREADED true)
    ADD_DEFINITIONS(-D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
  ELSE(MULTI_THREADED)
    MESSAGE("** Disabling multi threading.")
    SET(MULTI_THREADED_BUILD false)

    ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS -DSQLITE_THREADSAFE=0)
  ENDIF(MULTI_THREADED)
ELSE(BOOST_WT_MT_FOUND)
  SET(MULTI_THREADED_BUILD false)
  MESSAGE("** Disabling multi threading: could not find multi-threaded boost libraries")
  ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS -DSQLITE_THREADSAFE=0)
ENDIF(BOOST_WT_MT_FOUND)

# decide on GraphicsMagick vs skia
# todo: set default to whatever was found
IF (WIN32 AND NOT MINGW)
  SET(WT_WRASTERIMAGE_DEFAULT_IMPLEMENTATION "Direct2D")
ELSE (WIN32 AND NOT MINGW)
  SET(WT_WRASTERIMAGE_DEFAULT_IMPLEMENTATION "none")
ENDIF (WIN32 AND NOT MINGW)

SET(WT_WRASTERIMAGE_IMPLEMENTATION ${WT_WRASTERIMAGE_DEFAULT_IMPLEMENTATION} CACHE STRING "Implementation for WRasterImage server-side rendering")
IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  MESSAGE(STATUS "Informational: WT_WRASTERIMAGE_IMPLEMENTATION should be either GraphicsMagick, skia, or Direct2D")
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  SET_PROPERTY(CACHE WT_WRASTERIMAGE_IMPLEMENTATION PROPERTY STRINGS GraphicsMagick skia Direct2D none)
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)

IF (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "GraphicsMagick")
  IF (NOT GM_FOUND)
    MESSAGE(FATAL_ERROR "WT_WRASTERIMAGE_IMPLEMENTATION set to GraphicsMagick but GM is not found. Indicate the location of your graphicsmagick library using -DGM_PREFIX=...")
  ENDIF (NOT GM_FOUND)
  SET(WT_HAS_WRASTERIMAGE true)
ELSEIF (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "skia")
  IF (NOT SKIA_FOUND)
    MESSAGE(FATAL_ERROR "WT_WRASTERIMAGE_IMPLEMENTATION set to skia but skia is not found. Indicate the location of your skia library using -DSKIA_PREFIX=...")
  ENDIF (NOT SKIA_FOUND)
  SET(WT_HAS_WRASTERIMAGE true)
ELSEIF (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "Direct2D")
  IF (WIN32 AND NOT MINGW)
    SET(WT_HAS_WRASTERIMAGE true)
  ENDIF (WIN32 AND NOT MINGW)
ELSE (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "GraphicsMagick")
    MESSAGE("** Disabled WRasterimage support. Set WT_WRASTERIMAGE_IMPLEMENTATION to GraphicsMagick, skia, or Direct2D.")
ENDIF (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "GraphicsMagick")

SET(WT_CPP17_ANY_DEFAULT_IMPLEMENTATION "thelink2012")
SET(WT_CPP17_ANY_IMPLEMENTATION ${WT_CPP17_ANY_DEFAULT_IMPLEMENTATION} CACHE STRING
  "Implementation for Wt::cpp17::any. Defaults to \"thelink2012\": an included implementation of any by GitHub user thelink2012. Use \"std\" for std::any, or \"experimental\" for std::experimental::any, if compiler support is available. You may have to change CMAKE_CXX_STANDARD to enable C++17 support in the compiler.")
   
IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  MESSAGE(STATUS "Informational: WT_CPP17_ANY_IMPLEMENTATION should be thelink2012, experimental, or std")
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  SET_PROPERTY(CACHE WT_CPP17_ANY_IMPLEMENTATION PROPERTY STRINGS thelink2012 experimental std)
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)

IF (${WT_CPP17_ANY_IMPLEMENTATION} STREQUAL "thelink2012")
  SET(WT_ANY_IS_THELINK2012_ANY ON)
ELSEIF (${WT_CPP17_ANY_IMPLEMENTATION} STREQUAL "experimental")
  SET(WT_ANY_IS_EXPERIMENTAL_ANY ON)
ELSEIF (${WT_CPP17_ANY_IMPLEMENTATION} STREQUAL "std")
  SET(WT_ANY_IS_STD_ANY ON)
ELSE (${WT_CPP17_ANY_IMPLEMENTATION} STREQUAL "thelink2012")
  MESSAGE(FATAL_ERROR "WT_CPP17_ANY_IMPLEMENTATION must be one of thelink2012, experimental, or std")
ENDIF (${WT_CPP17_ANY_IMPLEMENTATION} STREQUAL "thelink2012")

SET(WT_ASIO_DEFAULT_IMPLEMENTATION "boost")
SET(WT_ASIO_IMPLEMENTATION ${WT_ASIO_DEFAULT_IMPLEMENTATION} CACHE STRING
"Implementation of Asio to use. Defaults to \"boost\". Use \"standalone\" and provide its path in ASIO_PREFIX to use the standalone version.")

IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  MESSAGE(STATUS "Informational: WT_ASIO_IMPLEMENTATION should be boost, or standalone")
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  SET_PROPERTY(CACHE WT_ASIO_IMPLEMENTATION PROPERTY STRINGS boost standalone)
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)

IF (${WT_ASIO_IMPLEMENTATION} STREQUAL "boost")
  SET(WT_ASIO_IS_BOOST_ASIO ON)
ELSEIF(${WT_ASIO_IMPLEMENTATION} STREQUAL "standalone")
  SET(WT_ASIO_IS_STANDALONE_ASIO ON)
ELSE(${WT_ASIO_IMPLEMENTATION} STREQUAL "boost")
  MESSAGE(FATAL_ERROR "WT_ASIO_IMPLEMENTATION must be boost or standalone")
ENDIF(${WT_ASIO_IMPLEMENTATION} STREQUAL "boost")

IF(WT_ASIO_IS_STANDALONE_ASIO)
IF(NOT EXISTS "${ASIO_PREFIX}/include/asio.hpp")
MESSAGE(FATAL_ERROR "WT_ASIO_IMPLEMENTATION is set to standalone, but Asio was not found in ASIO_PREFIX")
ENDIF(NOT EXISTS "${ASIO_PREFIX}/include/asio.hpp")
INCLUDE_DIRECTORIES("${ASIO_PREFIX}/include")
ENDIF(WT_ASIO_IS_STANDALONE_ASIO)

FIND_PACKAGE(Doxygen)

# Boost is used nearly everywhere, so we can put these here
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
LINK_DIRECTORIES(${BOOST_LIB_DIRS})
IF(WIN32)
  IF(BOOST_DYNAMIC)
    ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
  ELSE(BOOST_DYNAMIC)
    # You could expect that this is the default when BOOST_ALL_DYN_LINK
    # is not set, but this is problematic for cygwin
    ADD_DEFINITIONS(-DBOOST_THREAD_USE_LIB)
  ENDIF(BOOST_DYNAMIC)
  IF(HARU_DYNAMIC)
    ADD_DEFINITIONS(-DHPDF_DLL)
  ENDIF(HARU_DYNAMIC)
  ADD_DEFINITIONS(
      -D_CRT_SECURE_NO_WARNINGS
      -D_SCL_SECURE_NO_WARNINGS
      )
ENDIF(WIN32)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(DEBUG)
  SET(WT_DEBUG_ENABLED 1)
ELSE(DEBUG)
  SET(WT_DEBUG_ENABLED 0)
ENDIF(DEBUG)

IF(INSTALL_DOCUMENTATION)
  ADD_CUSTOM_TARGET(doc ALL)
ELSE(INSTALL_DOCUMENTATION)
  ADD_CUSTOM_TARGET(doc)
ENDIF(INSTALL_DOCUMENTATION)

IF (ASCIIDOC_FOUND)
  MACRO (ASCIIDOC_FILE target infile outfile)
    ADD_CUSTOM_TARGET(${target}
       ${ASCIIDOC_EXECUTABLE} -a toc -a numbered -a icons -a  theme=emweb -a pygments -a linkcss -o ${outfile} ${infile}
       COMMENT "Asciidoc ${infile}")
    ADD_DEPENDENCIES(doc ${target})
  ENDMACRO (ASCIIDOC_FILE)
ENDIF (ASCIIDOC_FOUND)

IF (DOXYGEN_FOUND)
  ADD_CUSTOM_TARGET(doxygen
    ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Doxygen ...")
  ADD_DEPENDENCIES(doc doxygen)
  ADD_CUSTOM_TARGET(doxygen-examples
    ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples
    COMMENT "Doxygen for examples ...")
  ADD_DEPENDENCIES(doc doxygen-examples)
ENDIF (DOXYGEN_FOUND)

# we enable rpath support for APPLE, this probably goes against policy
# linux distributions?
IF(APPLE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  ENDIF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_MACOSX_RPATH TRUE)
ENDIF(APPLE)

SUBDIRS(src)

IF(BUILD_EXAMPLES)
  IF(WIN32)
    SUBDIRS(examples)
  ELSE(WIN32)
    IF (INSTALL_EXAMPLES)
      SUBDIRS(examples)
    ELSE (INSTALL_EXAMPLES)
      SUBDIRS(EXCLUDE_FROM_ALL examples)
    ENDIF (INSTALL_EXAMPLES)
  ENDIF(WIN32)
ENDIF(BUILD_EXAMPLES)

IF(BUILD_TESTS)
  SUBDIRS(test)
ENDIF(BUILD_TESTS)

IF(INSTALL_RESOURCES)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/Wt/)
ENDIF(INSTALL_RESOURCES)

IF(INSTALL_EXAMPLES)
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/examples/ DESTINATION
      ${CMAKE_INSTALL_PREFIX}/${EXAMPLES_DESTINATION} PATTERN "examples/*")
ENDIF(INSTALL_EXAMPLES)

IF(INSTALL_DOCUMENTATION)
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/reference/html DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_DESTINATION}/reference)
  INSTALL(FILES ${PROJECT_SOURCE_DIR}/doc/reference/wt.qch DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_DESTINATION}/reference)
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/examples/html DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_DESTINATION}/examples)
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/tutorial DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_DESTINATION})
ENDIF(INSTALL_DOCUMENTATION)

IF(NOT EXISTS ${CONFIGDIR}/wt_config.xml)
  INSTALL(FILES ${WT_BINARY_DIR}/wt_config.xml DESTINATION ${CONFIGDIR})
ENDIF (NOT EXISTS ${CONFIGDIR}/wt_config.xml)

IF(ENABLE_HARU AND HARU_FOUND)
  SET(HAVE_HARU ON)
  SET(WT_HAS_WPDFIMAGE true)
ENDIF(ENABLE_HARU AND HARU_FOUND)

IF(ENABLE_SSL AND SSL_FOUND)
  SET(HAVE_SSL ON)
  SET(WT_WITH_SSL true)
ENDIF(ENABLE_SSL AND SSL_FOUND)

IF(ENABLE_PANGO AND PANGO_FT2_FOUND)
  SET(HAVE_PANGO ON)
ENDIF(ENABLE_PANGO AND PANGO_FT2_FOUND)

IF(ENABLE_OPENGL AND GL_FOUND AND WT_HAS_WRASTERIMAGE)
  SET(WT_USE_OPENGL TRUE)
  SET(HAVE_GL TRUE)
ENDIF(ENABLE_OPENGL AND GL_FOUND AND WT_HAS_WRASTERIMAGE)

IF(ENABLE_UNWIND AND UNWIND_FOUND)
  SET(HAVE_UNWIND ON)
ENDIF(ENABLE_UNWIND AND UNWIND_FOUND)

# Compile time constants & make sure our build finds it
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Wt)
SET(WCONFIG_H_PATH ${CMAKE_CURRENT_BINARY_DIR}/Wt/WConfig.h)
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/WConfig.h.in
  ${WCONFIG_H_PATH}
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INSTALL_FILES(/include/Wt FILES ${WCONFIG_H_PATH})

# Generate wt_config.xml from wt_config.xml.in
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/wt_config.xml.in
  ${WT_BINARY_DIR}/wt_config.xml
)

# CMake package configuration
IF (${WT_ASIO_IMPLEMENTATION} STREQUAL "boost" OR (${Boost_USE_STATIC_LIBS}))
  SET(_WTCONFIG_CMAKE_FIND_BOOST TRUE)
ELSE()
  SET(_WTCONFIG_CMAKE_FIND_BOOST FALSE)
ENDIF()
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/wt-config.cmake.in
  ${WT_BINARY_DIR}/wt-config.cmake
  @ONLY
)
UNSET(_WTCONFIG_CMAKE_FIND_BOOST)
INSTALL(FILES
  ${WT_BINARY_DIR}/wt-config.cmake
  DESTINATION ${CMAKE_INSTALL_DIR}/wt)
