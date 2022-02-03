set(XERCESC_ROOT "${XERCESC_ROOT}" CACHE PATH "Directory to look in for Apache Xerces C++")
set(XMLSECURITYC_ROOT "${XMLSECURITYC_ROOT}" CACHE PATH "Directory to look in for Apache XML Security for C++")

list(APPEND CMAKE_PREFIX_PATH ${XERCESC_ROOT})
find_package(XercesC)
if(NOT XercesC_FOUND)
  set(FindXmlSecurityC_FOUND FALSE)
  return()
endif()

find_package(OpenSSL)
if(NOT OpenSSL_FOUND)
  set(FindXmlSecurityC FOUND FALSE)
  return()
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_XmlSecurityC QUIET xml-security-c)
endif()

find_path(XmlSecurityC_INCLUDE_DIR
  NAMES xsec/framework/XSECVersion.hpp
  PATHS ${PC_XmlSecurityC_INCLUDE_DIRS}
  PATH_SUFFIXES include
)
find_library(XmlSecurityC_LIBRARY_RELEASE
  NAMES xml-security-c xsec_2
  PATHS ${PC_XmlSecurityC_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
find_library(XmlSecurityC_LIBRARY_DEBUG
  NAMES xml-security-c xsec_2D
  PATHS ${PC_XmlSecurityC_LIBRARY_DIRS}
  PATH_SUFFIXES lib
)
if(XmlSecurityC_INCLUDE_DIR)
  file(READ "${XmlSecurityC_INCLUDE_DIR}/xsec/framework/XSECVersion.hpp" _ver)
  string(REGEX MATCH "XSEC_VERSION_MAJOR[ ]+([0-9.]*)" _ ${_ver})
  set(XmlSecurityC_VERSION_MAJOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "XSEC_VERSION_MEDIUM[ ]+([0-9.]*)" _ ${_ver})
  set(XmlSecurityC_VERSION_MINOR ${CMAKE_MATCH_1})
  string(REGEX MATCH "XSEC_VERSION_MINOR[ ]+([0-9.]*)" _ ${_ver})
  set(XmlSecurityC_VERSION_PATCH ${CMAKE_MATCH_1})
  set(XmlSecurityC_VERSION "${XmlSecurityC_VERSION_MAJOR}.${XmlSecurityC_VERSION_MINOR}.${XmlSecurityC_VERSION_PATCH}")
  unset(_ver)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XmlSecurityC
  FOUND_VAR XmlSecurityC_FOUND
  REQUIRED_VARS
    XmlSecurityC_LIBRARY_RELEASE
    XmlSecurityC_INCLUDE_DIR
  VERSION_VAR XmlSecurityC_VERSION
)

if(XmlSecurityC_FOUND)
  if(NOT TARGET XmlSecurityC::XmlSecurityC)
    add_library(XmlSecurityC::XmlSecurityC UNKNOWN IMPORTED)
  endif()
  if(XmlSecurityC_LIBRARY_RELEASE)
    set_property(TARGET XmlSecurityC::XmlSecurityC APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(XmlSecurityC::XmlSecurityC PROPERTIES
      IMPORTED_LOCATION_RELEASE "${XmlSecurityC_LIBRARY_RELEASE}"
    )
  endif()
  if(XmlSecurityC_LIBRARY_DEBUG)
    set_property(TARGET XmlSecurityC::XmlSecurityC APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(XmlSecurityC::XmlSecurityC PROPERTIES
      IMPORTED_LOCATION_DEBUG "${XmlSecurityC_LIBRARY_DEBUG}"
    )
  endif()
  set_target_properties(XmlSecurityC::XmlSecurityC PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_XmlSecurityC_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${XmlSecurityC_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LIBRARIES "XercesC::XercesC;OpenSSL::Crypto"
  )
endif()

mark_as_advanced(XmlSecurityC_INCLUDE_DIR XmlSecurityC_LIBRARY_RELEASE XmlSecurityC_LIBRARY_DEBUG)
set(XmlSecurityC_VERSION_STRING ${XmlSecurityC_VERSION})
