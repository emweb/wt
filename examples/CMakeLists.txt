# Normally, this is not a toplevel CMakeLists.txt file. Except when it is
# used in combination with the Windows SDK, then it is prefixed with the
# required configuration settings to allow building the examples of the
# binary Wt distribution as a standalone project.
#
# This CMakeLists.txt is complex because it supports all different modes
# of building Wt on all supported platforms. Your own CMakeLists.txt can
# be much simpler. For example (for Windows):
#
# CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
# Project(myproject)
#
# INCLUDE_DIRECTORIES(c:/wt-installation-path/include)
# LINK_DIRECTORIES(c:/wt-installation-path/lib)
#
# ADD_DEFINITIONS(-DHPDF_DLL)
#
# ADD_EXECUTABLE(myprog.wt
#   MyProg1.cpp
#   OtherFile.cpp
#   AndEvenMoreCode.cpp
# )
#
# TARGET_LINK_LIBRARIES(myprog.wt
#   debug wtd      optimized wt
#   debug wthttpd  optimized wthttp
#   debug libhpdfd optimized libhpdf # only required for if you write pdfs
# )

SET(EXAMPLES_WT_LIB wt)
SET(EXAMPLES_WTDBO_LIB wtdbo)
SET(EXAMPLES_WTDBOSQLITE3_LIB wtdbosqlite3)
SET(EXAMPLES_WTDBOPOSTGRES_LIB wtdbopostgres)
SET(EXAMPLES_WTDBOMYSQL_LIB wtdbomysql)

IF("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  IF(WIN32)
    # preamble to make this a toplevel CMakeLists.txt for Windows, intended
    # for being used 
    CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

    Project(WtExamples)

    find_package(Boost)
    find_package(Wt REQUIRED Wt HTTP)

    set_target_properties(Wt::Wt PROPERTIES
      MAP_IMPORTED_CONFIG_RELEASE MinSizeRel
      MAP_IMPORTED_CONFIG_RELWITHDEBINFO MinSizeRel
    )
    set_target_properties(Wt::HTTP PROPERTIES
      MAP_IMPORTED_CONFIG_RELEASE MinSizeRel
      MAP_IMPORTED_CONFIG_RELWITHDEBINFO MinSizeRel
    )

    SET(HAVE_HARU true)
    SET(HARU_LIB optimized libhpdf debug libhpdfd)

    SET(EXAMPLES_CONNECTOR Wt::HTTP)
    SET(EXAMPLES_WT_LIB Wt::Wt)
    SET(EXAMPLES_WTDBO_LIB Wt::Dbo)
    SET(EXAMPLES_WTDBOSQLITE3_LIB Wt::DboSqlite3)
    SET(EXAMPLES_WTDBOPOSTGRES_LIB Wt::DboPostgres)
    SET(EXAMPLES_WTDBOMYSQL_LIB Wt::DboMySQL)

    INCLUDE_DIRECTORIES($(CMAKE_SOURCE_DIR)/../../../include)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../../../lib)

    IF(${Boost_FOUND})
      SET(BOOST_FS_LIB ${Boost_FILESYSTEM_LIBRARY})
      SET(BOOST_WT_FOUND true)
      INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
      LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
    ENDIF()

    # We ship libharu as DLL. Compiling/linking against libharu DLL requires
    # this definition to be set.
    ADD_DEFINITIONS(-DHPDF_DLL)
    # Windows SDK is always multithreaded
    SET(BOOST_WT_MT_FOUND true)
    SET(MULTI_THREADED true)
    SET(MULTI_THREADED_BUILD true)
    SET(WT_HAS_WRASTERIMAGE true)
  ENDIF(WIN32)
