kde4_header()
project(kolourpaint)

add_subdirectory( cursors ) 
add_subdirectory( pics ) 
add_subdirectory( pixmapfx ) 
add_subdirectory( tools ) 
add_subdirectory( views ) 
add_subdirectory( widgets ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kolourpaint_SRCS 
   kolourpaint.cpp 
   kpdocument.cpp 
   kpdocumentmetainfo.cpp 
   kpdocumentsaveoptions.cpp 
   kpdocumentsaveoptionswidget.cpp 
   kpview.cpp 
   kpcolor.cpp 
   kpcommandhistory.cpp 
   kpmainwindow.cpp 
   kpmainwindow_edit.cpp 
   kpmainwindow_help.cpp 
   kpmainwindow_image.cpp 
   kpmainwindow_tools.cpp 
   kpmainwindow_file.cpp 
   kpmainwindow_settings.cpp 
   kpmainwindow_statusbar.cpp 
   kpmainwindow_text.cpp 
   kpmainwindow_view.cpp 
   kpselection.cpp 
   kpselectiondrag.cpp 
   kpselectiontransparency.cpp 
   kpsinglekeytriggersaction.cpp 
   kptemppixmap.cpp 
   kptextstyle.cpp 
   kpthumbnail.cpp 
   kptool.cpp 
   kpviewmanager.cpp 
   kpviewscrollablecontainer.cpp 
   kpwidgetmapper.cpp )

kde4_automoc(${kolourpaint_SRCS})

kde4_add_executable(kolourpaint ${kolourpaint_SRCS})

target_link_libraries(kolourpaint  ${KDE4_KDECORE_LIBS} kdeprint kolourpaintcursors kolourpaintpixmapfx kolourpainttools kolourpaintviews kolourpaintwidgets )

install_targets(/bin kolourpaint )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kolourpaint.desktop )
install_files( ${DATA_INSTALL_DIR}/kolourpaint FILES kolourpaintui.rc )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = cursors pics pixmapfx tools views widgets
#
#bin_PROGRAMS = kolourpaint
#
#
#kolourpaint.o: kolourpaintlicense.h kolourpaintversion.h
#
#kolourpaintlicense.h : $(srcdir)/COPYING
#	echo "static const char * const kpLicenseText =" > kolourpaintlicense.h
#	cat $(srcdir)/COPYING | sed -e 's/"/\\"/g' -e 's/$$/\\n"/g' -e 's/^/    "/g' >> kolourpaintlicense.h
#	echo ";" >> kolourpaintlicense.h
#
#kolourpaintversion.h : $(srcdir)/VERSION
#	echo "static const char * const kpVersionText =" > kolourpaintversion.h
#	cat $(srcdir)/VERSION | sed -e 's/"/\\"/g' -e 's/$$/"/g' -e 's/^/    "/g' >> kolourpaintversion.h
#	echo ";" >> kolourpaintversion.h
#
#CLEANFILES = kolourpaintlicense.h kolourpaintversion.h
#
#
#kolourpaint_SOURCES = kolourpaint.cpp \
#    kpdocument.cpp \
#    kpdocumentmetainfo.cpp \
#    kpdocumentsaveoptions.cpp \
#    kpdocumentsaveoptionswidget.cpp \
#    kpview.cpp \
#    kpcolor.cpp kpcommandhistory.cpp \
#    kpmainwindow.cpp \
#    kpmainwindow_edit.cpp kpmainwindow_help.cpp \
#    kpmainwindow_image.cpp kpmainwindow_tools.cpp \
#    kpmainwindow_file.cpp kpmainwindow_settings.cpp kpmainwindow_statusbar.cpp \
#    kpmainwindow_text.cpp \
#    kpmainwindow_view.cpp \
#    kpselection.cpp kpselectiondrag.cpp kpselectiontransparency.cpp \
#    kpsinglekeytriggersaction.cpp \
#    kptemppixmap.cpp kptextstyle.cpp \
#    kpthumbnail.cpp \
#    kptool.cpp \
#    kpviewmanager.cpp \
#    kpviewscrollablecontainer.cpp \
#    kpwidgetmapper.cpp
#kolourpaint_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kolourpaint_LDADD = $(LIB_KDEPRINT) \
#    cursors/libkolourpaintcursors.la \
#    pixmapfx/libkolourpaintpixmapfx.la \
#    tools/libkolourpainttools.la \
#    views/libkolourpaintviews.la \
#    widgets/libkolourpaintwidgets.la
#
#AM_CPPFLAGS = -I$(srcdir)/cursors -I$(srcdir)/interfaces \
#    -I$(srcdir)/pixmapfx \
#    -I$(srcdir)/tools \
#    -I$(srcdir)/views \
#    -I$(srcdir)/widgets $(all_includes)
#
#METASOURCES = AUTO
#
#rcdir = $(kde_datadir)/kolourpaint
#rc_DATA = kolourpaintui.rc
#
#xdg_apps_DATA = kolourpaint.desktop
#
#messages: rc.cpp
#	$(EXTRACTRC) *.rc *.ui \
#		cursors/*.rc cursors/*.ui \
#		pixmapfx/*.rc pixmapfx/*.ui \
#		tools/*.rc tools/*.ui \
#		widgets/*.rc widgets/*.ui \
#		>> rc.cpp
#	$(XGETTEXT) *.cpp *.h \
#		cursors/*.cpp cursors/*.h \
#		pixmapfx/*.cpp pixmapfx/*.h \
#		tools/*.cpp tools/*.h \
#		widgets/*.cpp widgets/*.h \
#		-o $(podir)/kolourpaint.pot
#
