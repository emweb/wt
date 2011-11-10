SUBDIRS(Auth Chart Dbo Json Http Mail Render Test)

IF(ENABLE_EXT)
  SUBDIRS(Ext)
ENDIF(ENABLE_EXT)

INSTALL_FILES(/include/Wt "^W.*[^C~]$")
INSTALL_FILES(/include/Wt "SyncLock")
#INSTALL(
#  FILES
#    "^W.*[^C~]$"
#    ${WCONFIG_H_PATH}
#  DESTINATION include
#)