ENDIF("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
SET(EXAMPLES_DBO_LIBS ${EXAMPLES_WTDBO_LIB} ${EXAMPLES_WTDBOSQLITE3_LIB}) 

# Normal example CMakeLists.txt starts here
IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    ADD_DEFINITIONS(-D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
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

  if(ANDROID)
    set(WT_ANDROID plaforms/android/wt-android)
    make_directory(${CMAKE_CURRENT_BINARY_DIR}/wt-android)
    make_directory(${CMAKE_CURRENT_BINARY_DIR}/wt-android/libs/armeabi)

    SET(MYLIST ${ARGV})
    LIST(INSERT MYLIST 1 SHARED)
    ADD_LIBRARY(${MYLIST})
    set_target_properties(${name} PROPERTIES 
      LIBRARY_OUTPUT_DIRECTORY 
      "${CMAKE_CURRENT_BINARY_DIR}/wt-android/libs/armeabi"
      OUTPUT_NAME
      wt-jni)

    string(REPLACE ".wt" "" APP_NAME ${name})
    string(LENGTH ${APP_NAME} APP_NAME_LENGTH)
    math(EXPR APP_NAME_LENGTH_MINUS_ONE "${APP_NAME_LENGTH} - 1")
    string(SUBSTRING ${APP_NAME} 0 1 APP_NAME_BEGIN)
    string(TOUPPER ${APP_NAME_BEGIN} APP_NAME_BEGIN_UPPER)
    string(SUBSTRING ${APP_NAME} 1 ${APP_NAME_LENGTH_MINUS_ONE} APP_NAME_END)
    set(APP_NAME "Wt${APP_NAME_BEGIN_UPPER}${APP_NAME_END}")

    make_directory(${CMAKE_CURRENT_BINARY_DIR}/wt-android/assets)
    set(WT_ANDROID target/android/wt-android)
    configure_file(${WT_SOURCE_DIR}/${WT_ANDROID}/src/eu/webtoolkit/android/WtAndroid.java
      ${CMAKE_CURRENT_BINARY_DIR}/wt-android/src/eu/webtoolkit/android/WtAndroid.java)
      make_directory(${CMAKE_CURRENT_BINARY_DIR}/wt-android/src/eu/webtoolkit/android/${APP_NAME}/)
     configure_file("${WT_SOURCE_DIR}/${WT_ANDROID}/src/eu/webtoolkit/android/WtExample.java.template"
      ${CMAKE_CURRENT_BINARY_DIR}/wt-android/src/eu/webtoolkit/android/${APP_NAME}/${APP_NAME}.java)

       make_directory(${CMAKE_CURRENT_BINARY_DIR}/wt-android/res)
    FILE(COPY ${WT_SOURCE_DIR}/${WT_ANDROID}/utils.xml
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/wt-android)
    
    configure_file("${WT_SOURCE_DIR}/${WT_ANDROID}/AndroidManifest.xml.template"
      ${CMAKE_CURRENT_BINARY_DIR}/wt-android/AndroidManifest.xml)

    add_custom_command(TARGET ${name}	
      POST_BUILD
      COMMAND ant -f ${CMAKE_CURRENT_BINARY_DIR}/wt-android/utils.xml
      -Dwt.dir=${WT_SOURCE_DIR} -Dexample.dir=${CMAKE_CURRENT_SOURCE_DIR} 
      -Dwt-android.dir=${CMAKE_CURRENT_BINARY_DIR}/wt-android/)

    IF(NOT DEFINED ANDROID_STRIP)
      MESSAGE(" Warning!!! ANDROID_STRIP is not defined!")
      MESSAGE(" Warning!!! The so file will not be stripped and your android")
      MESSAGE("            package might be too large to fit on the target")
      MESSAGE("            device!")
    ELSE(NOT DEFINED ANDROID_STRIP)
      add_custom_command(TARGET ${name}	
      POST_BUILD
      COMMAND ${ANDROID_STRIP} 
      "${CMAKE_CURRENT_BINARY_DIR}/wt-android/libs/armeabi/*")
    ENDIF(NOT DEFINED ANDROID_STRIP)

    add_custom_command(TARGET ${name}
    	POST_BUILD
	COMMAND ${ANDROID_SDK_DIR}/tools/android update project 
	--name wt-android --target ${ANDROID_SDK_TARGET_ID} 
	--path ${CMAKE_CURRENT_BINARY_DIR}/wt-android/)

    add_custom_command(TARGET ${name}	
      POST_BUILD
      COMMAND ant -f ${CMAKE_CURRENT_BINARY_DIR}/wt-android/build.xml debug)
  ENDIF(ANDROID)

  IF(MSVC)
    SET_TARGET_PROPERTIES(${name} PROPERTIES COMPILE_FLAGS "/W3 /Wp64 /wd4251 /wd4275")
	SET_TARGET_PROPERTIES(${name} PROPERTIES FOLDER "examples")
  ENDIF(MSVC)

  TARGET_LINK_LIBRARIES(${name} ${EXAMPLES_CONNECTOR} ${EXAMPLES_WT_LIB})

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

    SET(DOCROOTSUBFOLDER)
    SET(_${EXAMPLENAME}_POTENTIAL_DOCROOTSUBFOLDER ${WT_SOURCE_DIR}/examples/${EXAMPLESUBDIR}/docroot)
    IF(EXISTS ${_${EXAMPLENAME}_POTENTIAL_DOCROOTSUBFOLDER})
	SET(DOCROOTSUBFOLDER docroot)
    ENDIF(EXISTS ${_${EXAMPLENAME}_POTENTIAL_DOCROOTSUBFOLDER})

    SET(APPROOTSUBFOLDER)
    SET(_${EXAMPLENAME}_POTENTIAL_APPROOTSUBFOLDER ${WT_SOURCE_DIR}/examples/${EXAMPLESUBDIR}/approot)
    IF(EXISTS ${_${EXAMPLENAME}_POTENTIAL_APPROOTSUBFOLDER})
	SET(APPROOTSUBFOLDER approot)
    ENDIF(EXISTS ${_${EXAMPLENAME}_POTENTIAL_APPROOTSUBFOLDER})

    INSTALL( TARGETS ${name} DESTINATION ${EXAMPLES_DESTINATION}/${${name}_SUBDIR} )

    IF(WIN32)
      SET(SCRIPT_DOCROOT "--docroot .")
      IF(DOCROOTSUBFOLDER)
        SET(SCRIPT_DOCROOT "--docroot ${DOCROOTSUBFOLDER}")
      ENDIF(DOCROOTSUBFOLDER)
      SET(SCRIPT_APPROOT "")
      IF(APPROOTSUBFOLDER)
        SET(SCRIPT_APPROOT "--approot ${APPROOTSUBFOLDER}")
      ENDIF(APPROOTSUBFOLDER)

      CONFIGURE_FILE(${WT_SOURCE_DIR}/examples/run-example.bat.cmake ${WT_BINARY_DIR}/examples/${EXAMPLESUBDIR}/${EXAMPLENAME}.bat @ONLY)
      INSTALL(PROGRAMS ${WT_BINARY_DIR}/examples/${EXAMPLESUBDIR}/${EXAMPLENAME}.bat DESTINATION bin )
    ELSE(WIN32)
      CONFIGURE_FILE(${WT_SOURCE_DIR}/examples/run-example.cmake ${WT_BINARY_DIR}/examples/${EXAMPLESUBDIR}/${EXAMPLENAME} @ONLY)
      INSTALL(PROGRAMS ${WT_BINARY_DIR}/examples/${EXAMPLESUBDIR}/${EXAMPLENAME} DESTINATION ${EXAMPLES_DESTINATION}/${EXAMPLESUBDIR})
    ENDIF(WIN32)
  ENDIF(INSTALL_EXAMPLES)

ENDMACRO(WT_ADD_EXAMPLE)

SUBDIRS(
  blog
  charts
  chart3D
  codeview
  composer
  dialog
  dragdrop
  feature
  filetreetable
  filedrop
  form
  gitmodel
  hangman
  hello
  http-client
  javascript
  leaflet
  mandelbrot
  mission
  onethread
  painting
  planner
  qrlogin
  simplechat
  style
  te-benchmark
  treelist
  treeview
  treeview-dragdrop
  webgl
  widgetgallery
  wt-homepage
  wtwithqt
)
