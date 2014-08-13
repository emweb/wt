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
  FILE_TO_STRING(web/skeleton/Wt.js     Wt_js.C Wt_js)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(web/skeleton/Wt.min.js Wt_js.C Wt_js)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(web/skeleton/Boot.js     Boot_js.C Boot_js)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(web/skeleton/Boot.min.js Boot_js.C Boot_js)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

FILE_TO_STRING(web/skeleton/jquery.min.js JQuery_js.C JQuery_js)

FILE_TO_STRING(xml/wt.xml Wt_xml.C Wt_xml)
FILE_TO_STRING(xml/auth_strings.xml AuthStrings_xml.C AuthStrings_xml)
FILE_TO_STRING(xml/bootstrap_theme.xml
	BootstrapTheme_xml.C BootstrapTheme_xml)
FILE_TO_STRING(xml/bootstrap3_theme.xml
	Bootstrap3Theme_xml.C Bootstrap3Theme_xml)
FILE_TO_STRING(xml/auth_css_theme.xml
	AuthCssTheme_xml.C AuthCssTheme_xml)

SET(libsources
Wt/PopupWindow.C
Wt/Resizable.C
Wt/ServerSideFontMetrics.C
Wt/SizeHandle.C
Wt/StdGridLayoutImpl2.C
Wt/StdLayoutImpl.C
Wt/StdLayoutItemImpl.C
Wt/StdWidgetItemImpl.C
Wt/Utils.C
Wt/WAbstractArea.C
Wt/WAbstractItemDelegate.C
Wt/WAbstractItemModel.C
Wt/WAbstractItemView.C
Wt/WAbstractListModel.C
Wt/WAbstractMedia.C
Wt/WAbstractProxyModel.C
Wt/WAbstractSpinBox.C
Wt/WAbstractTableModel.C
Wt/WAbstractToggleButton.C
Wt/WAccordionLayout.C
Wt/WAggregateProxyModel.C
Wt/WAnchor.C
Wt/WAnimation.C
Wt/WApplication.C
Wt/WAudio.C
Wt/WBatchEditProxyModel.C
Wt/WBoostAny.C
Wt/WBootstrapTheme.C
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
Wt/WCssTheme.C
Wt/WDate.C
Wt/WDateEdit.C
Wt/WDatePicker.C
Wt/WDateTime.C
Wt/WDateValidator.C
Wt/WDefaultLayout.C
Wt/WDefaultLoadingIndicator.C
Wt/WDialog.C
Wt/WDoubleSpinBox.C
Wt/WDoubleValidator.C
Wt/WEnvironment.C
Wt/WEvent.C
Wt/WException.C
Wt/WFileResource.C
Wt/WFileUpload.C
Wt/WFitLayout.C
Wt/WFlashObject.C
Wt/WFont.C
Wt/WFontMetrics.C
Wt/WFormModel.C
Wt/WFormWidget.C
Wt/WGLWidget.C
Wt/WAbstractGLImplementation.C
Wt/WClientGLWidget.C
Wt/WGoogleMap.C
Wt/WGradient.C
Wt/WGridLayout.C
Wt/WGroupBox.C
Wt/WHBoxLayout.C
Wt/WIcon.C
Wt/WIconPair.C
Wt/WImage.C
Wt/WInPlaceEdit.C
Wt/WIntValidator.C
Wt/WInteractWidget.C
Wt/WIOService.C
Wt/WLocale.C
Wt/WLocalDateTime.C
Wt/WIdentityProxyModel.C
Wt/WItemDelegate.C
Wt/WItemSelectionModel.C
Wt/WJavaScript.C
Wt/WJavaScriptPreamble.C
Wt/WJavaScriptSlot.C
Wt/WLabel.C
Wt/WLayout.C
Wt/WLayoutItem.C
Wt/WLayoutItemImpl.C
Wt/WLength.C
Wt/WLengthValidator.C
Wt/WLineEdit.C
Wt/WLineF.C
Wt/WLink.C
Wt/WLoadingIndicator.C
Wt/WLocalizedStrings.C
Wt/WLogger.C
Wt/WMatrix4x4.C
Wt/WMeasurePaintDevice.C
Wt/WMediaPlayer.C
Wt/WMemoryResource.C
Wt/WMenu.C
Wt/WMenuItem.C
Wt/WMessageBox.C
Wt/WMessageResourceBundle.C
Wt/WMessageResources.C
Wt/WModelIndex.C
Wt/WNavigationBar.C
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
Wt/WPopupWidget.C
Wt/WProgressBar.C
Wt/WPushButton.C
Wt/WRadioButton.C
Wt/WRandom.C
Wt/WReadOnlyProxyModel.C
Wt/WRectArea.C
Wt/WRectF.C
Wt/WRegExp.C
Wt/WRegExpValidator.C
Wt/WResource.C
Wt/WScrollArea.C
Wt/WScrollBar.C
Wt/WSelectionBox.C
Wt/WServer.C
Wt/WShadow.C
Wt/WSignal.C
Wt/WSlider.C
Wt/WSocketNotifier.C
Wt/WSortFilterProxyModel.C
Wt/WSpinBox.C
Wt/WSplitButton.C
Wt/WSound.C
Wt/WSslCertificate.C
Wt/WSslInfo.C
Wt/WStackedWidget.C
Wt/WStandardItem.C
Wt/WStandardItemModel.C
Wt/WStatelessSlot.C
Wt/WString.C
Wt/WStreamResource.C
Wt/WStringListModel.C
Wt/WStringStream.C
Wt/WStringUtil.C
Wt/WSuggestionPopup.C
Wt/WSvgImage.C
Wt/WTabWidget.C
Wt/WTable.C
Wt/WTableCell.C
Wt/WTableColumn.C
Wt/WTableRow.C
Wt/WTemplate.C
Wt/WTemplateFormView.C
Wt/WTableView.C
Wt/WText.C
Wt/WTextArea.C
Wt/WTextEdit.C
Wt/WTheme.C
Wt/WTime.C
Wt/WTimer.C
Wt/WTimerWidget.C
Wt/WToolBar.C
Wt/WTransform.C
Wt/WTree.C
Wt/WTreeNode.C
Wt/WTreeTable.C
Wt/WTreeTableNode.C
Wt/WTreeView.C
Wt/WValidationStatus.C
Wt/WValidator.C
Wt/WVBoxLayout.C
Wt/WVector3.C
Wt/WVector4.C
Wt/WVectorImage.C
Wt/WVideo.C
Wt/WVmlImage.C
Wt/WViewWidget.C
Wt/WVirtualImage.C
Wt/WWebWidget.C
Wt/WWidget.C
Wt/WWidgetItem.C
Wt/Auth/AbstractPasswordService.C
Wt/Auth/AbstractUserDatabase.C
Wt/Auth/AuthModel.C
Wt/Auth/AuthService.C
Wt/Auth/AuthWidget.C
Wt/Auth/FacebookService.C
Wt/Auth/FormBaseModel.C
Wt/Auth/GoogleService.C
Wt/Auth/HashFunction.C
Wt/Auth/Identity.C
Wt/Auth/Login.C
Wt/Auth/LostPasswordWidget.C
Wt/Auth/OAuthService.C
Wt/Auth/passwdqc_check.c
Wt/Auth/PasswordHash.C
Wt/Auth/PasswordPromptDialog.C
Wt/Auth/PasswordService.C
Wt/Auth/PasswordStrengthValidator.C
Wt/Auth/PasswordVerifier.C
Wt/Auth/RegistrationModel.C
Wt/Auth/RegistrationWidget.C
Wt/Auth/Token.C
Wt/Auth/UpdatePasswordWidget.C
Wt/Auth/User.C
Wt/Auth/AuthUtils.C
Wt/Auth/MailUtils.C
Wt/Chart/WAbstractChart.C
Wt/Chart/WAxis.C
Wt/Chart/WDataSeries.C
Wt/Chart/WPieChart.C
Wt/Chart/WCartesianChart.C
Wt/Chart/WCartesian3DChart.C
Wt/Chart/WAbstractChartImplementation.C
Wt/Chart/WChart2DImplementation.C
Wt/Chart/WChart3DImplementation.C
Wt/Chart/WChartPalette.C
Wt/Chart/WStandardPalette.C
Wt/Chart/WStandardColorMap.C
Wt/Chart/WLegend.C
Wt/Chart/WLegend3D.C
Wt/Chart/WAbstractDataSeries3D.C
Wt/Chart/WAbstractGridData.C
Wt/Chart/WGridData.C
Wt/Chart/WEquidistantGridData.C
Wt/Chart/WScatterData.C
Wt/Chart/WSelection.C
Wt/Json/Array.C
Wt/Json/Object.C
Wt/Json/Parser.C
Wt/Json/Serializer.C
Wt/Json/Value.C
Wt/Http/HttpUtils.C
Wt/Http/Client.C
Wt/Http/Message.C
Wt/Http/Request.C
Wt/Http/Response.C
Wt/Http/ResponseContinuation.C
Wt/Http/WtClient.C
Wt/Mail/Client.C
Wt/Mail/Mailbox.C
Wt/Mail/Message.C
Wt/Payment/Address.C
Wt/Payment/PayPal.C
Wt/Payment/Customer.C
Wt/Payment/Money.C
Wt/Payment/Order.C
Wt/Payment/OrderItem.C
Wt/Payment/Result.C
Wt/Render/Block.C
Wt/Render/CssData.C
Wt/Render/CssData_p.C
Wt/Render/CssParser.C
Wt/Render/Line.C
Wt/Render/LayoutBox.C
Wt/Render/RenderUtils.C
Wt/Render/Specificity.C
Wt/Render/WTextRenderer.C
web/md5.c
web/sha1.c
web/CgiParser.C
web/Configuration.C
web/DomElement.C
web/EscapeOStream.C
web/FileServe.C
web/ColorUtils.C
web/ImageUtils.C
web/RefEncoder.C
web/SoundManager.C
web/WebController.C
web/WebMain.C
web/WebRequest.C
web/WebStream.C
web/WebSession.C
web/WebSocketMessage.C
web/WebRenderer.C
web/WebUtils.C
web/FileUtils.C
web/PdfUtils.C
web/TimeUtil.C
web/XSSFilter.C
web/XSSUtils.C
web/SslUtils.C
web/UriUtils.C
web/random_device.cpp
web/base64.cpp
Plain_html.C
Boot_html.C
Hybrid_html.C
Boot_js.C
Wt_js.C
Wt_xml.C
AuthStrings_xml.C
BootstrapTheme_xml.C
Bootstrap3Theme_xml.C
AuthCssTheme_xml.C
JQuery_js.C
)

