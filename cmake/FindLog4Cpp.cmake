set(LOG4CPP_ROOT "${LOG4CPP_ROOT}" CACHE PATH "Directory to look in for Log4Cpp")

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_Log4Cpp QUIET log4cpp)
endif()

find_path(Log4Cpp_INCLUDE_DIR
  NAMES log4cpp/config.h
  PATHS ${PC_Log4Cpp_INCLUDE_DIRS} "${LOG4CPP_ROOT}"
  PATH_SUFFIXES include
)
find_library(Log4Cpp_LIBRARY_RELEASE
  NAMES log4cpp log4cpp2
  PATHS ${PC_Log4Cpp_LIBRARY_DIRS} "${LOG4CPP_ROOT}"
  PATH_SUFFIXES lib
)
find_library(Log4Cpp_LIBRARY_DEBUG
  NAMES log4cpp log4cpp2D
  PATHS ${PC_Log4Cpp_LIBRARY_DIRS} "${LOG4CPP_ROOT}"
  PATH_SUFFIXES lib
)
if(Log4Cpp_INCLUDE_DIR)
  file(READ "${Log4Cpp_INCLUDE_DIR}/log4cpp/config.h" _ver)
  string(REGEX MATCH "LOG4CPP_VERSION[ ]+[\"]([0-9.]*)[\"]" _ ${_ver})
  set(Log4Cpp_VERSION ${CMAKE_MATCH_1})
  unset(_ver)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Log4Cpp
  FOUND_VAR Log4Cpp_FOUND
  REQUIRED_VARS
    Log4Cpp_LIBRARY_RELEASE
    Log4Cpp_INCLUDE_DIR
  VERSION_VAR Log4Cpp_VERSION
)

if(Log4Cpp_FOUND)
  if(NOT TARGET Log4Cpp::Log4Cpp)
    add_library(Log4Cpp::Log4Cpp UNKNOWN IMPORTED)
  endif()
  if(Log4Cpp_LIBRARY_RELEASE)
    set_property(TARGET Log4Cpp::Log4Cpp APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(Log4Cpp::Log4Cpp PROPERTIES
     IMPORTED_LOCATION_RELEASE "${Log4Cpp_LIBRARY_RELEASE}"
    )
  endif()
  if(Log4Cpp_LIBRARY_DEBUG)
    set_property(TARGET Log4Cpp::Log4Cpp APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(Log4Cpp::Log4Cpp PROPERTIES
      IMPORTED_LOCATION_DEBUG "${Log4Cpp_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(Log4Cpp::Log4Cpp PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_Log4Cpp_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Log4Cpp_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(Log4Cpp_INCLUDE_DIR Log4Cpp_LIBRARY_RELEASE Log4Cpp_LIBRARY_DEBUG)
set(Log4Cpp_VERSION_STRING ${Log4Cpp_VERSION})
