SUBDIRS(Auth Core Chart Dbo Form Json Http Mail Payment Render Signals Test)

INSTALL_FILES(/include/Wt "^W.*[.]h$")
INSTALL_FILES(/include/Wt "^Utils[.]h$")
INSTALL_FILES(/include/Wt "^cpp17$")
INSTALL_FILES(/include/Wt "^cpp20$")
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

IF (WT_DATE_TZ_USE_DATE)
  ADD_SUBDIRECTORY(Date)
ENDIF()
