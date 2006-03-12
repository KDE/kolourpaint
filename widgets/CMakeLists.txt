kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../cursors -I$(srcdir)/../interfaces \
#    -I$(srcdir)/../pixmapfx \
#    -I$(srcdir)/../tools \
#    -I$(srcdir)/../views \
#    -I$(srcdir)/../widgets $(all_includes)
#
#noinst_LTLIBRARIES = libkolourpaintwidgets.la
#libkolourpaintwidgets_la_SOURCES = kpcolorsimilaritycube.cpp \
#    kpcolorsimilaritydialog.cpp \
#    kpcolortoolbar.cpp \
#    kpresizesignallinglabel.cpp \
#    kpsqueezedtextlabel.cpp \
#    kptooltoolbar.cpp \
#    kptoolwidgetbase.cpp kptoolwidgetbrush.cpp \
#    kptoolwidgeterasersize.cpp kptoolwidgetfillstyle.cpp \
#    kptoolwidgetlinewidth.cpp \
#    kptoolwidgetopaqueortransparent.cpp \
#    kptoolwidgetspraycansize.cpp
#
#METASOURCES = AUTO
#
