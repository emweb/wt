CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE true)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0002 OLD)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(WT)

SET(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${WT_SOURCE_DIR}
  ${WT_SOURCE_DIR}/cmake
)

SET(VERSION_SERIES 3)
SET(VERSION_MAJOR 3)
SET(VERSION_MINOR 4)

SET(WT_SOVERSION 38)
SET(WTEXT_SOVERSION 38)
SET(WTHTTP_SOVERSION 38)
SET(WTFCGI_SOVERSION 38)
SET(WTISAPI_SOVERSION 14)
SET(WTDBO_SOVERSION 38)
SET(WTDBOSQLITE3_SOVERSION 38)
SET(WTDBOPOSTGRES_SOVERSION 38)
SET(WTDBOFIREBIRD_SOVERSION 38)
SET(WTDBOMYSQL_SOVERSION 38)
SET(WTTEST_SOVERSION 8)

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
  add_definitions(-DANDROID -DWT_NO_STD_WSTRING=ON -DWT_NO_STD_LOCALE=ON)
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
OPTION(INSTALL_EXAMPLES "Install examples (binaries and source)" OFF)
OPTION(INSTALL_RESOURCES "Install resources directory" ON)
OPTION(INSTALL_FINDWT_CMAKE_FILE "Install FindWt.cmake in systemwide cmake dir (in addition to CMAKE_INSTALL_PREFIX/cmake)" OFF)
OPTION(ENABLE_SSL "Enable cryptography functions, using OpenSSL" ON)
OPTION(ENABLE_HARU "Enable Haru Free PDF Library, which is used to provide support for painting to PDF (WPdfImage)" ON)
OPTION(ENABLE_PANGO "Enable Pango Library, which is used for improved font support (WPdfImage and WRasterImage)" ON)
OPTION(ENABLE_EXT "Build Wt Ext library with JavaScript-only widgets (http://extjs.com/) DEPRECATED" OFF)
OPTION(ENABLE_SQLITE "Build SQLite3 backend for Wt::Dbo" ON)
OPTION(ENABLE_POSTGRES "Build PostgreSQL backend for Wt::Dbo" ON)
OPTION(ENABLE_FIREBIRD "Build FirebirdSQL backend for Wt::Dbo" ON)
OPTION(ENABLE_MYSQL "Build mariadb/mysql backend for Wt::Dbo" ON)
OPTION(ENABLE_QT4 "Build Qt4 interworking library (libwtwithqt" ON)
OPTION(ENABLE_LIBWTTEST "Build Wt::Test" ON)
OPTION(ENABLE_LIBWTDBO "Build Wt::Dbo" ON)
OPTION(WT_NO_STD_LOCALE "Build Wt to run on a system without std::locale support" OFF)
OPTION(WT_NO_STD_WSTRING "Build Wt to run on a system without std::wstring support" OFF)
OPTION(ENABLE_OPENGL "Build Wt with support for server-side opengl rendering" ON)

# C++11 vs C++98
# Binary compatibility is not guaranteed. We give our users the choice on
# how to compile Wt
# We're not sure yet if you can safely link Wt compiled in 03 mode against
# an application compiled in 11 mode. Boost probably causes problems here
# (specially boost.signals2)
IF(NOT MSVC)
  # For now, don't auto-detect
  #IF(CMAKE_COMPILER_IS_GNUCXX)
  #  execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpversion"
  #                  OUTPUT_VARIABLE GCC_VERSION)
  #  IF(${GCC_VERSION} VERSION_GREATER "4.5.99")
  #    SET(HAS_CXX11 ON)
  #  ENDIF(${GCC_VERSION} VERSION_GREATER "4.5.99")
  #ENDIF(CMAKE_COMPILER_IS_GNUCXX)
  SET(WT_CPP_11_MODE "" CACHE STRING "C++ mode to compile Wt in (leave empty for your compiler's default, or set to -std=c++11, -std=c++0x, ...)")
  IF(WT_CPP_11_MODE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WT_CPP_11_MODE}")
    SET(HAS_CXX11 ON)
  ENDIF(WT_CPP_11_MODE)
ELSE(NOT MSVC)
  # For once, msvs is easier than gcc/llvm
  IF(MSVC_VERSION GREATER 1600)
    SET(HAS_CXX11 ON)
  ENDIF(MSVC_VERSION GREATER 1600)
ENDIF(NOT MSVC)

IF(APPLE)
  OPTION(USE_BOOST_FRAMEWORK "Uses a Boost framework" OFF)
