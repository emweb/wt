set(OPENSAML_ROOT "${OPENSAML_ROOT}" CACHE PATH "Directory to look in for Shibboleth's OpenSAML")

find_package(XmlTooling)
if(NOT XmlTooling_FOUND)
  set(OpenSAML_FOUND FALSE)
  return()
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_OpenSAML QUIET opensaml)
endif()

find_path(OpenSAML_INCLUDE_DIR
  NAMES saml/version.h
  PATHS ${PC_OpenSAML_INCLUDE_DIRS}
  PATH_SUFFIXES include
)
find_library(OpenSAML_LIBRARY_RELEASE
  NAMES saml saml3
  PATHS ${PC_OpenSAML_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
find_library(OpenSAML_LIBRARY_DEBUG
  NAMES saml saml3D
  PATHS ${PC_OpenSAML_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
if(OpenSAML_INCLUDE_DIR)
  file(READ "${OpenSAML_INCLUDE_DIR}/saml/version.h" _ver)
  string(REGEX MATCH "OPENSAML_VERSION_MAJOR[ ]+([0-9.]*)" _ ${_ver})
  set(OpenSAML_VERSION_MAJOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "OPENSAML_VERSION_MINOR[ ]+([0-9.]*)" _ ${_ver})
  set(OpenSAML_VERSION_MINOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "OPENSAML_VERSION_REVISION[ ]+([0-9.]*)" _ ${_ver})
  set(OpenSAML_VERSION_PATCH ${CMAKE_MATCH_1})
  set(OpenSAML_VERSION "${OpenSAML_VERSION_MAJOR}.${OpenSAML_VERSION_MINOR}.${OpenSAML_VERSION_PATCH}")
  unset(_ver)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSAML
  FOUND_VAR OpenSAML_FOUND
  REQUIRED_VARS
    OpenSAML_LIBRARY_RELEASE
    OpenSAML_INCLUDE_DIR
  VERSION_VAR OpenSAML_VERSION
)

if(OpenSAML_FOUND)
  if(NOT TARGET Shibboleth::OpenSAML)
    add_library(Shibboleth::OpenSAML UNKNOWN IMPORTED)
  endif()
  if(OpenSAML_LIBRARY_RELEASE)
    set_property(TARGET Shibboleth::OpenSAML APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(Shibboleth::OpenSAML PROPERTIES
      IMPORTED_LOCATION_RELEASE "${OpenSAML_LIBRARY_RELEASE}"
    )
  endif()
  if(OpenSAML_LIBRARY_DEBUG)
    set_property(TARGET Shibboleth::OpenSAML APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(Shibboleth::OpenSAML PROPERTIES
      IMPORTED_LOCATION_DEBUG "${OpenSAML_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(Shibboleth::OpenSAML PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_OpenSAML_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${OpenSAML_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LIBRARIES Shibboleth::XmlTooling
  )
endif()

mark_as_advanced(OpenSAML_INCLUDE_DIR OpenSAML_LIBRARY_RELEASE OpenSAML_LIBRARY_DEBUG)
set(OpenSAML_VERSION_STRING ${OpenSAML_VERSION})
