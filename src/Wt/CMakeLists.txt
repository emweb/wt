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

if(ASCIIDOCTOR_FOUND)
  ASCIIDOC_FILE(wt-tutorial
    ${WT_SOURCE_DIR}/doc/tutorial/wt.adoc
    ${WT_SOURCE_DIR}/doc/tutorial/wt.html)
endif()

IF (WT_DATE_TZ_USE_DATE)
  ADD_SUBDIRECTORY(Date)
ENDIF()
