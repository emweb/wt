SUBDIRS(web Wt)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/web
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wt/Date/include
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

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(js/WCartesianChart.js WCartesianChart_js.C WCartesianChart_js)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FILE_TO_STRING(js/WCartesianChart.min.js WCartesianChart_js.C WCartesianChart_js)
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
Wt/FlexLayoutImpl.h Wt/FlexLayoutImpl.C
Wt/FlexItemImpl.h Wt/FlexItemImpl.C
Wt/PopupWindow.h Wt/PopupWindow.C
Wt/Resizable.h Wt/Resizable.C
Wt/ResizeSensor.h Wt/ResizeSensor.C
Wt/ServerSideFontMetrics.h Wt/ServerSideFontMetrics.C
Wt/SizeHandle.h Wt/SizeHandle.C
Wt/StdGridLayoutImpl2.h Wt/StdGridLayoutImpl2.C
Wt/StdLayoutImpl.h Wt/StdLayoutImpl.C
Wt/StdLayoutItemImpl.h Wt/StdLayoutItemImpl.C
Wt/StdWidgetItemImpl.h Wt/StdWidgetItemImpl.C
Wt/Utils.h Wt/Utils.C
Wt/WAbstractArea.h Wt/WAbstractArea.C
Wt/WAbstractItemDelegate.h Wt/WAbstractItemDelegate.C
Wt/WAbstractItemModel.h Wt/WAbstractItemModel.C
Wt/WAbstractItemView.h Wt/WAbstractItemView.C
Wt/WAbstractListModel.h Wt/WAbstractListModel.C
Wt/WAbstractMedia.h Wt/WAbstractMedia.C
Wt/WAbstractProxyModel.h Wt/WAbstractProxyModel.C
Wt/WAbstractSpinBox.h Wt/WAbstractSpinBox.C
Wt/WAbstractTableModel.h Wt/WAbstractTableModel.C
Wt/WAbstractToggleButton.h Wt/WAbstractToggleButton.C
Wt/WAggregateProxyModel.h Wt/WAggregateProxyModel.C
Wt/WAnchor.h Wt/WAnchor.C
Wt/WAnimation.h Wt/WAnimation.C
Wt/WAny.h Wt/WAny.C
Wt/WApplication.h Wt/WApplication.C
Wt/WAudio.h Wt/WAudio.C
Wt/WBatchEditProxyModel.h Wt/WBatchEditProxyModel.C
Wt/WBootstrapTheme.h Wt/WBootstrapTheme.C
Wt/WBorder.h Wt/WBorder.C
Wt/WBorderLayout.h Wt/WBorderLayout.C
Wt/WBoxLayout.h Wt/WBoxLayout.C
Wt/WBreak.h Wt/WBreak.C
Wt/WBrush.h Wt/WBrush.C
Wt/WButtonGroup.h Wt/WButtonGroup.C
Wt/WCalendar.h Wt/WCalendar.C
Wt/WCanvasPaintDevice.h Wt/WCanvasPaintDevice.C
Wt/WCheckBox.h Wt/WCheckBox.C
Wt/WCircleArea.h Wt/WCircleArea.C
Wt/WColor.h Wt/WColor.C
Wt/WCombinedLocalizedStrings.h Wt/WCombinedLocalizedStrings.C
Wt/WComboBox.h Wt/WComboBox.C
Wt/WCompositeWidget.h Wt/WCompositeWidget.C
Wt/WContainerWidget.h Wt/WContainerWidget.C
Wt/WCssDecorationStyle.h Wt/WCssDecorationStyle.C
Wt/WCssStyleSheet.h Wt/WCssStyleSheet.C
Wt/WCssTheme.h Wt/WCssTheme.C
Wt/WDate.h Wt/WDate.C
Wt/WDateEdit.h Wt/WDateEdit.C
Wt/WDatePicker.h Wt/WDatePicker.C
Wt/WDateTime.h Wt/WDateTime.C
Wt/WDateValidator.h Wt/WDateValidator.C
Wt/WDefaultLoadingIndicator.h Wt/WDefaultLoadingIndicator.C
Wt/WDialog.h Wt/WDialog.C
Wt/WDoubleSpinBox.h Wt/WDoubleSpinBox.C
Wt/WDoubleValidator.h Wt/WDoubleValidator.C
Wt/WEnvironment.h Wt/WEnvironment.C
Wt/WEvent.h Wt/WEvent.C
Wt/WException.h Wt/WException.C
Wt/WFileDropWidget.h Wt/WFileDropWidget.C
Wt/WFileResource.h Wt/WFileResource.C
Wt/WFileUpload.h Wt/WFileUpload.C
Wt/WFitLayout.h Wt/WFitLayout.C
Wt/WFlashObject.h Wt/WFlashObject.C
Wt/WFont.h Wt/WFont.C
Wt/WFontMetrics.h Wt/WFontMetrics.C
Wt/WFormModel.h Wt/WFormModel.C
Wt/WFormWidget.h Wt/WFormWidget.C
Wt/WGLWidget.h Wt/WGLWidget.C
Wt/WAbstractGLImplementation.h Wt/WAbstractGLImplementation.C
Wt/WClientGLWidget.h Wt/WClientGLWidget.C
Wt/WGoogleMap.h Wt/WGoogleMap.C
Wt/WGradient.h Wt/WGradient.C
Wt/WGridLayout.h Wt/WGridLayout.C
Wt/WGroupBox.h Wt/WGroupBox.C
Wt/WHBoxLayout.h Wt/WHBoxLayout.C
Wt/WIcon.h Wt/WIcon.C
Wt/WIconPair.h Wt/WIconPair.C
Wt/WImage.h Wt/WImage.C
Wt/WInPlaceEdit.h Wt/WInPlaceEdit.C
Wt/WIntValidator.h Wt/WIntValidator.C
Wt/WInteractWidget.h Wt/WInteractWidget.C
Wt/WIOService.h Wt/WIOService.C
Wt/WLocale.h Wt/WLocale.C
Wt/WLocalDateTime.h Wt/WLocalDateTime.C
Wt/WIdentityProxyModel.h Wt/WIdentityProxyModel.C
Wt/WItemDelegate.h Wt/WItemDelegate.C
Wt/WItemSelectionModel.h Wt/WItemSelectionModel.C
Wt/WJavaScript.h Wt/WJavaScript.C
Wt/WJavaScriptExposableObject.h Wt/WJavaScriptExposableObject.C
Wt/WJavaScriptHandle.h Wt/WJavaScriptHandle.C
Wt/WJavaScriptObjectStorage.h Wt/WJavaScriptObjectStorage.C
Wt/WJavaScriptPreamble.h Wt/WJavaScriptPreamble.C
Wt/WJavaScriptSlot.h Wt/WJavaScriptSlot.C
Wt/WLabel.h Wt/WLabel.C
Wt/WLayout.h Wt/WLayout.C
Wt/WLayoutImpl.h Wt/WLayoutImpl.C
Wt/WLayoutItem.h Wt/WLayoutItem.C
Wt/WLayoutItemImpl.h Wt/WLayoutItemImpl.C
Wt/WLeafletMap.h Wt/WLeafletMap.C
Wt/WLength.h Wt/WLength.C
Wt/WLengthValidator.h Wt/WLengthValidator.C
Wt/WLineEdit.h Wt/WLineEdit.C
Wt/WLineF.h Wt/WLineF.C
Wt/WLink.h Wt/WLink.C
Wt/WLinkedCssStyleSheet.h Wt/WLinkedCssStyleSheet.C
Wt/WLoadingIndicator.h Wt/WLoadingIndicator.C
Wt/WLocalizedStrings.h Wt/WLocalizedStrings.C
Wt/WLogger.h Wt/WLogger.C
Wt/WMatrix4x4.h Wt/WMatrix4x4.C
Wt/WMeasurePaintDevice.h Wt/WMeasurePaintDevice.C
Wt/WMediaPlayer.h Wt/WMediaPlayer.C
Wt/WMemoryResource.h Wt/WMemoryResource.C
Wt/WMenu.h Wt/WMenu.C
Wt/WMenuItem.h Wt/WMenuItem.C
Wt/WMessageBox.h Wt/WMessageBox.C
Wt/WMessageResourceBundle.h Wt/WMessageResourceBundle.C
Wt/WMessageResources.h Wt/WMessageResources.C
Wt/WModelIndex.h Wt/WModelIndex.C
Wt/WNavigationBar.h Wt/WNavigationBar.C
Wt/WObject.h Wt/WObject.C
Wt/WOverlayLoadingIndicator.h Wt/WOverlayLoadingIndicator.C
Wt/WPaintDevice.h Wt/WPaintDevice.C
Wt/WPaintedWidget.h Wt/WPaintedWidget.C
Wt/WPainter.h Wt/WPainter.C
Wt/WPainterPath.h Wt/WPainterPath.C
Wt/WPanel.h Wt/WPanel.C
Wt/WPen.h Wt/WPen.C
Wt/WPoint.h Wt/WPoint.C
Wt/WPointF.h Wt/WPointF.C
Wt/WPolygonArea.h Wt/WPolygonArea.C
Wt/WPopupMenu.h Wt/WPopupMenu.C
Wt/WPopupWidget.h Wt/WPopupWidget.C
Wt/WProgressBar.h Wt/WProgressBar.C
Wt/WPushButton.h Wt/WPushButton.C
Wt/WRadioButton.h Wt/WRadioButton.C
Wt/WRandom.h Wt/WRandom.C
Wt/WReadOnlyProxyModel.h Wt/WReadOnlyProxyModel.C
Wt/WRectArea.h Wt/WRectArea.C
Wt/WRectF.h Wt/WRectF.C
Wt/WRegExpValidator.h Wt/WRegExpValidator.C
Wt/WResource.h Wt/WResource.C
Wt/WSelectionBox.h Wt/WSelectionBox.C
Wt/WServer.h Wt/WServer.C
Wt/WShadow.h Wt/WShadow.C
Wt/WSignal.h Wt/WSignal.C
Wt/WSlider.h Wt/WSlider.C
Wt/WSocketNotifier.h Wt/WSocketNotifier.C
Wt/WSortFilterProxyModel.h Wt/WSortFilterProxyModel.C
Wt/WSpinBox.h Wt/WSpinBox.C
Wt/WSplitButton.h Wt/WSplitButton.C
Wt/WSound.h Wt/WSound.C
Wt/WSslCertificate.h Wt/WSslCertificate.C
Wt/WSslInfo.h Wt/WSslInfo.C
Wt/WStackedWidget.h Wt/WStackedWidget.C
Wt/WStandardItem.h Wt/WStandardItem.C
Wt/WStandardItemModel.h Wt/WStandardItemModel.C
Wt/WStatelessSlot.h Wt/WStatelessSlot.C
Wt/WString.h Wt/WString.C
Wt/WStreamResource.h Wt/WStreamResource.C
Wt/WStringListModel.h Wt/WStringListModel.C
Wt/WStringStream.h Wt/WStringStream.C
Wt/WStringUtil.h Wt/WStringUtil.C
Wt/WSuggestionPopup.h Wt/WSuggestionPopup.C
Wt/WSvgImage.h Wt/WSvgImage.C
Wt/WTabWidget.h Wt/WTabWidget.C
Wt/WTable.h Wt/WTable.C
Wt/WTableCell.h Wt/WTableCell.C
Wt/WTableColumn.h Wt/WTableColumn.C
Wt/WTableRow.h Wt/WTableRow.C
Wt/WTemplate.h Wt/WTemplate.C
Wt/WTemplateFormView.h Wt/WTemplateFormView.C
Wt/WTableView.h Wt/WTableView.C
Wt/WText.h Wt/WText.C
Wt/WTextArea.h Wt/WTextArea.C
Wt/WTextEdit.h Wt/WTextEdit.C
Wt/WTheme.h Wt/WTheme.C
Wt/WTime.h Wt/WTime.C
Wt/WTimeEdit.h Wt/WTimeEdit.C
Wt/WTimer.h Wt/WTimer.C
Wt/WTimerWidget.h Wt/WTimerWidget.C
Wt/WTimePicker.h Wt/WTimePicker.C
Wt/WTimeValidator.h Wt/WTimeValidator.C
Wt/WToolBar.h Wt/WToolBar.C
Wt/WTransform.h Wt/WTransform.C
Wt/WTree.h Wt/WTree.C
Wt/WTreeNode.h Wt/WTreeNode.C
Wt/WTreeTable.h Wt/WTreeTable.C
Wt/WTreeTableNode.h Wt/WTreeTableNode.C
Wt/WTreeView.h Wt/WTreeView.C
Wt/WValidator.h Wt/WValidator.C
Wt/WVBoxLayout.h Wt/WVBoxLayout.C
Wt/WVector3.h Wt/WVector3.C
Wt/WVector4.h Wt/WVector4.C
Wt/WVectorImage.h Wt/WVectorImage.C
Wt/WVideo.h Wt/WVideo.C
Wt/WVmlImage.h Wt/WVmlImage.C
Wt/WViewWidget.h Wt/WViewWidget.C
Wt/WVirtualImage.h Wt/WVirtualImage.C
Wt/WWebWidget.h Wt/WWebWidget.C
Wt/WWidget.h Wt/WWidget.C
Wt/WWidgetItem.h Wt/WWidgetItem.C
Wt/WWidgetItemImpl.h Wt/WWidgetItemImpl.C
Wt/Auth/AbstractPasswordService.h Wt/Auth/AbstractPasswordService.C
Wt/Auth/AbstractUserDatabase.h Wt/Auth/AbstractUserDatabase.C
Wt/Auth/AuthModel.h Wt/Auth/AuthModel.C
Wt/Auth/AuthService.h Wt/Auth/AuthService.C
Wt/Auth/AuthWidget.h Wt/Auth/AuthWidget.C
Wt/Auth/FacebookService.h Wt/Auth/FacebookService.C
Wt/Auth/FormBaseModel.h Wt/Auth/FormBaseModel.C
Wt/Auth/GoogleService.h Wt/Auth/GoogleService.C
Wt/Auth/HashFunction.h Wt/Auth/HashFunction.C
Wt/Auth/Identity.h Wt/Auth/Identity.C
Wt/Auth/Login.h Wt/Auth/Login.C
Wt/Auth/LostPasswordWidget.h Wt/Auth/LostPasswordWidget.C
Wt/Auth/OAuthService.h Wt/Auth/OAuthService.C
Wt/Auth/OAuthWidget.h Wt/Auth/OAuthWidget.C
Wt/Auth/passwdqc.h Wt/Auth/passwdqc_check.c
Wt/Auth/PasswordHash.h Wt/Auth/PasswordHash.C
Wt/Auth/PasswordPromptDialog.h Wt/Auth/PasswordPromptDialog.C
Wt/Auth/PasswordService.h Wt/Auth/PasswordService.C
Wt/Auth/PasswordStrengthValidator.h Wt/Auth/PasswordStrengthValidator.C
Wt/Auth/PasswordVerifier.h Wt/Auth/PasswordVerifier.C
Wt/Auth/RegistrationModel.h Wt/Auth/RegistrationModel.C
Wt/Auth/RegistrationWidget.h Wt/Auth/RegistrationWidget.C
Wt/Auth/Token.h Wt/Auth/Token.C
Wt/Auth/UpdatePasswordWidget.h Wt/Auth/UpdatePasswordWidget.C
Wt/Auth/User.h Wt/Auth/User.C
Wt/Auth/AuthUtils.h Wt/Auth/AuthUtils.C
Wt/Auth/MailUtils.h Wt/Auth/MailUtils.C
Wt/Auth/OAuthClient.h Wt/Auth/OAuthClient.C
Wt/Auth/OAuthTokenEndpoint.h Wt/Auth/OAuthTokenEndpoint.C
Wt/Auth/OidcUserInfoEndpoint.h Wt/Auth/OidcUserInfoEndpoint.C
Wt/Auth/OAuthAuthorizationEndpointProcess.h Wt/Auth/OAuthAuthorizationEndpointProcess.C
Wt/Auth/IssuedToken.h Wt/Auth/IssuedToken.C
Wt/Auth/OidcService.h Wt/Auth/OidcService.C
Wt/Chart/WAbstractChart.h Wt/Chart/WAbstractChart.C
Wt/Chart/WAbstractChartModel.h Wt/Chart/WAbstractChartModel.C
Wt/Chart/WAxis.h Wt/Chart/WAxis.C
Wt/Chart/WAxisSliderWidget.h Wt/Chart/WAxisSliderWidget.C
Wt/Chart/WDataSeries.h Wt/Chart/WDataSeries.C
Wt/Chart/WPieChart.h Wt/Chart/WPieChart.C
Wt/Chart/WCartesianChart.h Wt/Chart/WCartesianChart.C
Wt/Chart/WCartesian3DChart.h Wt/Chart/WCartesian3DChart.C
Wt/Chart/WAbstractChartImplementation.h Wt/Chart/WAbstractChartImplementation.C
Wt/Chart/WChart2DImplementation.h Wt/Chart/WChart2DImplementation.C
Wt/Chart/WChart3DImplementation.h Wt/Chart/WChart3DImplementation.C
Wt/Chart/WChartPalette.h Wt/Chart/WChartPalette.C
Wt/Chart/WStandardChartProxyModel.h Wt/Chart/WStandardChartProxyModel.C
Wt/Chart/WStandardPalette.h Wt/Chart/WStandardPalette.C
Wt/Chart/WStandardColorMap.h Wt/Chart/WStandardColorMap.C
Wt/Chart/WLegend.h Wt/Chart/WLegend.C
Wt/Chart/WLegend3D.h Wt/Chart/WLegend3D.C
Wt/Chart/WAbstractDataSeries3D.h Wt/Chart/WAbstractDataSeries3D.C
Wt/Chart/WAbstractGridData.h Wt/Chart/WAbstractGridData.C
Wt/Chart/WGridData.h Wt/Chart/WGridData.C
Wt/Chart/WEquidistantGridData.h Wt/Chart/WEquidistantGridData.C
Wt/Chart/WScatterData.h Wt/Chart/WScatterData.C
Wt/Chart/WSelection.h Wt/Chart/WSelection.C
Wt/Core/observable.hpp Wt/Core/observable.cpp
Wt/Core/observing_ptr.hpp Wt/Core/observing_ptr.cpp
Wt/cpp17/any.hpp Wt/cpp17/any/any.hpp
Wt/Date/date.h Wt/Date/include/date/date.h
Wt/Date/tz.h Wt/Date/include/date/tz.h Wt/Date/src/tz.cpp
Wt/Json/Array.h Wt/Json/Array.C
Wt/Json/Object.h Wt/Json/Object.C
Wt/Json/Parser.h Wt/Json/Parser.C
Wt/Json/Serializer.h Wt/Json/Serializer.C
Wt/Json/Value.h Wt/Json/Value.C
Wt/Http/HttpUtils.h Wt/Http/HttpUtils.C
Wt/Http/Client.h Wt/Http/Client.C
Wt/Http/Message.h Wt/Http/Message.C
Wt/Http/Request.h Wt/Http/Request.C
Wt/Http/Response.h Wt/Http/Response.C
Wt/Http/ResponseContinuation.h Wt/Http/ResponseContinuation.C
Wt/Http/WtClient.h Wt/Http/WtClient.C
Wt/Mail/Client.h Wt/Mail/Client.C
Wt/Mail/Mailbox.h Wt/Mail/Mailbox.C
Wt/Mail/Message.h Wt/Mail/Message.C
Wt/Payment/Address.h Wt/Payment/Address.C
Wt/Payment/PayPal.h Wt/Payment/PayPal.C
Wt/Payment/Customer.h Wt/Payment/Customer.C
Wt/Payment/Money.h Wt/Payment/Money.C
Wt/Payment/Order.h Wt/Payment/Order.C
Wt/Payment/OrderItem.h Wt/Payment/OrderItem.C
Wt/Payment/Result.h Wt/Payment/Result.C
Wt/Render/Block.h Wt/Render/Block.C
Wt/Render/CssData.h Wt/Render/CssData.C
Wt/Render/CssData_p.h Wt/Render/CssData_p.C
Wt/Render/CssParser.h Wt/Render/CssParser.C
Wt/Render/Line.h Wt/Render/Line.C
Wt/Render/LayoutBox.h Wt/Render/LayoutBox.C
Wt/Render/RenderUtils.h Wt/Render/RenderUtils.C
Wt/Render/Specificity.h Wt/Render/Specificity.C
Wt/Render/WTextRenderer.h Wt/Render/WTextRenderer.C
Wt/Signals/signals.hpp Wt/Signals/signals.cpp
web/md5.h web/md5.c
web/sha1.h web/sha1.c
web/CgiParser.h web/CgiParser.C
web/Configuration.h web/Configuration.C
web/DomElement.h web/DomElement.C
web/EntryPoint.h web/EntryPoint.C
web/EscapeOStream.h web/EscapeOStream.C
web/FileServe.h web/FileServe.C
web/ColorUtils.h web/ColorUtils.C
web/ImageUtils.h web/ImageUtils.C
web/RefEncoder.h web/RefEncoder.C
web/SoundManager.h web/SoundManager.C
web/WebController.h web/WebController.C
web/WebMain.h web/WebMain.C
web/WebRequest.h web/WebRequest.C
web/WebStream.h web/WebStream.C
web/WebSession.h web/WebSession.C
web/WebSocketMessage.h web/WebSocketMessage.C
web/WebRenderer.h web/WebRenderer.C
web/WebUtils.h web/WebUtils.C
web/FileUtils.h web/FileUtils.C
web/PdfUtils.h web/PdfUtils.C
web/StringUtils.h web/StringUtils.cpp
web/TimeUtil.h web/TimeUtil.C
web/XSSFilter.h web/XSSFilter.C
web/XSSUtils.h web/XSSUtils.C
web/SslUtils.h web/SslUtils.C
web/UriUtils.h web/UriUtils.C
web/base64.h web/base64.cpp
Plain_html.C
Boot_html.C
Hybrid_html.C
Boot_js.C
Wt_js.C
WCartesianChart_js.C
Wt_xml.C
AuthStrings_xml.C
BootstrapTheme_xml.C
Bootstrap3Theme_xml.C
AuthCssTheme_xml.C
JQuery_js.C
)

