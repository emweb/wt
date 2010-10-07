IF(NOT HAVE_GM)

  MESSAGE(STATUS "** Not building style example: requires WRasterImage.")

ELSE(NOT HAVE_GM)

  WT_ADD_EXAMPLE(styleexample.wt CornerImage.C RoundedWidget.C StyleExample.C)

  INCLUDE_DIRECTORIES(
   ${WT_SOURCE_DIR}/src
  )

  ADD_DEPENDENCIES(styleexample.wt wt ${EXAMPLES_CONNECTOR})

ENDIF(NOT HAVE_GM)

