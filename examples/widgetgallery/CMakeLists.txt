IF(BOOST_WT_FOUND)
SET(WIDGETGALLERY_SOURCES
  main.C
  BaseTemplate.h BaseTemplate.C
  DeferredWidget.h
  FormWidgets.h FormWidgets.C
  GraphicsWidgets.h GraphicsWidgets.C
  Layout.h Layout.C
  Media.h Media.C
  Navigation.h Navigation.C
  Sample.h
  Theme.h Theme.C
  Topic.h Topic.C
  TopicTemplate.h TopicTemplate.C
  TreesTables.h TreesTables.C
  WidgetGallery.h WidgetGallery.C
  ../treeview-dragdrop/CsvUtil.h ../treeview-dragdrop/CsvUtil.C
  examples/DataModels.h examples/DataModels.cpp
)
# no popen on windows
IF(NOT WIN32)
SET(WIDGETGALLERY_SOURCES
  ${WIDGETGALLERY_SOURCES}
  ../gitmodel/Git.h ../gitmodel/Git.C
)
ENDIF(NOT WIN32)

WT_ADD_EXAMPLE(widgetgallery.wt
  ${WIDGETGALLERY_SOURCES}
)

if(NOT WIN32)
  target_link_libraries(widgetgallery.wt ${BOOST_FS_LIB} ${BOOST_SYSTEM_LIB})
endif()

IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(widgetgallery.wt Boost::headers)
ENDIF()

if(HAVE_HARU)
  target_link_libraries(widgetgallery.wt HPDF::HPDF)
endif()

#
# If you have Wt installed somewhere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
target_include_directories(widgetgallery.wt PRIVATE ${WT_SOURCE_DIR}/src)

ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building widgetgallery example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