set(_TZ_BUILD_DEFS "")

IF (WIN32)
  set(_TZ_BUILD_DEFS USE_OS_TZDB=0 HAS_REMOTE_API=0)
ELSE (WIN32)
  set(_TZ_BUILD_DEFS USE_OS_TZDB=1)
ENDIF (WIN32)

IF (SHARED_LIBS)
  list(APPEND _TZ_BUILD_DEFS DATE_BUILD_DLL)
ELSE (SHARED_LIBS)
  list(APPEND _TZ_BUILD_DEFS DATE_BUILD_LIB)
ENDIF (SHARED_LIBS)

set_source_files_properties(Wt/Date/src/tz.cpp PROPERTIES COMPILE_DEFINITIONS "${_TZ_BUILD_DEFS}")

unset(_TZ_BUILD_DEFS)

ADD_DEFINITIONS(-DWT_BUILDING)

IF (MULTI_THREADED_BUILD)
  SET(libsources ${libsources} web/SocketNotifier.h web/SocketNotifier.C)
ENDIF(MULTI_THREADED_BUILD)

# How to include x86.S ?
SET(libsources ${libsources}
      Wt/Auth/bcrypt/crypt_blowfish.h Wt/Auth/bcrypt/crypt_blowfish.c
      Wt/Auth/bcrypt/crypt_gensalt.h Wt/Auth/bcrypt/crypt_gensalt.c
      Wt/Auth/bcrypt/wrapper.c)

