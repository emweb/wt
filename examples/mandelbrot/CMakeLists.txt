IF(NOT WT_HAS_WRASTERIMAGE)
  MESSAGE(STATUS "** Not building mandelbrot example: requires Wt::WRasterImage.")
ELSE(NOT WT_HAS_WRASTERIMAGE)

  WT_ADD_EXAMPLE(mandelbrot.wt MandelbrotImage.C MandelbrotExample.C)

  INCLUDE_DIRECTORIES(
    ${GD_INCLUDE_DIRS}
    ${WT_SOURCE_DIR}/src
  )

ENDIF(NOT WT_HAS_WRASTERIMAGE)

