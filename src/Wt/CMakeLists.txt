SUBDIRS(Auth Core Chart Date Dbo Json Http Mail Payment Render Signals Test)

INSTALL_FILES(/include/Wt "^W.*[.]h$")
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
