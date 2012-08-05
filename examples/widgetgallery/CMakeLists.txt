WT_ADD_EXAMPLE(widgetgallery.wt
  main.C
  BasicControls.C
  ChartWidgets.C
  ControlsWidget.C
  DialogWidgets.C
  EventsDemo.C
  EventDisplayer.C
  WidgetGallery.C
  ExtWidgets.C
  FormWidgets.C
  GraphicsWidgets.C
  SpecialPurposeWidgets.C
  MvcWidgets.C
  Validators.C
  StyleLayout.C
  PaintBrush.C
  EmwebLoadingIndicator.C
  ../charts/ChartsExample.C
  ../charts/ChartConfig.C
  ../charts/CsvUtil.C
  ../charts/PanelList.C
  ../dragdrop/Character.C
  ../dragdrop/DragExample.C
  ../painting/PaintExample.C
  ../painting/ShapesWidget.C
  ../treeview/TreeViewExample.C
)
TARGET_LINK_LIBRARIES(widgetgallery.wt wtext)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ../charts
  ../dragdrop
  ../painting
  ../treeview
)