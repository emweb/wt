IF(BOOST_WT_MT_FOUND)
  IF(MULTI_THREADED)
    ADD_DEFINITIONS(-DWT_THREADED -D_REENTRANT -DBOOST_SPIRIT_THREADSAFE)
  ELSE(MULTI_THREADED)
    ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
  ENDIF(MULTI_THREADED)
ELSE(BOOST_WT_MT_FOUND)
  ADD_DEFINITIONS(-DBOOST_DISABLE_THREADS)
ENDIF(BOOST_WT_MT_FOUND)

ADD_DEFINITIONS(-DWT_NO_DEPRECATED)

SUBDIRS(
  bobsmith
  charts
  composer
  dialog
  dragdrop
  extkitchen
  filetreetable
  form
  gitmodel
  hello
  hello-widgetset
  javascript
  mandelbrot
  mission
  painting
  simplechat
  style
  treelist
  treeview
  treeview-dragdrop
  wt-homepage
  hangman
  wtwithqt
  widgetgallery
)
