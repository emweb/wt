ADD_EXECUTABLE(composer.wt 
  AddresseeEdit.C
  AttachmentEdit.C
  ComposeExample.C
  Composer.C
  ContactSuggestions.C
  Label.C
  Option.C
  OptionList.C
)
TARGET_LINK_LIBRARIES(composer.wt ${EXAMPLES_CONNECTOR})

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(
  ${WT_SOURCE_DIR}/src
  ${WT_SOURCE_DIR}/examples/treelist
)

#
# Create a deploy script
#
SET(APPNAME composer)
SET(APPBIN composer.wt)
SET(APPRESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/composer.xml ${CMAKE_CURRENT_SOURCE_DIR}/composer.css ${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png ${CMAKE_CURRENT_SOURCE_DIR}/../treelist/icons")

CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)

ADD_DEPENDENCIES(composer.wt wt ${EXAMPLES_CONNECTOR})
