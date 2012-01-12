SUBDIRS(Dbo)
INSTALL_FILES(/include/Wt/Auth "^[^bD.][^.]+[^hC~]$")
INSTALL_FILES(/include/Wt/Auth "^PasswordHash$")

IF (ASCIIDOC_FOUND)
  IF(NOT WIN32)
    ADD_CUSTOM_TARGET(doc)
  ENDIF(NOT WIN32)
  ASCIIDOC_FILE(wt-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.html)
ENDIF (ASCIIDOC_FOUND)