ADD_DEFINITIONS(-DWT_BUILDING)

IF (MULTI_THREADED_BUILD)
  SET(libsources ${libsources} web/SocketNotifier.C)
ENDIF(MULTI_THREADED_BUILD)

# How to include x86.S ?
SET(libsources ${libsources}
      Wt/Auth/bcrypt/crypt_blowfish.c
      Wt/Auth/bcrypt/crypt_gensalt.c
      Wt/Auth/bcrypt/wrapper.c)

IF(HAVE_HARU)
  SET(libsources ${libsources} Wt/WPdfImage.C Wt/Render/WPdfRenderer.C)
  SET(BOOST_WT_LIBRARIES ${BOOST_WT_LIBRARIES} ${BOOST_FS_LIB})
  ADD_DEFINITIONS(-DHAVE_PDF_IMAGE)
ENDIF(HAVE_HARU)

IF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  SET(libsources ${libsources} Wt/WRasterImage-gm.C)
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "skia")
  SET(libsources ${libsources} Wt/WRasterImage-skia.C)
ENDIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")

IF(HAVE_GL)
  SET(libsources ${libsources} Wt/WServerGLWidget.C)
  IF(NOT USE_SYSTEM_GLEW)
    ADD_DEFINITIONS(-DGLEW_STATIC)
    SET(libsources
      ${libsources}
      3rdparty/glew-1.10.0/src/glew.c
    )
  ENDIF(NOT USE_SYSTEM_GLEW)
