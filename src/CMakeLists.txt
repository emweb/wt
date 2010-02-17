SUBDIRS(web Wt)

INCLUDE(FindThreads)

IF(NOT BOOST_WT_FOUND)
  SET(ERR
     "Could not find a boost installation in " ${BOOST_DIR} ".\n\n"
      "There are two methods in Wt to find boost:\n\n"
      "1. Find boost through cmake (recommended)\n"
      "This requires cmake 2.6, and is in "
      "that case the default (unless BOOST_COMPILER or BOOST_VERSION is "
      "defined). This method requires a multi-threaded boost installation.\n"
      "You may need to add your boost version number to "
      "Boost_ADDITIONAL_VERSIONS, and/or set BOOST_DIR to the location where "
      "boost is installed.\n\n"
      "2. Use the Wt-proprietary method to find boost.\n"
      "This requires you to define three variables:\n"
      "BOOST_DIR: set to the directory where you installed boost\n"
      "BOOST_COMPILER: set to the compiler signature as you find them in "
      "library names (e.g.: gcc42)\n"
      "BOOST_VERSION: set to the boost version, again as you find them in "
      "the library names (e.g.: 1_38)\n\n"
      "Note: WT_BOOST_DISCOVERY is the variable that selects the boost "
      "discovery method. When set to false, method 1 is used (default if "
      "cmake version is recent enough and BOOST_COMPILER and BOOST_VERSION "
      "are not defined). When set to true, method 2 is used.\n"
      "Note 2: the code to discover boost is located in cmake/WtFindBoost.txt\n"
      "Note 3: on windows, set BOOST_DIR to the full path, eg "
      "c:/Program Files/boost/boost_1_38\n\n"
      "Wt requires the following C++ boost libraries: date_time, regex, program_options, signals, and optionally thread")
  MESSAGE(FATAL_ERROR ${ERR})
ENDIF(NOT BOOST_WT_FOUND)

IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    ADD_DEFINITIONS(-DWT_THREADED -D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
    MESSAGE("** Enabling multi threading.")
  ELSE(MULTI_THREADED)
    MESSAGE("** Disabling multi threading.")
    ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS -DSQLITE_THREADSAFE=0)
  ENDIF(MULTI_THREADED)
ELSE(BOOST_WT_MT_FOUND)
  MESSAGE("** Disabling multi threading.")
  ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS -DSQLITE_THREADSAFE=0)
ENDIF(BOOST_WT_MT_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/web
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # for WConfig.h
  ${BOOST_INCLUDE_DIRS}
  Wt/Dbo/backend/amalgamation # for sqlite3.h
)

LINK_DIRECTORIES(${BOOST_LIB_DIRS})

MACRO (FILE_TO_STRING infile outfile var)
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${outfile}
	           COMMAND ${CMAKE_COMMAND}
                   ARGS -Dinfile:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/web/skeleton/${infile}
                        -Doutfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/${outfile}
                        -Dvar:STRING=${var}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/filetostring.cmake
		   MAIN_DEPENDENCY
		        ${CMAKE_CURRENT_SOURCE_DIR}/web/skeleton/${infile}
)
ENDMACRO (FILE_TO_STRING)

FILE_TO_STRING(Plain.html    Plain_html.C    Plain_html)
FILE_TO_STRING(Hybrid.html   Hybrid_html.C   Hybrid_html)
FILE_TO_STRING(Boot.html     Boot_html.C     Boot_html)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(Wt.js     Wt_js.C         Wt_js)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(Wt.min.js     Wt_js.C         Wt_js)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

FILE_TO_STRING(CommAjax.js   CommAjax_js.C   CommAjax_js)
FILE_TO_STRING(CommScript.js CommScript_js.C CommScript_js)
FILE_TO_STRING(jquery.min.js JQuery_js.C     JQuery_js)

SET(libsources
Wt/StdGridLayoutImpl.C
Wt/StdLayoutImpl.C
Wt/StdLayoutItemImpl.C
Wt/StdWidgetItemImpl.C
Wt/WAbstractArea.C
Wt/WAbstractItemDelegate.C
Wt/WAbstractItemModel.C
Wt/WAbstractItemView.C
Wt/WAbstractListModel.C
Wt/WAbstractProxyModel.C
Wt/WAbstractTableModel.C
Wt/WAbstractToggleButton.C
Wt/WAccordionLayout.C
Wt/WAggregateProxyModel.C
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
Wt/WCombinedLocalizedStrings.C
Wt/WComboBox.C
Wt/WCompositeWidget.C
Wt/WContainerWidget.C
Wt/WCssDecorationStyle.C
Wt/WCssStyleSheet.C
Wt/WDate.C
Wt/WDatePicker.C
Wt/WDateTime.C
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
Wt/WFlashObject.C
Wt/WFont.C
Wt/WFormWidget.C
Wt/WGoogleMap.C
Wt/WGridLayout.C
Wt/WGroupBox.C
Wt/WHBoxLayout.C
Wt/WIconPair.C
Wt/WImage.C
Wt/WInPlaceEdit.C
Wt/WIntValidator.C
Wt/WInteractWidget.C
Wt/WItemDelegate.C
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
Wt/WLocalizedStrings.C
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
Wt/WRegExp.C
Wt/WRegExpValidator.C
Wt/WResource.C
Wt/WScrollArea.C
Wt/WScrollBar.C
Wt/WSelectionBox.C
Wt/WSignal.C
Wt/WSlider.C
Wt/WSocketNotifier.C
Wt/WSortFilterProxyModel.C
Wt/WSound.C
Wt/WStackedWidget.C
Wt/WStandardItem.C
Wt/WStandardItemModel.C
Wt/WStatelessSlot.C
Wt/WString.C
Wt/WSubMenuItem.C
Wt/WStringListModel.C
Wt/WStringUtil.C
Wt/WSuggestionPopup.C
Wt/WSvgImage.C
Wt/WTabWidget.C
Wt/WTable.C
Wt/WTableCell.C
Wt/WTableColumn.C
Wt/WTableRow.C
Wt/WTemplate.C
Wt/WTableView.C
Wt/WText.C
Wt/WTextArea.C
Wt/WTextEdit.C
Wt/WTime.C
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
Wt/Http/Client.C
Wt/Http/Request.C
Wt/Http/Response.C
Wt/Http/ResponseContinuation.C
Wt/Test/WTestEnvironment.C
web/CgiParser.C
web/Configuration.C
web/DomElement.C
web/EscapeOStream.C
web/FileServe.C
web/SoundManager.C
web/Utils.C
web/XSSFilter.C
web/XSSUtils.C
web/WebRequest.C
web/WebStream.C
web/WebSession.C
web/WebRenderer.C
web/WebController.C
web/WtException.C
web/WtRandom.C
web/TimeUtil.C
web/random_device.cpp
Plain_html.C
Boot_html.C
Hybrid_html.C
Wt_js.C
CommAjax_js.C
CommScript_js.C
JQuery_js.C
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
TARGET_LINK_LIBRARIES(wt ${BOOST_WT_LIBRARIES})

IF(BOOST_WT_MT_FOUND)
  TARGET_LINK_LIBRARIES(wt ${CMAKE_THREAD_LIBS_INIT})
ENDIF(BOOST_WT_MT_FOUND)

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
  SET_TARGET_PROPERTIES(wt PROPERTIES COMPILE_FLAGS "/wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101")
  TARGET_LINK_LIBRARIES(wt winmm wsock32)
ENDIF(WIN32)

SUBDIRS(fcgi http)