ENDIF(APPLE)

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

  SET(RUNDIR "/var/run/wt" CACHE PATH
    "Default path for wt session management (only used by FCGI connector; not relative to CMAKE_INSTALL_PREFIX)")

  IF( NOT DEFINED CONFIGDIR )
    SET(CONFIGDIR "/etc/wt" CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )

  SET(USERLIB_PREFIX_DEFAULT "/usr")

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
SET(WTHTTP_CONFIGURATION ${CONFIGDIR}/wthttpd CACHE PATH "Path for the wthttpd configuration file")

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
INCLUDE(cmake/WtFindAsciidoc.txt)
INCLUDE(cmake/WtFindHaru.txt)
INCLUDE(cmake/WtFindGm.txt)
INCLUDE(cmake/WtFindGL.txt)
INCLUDE(cmake/WtFindSkia.txt)

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
      "c:/Program Files/boost/boost_1_38\n\n"
      "Note 4: if you do not have boost.random, set WT_NO_BOOST_RANDOM to ON"
      "Wt requires the following C++ boost libraries: date_time, regex, program_options, signals, random, and optionally thread")
  MESSAGE(FATAL_ERROR ${ERR})
ENDIF(NOT BOOST_WT_FOUND)

IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    MESSAGE("** Enabling multi threading.")
    SET(MULTI_THREADED_BUILD true)

    ADD_DEFINITIONS(-DWT_THREADED -D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
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

# decide on signals vs signals2
# boost 1.54 deprecated boost signals -> use signals2
IF (Boost_VERSION GREATER 105300)
  MESSAGE(STATUS "Boost ${Boost_VERSION} > 1.53, WT_SIGNALS_IMPLEMENTATION = boost.signals2 recommended")
  SET(DEFAULT_WT_SIGNALS_IMPLEMENTATION "boost.signals2")
ELSE (Boost_VERSION GREATER 105300)
  SET(DEFAULT_WT_SIGNALS_IMPLEMENTATION "boost.signals")
  MESSAGE(STATUS "Boost ${Boost_VERSION} < 1.54, WT_SIGNALS_IMPLEMENTATION = boost.signals recommended")
ENDIF (Boost_VERSION GREATER 105300)
SET(WT_SIGNALS_IMPLEMENTATION ${DEFAULT_WT_SIGNALS_IMPLEMENTATION} CACHE STRING "Select what implementation should be used for Wt signals")
IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  MESSAGE(STATUS "Informational: WT_SIGNALS_IMPLEMENTATION should be either boost.signals or boost.signals2")
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  SET_PROPERTY(CACHE WT_SIGNALS_IMPLEMENTATION PROPERTY STRINGS boost.signals boost.signals2)
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)


IF ("${WT_SIGNALS_IMPLEMENTATION}" STREQUAL "boost.signals")
  MESSAGE(STATUS "Selecting boost.signals")
  SET(WT_USE_BOOST_SIGNALS ON)
  SET(WT_USE_BOOST_SIGNALS2 OFF)
ELSEIF ("${WT_SIGNALS_IMPLEMENTATION}" STREQUAL "boost.signals2")
  MESSAGE(STATUS "Selecting boost.signals2")
  SET(WT_USE_BOOST_SIGNALS OFF)
  SET(WT_USE_BOOST_SIGNALS2 ON)
ENDIF ("${WT_SIGNALS_IMPLEMENTATION}" STREQUAL "boost.signals")


# decide on GraphicsMagick vs skia
# todo: set default to whatever was found
SET(WT_WRASTERIMAGE_DEFAULT_IMPLEMENTATION "none")
SET(WT_WRASTERIMAGE_IMPLEMENTATION ${WT_WRASTERIMAGE_DEFAULT_IMPLEMENTATION} CACHE STRING "Implementation for WRasterImage server-side rendering")
IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  MESSAGE(STATUS "Informational: WT_WRASTERIMAGE_IMPLEMENTATION should be either GraphicsMagick or skia")
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8)
  SET_PROPERTY(CACHE WT_WRASTERIMAGE_IMPLEMENTATION PROPERTY STRINGS GraphicsMagick skia none)
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
ELSE (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "GraphicsMagick")
    MESSAGE("** Disabled WRasterimage support. Set WT_WRASTERIMAGE_IMPLEMENTATION to GraphicsMagick or skia.")
ENDIF (${WT_WRASTERIMAGE_IMPLEMENTATION} STREQUAL "GraphicsMagick")

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

ADD_CUSTOM_TARGET(doc)

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

IF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR )
  SET( WT_CMAKE_FINDER_INSTALL_DIR "${CMAKE_ROOT}/Modules" )
ENDIF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR)

IF (INSTALL_FINDWT_CMAKE_FILE)
  INSTALL(FILES ${PROJECT_SOURCE_DIR}/cmake/FindWt.cmake DESTINATION
    ${WT_CMAKE_FINDER_INSTALL_DIR} )
ENDIF (INSTALL_FINDWT_CMAKE_FILE)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/cmake/FindWt.cmake DESTINATION
    ${CMAKE_INSTALL_PREFIX}/cmake )

IF(INSTALL_RESOURCES)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/Wt/)
ENDIF(INSTALL_RESOURCES)

IF(INSTALL_EXAMPLES)
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/examples/ DESTINATION
      ${CMAKE_INSTALL_PREFIX}/${EXAMPLES_DESTINATION} PATTERN "examples/*")
ENDIF(INSTALL_EXAMPLES)

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

