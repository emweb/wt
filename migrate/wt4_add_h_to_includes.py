#!/usr/bin/env python3

substitutions_string = """\
WQApplication,WQApplication.h
Wt/Auth/AbstractPasswordService,Wt/Auth/AbstractPasswordService.h
Wt/Auth/AbstractUserDatabase,Wt/Auth/AbstractUserDatabase.h
Wt/Auth/AuthModel,Wt/Auth/AuthModel.h
Wt/Auth/AuthService,Wt/Auth/AuthService.h
Wt/Auth/AuthWidget,Wt/Auth/AuthWidget.h
Wt/Auth/Dbo/AuthInfo,Wt/Auth/Dbo/AuthInfo.h
Wt/Auth/Dbo/UserDatabase,Wt/Auth/Dbo/UserDatabase.h
Wt/Auth/FacebookService,Wt/Auth/FacebookService.h
Wt/Auth/FormBaseModel,Wt/Auth/FormBaseModel.h
Wt/Auth/GoogleService,Wt/Auth/GoogleService.h
Wt/Auth/HashFunction,Wt/Auth/HashFunction.h
Wt/Auth/Identity,Wt/Auth/Identity.h
Wt/Auth/IssuedToken,Wt/Auth/IssuedToken.h
Wt/Auth/Login,Wt/Auth/Login.h
Wt/Auth/LostPasswordWidget,Wt/Auth/LostPasswordWidget.h
Wt/Auth/OAuthAuthorizationEndpointProcess,Wt/Auth/OAuthAuthorizationEndpointProcess.h
Wt/Auth/OAuthClient,Wt/Auth/OAuthClient.h
Wt/Auth/OAuthService,Wt/Auth/OAuthService.h
Wt/Auth/OAuthTokenEndpoint,Wt/Auth/OAuthTokenEndpoint.h
Wt/Auth/OAuthWidget,Wt/Auth/OAuthWidget.h
Wt/Auth/OidcService,Wt/Auth/OidcService.h
Wt/Auth/OidcUserInfoEndpoint,Wt/Auth/OidcUserInfoEndpoint.h
Wt/Auth/PasswordHash,Wt/Auth/PasswordHash.h
Wt/Auth/PasswordPromptDialog,Wt/Auth/PasswordPromptDialog.h
Wt/Auth/PasswordService,Wt/Auth/PasswordService.h
Wt/Auth/PasswordStrengthValidator,Wt/Auth/PasswordStrengthValidator.h
Wt/Auth/PasswordVerifier,Wt/Auth/PasswordVerifier.h
Wt/Auth/RegistrationModel,Wt/Auth/RegistrationModel.h
Wt/Auth/RegistrationWidget,Wt/Auth/RegistrationWidget.h
Wt/Auth/Token,Wt/Auth/Token.h
Wt/Auth/UpdatePasswordWidget,Wt/Auth/UpdatePasswordWidget.h
Wt/Auth/User,Wt/Auth/User.h
Wt/Auth/WAuthGlobal,Wt/Auth/WAuthGlobal.h
Wt/Chart/WAbstractChart,Wt/Chart/WAbstractChart.h
Wt/Chart/WAbstractChartImplementation,Wt/Chart/WAbstractChartImplementation.h
Wt/Chart/WAbstractChartModel,Wt/Chart/WAbstractChartModel.h
Wt/Chart/WAbstractColorMap,Wt/Chart/WAbstractColorMap.h
Wt/Chart/WAbstractDataSeries3D,Wt/Chart/WAbstractDataSeries3D.h
Wt/Chart/WAbstractGridData,Wt/Chart/WAbstractGridData.h
Wt/Chart/WAxis,Wt/Chart/WAxis.h
Wt/Chart/WAxisSliderWidget,Wt/Chart/WAxisSliderWidget.h
Wt/Chart/WCartesian3DChart,Wt/Chart/WCartesian3DChart.h
Wt/Chart/WCartesianChart,Wt/Chart/WCartesianChart.h
Wt/Chart/WChart2DImplementation,Wt/Chart/WChart2DImplementation.h
Wt/Chart/WChart3DImplementation,Wt/Chart/WChart3DImplementation.h
Wt/Chart/WChartGlobal,Wt/Chart/WChartGlobal.h
Wt/Chart/WChartPalette,Wt/Chart/WChartPalette.h
Wt/Chart/WDataSeries,Wt/Chart/WDataSeries.h
Wt/Chart/WEquidistantGridData,Wt/Chart/WEquidistantGridData.h
Wt/Chart/WGridData,Wt/Chart/WGridData.h
Wt/Chart/WLegend,Wt/Chart/WLegend.h
Wt/Chart/WLegend3D,Wt/Chart/WLegend3D.h
Wt/Chart/WPieChart,Wt/Chart/WPieChart.h
Wt/Chart/WScatterData,Wt/Chart/WScatterData.h
Wt/Chart/WSelection,Wt/Chart/WSelection.h
Wt/Chart/WStandardChartProxyModel,Wt/Chart/WStandardChartProxyModel.h
Wt/Chart/WStandardColorMap,Wt/Chart/WStandardColorMap.h
Wt/Chart/WStandardPalette,Wt/Chart/WStandardPalette.h
Wt/Dbo/Call,Wt/Dbo/Call.h
Wt/Dbo/DbAction,Wt/Dbo/DbAction.h
Wt/Dbo/Dbo,Wt/Dbo/Dbo.h
Wt/Dbo/Exception,Wt/Dbo/Exception.h
Wt/Dbo/Field,Wt/Dbo/Field.h
Wt/Dbo/FixedSqlConnectionPool,Wt/Dbo/FixedSqlConnectionPool.h
Wt/Dbo/Impl,Wt/Dbo/Impl.h
Wt/Dbo/Json,Wt/Dbo/Json.h
Wt/Dbo/Query,Wt/Dbo/Query.h
Wt/Dbo/QueryColumn,Wt/Dbo/QueryColumn.h
Wt/Dbo/QueryModel,Wt/Dbo/QueryModel.h
Wt/Dbo/Session,Wt/Dbo/Session.h
Wt/Dbo/SqlConnection,Wt/Dbo/SqlConnection.h
Wt/Dbo/SqlConnectionPool,Wt/Dbo/SqlConnectionPool.h
Wt/Dbo/SqlStatement,Wt/Dbo/SqlStatement.h
Wt/Dbo/SqlTraits,Wt/Dbo/SqlTraits.h
Wt/Dbo/StdSqlTraits,Wt/Dbo/StdSqlTraits.h
Wt/Dbo/Transaction,Wt/Dbo/Transaction.h
Wt/Dbo/Types,Wt/Dbo/Types.h
Wt/Dbo/WtSqlTraits,Wt/Dbo/WtSqlTraits.h
Wt/Dbo/backend/Firebird,Wt/Dbo/backend/Firebird.h
Wt/Dbo/backend/MySQL,Wt/Dbo/backend/MySQL.h
Wt/Dbo/backend/Postgres,Wt/Dbo/backend/Postgres.h
Wt/Dbo/backend/Sqlite3,Wt/Dbo/backend/Sqlite3.h
Wt/Dbo/backend/MSSQLServer,Wt/Dbo/backend/MSSQLServer.h
Wt/Dbo/collection,Wt/Dbo/collection.h
Wt/Dbo/ptr,Wt/Dbo/ptr.h
Wt/Dbo/ptr_tuple,Wt/Dbo/ptr_tuple.h
Wt/Dbo/weak_ptr,Wt/Dbo/weak_ptr.h
Wt/Http/Client,Wt/Http/Client.h
Wt/Http/Message,Wt/Http/Message.h
Wt/Http/Method,Wt/Http/Method.h
Wt/Http/Request,Wt/Http/Request.h
Wt/Http/Response,Wt/Http/Response.h
Wt/Http/ResponseContinuation,Wt/Http/ResponseContinuation.h
Wt/Http/WtClient,Wt/Http/WtClient.h
Wt/Json/Array,Wt/Json/Array.h
Wt/Json/Object,Wt/Json/Object.h
Wt/Json/Parser,Wt/Json/Parser.h
Wt/Json/Serializer,Wt/Json/Serializer.h
Wt/Json/Value,Wt/Json/Value.h
Wt/Mail/Client,Wt/Mail/Client.h
Wt/Mail/Mailbox,Wt/Mail/Mailbox.h
Wt/Mail/Message,Wt/Mail/Message.h
Wt/Payment/Address,Wt/Payment/Address.h
Wt/Payment/Customer,Wt/Payment/Customer.h
Wt/Payment/Money,Wt/Payment/Money.h
Wt/Payment/Order,Wt/Payment/Order.h
Wt/Payment/OrderItem,Wt/Payment/OrderItem.h
Wt/Payment/PayPal,Wt/Payment/PayPal.h
Wt/Payment/Result,Wt/Payment/Result.h
Wt/Render/WPdfRenderer,Wt/Render/WPdfRenderer.h
Wt/Render/WTextRenderer,Wt/Render/WTextRenderer.h
Wt/Test/WTestEnvironment,Wt/Test/WTestEnvironment.h
Wt/Utils,Wt/Utils.h
Wt/WAbstractArea,Wt/WAbstractArea.h
Wt/WAbstractGLImplementation,Wt/WAbstractGLImplementation.h
Wt/WAbstractItemDelegate,Wt/WAbstractItemDelegate.h
Wt/WAbstractItemModel,Wt/WAbstractItemModel.h
Wt/WAbstractItemView,Wt/WAbstractItemView.h
Wt/WAbstractListModel,Wt/WAbstractListModel.h
Wt/WAbstractMedia,Wt/WAbstractMedia.h
Wt/WAbstractProxyModel,Wt/WAbstractProxyModel.h
Wt/WAbstractSpinBox,Wt/WAbstractSpinBox.h
Wt/WAbstractTableModel,Wt/WAbstractTableModel.h
Wt/WAbstractToggleButton,Wt/WAbstractToggleButton.h
Wt/WAggregateProxyModel,Wt/WAggregateProxyModel.h
Wt/WAnchor,Wt/WAnchor.h
Wt/WAnimation,Wt/WAnimation.h
Wt/WAny,Wt/WAny.h
Wt/WApplication,Wt/WApplication.h
Wt/WAudio,Wt/WAudio.h
Wt/WBatchEditProxyModel,Wt/WBatchEditProxyModel.h
Wt/WBoostAny,Wt/WAny.h
Wt/WBootstrapTheme,Wt/WBootstrapTheme.h
Wt/WBorder,Wt/WBorder.h
Wt/WBorderLayout,Wt/WBorderLayout.h
Wt/WBoxLayout,Wt/WBoxLayout.h
Wt/WBreak,Wt/WBreak.h
Wt/WBrush,Wt/WBrush.h
Wt/WButtonGroup,Wt/WButtonGroup.h
Wt/WCalendar,Wt/WCalendar.h
Wt/WCanvasPaintDevice,Wt/WCanvasPaintDevice.h
Wt/WCheckBox,Wt/WCheckBox.h
Wt/WCircleArea,Wt/WCircleArea.h
Wt/WClientGLWidget,Wt/WClientGLWidget.h
Wt/WColor,Wt/WColor.h
Wt/WCombinedLocalizedStrings,Wt/WCombinedLocalizedStrings.h
Wt/WComboBox,Wt/WComboBox.h
Wt/WCompositeWidget,Wt/WCompositeWidget.h
Wt/WContainerWidget,Wt/WContainerWidget.h
Wt/WCssDecorationStyle,Wt/WCssDecorationStyle.h
Wt/WCssStyleSheet,Wt/WCssStyleSheet.h
Wt/WCssTheme,Wt/WCssTheme.h
Wt/WDate,Wt/WDate.h
Wt/WDateEdit,Wt/WDateEdit.h
Wt/WDatePicker,Wt/WDatePicker.h
Wt/WDateTime,Wt/WDateTime.h
Wt/WDateValidator,Wt/WDateValidator.h
Wt/WDefaultLoadingIndicator,Wt/WDefaultLoadingIndicator.h
Wt/WDialog,Wt/WDialog.h
Wt/WDoubleSpinBox,Wt/WDoubleSpinBox.h
Wt/WDoubleValidator,Wt/WDoubleValidator.h
Wt/WEnvironment,Wt/WEnvironment.h
Wt/WEvent,Wt/WEvent.h
Wt/WException,Wt/WException.h
Wt/WFileDropWidget,Wt/WFileDropWidget.h
Wt/WFileResource,Wt/WFileResource.h
Wt/WFileUpload,Wt/WFileUpload.h
Wt/WFitLayout,Wt/WFitLayout.h
Wt/WFlags,Wt/WFlags.h
Wt/WFlashObject,Wt/WFlashObject.h
Wt/WFont,Wt/WFont.h
Wt/WFontMetrics,Wt/WFontMetrics.h
Wt/WFormModel,Wt/WFormModel.h
Wt/WFormWidget,Wt/WFormWidget.h
Wt/WGLWidget,Wt/WGLWidget.h
Wt/WGenericMatrix,Wt/WGenericMatrix.h
Wt/WGlobal,Wt/WGlobal.h
Wt/WGoogleMap,Wt/WGoogleMap.h
Wt/WGradient,Wt/WGradient.h
Wt/WGridLayout,Wt/WGridLayout.h
Wt/WGroupBox,Wt/WGroupBox.h
Wt/WHBoxLayout,Wt/WHBoxLayout.h
Wt/WIOService,Wt/WIOService.h
Wt/WIcon,Wt/WIcon.h
Wt/WIconPair,Wt/WIconPair.h
Wt/WIdentityProxyModel,Wt/WIdentityProxyModel.h
Wt/WImage,Wt/WImage.h
Wt/WInPlaceEdit,Wt/WInPlaceEdit.h
Wt/WIntValidator,Wt/WIntValidator.h
Wt/WInteractWidget,Wt/WInteractWidget.h
Wt/WItemDelegate,Wt/WItemDelegate.h
Wt/WItemSelectionModel,Wt/WItemSelectionModel.h
Wt/WJavaScript,Wt/WJavaScript.h
Wt/WJavaScriptExposableObject,Wt/WJavaScriptExposableObject.h
Wt/WJavaScriptHandle,Wt/WJavaScriptHandle.h
Wt/WJavaScriptObjectStorage,Wt/WJavaScriptObjectStorage.h
Wt/WJavaScriptPreamble,Wt/WJavaScriptPreamble.h
Wt/WJavaScriptSlot,Wt/WJavaScriptSlot.h
Wt/WLabel,Wt/WLabel.h
Wt/WLayout,Wt/WLayout.h
Wt/WLayoutImpl,Wt/WLayoutImpl.h
Wt/WLayoutItem,Wt/WLayoutItem.h
Wt/WLayoutItemImpl,Wt/WLayoutItemImpl.h
Wt/WLeafletMap,Wt/WLeafletMap.h
Wt/WLength,Wt/WLength.h
Wt/WLengthValidator,Wt/WLengthValidator.h
Wt/WLineEdit,Wt/WLineEdit.h
Wt/WLineF,Wt/WLineF.h
Wt/WLink,Wt/WLink.h
Wt/WLinkedCssStyleSheet,Wt/WLinkedCssStyleSheet.h
Wt/WLoadingIndicator,Wt/WLoadingIndicator.h
Wt/WLocalDateTime,Wt/WLocalDateTime.h
Wt/WLocale,Wt/WLocale.h
Wt/WLocalizedStrings,Wt/WLocalizedStrings.h
Wt/WLogger,Wt/WLogger.h
Wt/WMatrix4x4,Wt/WMatrix4x4.h
Wt/WMeasurePaintDevice,Wt/WMeasurePaintDevice.h
Wt/WMediaPlayer,Wt/WMediaPlayer.h
Wt/WMemoryResource,Wt/WMemoryResource.h
Wt/WMenu,Wt/WMenu.h
Wt/WMenuItem,Wt/WMenuItem.h
Wt/WMessageBox,Wt/WMessageBox.h
Wt/WMessageResourceBundle,Wt/WMessageResourceBundle.h
Wt/WMessageResources,Wt/WMessageResources.h
Wt/WModelIndex,Wt/WModelIndex.h
Wt/WNavigationBar,Wt/WNavigationBar.h
Wt/WObject,Wt/WObject.h
Wt/WOverlayLoadingIndicator,Wt/WOverlayLoadingIndicator.h
Wt/WPaintDevice,Wt/WPaintDevice.h
Wt/WPaintedWidget,Wt/WPaintedWidget.h
Wt/WPainter,Wt/WPainter.h
Wt/WPainterPath,Wt/WPainterPath.h
Wt/WPanel,Wt/WPanel.h
Wt/WPdfImage,Wt/WPdfImage.h
Wt/WPen,Wt/WPen.h
Wt/WPoint,Wt/WPoint.h
Wt/WPointF,Wt/WPointF.h
Wt/WPolygonArea,Wt/WPolygonArea.h
Wt/WPopupMenu,Wt/WPopupMenu.h
Wt/WPopupMenuItem,Wt/WPopupMenuItem.h
Wt/WPopupWidget,Wt/WPopupWidget.h
Wt/WProgressBar,Wt/WProgressBar.h
Wt/WPushButton,Wt/WPushButton.h
Wt/WRadioButton,Wt/WRadioButton.h
Wt/WRandom,Wt/WRandom.h
Wt/WRasterImage,Wt/WRasterImage.h
Wt/WReadOnlyProxyModel,Wt/WReadOnlyProxyModel.h
Wt/WRectArea,Wt/WRectArea.h
Wt/WRectF,Wt/WRectF.h
Wt/WRegExpValidator,Wt/WRegExpValidator.h
Wt/WResource,Wt/WResource.h
Wt/WSelectionBox,Wt/WSelectionBox.h
Wt/WServer,Wt/WServer.h
Wt/WServerGLWidget,Wt/WServerGLWidget.h
Wt/WShadow,Wt/WShadow.h
Wt/WSignal,Wt/WSignal.h
Wt/WSlider,Wt/WSlider.h
Wt/WSocketNotifier,Wt/WSocketNotifier.h
Wt/WSortFilterProxyModel,Wt/WSortFilterProxyModel.h
Wt/WSound,Wt/WSound.h
Wt/WSpinBox,Wt/WSpinBox.h
Wt/WSplitButton,Wt/WSplitButton.h
Wt/WSslCertificate,Wt/WSslCertificate.h
Wt/WSslInfo,Wt/WSslInfo.h
Wt/WStackedWidget,Wt/WStackedWidget.h
Wt/WStandardItem,Wt/WStandardItem.h
Wt/WStandardItemModel,Wt/WStandardItemModel.h
Wt/WStatelessSlot,Wt/WStatelessSlot.h
Wt/WStreamResource,Wt/WStreamResource.h
Wt/WString,Wt/WString.h
Wt/WStringListModel,Wt/WStringListModel.h
Wt/WStringStream,Wt/WStringStream.h
Wt/WStringUtil,Wt/WStringUtil.h
Wt/WSubMenuItem,Wt/WSubMenuItem.h
Wt/WSuggestionPopup,Wt/WSuggestionPopup.h
Wt/WSvgImage,Wt/WSvgImage.h
Wt/WTabWidget,Wt/WTabWidget.h
Wt/WTable,Wt/WTable.h
Wt/WTableCell,Wt/WTableCell.h
Wt/WTableColumn,Wt/WTableColumn.h
Wt/WTableRow,Wt/WTableRow.h
Wt/WTableView,Wt/WTableView.h
Wt/WTemplate,Wt/WTemplate.h
Wt/WTemplateFormView,Wt/WTemplateFormView.h
Wt/WText,Wt/WText.h
Wt/WTextArea,Wt/WTextArea.h
Wt/WTextEdit,Wt/WTextEdit.h
Wt/WTheme,Wt/WTheme.h
Wt/WTime,Wt/WTime.h
Wt/WTimeEdit,Wt/WTimeEdit.h
Wt/WTimePicker,Wt/WTimePicker.h
Wt/WTimeValidator,Wt/WTimeValidator.h
Wt/WTimer,Wt/WTimer.h
Wt/WTimerWidget,Wt/WTimerWidget.h
Wt/WToolBar,Wt/WToolBar.h
Wt/WTransform,Wt/WTransform.h
Wt/WTree,Wt/WTree.h
Wt/WTreeNode,Wt/WTreeNode.h
Wt/WTreeTable,Wt/WTreeTable.h
Wt/WTreeTableNode,Wt/WTreeTableNode.h
Wt/WTreeView,Wt/WTreeView.h
Wt/WVBoxLayout,Wt/WVBoxLayout.h
Wt/WValidator,Wt/WValidator.h
Wt/WVector3,Wt/WVector3.h
Wt/WVector4,Wt/WVector4.h
Wt/WVectorImage,Wt/WVectorImage.h
Wt/WVideo,Wt/WVideo.h
Wt/WViewWidget,Wt/WViewWidget.h
Wt/WVirtualImage,Wt/WVirtualImage.h
Wt/WVmlImage,Wt/WVmlImage.h
Wt/WWebWidget,Wt/WWebWidget.h
Wt/WWidget,Wt/WWidget.h
Wt/WWidgetItem,Wt/WWidgetItem.h
Wt/WWidgetItemImpl,Wt/WWidgetItemImpl.h
"""