ENDIF(HAVE_GL)

IF(HAVE_HARU OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  IF(HAVE_PANGO)
    SET(libsources ${libsources} Wt/FontSupportPango.C)
  ELSE(HAVE_PANGO)
    SET(libsources ${libsources} Wt/FontSupportSimple.C)
  ENDIF(HAVE_PANGO)
ELSE(HAVE_HARU OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  SET(HAVE_PANGO false) # font support is not needed
ENDIF(HAVE_HARU OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")

# check if we need -lrt, used in boost::chrone::steady_clock::now()
SET(RT_test_fname "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/rt.c")
FILE(WRITE
    "${RT_test_fname}"
    "#include <time.h>\nint main() { struct timespec time1; clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1); return 0; }"
    )
TRY_COMPILE(RT_required
    "${CMAKE_BINARY_DIR}" "${RT_test_fname}"
    CMAKE_FLAGS
    -DLINK_LIBRARIES:STRING=rt
    )
IF(RT_required)
  SET(RT_LIBRARY "rt")
ENDIF(RT_required)

IF(WIN32)
  IF(SHARED_LIBS)
    CONFIGURE_FILE(Wt/wt-version.rc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/wt-version.rc)
    SET(libsources ${libsources} ${CMAKE_CURRENT_BINARY_DIR}/wt-version.rc)
  ENDIF(SHARED_LIBS)
ENDIF(WIN32)

ADD_LIBRARY(wt ${libsources})
TARGET_LINK_LIBRARIES(wt
    ${BOOST_WT_LIBRARIES}
    ${WT_SOCKET_LIBRARY}
    ${WT_MATH_LIBRARY}
    ${RT_LIBRARY}
    )

IF(ENABLE_LIBWTTEST)
  ADD_LIBRARY(wttest
    Wt/Test/WTestEnvironment.C
  )

  TARGET_LINK_LIBRARIES(wttest wt)

  INSTALL(TARGETS wttest
	  RUNTIME DESTINATION bin
	  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	  ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  IF(MSVC)
    SET_TARGET_PROPERTIES(wttest PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
  ENDIF(MSVC)

  SET_TARGET_PROPERTIES(
    wttest
  PROPERTIES
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    SOVERSION ${WTTEST_SOVERSION}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
ELSE(ENABLE_LIBWTTEST)
  MESSAGE("** Not building libwttest")
ENDIF(ENABLE_LIBWTTEST)

IF(HAVE_SSL)
  TARGET_LINK_LIBRARIES(wt ${SSL_LIBRARIES})
  INCLUDE_DIRECTORIES(${SSL_INCLUDE_DIRS})
ELSE(HAVE_SSL)
  MESSAGE("** Disabling crypto support (Auth::SHA1HashFunction, HTTPS support): requires OpenSSL.")
  IF(ENABLE_SSL)
    MESSAGE("   Indicate the location of your OpenSSL library using -DSSL_PREFIX=...")
  ENDIF(ENABLE_SSL)
ENDIF(HAVE_SSL)

IF(HAVE_HARU)
  TARGET_LINK_LIBRARIES(wt ${HARU_LIBRARIES})
  INCLUDE_DIRECTORIES(${HARU_INCLUDE_DIRS})
ELSE(HAVE_HARU)
  MESSAGE("** Disabling PDF support (WPdfImage, WPdfRenderer): requires libharu.")
  IF(ENABLE_HARU)
    MESSAGE("   Indicate the location of your haru library using -DHARU_PREFIX=...")
  ENDIF(ENABLE_HARU)
ENDIF(HAVE_HARU)

IF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  TARGET_LINK_LIBRARIES(wt ${GM_LIBRARIES})
  INCLUDE_DIRECTORIES(${GM_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DHAVE_GRAPHICSMAGICK)
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "skia")
  TARGET_LINK_LIBRARIES(wt ${SKIA_LIBRARIES})
  INCLUDE_DIRECTORIES(${SKIA_INCLUDE_DIRS})
  ADD_DEFINITIONS(${SKIA_DEFINES})
ELSE("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  MESSAGE("** Disabling raster image support (WRasterImage): set WT_WRASTERIMAGE_IMPLEMENTATION to GraphicsMagick or skia.")
ENDIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")

IF(HAVE_GL)
  INCLUDE_DIRECTORIES(${GL_INCLUDE_DIRS})
  IF(USE_SYSTEM_GLEW)
    TARGET_LINK_LIBRARIES(wt GLEW)
  ENDIF(USE_SYSTEM_GLEW)
  TARGET_LINK_LIBRARIES(wt ${GL_LIBRARIES})
ELSE(HAVE_GL)
  MESSAGE("** Disabling server-side opengl rendering for WGLWidget: requires gl, glew and WRasterImage (GraphicsMagick or skia).")
ENDIF(HAVE_GL)

IF(HAVE_PANGO)
  TARGET_LINK_LIBRARIES(wt ${PANGO_FT2_LIBRARIES})
  INCLUDE_DIRECTORIES(${PANGO_FT2_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DHAVE_PANGO)
  MESSAGE("** Enabling advanced font support using libpango")
ELSE(HAVE_PANGO)
  MESSAGE("** Disabling advanced font rendering support: requires libpango.")
  IF(ENABLE_PANGO)
    MESSAGE("   Wt will use pkg-config to retrieve your libpango installation location.")
  ENDIF(ENABLE_PANGO)
ENDIF(HAVE_PANGO)

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
  DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
)

IF(MSVC)
  SET_TARGET_PROPERTIES(wt PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
  TARGET_LINK_LIBRARIES(wt winmm)
ENDIF(MSVC)

SUBDIRS(isapi fcgi http)
