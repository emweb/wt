set(XMLTOOLING_ROOT "${XMLTOOLING_ROOT}" CACHE PATH "Directory to look in for Shibboleth's XMLTooling")

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_XmlTooling QUIET xmltooling)
endif()

find_path(XmlTooling_INCLUDE_DIR
  NAMES xmltooling/version.h
  PATHS ${PC_XmlTooling_INCLUDE_DIRS}
  PATH_SUFFIXES include
)

if(XmlTooling_INCLUDE_DIR)
  if(WIN32)
    file(READ "${XmlTooling_INCLUDE_DIR}/xmltooling/config_pub_win32.h" _config)
  else()
    file(READ "${XmlTooling_INCLUDE_DIR}/xmltooling/config_pub.h" _config)
  endif()
  string(REGEX MATCH "XMLTOOLING_LOG4SHIB 1" _log4shib ${_config})
  if(_log4shib)
    set(_xmltooling_log4shib ON)
  else()
    set(_xmltooling_log4cpp ON)
  endif()
  unset(_log4shib)
  unset(_config)
endif()

if(_xmltooling_log4shib)
  find_package(Log4Shib)
  if(NOT Log4Shib_FOUND)
    set(XmlTooling_FOUND FALSE)
    return()
  endif()
endif()

if(_xmltooling_log4cpp)
  find_package(Log4Cpp)
  if(NOT Log4Cpp_FOUND)
    set(XmlTooling_FOUND FALSE)
    return()
  endif()
endif()

find_package(XmlSecurityC)
if(NOT XmlSecurityC_FOUND)
  set(XmlTooling_FOUND FALSE)
  return()
endif()
find_library(XmlTooling_LIBRARY_RELEASE
  NAMES xmltooling xmltooling3
  PATHS ${PC_XmlTooling_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
find_library(XmlTooling_LIBRARY_DEBUG
  NAMES xmltooling xmltooling3D
  PATHS ${PC_XmlTooling_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
if(XmlTooling_INCLUDE_DIR)
  file(READ "${XmlTooling_INCLUDE_DIR}/xmltooling/version.h" _ver)
  string(REGEX MATCH "XMLTOOLING_VERSION_MAJOR[ ]+([0-9.]*)" _ ${_ver})
  set(XmlTooling_VERSION_MAJOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "XMLTOOLING_VERSION_MINOR[ ]+([0-9.]*)" _ ${_ver})
  set(XmlTooling_VERSION_MINOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "XMLTOOLING_VERSION_REVISION[ ]+([0-9.]*)" _ ${_ver})
  set(XmlTooling_VERSION_PATCH ${CMAKE_MATCH_1})
  set(XmlTooling_VERSION "${XmlTooling_VERSION_MAJOR}.${XmlTooling_VERSION_MINOR}.${XmlTooling_VERSION_PATCH}")
  unset(_ver)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XmlTooling
  FOUND_VAR XmlTooling_FOUND
  REQUIRED_VARS
    XmlTooling_LIBRARY_RELEASE
    XmlTooling_INCLUDE_DIR
  VERSION_VAR XmlTooling_VERSION
)

if(XmlTooling_FOUND)
  if(NOT TARGET Shibboleth::XmlTooling)
    add_library(Shibboleth::XmlTooling UNKNOWN IMPORTED)
  endif()
  if(XmlTooling_LIBRARY_RELEASE)
    set_property(TARGET Shibboleth::XmlTooling APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(Shibboleth::XmlTooling PROPERTIES
      IMPORTED_LOCATION_RELEASE "${XmlTooling_LIBRARY_RELEASE}"
    )
  endif()
  if(XmlTooling_LIBRARY_DEBUG)
    set_property(TARGET Shibboleth::XmlTooling APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(Shibboleth::XmlTooling PROPERTIES
      IMPORTED_LOCATION_DEBUG "${XmlTooling_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(Shibboleth::XmlTooling PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_XmlTooling_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${XmlTooling_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LIBRARIES "XmlSecurityC::XmlSecurityC;Log4Shib::Log4Shib"
  )
endif()

mark_as_advanced(XmlTooling_INCLUDE_DIR XmlTooling_LIBRARY_RELEASE XmlTooling_LIBRARY_DEBUG)
set(XmlTooling_VERSION_STRING ${XmlTooling_VERSION})
