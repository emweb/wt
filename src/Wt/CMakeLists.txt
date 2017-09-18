SUBDIRS(Auth Core Chart Dbo Json Http Mail Payment Render Signals Test)

IF(INSTALL_EXTENSIONLESS_HEADERS)
  INSTALL_FILES(/include/Wt "^W.*[^C~]$")
ELSE(INSTALL_EXTENSIONLESS_HEADERS)
  INSTALL_FILES(/include/Wt "^W.*[.]h$")
ENDIF(INSTALL_EXTENSIONLESS_HEADERS)
INSTALL_FILES(/include/Wt "^Date$")
IF(INSTALL_EXTENSIONLESS_HEADERS)
  INSTALL_FILES(/include/Wt "^Utils$")
ENDIF(INSTALL_EXTENSIONLESS_HEADERS)
INSTALL_FILES(/include/Wt "^Utils[.]h$")
INSTALL_FILES(/include/Wt "^cpp17$")
INSTALL_FILES(/include/Wt "^AsioWrapper$")
#INSTALL(
#  FILES
#    "^W.*[^C~]$"
#    ${WCONFIG_H_PATH}
#  DESTINATION include
#)

IF (ASCIIDOC_FOUND)
  ASCIIDOC_FILE(wt-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/wt.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/wt.html)
ENDIF (ASCIIDOC_FOUND)
