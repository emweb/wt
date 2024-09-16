IF(NOT WIN32)

IF(WT_CPP17_FILESYSTEM_IMPLEMENTATION STREQUAL "std" OR BOOST_FS_LIB)
  WT_ADD_EXAMPLE(gitview.wt
    Git.C
    GitModel.C
    ../wt-homepage/SourceView.C
    GitView.C
  )
  TARGET_LINK_LIBRARIES(gitview.wt ${BOOST_FS_LIB} ${BOOST_SYSTEM_LIB})

  INCLUDE_DIRECTORIES(
    ${WT_SOURCE_DIR}/src
  )

  ADD_DEPENDENCIES(gitview.wt wt ${EXAMPLES_CONNECTOR})

ELSE()

  MESSAGE(STATUS "** Not building gitmodel example: requires std::filesystem or boost_filesystem library.")


ENDIF()

ELSE(NOT WIN32)

  MESSAGE(STATUS "** Not building gitmodel example: uses POSIX calls not available on win32")

ENDIF(NOT WIN32)
