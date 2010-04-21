ADD_EXECUTABLE(planner.wt planner.C PlannerApplication.C UserAccount.C Shape.C ShapesWidget.C MyCaptcha.C Login.C TimeSuggestions.C EntryDialog.C AllEntriesDialog.C CalendarCell.C PlannerCalendar.C)
TARGET_LINK_LIBRARIES(planner.wt ${EXAMPLES_CONNECTOR} wtdbo wtdbosqlite3)

#
# If you have Wt installed somehwere, you should use the
# installed Wt header files for your own Wt projects.
# e.g. INCLUDE_DIRECTORIES(/usr/local/include)
# instead of the following:
#
INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

#
# Create a deploy script
#
SET(APPNAME planner)
SET(APPBIN planner.wt)
SET(APPRESOURCES "")
CONFIGURE_FILE(
  ${WT_SOURCE_DIR}/deploy.sh
  ${CMAKE_CURRENT_BINARY_DIR}/deploy.sh
)
