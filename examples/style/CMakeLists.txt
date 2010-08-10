IF(NOT GM_FOUND)

  MESSAGE(STATUS "** Not building style example: requires WRasterImage.")

ELSE(NOT GM_FOUND)

  WT_ADD_EXAMPLE(styleexample.wt CornerImage.C RoundedWidget.C StyleExample.C)

  INCLUDE_DIRECTORIES(
   ${WT_SOURCE_DIR}/src
  )

  ADD_DEPENDENCIES(styleexample.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT GM_FOUND)

