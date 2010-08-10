
WT_ADD_EXAMPLE(javascript.wt
  JavascriptExample.C
  Popup.C
)

INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

ADD_DEPENDENCIES(javascript.wt wt ${EXAMPLES_CONNECTOR})
