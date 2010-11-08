SUBDIRS(web Wt)

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
                   ARGS -Dinfile:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/${infile}
                        -Doutfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/${outfile}
                        -Dvar:STRING=${var}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/filetostring.cmake
		   MAIN_DEPENDENCY
		        ${CMAKE_CURRENT_SOURCE_DIR}/${infile}
)
ENDMACRO (FILE_TO_STRING)

FILE_TO_STRING(web/skeleton/Plain.html    Plain_html.C      Plain_html)
FILE_TO_STRING(web/skeleton/Hybrid.html   Hybrid_html.C     Hybrid_html)
FILE_TO_STRING(web/skeleton/Boot.html     Boot_html.C       Boot_html)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(web/skeleton/Wt.js     Wt_js.C         Wt_js)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(web/skeleton/Wt.min.js     Wt_js.C         Wt_js)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

FILE_TO_STRING(web/skeleton/CommAjax.js   CommAjax_js.C   CommAjax_js)
FILE_TO_STRING(web/skeleton/CommScript.js CommScript_js.C CommScript_js)
FILE_TO_STRING(web/skeleton/jquery.min.js JQuery_js.C     JQuery_js)

FILE_TO_STRING(xml/wt.xml                 WtMessages_xml.C  WtMessages_xml)

SET(libsources
Wt/SizeHandle.C
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
Wt/WAbstractServer.C
Wt/WAbstractTableModel.C
Wt/WAbstractToggleButton.C
Wt/WAccordionLayout.C
Wt/WAggregateProxyModel.C
Wt/WAnchor.C
Wt/WApplication.C
Wt/WBatchEditProxyModel.C
Wt/WBoostAny.C
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
Wt/WGLWidget.C
Wt/WGoogleMap.C
Wt/WGridLayout.C
Wt/WGroupBox.C
Wt/WHBoxLayout.C
Wt/WHTML5Audio.C
Wt/WHTML5Media.C
Wt/WHTML5Video.C
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
Wt/WMatrix4x4.C
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
Wt/WProgressBar.C
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
Wt/WShadow.C
Wt/WSignal.C
Wt/WSlider.C
Wt/WSocketNotifier.C
Wt/WSortFilterProxyModel.C
Wt/WSpinBox.C
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
web/WebSocketMessage.C
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
WtMessages_xml.C
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

IF (MULTI_THREADED_BUILD)
  SET(libsources ${libsources} web/SocketNotifier.C)
ENDIF(MULTI_THREADED_BUILD)

IF(HAVE_HARU)
  SET(libsources ${libsources} Wt/WPdfImage.C)
ENDIF(HAVE_HARU)

IF(HAVE_GM)
  SET(libsources ${libsources} Wt/WRasterImage.C)
ENDIF(HAVE_GM)

ADD_LIBRARY(wt ${libsources})
TARGET_LINK_LIBRARIES(wt ${BOOST_WT_LIBRARIES})

IF(HAVE_HARU)
  TARGET_LINK_LIBRARIES(wt ${HARU_LIBRARIES})
  INCLUDE_DIRECTORIES(${HARU_INCLUDE_DIRS})
ELSE(HAVE_HARU)
  MESSAGE("** Disabling PDF support (WPdfImage): requires libharu.")
  IF(ENABLE_HARU)
    MESSAGE("   Indicate the location of your haru library using -DHARU_PREFIX=...")
  ENDIF(ENABLE_HARU)
ENDIF(HAVE_HARU)

IF(HAVE_GM)
  TARGET_LINK_LIBRARIES(wt ${GM_LIBRARIES})
  INCLUDE_DIRECTORIES(${GM_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DHAVE_RASTER_IMAGE)
ELSE(HAVE_GM)
  MESSAGE("** Disabling raster image support (WRasterImage): requires graphicsmagick.")
  IF(ENABLE_GM)
    MESSAGE("   Indicate the location of your graphicsmagick library using -DGM_PREFIX=...")
  ENDIF(ENABLE_GM)
ENDIF(HAVE_GM)

IF(MULTI_THREADED_BUILD)
  TARGET_LINK_LIBRARIES(wt ${CMAKE_THREAD_LIBS_INIT})
ENDIF(MULTI_THREADED_BUILD)

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
  TARGET_LINK_LIBRARIES(wt ws2_32)
ENDIF(WIN32)

IF(MSVC)
  SET_TARGET_PROPERTIES(wt PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
  TARGET_LINK_LIBRARIES(wt winmm)
ENDIF(MSVC)

SUBDIRS(isapi fcgi http)