substitutions = {}

for subst in substitutions_string.splitlines():
    key, value = subst.split(',')
    substitutions[key] = value

def do_substitutions_in_line(line, start, end):
    for old, new in substitutions.items():
        if line.startswith('#include ' + start + old + end):
            return line.replace('#include ' + start + old + end,
                                '#include ' + start + new + end,
                                1)
    return line

def do_substitutions_in_file(filename, verbose, list_affected):
    if verbose:
        print('Handling file: ' + filename)
    affected = False
    lines = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('#include <'):
                lines.append(do_substitutions_in_line(line, '<', '>'))
            elif line.startswith('#include "'):
                lines.append(do_substitutions_in_line(line, '"', '"'))
            else:
                lines.append(line)
            if lines[-1] != line:
                affected = True
    if affected:
        if list_affected:
            print(filename)
            return
        elif verbose:
            print('  Substitutions found, writing new contents to file')
        with open(filename, 'w') as f:
            for line in lines:
                f.write(line)

def do_substitutions_in_dir(dir_path, verbose, list_affected, extensions):
    import os
    for dirpath, dirnames, filenames in os.walk(dir_path):
        for filename in filenames:
            _, ext = os.path.splitext(filename)
            if ext in extensions:
                do_substitutions_in_file(os.path.join(dirpath, filename), verbose=verbose, list_affected=list_affected)

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Change Wt header includes to have .h extension')
    parser.add_argument('directory', type=str, nargs='?', default='.', help='Where to perform the substitutions. Defaults to working directory.')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose mode, prints filenames as they are processed')
    parser.add_argument('--list-affected', action='store_true', help='Only print the files that would be affected')
    parser.add_argument('--extensions', type=str, default='cpp,C,hpp,h', help='A comma-separated list of the extensions of files that are processed, defaults to cpp,C,hpp,h')
    args = parser.parse_args()
    extensions = ['.' + ext for ext in args.extensions.split(',')]
    do_substitutions_in_dir(args.directory, verbose=args.verbose, list_affected=args.list_affected, extensions=extensions)

