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
  TARGET_LINK_LIBRARIES(${name} ${EXAMPLES_CONNECTOR} wt)

  IF(INSTALL_EXAMPLES)
    # Finding the directory where to install the binary and the wrapper/runner script is tricky 
    # because examples do not use project(...), or there is more than one example in one subdirectory
    SET( ${name}_SOURCEFILES ${ARGV} )
    LIST( REMOVE_AT ${name}_SOURCEFILES 0 )
    LIST(GET ${name}_SOURCEFILES 0 ${name}_FIRST_SOURCEFILE )
    GET_SOURCE_FILE_PROPERTY(${name}_FIRST_SOURCEFILE_FULLPATH ${${name}_FIRST_SOURCEFILE} LOCATION)
    STRING(REPLACE ${WT_SOURCE_DIR}/examples/ "" ${name}_SUBDIRANDSOURCE ${${name}_FIRST_SOURCEFILE_FULLPATH})
    STRING(REPLACE /${${name}_FIRST_SOURCEFILE} "" ${name}_SUBDIR ${${name}_SUBDIRANDSOURCE})
    
    SET( EXAMPLESUBDIR ${${name}_SUBDIR} )
    SET( EXAMPLESUBDIRFROMPREFIX ${EXAMPLES_DESTINATION}/${EXAMPLESUBDIR} )
    STRING( REPLACE .wt "" EXAMPLENAME ${name} )
    
    INSTALL( TARGETS ${name} DESTINATION ${EXAMPLES_DESTINATION}/${${name}_SUBDIR} )

    IF(WIN32)
      CONFIGURE_FILE(${WT_SOURCE_DIR}/examples/run-example.bat.cmake ${WT_BINARY_DIR}/${EXAMPLENAME}.bat @ONLY)
      INSTALL(PROGRAMS ${WT_BINARY_DIR}/${EXAMPLENAME}.bat DESTINATION bin )

      IF(INSTALL_RESOURCES)
	INSTALL(DIRECTORY ${WT_SOURCE_DIR}/resources DESTINATION ${EXAMPLES_DESTINATION}/${EXAMPLESUBDIR})  # Can't safely make symlinks of Windows
      ENDIF(INSTALL_RESOURCES)
    ELSE(WIN32)
      CONFIGURE_FILE(${WT_SOURCE_DIR}/examples/run-example.cmake ${WT_BINARY_DIR}/${EXAMPLENAME} @ONLY)
      INSTALL(PROGRAMS ${WT_BINARY_DIR}/${EXAMPLENAME} DESTINATION ${EXAMPLES_DESTINATION}/${EXAMPLESUBDIR})
      IF(INSTALL_RESOURCES)
	INSTALL( CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/share/Wt/resources ${CMAKE_INSTALL_PREFIX}/${EXAMPLES_DESTINATION}/${EXAMPLESUBDIR}/resources)" )
      ENDIF(INSTALL_RESOURCES)
    ENDIF(WIN32)
  ENDIF(INSTALL_EXAMPLES)

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