IF(HAVE_HARU)
  SET(libsources ${libsources} Wt/WPdfImage.h Wt/WPdfImage.C Wt/Render/WPdfRenderer.h Wt/Render/WPdfRenderer.C)
  ADD_DEFINITIONS(-DHAVE_PDF_IMAGE)
ENDIF(HAVE_HARU)

IF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  SET(libsources ${libsources} Wt/WRasterImage.h Wt/WRasterImage-gm.C)
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "skia")
  SET(libsources ${libsources} Wt/WRasterImage.h Wt/WRasterImage-skia.C)
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
  SET(libsources ${libsources} Wt/WRasterImage.h Wt/WRasterImage-d2d1.C)
ENDIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")

IF(HAVE_GL)
  SET(libsources ${libsources} Wt/WServerGLWidget.h Wt/WServerGLWidget.C)
  IF(NOT USE_SYSTEM_GLEW)
    ADD_DEFINITIONS(-DGLEW_STATIC)
    SET(libsources
      ${libsources}
      3rdparty/glew-1.10.0/include/GL/glew.h
      3rdparty/glew-1.10.0/include/GL/glxew.h
      3rdparty/glew-1.10.0/include/GL/wglew.h
      3rdparty/glew-1.10.0/src/glew.c
    )
  ENDIF(NOT USE_SYSTEM_GLEW)
