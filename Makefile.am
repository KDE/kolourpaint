SUBDIRS = pics pixmapfx tools widgets

bin_PROGRAMS = kolourpaint

kolourpaint.o: kolourpaintlicense.h

kolourpaintlicense.h : $(srcdir)/LICENCE
	echo "const char *kpLicenseText =" > kolourpaintlicense.h
	cat $(srcdir)/LICENCE | sed -e 's/"/\\"/g' -e 's/$$/\\n"/g' -e 's/^/    "/g' >> kolourpaintlicense.h
	echo ";" >> kolourpaintlicense.h

CLEANFILES = kolourpaintlicense.h

kolourpaint_SOURCES = kolourpaint.cpp kpdocument.cpp kpview.cpp \
    kpcommandhistory.cpp \
    kpconfigdialog.cpp \
    kpmainwindow.cpp \
    kpmainwindow_edit.cpp kpmainwindow_image.cpp kpmainwindow_tools.cpp \
    kpmainwindow_file.cpp kpmainwindow_settings.cpp kpmainwindow_view.cpp \
    kpselection.cpp \
    kpviewmanager.cpp
kolourpaint_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolourpaint_LDADD = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDEPRINT) \
    pixmapfx/libkolourpaintpixmapfx.la tools/libkolourpainttools.la \
    widgets/libkolourpaintwidgets.la

AM_CPPFLAGS = -I $(srcdir) -I $(srcdir)/pixmapfx \
    -I $(srcdir)/tools -I $(srcdir)/widgets \
    $(all_includes)

METASOURCES = AUTO

rcdir = $(kde_datadir)/kolourpaint
rc_DATA = kolourpaintui.rc

xdg_apps_DATA = kolourpaint.desktop

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui \
		pixmapfx/*.rc pixmapfx/*.ui \
		tools/*.rc tools/*.ui \
		widgets/*.rc widgets/*.ui \
		>> rc.cpp
	$(XGETTEXT) *.cpp *.h \
		pixmapfx/*.cpp pixmapfx/*.h \
		tools/*.cpp tools/*.h \
		widgets/*.cpp widgets/*.h \
		-o $(podir)/kolourpaint.pot

