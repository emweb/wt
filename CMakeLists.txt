CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(WT)
SET(CMAKE_MODULE_PATH ${WT_SOURCE_DIR} ${WT_SOURCE_DIR}/cmake)

SET(VERSION_SERIES 3)
SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 1)

SET(WT_SOVERSION 20)
SET(WTEXT_SOVERSION 13)
SET(WTHTTP_SOVERSION 7)
SET(WTFCGI_SOVERSION 7)
SET(WTDBO_SOVERSION 2)

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

IF(WIN32)
  SET(USERLIB_ROOT "c:/libraries" CACHE PATH "directory containing the dependency libraries")
  SET(LIB_INSTALL_DIR "lib" CACHE STRING "Default path for libraries within ${CMAKE_INSTALL_PREFIX}")
  SET(RUNDIR "c:/witty" CACHE PATH "Not really used in WIN32")
  IF( NOT DEFINED CONFIGDIR )
    SET(CONFIGDIR ${RUNDIR} CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )
  SET(DEPLOYROOT c:/witty/wt-examples CACHE PATH "Path to deploy examples into")
  SET(BOOST_DIR "c:/Program Files/Boost" CACHE PATH "Use boost in directory")
  SET(GD_DIR ${USERLIB_ROOT} CACHE PATH "Search path for libgd")
  OPTION(BOOST_DYNAMIC "Link to boost DLLs (OFF means static link)" OFF)
ELSE(WIN32)
  SET(USERLIB_ROOT /usr CACHE PATH "Other installation prefix for dependent libraries")
  SET(LIB_INSTALL_DIR "lib" CACHE STRING "Default path for libraries within ${CMAKE_INSTALL_PREFIX}")
  SET(RUNDIR "/var/run/wt" CACHE PATH "Default path for wt session management (only used by FCGI connector; not relative to CMAKE_INSTALL_PREFIX)")
  IF( NOT DEFINED CONFIGDIR )
    SET(CONFIGDIR "/etc/wt" CACHE STRING "Path for the configuration files")
  ENDIF( NOT DEFINED CONFIGDIR )
  SET(DEPLOYROOT /var/www/localhost/htdocs/wt-examples CACHE PATH "Path to deploy examples into)")
  SET(BOOST_DIR /usr CACHE PATH "Boost installation path prefix")
  SET(GD_DIR "/usr/lib" CACHE PATH "Search path for libgd2")
ENDIF(WIN32)

OPTION(DEBUG "Support for debugging, must be enabled also in wt_config.xml" OFF)
OPTION(BUILD_TESTS "Build Wt tests" ON)

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
INCLUDE(cmake/WtFindZlib.txt)
INCLUDE(cmake/WtFindSsl.txt)
INCLUDE(cmake/WtFindMysqlpp.txt)
INCLUDE(cmake/WtFindAsciidoc.txt)

FIND_PACKAGE(Qt4)
IF(QT_FOUND)
  INCLUDE(${QT_USE_FILE})
ENDIF(QT_FOUND)

FIND_PACKAGE(Doxygen)

# Boost is used nearly everywhere, so we can put these here
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
LINK_DIRECTORIES(${BOOST_LIB_DIRS})
IF(WIN32)
  IF(BOOST_DYNAMIC)
    ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
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

IF (ASCIIDOC_FOUND)
  MACRO (ASCIIDOC_FILE target infile outfile)
    ADD_CUSTOM_TARGET(${target}
       ${ASCIIDOC_EXECUTABLE} -a toc -a numbered -o ${outfile} ${infile}
       COMMENT "Asciidoc ${infile}")
    ADD_CUSTOM_TARGET(doc)
    ADD_DEPENDENCIES(doc ${target})
  ENDMACRO (ASCIIDOC_FILE)
ENDIF (ASCIIDOC_FOUND)

IF (DOXYGEN_FOUND)
  ADD_CUSTOM_TARGET(doxygen
    ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Doxygen ...")
  ADD_CUSTOM_TARGET(doc)
  ADD_DEPENDENCIES(doc doxygen)
  ADD_CUSTOM_TARGET(doxygen-examples
    ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples
    COMMENT "Doxygen for examples ...")
  ADD_DEPENDENCIES(doc doxygen-examples)
 ENDIF (DOXYGEN_FOUND)

IF(WIN32)
  SUBDIRS(src examples)
ELSE(WIN32)
  SUBDIRS(src EXCLUDE_FROM_ALL examples)
ENDIF(WIN32)

IF(BUILD_TESTS)
  SUBDIRS(test)
ENDIF(BUILD_TESTS)

IF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR )
  SET( WT_CMAKE_FINDER_INSTALL_DIR "share/cmake-2.4/Modules" )
ENDIF( NOT DEFINED WT_CMAKE_FINDER_INSTALL_DIR)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/cmake/FindWt.cmake DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${WT_CMAKE_FINDER_INSTALL_DIR} )

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/resources DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/Wt/)

IF(NOT EXISTS ${CONFIGDIR}/wt_config.xml)
  INSTALL(FILES ${WT_BINARY_DIR}/wt_config.xml DESTINATION ${CONFIGDIR})
ENDIF (NOT EXISTS ${CONFIGDIR}/wt_config.xml)
