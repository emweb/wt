SUBDIRS(Chart Http Test)

SUBDIRS(Dbo)

IF(ENABLE_EXT)
  SUBDIRS(Ext)
ENDIF(ENABLE_EXT)

INSTALL_FILES(/include/Wt "^W.*[^C~]$")
#INSTALL(
#  FILES
#    "^W.*[^C~]$"
#    ${WCONFIG_H_PATH}
#  DESTINATION include
#)
