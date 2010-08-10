WT_ADD_EXAMPLE(paintexample.wt
  PaintApplication.C PaintExample.C ShapesWidget.C
)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

