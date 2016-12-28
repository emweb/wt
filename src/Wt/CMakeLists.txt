SUBDIRS(Auth Core Chart Dbo Json Http Mail Payment Render Signals Test)

INSTALL_FILES(/include/Wt "^W.*[^C~]$")
INSTALL_FILES(/include/Wt "^Date$")
INSTALL_FILES(/include/Wt "^Utils$")
INSTALL_FILES(/include/Wt "^cpp17$")
#INSTALL(
#  FILES
#    "^W.*[^C~]$"
#    ${WCONFIG_H_PATH}
#  DESTINATION include
#)

IF (ASCIIDOC_FOUND)
  IF(NOT WIN32)
    ADD_CUSTOM_TARGET(doc)
  ENDIF(NOT WIN32)
  ASCIIDOC_FILE(wt-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/wt.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/wt.html)
ENDIF (ASCIIDOC_FOUND)
