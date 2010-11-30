CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE true)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0002 OLD)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(WT)
SET(CMAKE_MODULE_PATH ${WT_SOURCE_DIR} ${WT_SOURCE_DIR}/cmake)

SET(VERSION_SERIES 3)
SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 7a)

SET(WT_SOVERSION 26)
SET(WTEXT_SOVERSION 26)
SET(WTHTTP_SOVERSION 26)
SET(WTFCGI_SOVERSION 26)
SET(WTISAPI_SOVERSION 4)
SET(WTDBO_SOVERSION 26)
SET(WTDBOSQLITE3_SOVERSION 26)
SET(WTDBOPOSTGRES_SOVERSION 26)

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
OPTION(INSTALL_RESOURCES "Install resources directory" ON)
OPTION(ENABLE_GM "Enable GraphicsMagick, for supporting painting to raster images (PNG, GIF, ...) (WRasterImage)" ON)
OPTION(ENABLE_HARU "Enable Haru Free PDF Library, which is used to provide support for painting to PDF (WPdfImage)" ON)
OPTION(ENABLE_EXT "Build Wt Ext library with JavaScript-only widgets (http://extjs.com/)" ON)
OPTION(ENABLE_SQLITE "Build SQLite3 backend for Wt::Dbo" ON)
OPTION(ENABLE_POSTGRES "Build PostgreSQL backend for Wt::Dbo" ON)
OPTION(WT_NO_STD_LOCALE "Build Wt ro run on a system without std::locale support" OFF)
OPTION(WT_NO_STD_WSTRING "Build Wt ro run on a system without std::wstring support" OFF)

IF(NOT SHARED_LIBS)
  IF(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" OFF)
  ELSE(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" ON)
  ENDIF(WIN32)
ENDIF(NOT SHARED_LIBS)

IF(WIN32)
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
IF(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION)
  SET(DEFAULT_WT_BOOST_DISCOVERY TRUE)
ELSE(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION)
  SET(DEFAULT_WT_BOOST_DISCOVERY FALSE)
ENDIF(DEFINED BOOST_COMPILER OR DEFINED BOOST_VERSION)
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
SET(MYSQL_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of mysql and mysql++ libraries "
  "(overrides USERLIB_PREFIX)")
SET(SQLITE3_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of sqlite3 library (overrides USERLIB_PREFIX)")
SET(HARU_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of libharu library (overrides USERLIB_PREFIX)")
SET(SSL_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of SSL library (overrides USERLIB_PREFIX)")
SET(ZLIB_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of zlib library (overrides USERLIB_PREFIX)")
SET(GM_PREFIX ${USERLIB_PREFIX} CACHE PATH
  "Installation prefix of GraphicsMgick library (overrides GM_PREFIX)")

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

INCLUDE(cmake/WtFindBoost.txt)
INCLUDE(cmake/WtFindFcgi.txt)
INCLUDE(cmake/WtFindZlib.txt)
INCLUDE(cmake/WtFindSsl.txt)
INCLUDE(cmake/WtFindMysqlpp.txt)
INCLUDE(cmake/WtFindPostgresql.txt)
INCLUDE(cmake/WtFindAsciidoc.txt)
INCLUDE(cmake/WtFindHaru.txt)
INCLUDE(cmake/WtFindGm.txt)

FIND_PACKAGE(Qt4)
IF(QT_FOUND)
  INCLUDE(${QT_USE_FILE})
ENDIF(QT_FOUND)

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
      "Wt requires the following C++ boost libraries: date_time, regex, program_options, signals, and optionally thread")
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
  ADD_DEFINITIONS(
      -D_CRT_SECURE_NO_WARNINGS
      -Dinline=__inline
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
       ${ASCIIDOC_EXECUTABLE} -a toc -a numbered -o ${outfile} ${infile}
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

SUBDIRS(src)

IF(BUILD_EXAMPLES)
  IF(WIN32)
    SUBDIRS(examples)
  ELSE(WIN32)
    SUBDIRS(EXCLUDE_FROM_ALL examples)
  ENDIF(WIN32)
ENDIF(BUILD_EXAMPLES)

IF(BUILD_TESTS)
  SUBDIRS(test)
ENDIF(BUILD_TESTS)

IF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR )
  SET( WT_CMAKE_FINDER_INSTALL_DIR "share/cmake-2.4/Modules" )
ENDIF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/cmake/FindWt.cmake DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${WT_CMAKE_FINDER_INSTALL_DIR} )

IF(INSTALL_RESOURCES)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/Wt/)
ENDIF(INSTALL_RESOURCES)

IF(NOT EXISTS ${DESTDIR}${CONFIGDIR}/wt_config.xml)
  INSTALL(FILES ${WT_BINARY_DIR}/wt_config.xml DESTINATION ${CONFIGDIR})
ENDIF (NOT EXISTS ${DESTDIR}${CONFIGDIR}/wt_config.xml)

IF(ENABLE_HARU AND HARU_FOUND)
  SET(HAVE_HARU ON)
  SET(WT_HAS_WPDFIMAGE true)
ENDIF(ENABLE_HARU AND HARU_FOUND)

IF(ENABLE_GM AND GM_FOUND)
  SET(HAVE_GM ON)
  SET(WT_HAS_WRASTERIMAGE true)
ENDIF(ENABLE_GM AND GM_FOUND)

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

