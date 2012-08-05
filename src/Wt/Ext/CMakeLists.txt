ADD_LIBRARY(wtext
  AbstractButton.C
  AbstractToggleButton.C
  AccordionLayoutImpl.C
  BorderLayoutImpl.C
  Button.C
  Calendar.C
  CheckBox.C
  ComboBox.C
  Component.C
  Container.C
  DataStore.C
  DateField.C
  DefaultLayoutImpl.C
  Dialog.C
  FitLayoutImpl.C
  FormField.C
  LayoutImpl.C
  LayoutItemImpl.C
  LineEdit.C
  Menu.C
  MenuItem.C
  MessageBox.C
  NumberField.C
  PagingToolBar.C
  Panel.C
  ProgressDialog.C
  RadioButton.C
  Splitter.C
  SplitterHandle.C
  TableView.C
  TabWidget.C
  TextEdit.C
  ToolBar.C
  ToolTipConfig.C
  Widget.C
  WWidgetItemImpl.C
)

TARGET_LINK_LIBRARIES(wtext wt ${BOOST_WT_LIBRARIES})
INSTALL(TARGETS wtext
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

IF(MSVC)
  SET_TARGET_PROPERTIES(wtext PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /wd4251 /wd4275 /wd4355 /wd4800 /wd4996 /wd4101 /wd4267")
ENDIF(MSVC)

SET_TARGET_PROPERTIES(
  wtext
PROPERTIES
  VERSION ${VERSION_SERIES}.${VERSION_MAJOR}.${VERSION_MINOR}
  SOVERSION ${WTEXT_SOVERSION}
  DEBUG_POSTFIX ${DEBUG_LIB_POSTFIX}
)

INSTALL_FILES(/include/Wt/Ext "^[^.]..+[^Ch~]$")
INSTALL_FILES(/include/Wt/Ext ExtDllDefs.h)
