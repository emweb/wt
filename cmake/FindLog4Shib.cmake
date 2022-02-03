set(LOG4SHIB_ROOT "${LOG4SHIB_ROOT}" CACHE PATH "Directory to look in for Log4Shib")

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_Log4Shib QUIET log4shib)
endif()

if(WIN32)
  set(_log4shib_config_file config-win32.h)
else()
  set(_log4shib_config_file config.h)
endif()

find_path(Log4Shib_INCLUDE_DIR
  NAMES log4shib/${_log4shib_config_file}
  PATHS ${PC_Log4Shib_INCLUDE_DIRS} "${LOG4SHIB_ROOT}"
  PATH_SUFFIXES include
)
find_library(Log4Shib_LIBRARY_RELEASE
  NAMES log4shib log4shib2
  PATHS ${PC_Log4Shib_LIBRARY_DIRS} "${LOG4SHIB_ROOT}"
  PATH_SUFFIXES lib
)
find_library(Log4Shib_LIBRARY_DEBUG
  NAMES log4shib log4shib2D
  PATHS ${PC_Log4Shib_LIBRARY_DIRS} "${LOG4SHIB_ROOT}"
  PATH_SUFFIXES lib
)
if(Log4Shib_INCLUDE_DIR)
  file(READ "${Log4Shib_INCLUDE_DIR}/log4shib/${_log4shib_config_file}" _ver)
  string(REGEX MATCH "LOG4SHIB_VERSION[ ]+[\"]([0-9.]*)[\"]" _ ${_ver})
  set(Log4Shib_VERSION ${CMAKE_MATCH_1})
  unset(_ver)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Log4Shib
  FOUND_VAR Log4Shib_FOUND
  REQUIRED_VARS
    Log4Shib_LIBRARY_RELEASE
    Log4Shib_INCLUDE_DIR
  VERSION_VAR Log4Shib_VERSION
)

if(Log4Shib_FOUND)
  if(NOT TARGET Log4Shib::Log4Shib)
    add_library(Log4Shib::Log4Shib UNKNOWN IMPORTED)
  endif()
  if(Log4Shib_LIBRARY_RELEASE)
    set_property(TARGET Log4Shib::Log4Shib APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(Log4Shib::Log4Shib PROPERTIES
      IMPORTED_LOCATION_RELEASE "${Log4Shib_LIBRARY_RELEASE}"
    )
  endif()
  if(Log4Shib_LIBRARY_DEBUG)
    set_property(TARGET Log4Shib::Log4Shib APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
            )
    set_target_properties(Log4Shib::Log4Shib PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Log4Shib_LIBRARY_DEBUG}"
            )
  endif()
  set_target_properties(Log4Shib::Log4Shib PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_Log4Shib_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Log4Shib_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(Log4Shib_INCLUDE_DIR Log4Shib_LIBRARY_RELEASE Log4Shib_LIBRARY_DEBUG)
set(Log4Shib_VERSION_STRING ${Log4Shib_VERSION})
