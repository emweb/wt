#[=======================================================================[.rst:
FindHPDF
--------

Finds libharu.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``HPDF::HPDF``
  The Haru library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``HPDF_FOUND``
  True if the system has the libharu library.
``HPDF_VERSION``
  The version of the libharu library which was found.

Cache Variables
^^^^^^^^^^^^^^^

The following variables may also be set:

``HPDF_INCLUDE_DIR``
  The directory containing ``hpdf_version.h``.
``HPDF_LIBRARY``
  The path to the libharu library.

#]=======================================================================]

if(NOT HARU_DYNAMIC)
  include(CMakeFindDependencyMacro)
  find_dependency(ZLIB)
  find_dependency(PNG)
endif()

get_property(_multi_config_generator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

find_path(HPDF_INCLUDE_DIR NAMES hpdf_version.h)
mark_as_advanced(HPDF_INCLUDE_DIR)

if(HPDF_INCLUDE_DIR)
  function(hpdf_version include_dir)
    file(STRINGS "${include_dir}/hpdf_version.h" _hpdf_version_h_contents REGEX " HPDF_(MAJOR|MINOR|BUGFIX)_VERSION ")
    string(REGEX REPLACE "^.*MAJOR_VERSION ([0-9]+).*$" \\1 _hpdf_major_version ${_hpdf_version_h_contents})
    string(REGEX REPLACE "^.*MINOR_VERSION ([0-9]+).*$" \\1 _hpdf_minor_version ${_hpdf_version_h_contents})
    string(REGEX REPLACE "^.*BUGFIX_VERSION ([0-9]+).*$" \\1 _hpdf_bugfix_version ${_hpdf_version_h_contents})
    set(HPDF_VERSION "${_hpdf_major_version}.${_hpdf_minor_version}.${_hpdf_bugfix_version}" PARENT_SCOPE)
  endfunction()

  hpdf_version(${HPDF_INCLUDE_DIR})

  # libharu 2.4.2 or later uses only hpdf as the library name
  # libharu 2.3.0 is different for Windows and other:
  # - Windows:
  #   - debug / dynamic: libhpdfd
  #   - debug / static: libhpdfsd
  #   - release / dynamic: libhpdf
  #   - release / static: libhpdfs
  # - other:
  #   - dynamic: hpdf
  #   - static: hpdf or hpdfs depending on how it was built (using autoconf or CMake)

  if(HPDF_VERSION VERSION_EQUAL "2.3.0")
    if(MSVC)
      if(HARU_DYNAMIC)
        set(_hpdf_library_name hpdf)
      else()
        set(_hpdf_library_name hpdfs)
      endif()
      find_library(HPDF_LIBRARY_DEBUG NAMES lib${_hpdf_library_name}d)
      mark_as_advanced(HPDF_LIBRARY_DEBUG)
      find_library(HPDF_LIBRARY_RELEASE NAMES lib${_hpdf_library_name})
      mark_as_advanced(HPDF_LIBRARY_RELEASE)
      unset(_hpdf_library_name)
      if (NOT _multi_config_generator AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(HPDF_LIBRARY "${HPDF_LIBRARY_DEBUG}")
      else()
        set(HPDF_LIBRARY "${HPDF_LIBRARY_RELEASE}")
      endif()
    else()
      find_library(HPDF_LIBRARY NAMES hpdf hpdfs NAMES_PER_DIR)
      mark_as_advanced(HPDF_LIBRARY)
    endif()
  else()
    find_library(HPDF_LIBRARY NAMES hpdf)
    mark_as_advanced(HPDF_LIBRARY)
  endif()
endif()

if(MSVC AND _multi_config_generator AND HPDF_VERSION VERSION_EQUAL "2.3.0")
  set(_required_libs HPDF_LIBRARY_DEBUG HPDF_LIBRARY_RELEASE)
else()
  set(_required_libs HPDF_LIBRARY)
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  HPDF
  REQUIRED_VARS ${_required_libs} HPDF_INCLUDE_DIR
  VERSION_VAR HPDF_VERSION
)
unset(_required_libs)

if(HPDF_FOUND AND NOT TARGET HPDF::HPDF)
  add_library(HPDF::HPDF UNKNOWN IMPORTED)
  set_target_properties(HPDF::HPDF PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${HPDF_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
  )
  if(NOT HARU_DYNAMIC)
    set_target_properties(HPDF::HPDF PROPERTIES
      INTERFACE_LINK_LIBRARIES "ZLIB::ZLIB;PNG::PNG")
  endif()
  # NOTE: despite the CMake documentation stating that on Windows
  #       IMPORTED_LOCATION refers to the .dll file and IMPORTED_IMPLIB refers to the .lib file,
  #       supplying the .lib file as the IMPORTED_LOCATION works just fine, and it will complain
  #       if we set IMPORTED_IMPLIB but not IMPORTED_LOCATION (we don't want to go through the hassle
  #       of also searching for the .dll file so that we can accurately set IMPORTED_LOCATION).
  set_target_properties(HPDF::HPDF PROPERTIES
    IMPORTED_LOCATION "${HPDF_LIBRARY}")
  if(MSVC AND _multi_config_generator AND HPDF_VERSION VERSION_EQUAL "2.3.0")
    set_target_properties(HPDF::HPDF PROPERTIES
      IMPORTED_LOCATION_DEBUG "${HPDF_LIBRARY_DEBUG}")
  endif()
  if(WIN32 AND HARU_DYNAMIC)
    target_compile_definitions(HPDF::HPDF INTERFACE HPDF_DLL)
  endif()
endif()

unset(_multi_config_generator)
