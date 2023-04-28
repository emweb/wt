#[=======================================================================[.rst:
FindAsciidoctor
---------------

Finds Asciidoctor.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Asciidoctor::asciidoctor``
  The Asciidoctor executable, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``ASCIIDOCTOR_FOUND``
  True if the system has the asciidoctor or asciidoctorj library.

Cache Variables
^^^^^^^^^^^^^^^

The following variables may also be set:

``ASCIIDOCTOR_EXECUTABLE``
  The full path to asciidoctor or asciidoctorj

#]=======================================================================]

find_program(ASCIIDOCTOR_EXECUTABLE NAMES asciidoctor asciidoctorj)
mark_as_advanced(ASCIIDOCTOR_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Asciidoctor
  REQUIRED_VARS ASCIIDOCTOR_EXECUTABLE
)

if(ASCIIDOCTOR_FOUND AND NOT TARGET Asciidoctor::asciidoctor)
  add_executable(Asciidoctor::asciidoctor IMPORTED)
  set_target_properties(Asciidoctor::asciidoctor PROPERTIES
    IMPORTED_LOCATION "${ASCIIDOCTOR_EXECUTABLE}")
endif()
