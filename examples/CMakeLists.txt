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
MACRO(WT_ADD_EXAMPLE name)
  IF(EXAMPLES_CONNECTOR MATCHES "wtisapi")
    SET(MYLIST ${ARGV})
    LIST(INSERT MYLIST 1 SHARED)
    ADD_LIBRARY(${MYLIST})
    SET_TARGET_PROPERTIES(${name}
      PROPERTIES
        LINK_FLAGS
         "/EXPORT:HttpExtensionProc /EXPORT:GetExtensionVersion /EXPORT:TerminateExtension"
    )
  ELSE(EXAMPLES_CONNECTOR MATCHES "wtisapi")
    ADD_EXECUTABLE(${ARGV})
  ENDIF(EXAMPLES_CONNECTOR MATCHES "wtisapi")
  IF(MSVC)
    SET_TARGET_PROPERTIES(${name} PROPERTIES COMPILE_FLAGS "/W3 /Wp64 /wd4251 /wd4275")
  ENDIF(MSVC)
  TARGET_LINK_LIBRARIES(${name} ${EXAMPLES_CONNECTOR})
ENDMACRO(WT_ADD_EXAMPLE)

SUBDIRS(
  blog
  bobsmith
  charts
  codeview
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
  webgl
  wt-homepage
  wtwithqt
  widgetgallery
)
