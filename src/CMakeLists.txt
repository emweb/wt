SUBDIRS(web Wt)

INCLUDE(FindThreads)

IF(NOT BOOST_WT_FOUND)
  MESSAGE("** Error finding Wt required library: ")
  MESSAGE("** Could not find a boost installation in " ${BOOST_DIR} ".")
  MESSAGE("** It may be necessary to set appropriate values for the")
  MESSAGE("   variables BOOST_DIR, BOOST_COMPILER, and BOOST_VERSION")
  MESSAGE(FATAL_ERROR "** Wt requires the following C++ boost libraries: boost_date_time, boost_regex, and boost_program_options, boost_signals, and optionally boost_thread")
ENDIF(NOT BOOST_WT_FOUND)

IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    ADD_DEFINITIONS(-DTHREADED -D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
    MESSAGE("** Enabling multi threading.")
  ELSE(MULTI_THREADED)
    MESSAGE("** Disabling multi threading.")
    ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
  ENDIF(MULTI_THREADED)
ELSE(BOOST_WT_MT_FOUND)
  MESSAGE("** Disabling multi threading.")
  ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
ENDIF(BOOST_WT_MT_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/web
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # for WConfig.h
  ${MXML_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${BOOST_LIB_DIRS})

ADD_EXECUTABLE(filetostring web/skeleton/FileToString.C)
GET_TARGET_PROPERTY(FILE_TO_STRING_EXE filetostring LOCATION)

MACRO (FILE_TO_STRING infile outfile var)
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${outfile}
	           COMMAND ${FILE_TO_STRING_EXE}
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/web/skeleton/${infile}
                        ${CMAKE_CURRENT_BINARY_DIR}/${outfile}
                        ${var}
		   MAIN_DEPENDENCY
		        ${CMAKE_CURRENT_SOURCE_DIR}/web/skeleton/${infile}
                   DEPENDS
		        filetostring)
ENDMACRO (FILE_TO_STRING)

FILE_TO_STRING(Plain.html    Plain_html.C    Plain_html)
FILE_TO_STRING(JsNoAjax.html JsNoAjax_html.C JsNoAjax_html)
FILE_TO_STRING(Boot.html     Boot_html.C     Boot_html)
FILE_TO_STRING(Wt.js         Wt_js.C         Wt_js)
FILE_TO_STRING(CommAjax.js   CommAjax_js.C   CommAjax_js)
FILE_TO_STRING(CommScript.js CommScript_js.C CommScript_js)

SET(libsources
${MXML_SOURCES}
Wt/StdGridLayoutImpl.C
Wt/StdLayoutImpl.C
Wt/StdLayoutItemImpl.C
Wt/StdWidgetItemImpl.C
Wt/WAbstractArea.C
Wt/WAbstractItemModel.C
Wt/WAbstractListModel.C
Wt/WAbstractProxyModel.C
Wt/WAbstractToggleButton.C
Wt/WAccordionLayout.C
Wt/WAnchor.C
Wt/WApplication.C
Wt/WBorder.C
Wt/WBorderLayout.C
Wt/WBoxLayout.C
Wt/WBreak.C
Wt/WBrush.C
Wt/WButtonGroup.C
Wt/WCalendar.C
Wt/WCanvasPaintDevice.C
Wt/WCheckBox.C
Wt/WCircleArea.C
Wt/WColor.C
Wt/WComboBox.C
Wt/WCompositeWidget.C
Wt/WContainerWidget.C
Wt/WCssDecorationStyle.C
Wt/WCssStyleSheet.C
Wt/WDate.C
Wt/WDatePicker.C
Wt/WDateValidator.C
Wt/WDefaultLayout.C
Wt/WDefaultLoadingIndicator.C
Wt/WDialog.C
Wt/WDoubleValidator.C
Wt/WEnvironment.C
Wt/WEvent.C
Wt/WFileResource.C
Wt/WFileUpload.C
Wt/WFitLayout.C
Wt/WFont.C
Wt/WFormWidget.C
Wt/WGridLayout.C
Wt/WGroupBox.C
Wt/WHBoxLayout.C
Wt/WIconPair.C
Wt/WImage.C
Wt/WInPlaceEdit.C
Wt/WIntValidator.C
Wt/WInteractWidget.C
Wt/WItemSelectionModel.C
Wt/WJavaScript.C
Wt/WJavaScriptSlot.C
Wt/WLabel.C
Wt/WLayout.C
Wt/WLayoutItem.C
Wt/WLayoutItemImpl.C
Wt/WLength.C
Wt/WLengthValidator.C
Wt/WLineEdit.C
Wt/WLineF.C
Wt/WLoadingIndicator.C
Wt/WLogger.C
Wt/WMemoryResource.C
Wt/WMenu.C
Wt/WMenuItem.C
Wt/WMessageBox.C
Wt/WMessageResourceBundle.C
Wt/WMessageResources.C
Wt/WModelIndex.C
Wt/WObject.C
Wt/WOverlayLoadingIndicator.C
Wt/WPaintDevice.C
Wt/WPaintedWidget.C
Wt/WPainter.C
Wt/WPainterPath.C
Wt/WPanel.C
Wt/WPen.C
Wt/WPoint.C
Wt/WPointF.C
Wt/WPolygonArea.C
Wt/WPopupMenu.C
Wt/WPopupMenuItem.C
Wt/WPushButton.C
Wt/WRadioButton.C
Wt/WRectArea.C
Wt/WRectF.C
Wt/WRegExpValidator.C
Wt/WResource.C
Wt/WScrollArea.C
Wt/WScrollBar.C
Wt/WSelectionBox.C
Wt/WSignal.C
Wt/WSlider.C
Wt/WSocketNotifier.C
Wt/WSortFilterProxyModel.C
Wt/WStackedWidget.C
Wt/WStandardItem.C
Wt/WStandardItemModel.C
Wt/WStatelessSlot.C
Wt/WString.C
Wt/WSubMenuItem.C
Wt/WLocalizedStrings.C
Wt/WStringListModel.C
Wt/WStringUtil.C
Wt/WSuggestionPopup.C
Wt/WSvgImage.C
Wt/WTabWidget.C
Wt/WTable.C
Wt/WTableCell.C
Wt/WTableColumn.C
Wt/WTableRow.C
Wt/WText.C
Wt/WTextArea.C
Wt/WTextEdit.C
Wt/WTimer.C
Wt/WTimerWidget.C
Wt/WTransform.C
Wt/WTree.C
Wt/WTreeNode.C
Wt/WTreeTable.C
Wt/WTreeTableNode.C
Wt/WTreeView.C
Wt/WValidationStatus.C
Wt/WValidator.C
Wt/WVBoxLayout.C
Wt/WVectorImage.C
Wt/WVmlImage.C
Wt/WViewWidget.C
Wt/WVirtualImage.C
Wt/WWebWidget.C
Wt/WWidget.C
Wt/WWidgetItem.C
web/CgiParser.C
web/Configuration.C
web/DomElement.C
web/EscapeOStream.C
web/FileServe.C
web/Utils.C
web/WebRequest.C
web/WebStream.C
web/WebSession.C
web/WebRenderer.C
web/WebController.C
web/WServerPushResource.C
web/WtException.C
web/WtRandom.C
web/TimeUtil.C
web/random_device.cpp
Plain_html.C
JsNoAjax_html.C
Boot_html.C
Wt_js.C
CommAjax_js.C
CommScript_js.C
Wt/Chart/WAbstractChart.C
Wt/Chart/WAxis.C
Wt/Chart/WDataSeries.C
Wt/Chart/WPieChart.C
Wt/Chart/WCartesianChart.C
Wt/Chart/WChart2DRenderer.C
Wt/Chart/WChartPalette.C
Wt/Chart/WStandardPalette.C
)

ADD_LIBRARY(wt ${libsources})
TARGET_LINK_LIBRARIES(wt ${BOOST_WT_LIBRARIES} ${MXML_LIBRARIES})
INSTALL(TARGETS wt
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/WtInstall.cmake
  ${WT_BINARY_DIR}/WtInstall.cmake
)

SET_TARGET_PROPERTIES(
  wt
PROPERTIES
  POST_INSTALL_SCRIPT ${WT_BINARY_DIR}/WtInstall.cmake
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  SOVERSION ${WT_SOVERSION}
  DEBUG_POSTFIX "d"
)

IF(WIN32)
  SET_TARGET_PROPERTIES(wt PROPERTIES COMPILE_FLAGS "/wd4251 /wd4275 /wd4355")
  TARGET_LINK_LIBRARIES(wt winmm wsock32)
ENDIF(WIN32)

SUBDIRS(fcgi http)

ADD_LIBRARY(wtext
  Wt/Ext/AbstractButton.C
  Wt/Ext/AbstractToggleButton.C
  Wt/Ext/AccordionLayoutImpl.C
  Wt/Ext/BorderLayoutImpl.C
  Wt/Ext/Button.C
  Wt/Ext/Calendar.C
  Wt/Ext/CheckBox.C
  Wt/Ext/ComboBox.C
  Wt/Ext/Component.C
  Wt/Ext/Container.C
  Wt/Ext/DataStore.C
  Wt/Ext/DateField.C
  Wt/Ext/DefaultLayoutImpl.C
  Wt/Ext/Dialog.C
  Wt/Ext/FitLayoutImpl.C
  Wt/Ext/FormField.C
  Wt/Ext/LayoutImpl.C
  Wt/Ext/LayoutItemImpl.C
  Wt/Ext/LineEdit.C
  Wt/Ext/Menu.C
  Wt/Ext/MenuItem.C
  Wt/Ext/MessageBox.C
  Wt/Ext/NumberField.C
  Wt/Ext/PagingToolBar.C
  Wt/Ext/Panel.C
  Wt/Ext/ProgressDialog.C
  Wt/Ext/RadioButton.C
  Wt/Ext/Splitter.C
  Wt/Ext/SplitterHandle.C
  Wt/Ext/TableView.C
  Wt/Ext/TabWidget.C
  Wt/Ext/TextEdit.C
  Wt/Ext/ToolBar.C
  Wt/Ext/ToolTipConfig.C
  Wt/Ext/Widget.C
  Wt/Ext/WWidgetItemImpl.C
)

TARGET_LINK_LIBRARIES(wtext wt ${BOOST_WT_LIBRARIES})
INSTALL(TARGETS wtext
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

IF(WIN32)
  SET_TARGET_PROPERTIES(wtext PROPERTIES COMPILE_FLAGS "/wd4251 /wd4275 /wd4355")
ENDIF(WIN32)

SET_TARGET_PROPERTIES(
  wtext
PROPERTIES
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  SOVERSION ${WTEXT_SOVERSION}
  DEBUG_POSTFIX "d"
)

