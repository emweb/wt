IF(BOOST_WT_FOUND)
SET(WIDGETGALLERY_SOURCES
  main.C
  # EmwebLoadingIndicator.C
  # EventsDemo.C
  FormWidgets.C
  GraphicsWidgets.C
  Layout.C
  Media.C
  Navigation.C
  # Services.C
  TopicTemplate.C
  TopicWidget.C
  TreesTables.C
  WidgetGallery.C
  ../treeview-dragdrop/CsvUtil.C
  examples/DataModels.cpp
  # ../charts/ChartsExample.C
  # ../charts/ChartConfig.C
  # ../charts/CsvUtil.C
  # ../charts/PanelList.C
  # ../dragdrop/Character.C
  # ../dragdrop/DragExample.C
  # ../painting/PaintExample.C
  # ../painting/ShapesWidget.C
  # ../treeview/TreeViewExample.C
)
# no popen on windows
IF(NOT WIN32)
SET(WIDGETGALLERY_SOURCES
  ${WIDGETGALLERY_SOURCES}
  ../gitmodel/Git.C
)
ENDIF(NOT WIN32)

WT_ADD_EXAMPLE(widgetgallery.wt
  ${WIDGETGALLERY_SOURCES}
)

IF(TARGET Boost::headers)
  TARGET_LINK_LIBRARIES(widgetgallery.wt Boost::headers)
ENDIF()

IF (HAVE_HARU)
  INCLUDE_DIRECTORIES(${HARU_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(widgetgallery.wt ${HARU_LIB})
ENDIF(HAVE_HARU)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ../charts
  ../painting
  ../gitmodel
)
ELSE(BOOST_WT_FOUND)
  MESSAGE(STATUS "** Not building widgetgallery example: requires boost headers.")
ENDIF(BOOST_WT_FOUND)
