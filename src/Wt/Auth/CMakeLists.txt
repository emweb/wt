SUBDIRS(Dbo)
INSTALL_FILES(/include/Wt/Auth "^[^bD.][^.]+[^hC~]$")
INSTALL_FILES(/include/Wt/Auth "^PasswordHash$")

IF (ASCIIDOC_FOUND)
  ASCIIDOC_FILE(auth-tutorial
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.doc
    ${CMAKE_SOURCE_DIR}/doc/tutorial/auth.html)
ENDIF (ASCIIDOC_FOUND)
