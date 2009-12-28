SUBDIRS(Dbo Ext Chart Http Test)

INSTALL_FILES(/include/Wt "^W.*[^C~]$")
#INSTALL(
#  FILES
#    "^W.*[^C~]$"
#    ${WCONFIG_H_PATH}
#  DESTINATION include
#)
