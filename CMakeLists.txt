CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(WT)
SET(CMAKE_MODULE_PATH ${WT_SOURCE_DIR})

SET(VERSION_SERIES 2)
SET(VERSION_MAJOR 99)
SET(VERSION_MINOR 4)

SET(WT_SOVERSION 16)
SET(WTEXT_SOVERSION 11)
SET(WTHTTP_SOVERSION 7)
SET(WTFCGI_SOVERSION 7)

#
# Various things that must be configured by the user or packager ...
#

IF(NOT SHARED_LIBS)
  IF(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" OFF)
  ELSE(WIN32)
    OPTION(SHARED_LIBS "Compile shared libraries" ON)
  ENDIF(WIN32)
ENDIF(NOT SHARED_LIBS)

# Fixup Windows declspec stuff
IF(NOT SHARED_LIBS)
  SET(WT_STATIC true)
  SET(WTHTTP_STATIC true)
  SET(WT_EXT_STATIC true)
ENDIF(NOT SHARED_LIBS)

IF(NOT MULTI_THREADED)
  OPTION(MULTI_THREADED "Build multi-threaded httpd deamon (if possible)" ON)
ENDIF(NOT MULTI_THREADED)

SET(BUILD_SHARED_LIBS ${SHARED_LIBS})

IF(WIN32)
  SET(USERLIB_ROOT "c:/libraries" CACHE PATH "directory containing the dependency libraries")
  SET(LIB_INSTALL_DIR "lib" CACHE STRING "Default path for libraries within ${CMAKE_INSTALL_PREFIX}")
  SET(RUNDIR "c:/witty" CACHE PATH "Not really used in WIN32")
  IF( NOT DEFINED CONFIGDIR )
    SET(CONFIGDIR ${RUNDIR} CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )
  SET(DEPLOYROOT c:/witty/wt-examples CACHE PATH "Path to deploy examples into")
  SET(BOOST_DIR ${USERLIB_ROOT} CACHE PATH "Use boost in directory")
  SET(BOOST_VERSION "1_35" CACHE STRING "Use boost version in BOOST_DIR")
  SET(GD_DIR ${USERLIB_ROOT} CACHE PATH "Search path for libgd")
  OPTION(BOOST_DYNAMIC "Link to boost DLLs (OFF means static link)" OFF)
  OPTION(MXML_SUPPLIED "Force use of supplied mxml over installed mxml." ON)
  OPTION(DEBUG "Support for debugging, must be enabled also in wt_config.xml" OFF)

ELSE(WIN32)

  SET(USERLIB_ROOT /usr CACHE PATH "Other installation prefix for dependent libraries")
  SET(LIB_INSTALL_DIR "lib" CACHE STRING "Default path for libraries within ${CMAKE_INSTALL_PREFIX}")
  SET(RUNDIR "/usr/wt/run" CACHE PATH "Default path for wt session management (only used by FCGI connector; not relative to CMAKE_INSTALL_PREFIX)")
  IF( NOT DEFINED CONFIGDIR )
    SET(CONFIGDIR "/etc/wt" CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )
  SET(DEPLOYROOT /var/www/localhost/htdocs/wt-examples CACHE PATH "Path to deploy examples into)")
  SET(BOOST_DIR /usr CACHE PATH "Boost installation path prefix")
  SET(BOOST_COMPILER gcc CACHE STRING "Boost libraries compiler signature")
  SET(BOOST_VERSION "1_35" CACHE STRING "Boost libraries version signature")
  SET(GD_DIR "/usr/lib" CACHE PATH "Search path for libgd2")
  OPTION(MXML_SUPPLIED "Force use of supplied mxml over installed mxml." ON)
  OPTION(DEBUG "Support for debugging, must be enabled also in wt_config.xml" OFF)
ENDIF(WIN32)

ADD_DEFINITIONS(-DWT_WITH_OLD_INTERNALPATH_API)

MARK_AS_ADVANCED( CONFIGDIR )
SET(CONFIGURATION "${CONFIGDIR}/wt_config.xml" CACHE PATH "Path to the wt configuration file")
SET(WTHTTP_CONFIGURATION ${CONFIGDIR}/wthttpd CACHE PATH "Path for the wthttpd configuration file")

SET(WEBUSER apache CACHE STRING "Webserver username (e.g. apache or www)")
SET(WEBGROUP apache CACHE STRING "Webserver groupname (e.g. apache or www or users)")
OPTION(CONNECTOR_FCGI "Compile in FCGI connector (libwtfcgi) ?" OFF)
OPTION(CONNECTOR_HTTP "Compile in stand-alone httpd connector (libwthttp) ?" ON)
SET(EXAMPLES_CONNECTOR wthttp CACHE STRING "Connector used for examples")

INCLUDE(cmake/WtFindBoost.txt)
INCLUDE(cmake/WtFindGd.txt)
INCLUDE(cmake/WtFindFcgi.txt)
INCLUDE(cmake/WtFindMxml.txt)
INCLUDE(cmake/WtFindZlib.txt)
INCLUDE(cmake/WtFindAsio.txt)
INCLUDE(cmake/WtFindSsl.txt)
INCLUDE(cmake/WtFindMysqlpp.txt)

FIND_PACKAGE(Qt4)

# Boost is used nearly everywhere, so we can put these here
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
LINK_DIRECTORIES(${BOOST_LIB_DIRS})
IF(WIN32)
  IF(BOOST_DYNAMIC)
    ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
  ENDIF(BOOST_DYNAMIC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
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

# Compile time constants & make sure our build finds it
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Wt)
SET(WCONFIG_H_PATH ${CMAKE_CURRENT_BINARY_DIR}/Wt/WConfig.h)
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/WConfig.h.in
  ${WCONFIG_H_PATH}
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INSTALL_FILES(/include/Wt FILES ${WCONFIG_H_PATH})

IF(WIN32)
  SUBDIRS(src examples)
ELSE(WIN32)
  SUBDIRS(src EXCLUDE_FROM_ALL examples)
ENDIF(WIN32)

IF(IS_DIRECTORY test)
  SUBDIRS(test)
ENDIF(IS_DIRECTORY test)

IF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR )
  SET( WT_CMAKE_FINDER_INSTALL_DIR "share/cmake-2.4/Modules" )
ENDIF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/cmake/FindWt.cmake DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${WT_CMAKE_FINDER_INSTALL_DIR} )

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/Wt/)

IF(NOT EXISTS ${CONFIGDIR}/wt_config.xml)
  INSTALL(FILES ${PROJECT_SOURCE_DIR}/wt_config.xml DESTINATION ${CONFIGDIR})
ENDIF (NOT EXISTS ${CONFIGDIR}/wt_config.xml)
