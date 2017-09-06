IF(NOT WT_HAS_WRASTERIMAGE)

  MESSAGE(STATUS "** Not building style example: requires WRasterImage.")

ELSE(NOT WT_HAS_WRASTERIMAGE)

  WT_ADD_EXAMPLE(styleexample.wt CornerImage.C RoundedWidget.C StyleExample.C)

  INCLUDE_DIRECTORIES(
   ${WT_SOURCE_DIR}/src
  )

ENDIF(NOT WT_HAS_WRASTERIMAGE)

