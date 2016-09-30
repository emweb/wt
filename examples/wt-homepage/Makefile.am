INC_DIR = ../blog

if DEBUG
AM_CFLAGS = -g3 -O0 -I$(INC_DIR)
AM_CXXFLAGS = -g3 -O0 -I$(INC_DIR)
else
AM_CFLAGS = -O2 -I$(INC_DIR)
AM_CXXFLAGS = -O2 -I$(INC_DIR)
endif

bin_PROGRAMS = homepage.wt
homepage_wt_SOURCES = ExampleSourceViewer.C FileItem.C Home.C JWtHome.C main.C \
					  SourceView.C WtHome.C ExampleSourceViewer.h FileItem.h \
					  Home.h JWtHome.h SourceView.h WtHome.h
