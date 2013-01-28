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

IF (HAVE_HARU)
  INCLUDE_DIRECTORIES(${HARU_INCLUDE_DIRS})
ENDIF(HAVE_HARU)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

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
