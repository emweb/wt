IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    ADD_DEFINITIONS(-DWT_THREADED -D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
  ELSE(MULTI_THREADED)
    ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
  ENDIF(MULTI_THREADED)
ELSE(BOOST_WT_MT_FOUND)
  ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
ENDIF(BOOST_WT_MT_FOUND)

ADD_DEFINITIONS(-DWT_DEPRECATED_3_0_0)

# The WT_ADD_EXAMPLE function builds a DLL in case of ISAPI, and an executable
# in case of FCGI or HTTPD connectors. It also makes sure that the example
# links to the proper libraries (connector + wt).
FUNCTION(WT_ADD_EXAMPLE name)
  IF(${EXAMPLES_CONNECTOR} STREQUAL "wtisapi")
    LIST(INSERT ARGV 1 "SHARED")
    ADD_LIBRARY(${ARGV})
    SET_TARGET_PROPERTIES(${name}
      PROPERTIES
        LINK_FLAGS
         "/EXPORT:HttpExtensionProc /EXPORT:GetExtensionVersion /EXPORT:TerminateExtension"
    )
  ELSE(${EXAMPLES_CONNECTOR} STREQUAL "wtisapi")
    ADD_EXECUTABLE(${ARGV})
  ENDIF(${EXAMPLES_CONNECTOR} STREQUAL "wtisapi")
  TARGET_LINK_LIBRARIES(${name} ${EXAMPLES_CONNECTOR})
ENDFUNCTION(WT_ADD_EXAMPLE)

SUBDIRS(
  blog
  bobsmith
  charts
  composer
  dialog
  dragdrop
  extkitchen
  feature
  filetreetable
  form
  gitmodel
  hangman
  hello
  javascript
  mandelbrot
  mission
  painting
  planner
  simplechat
  style
  treelist
  treeview
  treeview-dragdrop
  wt-homepage
  wtwithqt
  widgetgallery
)