ENDIF(HAVE_GL)

SET(WT_FONTSUPPORT_SIMPLE false)
SET(WT_FONTSUPPORT_PANGO false)
SET(WT_FONTSUPPORT_DIRECTWRITE false)
SET(HAVE_DIRECTWRITE false)
IF(WIN32 AND NOT MINGW)
  SET(HAVE_DIRECTWRITE true)
ENDIF(WIN32 AND NOT MINGW)

IF(HAVE_HARU OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick" OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
  IF(HAVE_PANGO AND NOT "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
    SET(libsources ${libsources} Wt/FontSupport.h Wt/FontSupportPango.C)
    SET(WT_FONTSUPPORT_PANGO true)
  ELSEIF(HAVE_DIRECTWRITE)
    SET(libsources ${libsources} Wt/FontSupport.h Wt/FontSupportDirectWrite.C)
    SET(WT_FONTSUPPORT_DIRECTWRITE true)
  ELSE(HAVE_PANGO AND NOT "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
    SET(libsources ${libsources} Wt/FontSupport.h Wt/FontSupportSimple.C)
    SET(WT_FONTSUPPORT_SIMPLE true)
  ENDIF(HAVE_PANGO AND NOT "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
ENDIF(HAVE_HARU OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick" OR "${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")

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

SET_PROPERTY(TARGET wt PROPERTY CXX_VISIBILITY_PRESET hidden)
SET_PROPERTY(TARGET wt PROPERTY VISIBILITY_INLINES_HIDDEN YES)

if (WT_ASIO_IS_STANDALONE_ASIO)
  TARGET_LINK_LIBRARIES(wt
    PRIVATE ${BOOST_WT_LIBRARIES})
else (WT_ASIO_IS_STANDALONE_ASIO)
  TARGET_LINK_LIBRARIES(wt
    PUBLIC ${BOOST_WT_LIBRARIES})
endif (WT_ASIO_IS_STANDALONE_ASIO)

TARGET_LINK_LIBRARIES(wt
  PUBLIC
    ${WT_SOCKET_LIBRARY}
  PRIVATE
    ${WT_MATH_LIBRARY}
    ${RT_LIBRARY}
    )

IF(ENABLE_LIBWTTEST)
  ADD_LIBRARY(wttest
    Wt/Test/WTestEnvironment.C
  )

  SET_PROPERTY(TARGET wttest PROPERTY CXX_VISIBILITY_PRESET hidden)
  SET_PROPERTY(TARGET wttest PROPERTY VISIBILITY_INLINES_HIDDEN YES)

  TARGET_LINK_LIBRARIES(wttest PUBLIC wt)

  IF(TARGET Boost::headers)
    TARGET_LINK_LIBRARIES(
      wttest
    PRIVATE
      Boost::headers
    )
  ENDIF()

  IF(MULTI_THREADED_BUILD)
    TARGET_LINK_LIBRARIES(wttest PRIVATE ${WT_THREAD_LIB})
  ENDIF(MULTI_THREADED_BUILD)

  INSTALL(TARGETS wttest
          EXPORT wt-target-test
	  RUNTIME DESTINATION bin
	  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	  ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

  INSTALL(EXPORT wt-target-test
          DESTINATION ${CMAKE_INSTALL_DIR}/wt
          NAMESPACE Wt::)

  IF(MSVC)
    SET_TARGET_PROPERTIES(wttest PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
	SET_TARGET_PROPERTIES(wttest PROPERTIES FOLDER "test")
  ENDIF(MSVC)

  SET_TARGET_PROPERTIES(
    wttest
  PROPERTIES
    EXPORT_NAME Test
    VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
    DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
  )
ELSE(ENABLE_LIBWTTEST)
  MESSAGE("** Not building libwttest")
ENDIF(ENABLE_LIBWTTEST)

IF(HAVE_SSL)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${OPENSSL_LIBRARIES})
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  IF(WIN32)
    TARGET_LINK_LIBRARIES(wt PRIVATE Crypt32.lib)
  ENDIF(WIN32)
ELSE(HAVE_SSL)
  MESSAGE("** Disabling crypto support (Auth::SHA1HashFunction, HTTPS support): requires OpenSSL.")
  IF(ENABLE_SSL)
    MESSAGE("   Indicate the location of your OpenSSL library using -DSSL_PREFIX=...")
  ENDIF(ENABLE_SSL)
ENDIF(HAVE_SSL)

IF(HAVE_HARU)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${HARU_LIBRARIES})
  INCLUDE_DIRECTORIES(${HARU_INCLUDE_DIRS})
ELSE(HAVE_HARU)
  MESSAGE("** Disabling PDF support (WPdfImage, WPdfRenderer): requires libharu.")
  IF(ENABLE_HARU)
    MESSAGE("   Indicate the location of your haru library using -DHARU_PREFIX=...")
  ENDIF(ENABLE_HARU)
ENDIF(HAVE_HARU)

IF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  TARGET_LINK_LIBRARIES(wt PRIVATE ${GM_LIBRARIES})
  INCLUDE_DIRECTORIES(${GM_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DHAVE_GRAPHICSMAGICK)
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "skia")
  TARGET_LINK_LIBRARIES(wt PRIVATE ${SKIA_LIBRARIES})
  INCLUDE_DIRECTORIES(${SKIA_INCLUDE_DIRS})
  ADD_DEFINITIONS(${SKIA_DEFINES})
ELSEIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "Direct2D")
  TARGET_LINK_LIBRARIES(wt PRIVATE D2d1 Dwrite Windowscodecs Shlwapi)
ELSE("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")
  MESSAGE("** Disabling raster image support (WRasterImage): set WT_WRASTERIMAGE_IMPLEMENTATION to GraphicsMagick or skia.")
ENDIF("${WT_WRASTERIMAGE_IMPLEMENTATION}" STREQUAL "GraphicsMagick")

IF(HAVE_GL)
  INCLUDE_DIRECTORIES(${GL_INCLUDE_DIRS})
  IF(USE_SYSTEM_GLEW)
    TARGET_LINK_LIBRARIES(wt PRIVATE GLEW)
  ENDIF(USE_SYSTEM_GLEW)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${GL_LIBRARIES})
ELSE(HAVE_GL)
  MESSAGE("** Disabling server-side opengl rendering for WGLWidget: requires gl, glew and WRasterImage (GraphicsMagick or skia).")
ENDIF(HAVE_GL)

IF(WT_FONTSUPPORT_SIMPLE)
  ADD_DEFINITIONS(-DWT_FONTSUPPORT_SIMPLE)
  MESSAGE("** Disabling advanced font rendering support: requires libpango.")
  IF(ENABLE_PANGO)
    MESSAGE("   Wt will use pkg-config to retrieve your libpango installation location.")
  ENDIF(ENABLE_PANGO)
ELSEIF(WT_FONTSUPPORT_PANGO)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${PANGO_FT2_LIBRARIES})
  INCLUDE_DIRECTORIES(${PANGO_FT2_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DWT_FONTSUPPORT_PANGO)
  MESSAGE("** Enabling advanced font support using libpango")
ELSEIF(WT_FONTSUPPORT_DIRECTWRITE)
  TARGET_LINK_LIBRARIES(wt PRIVATE Dwrite)
  ADD_DEFINITIONS(-DWT_FONTSUPPORT_DIRECTWRITE)
  MESSAGE("** Enabling advanced font support using DirectWrite")
ENDIF(WT_FONTSUPPORT_SIMPLE)

IF(HAVE_UNWIND)
  ADD_DEFINITIONS(-DWT_WITH_UNWIND)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${UNWIND_LIBRARIES})
  INCLUDE_DIRECTORIES(${UNWIND_INCLUDE_DIRS})
  MESSAGE("** Enabling stacktrace prints using libunwind")
ELSE(HAVE_UNWIND)
  IF(ENABLE_UNWIND)
    MESSAGE("** Disabling stacktrace prints: could not find libunwind")
  ENDIF(ENABLE_UNWIND)
ENDIF(HAVE_UNWIND)

IF(MULTI_THREADED_BUILD)
  TARGET_LINK_LIBRARIES(wt PRIVATE ${WT_THREAD_LIB})
ENDIF(MULTI_THREADED_BUILD)

TARGET_INCLUDE_DIRECTORIES(wt INTERFACE
  $<INSTALL_INTERFACE:include>)

INSTALL(TARGETS wt
    EXPORT wt-target-wt
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

INSTALL(EXPORT wt-target-wt
        DESTINATION ${CMAKE_INSTALL_DIR}/wt
        NAMESPACE Wt::)

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/WtInstall.cmake
  ${WT_BINARY_DIR}/WtInstall.cmake
)

SET_TARGET_PROPERTIES(
  wt
PROPERTIES
  EXPORT_NAME Wt
  POST_INSTALL_SCRIPT ${WT_BINARY_DIR}/WtInstall.cmake
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
)

IF(MSVC)
  SET_TARGET_PROPERTIES(wt PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
  TARGET_LINK_LIBRARIES(wt PRIVATE winmm)
ENDIF(MSVC)

SUBDIRS(isapi fcgi http)
