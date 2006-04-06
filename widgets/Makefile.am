INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../compat -I$(srcdir)/../cursors -I$(srcdir)/../interfaces \
    -I$(srcdir)/../pixmapfx \
    -I$(srcdir)/../tools \
    -I$(srcdir)/../views \
    -I$(srcdir)/../widgets $(all_includes)

noinst_LTLIBRARIES = libkolourpaintwidgets.la
libkolourpaintwidgets_la_SOURCES = kpcolorsimilaritycube.cpp \
    kpcolorsimilaritydialog.cpp \
    kpcolortoolbar.cpp \
    kpresizesignallinglabel.cpp \
    kpsqueezedtextlabel.cpp \
    kptooltoolbar.cpp \
    kptoolwidgetbase.cpp kptoolwidgetbrush.cpp \
    kptoolwidgeterasersize.cpp kptoolwidgetfillstyle.cpp \
    kptoolwidgetlinewidth.cpp \
    kptoolwidgetopaqueortransparent.cpp \
    kptoolwidgetspraycansize.cpp

# TODO: Why is this needed?  Isn't linking at the toplevel enough?
libkolourpaintwidgets_la_LIBADD = ../compat/libkolourpaintcompat.la

METASOURCES = AUTO

