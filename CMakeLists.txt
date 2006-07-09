project(kolourpaint)

include_directories(
    ${CMAKE_SOURCE_DIR}/kolourpaint/cursors/
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/
    ${CMAKE_SOURCE_DIR}/kolourpaint/compat/
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDES}
    )

add_subdirectory( cursors ) 
add_subdirectory( pics ) 
add_subdirectory( imagelib ) 
add_subdirectory( tools ) 
add_subdirectory( views ) 
add_subdirectory( widgets ) 


########### next target ###############
macro(CREATE_VERSION _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "static const char * const kpVersionText =")
    STRING(REGEX REPLACE "\n" "" _contents "${_contents}" )
	FILE(APPEND ${_out_FILE} "\"${_contents}\"")
	FILE(APPEND ${_out_FILE} ";\n")
endmacro(CREATE_VERSION)


macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintversion.h )

create_version(${CMAKE_SOURCE_DIR}/kolourpaint/VERSION kolourpaintversion.h)


macro(CREATE_LICENSE _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "static const char * const kpLicenseText =")
    STRING(REGEX REPLACE "\n" "\\\\n\"\n\"" _contents "${_contents}" )
    FILE(APPEND ${_out_FILE} "\"${_contents}\"")
    FILE(APPEND ${_out_FILE} ";\n")
endmacro(CREATE_LICENSE)

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintlicense.h )

create_license(${CMAKE_SOURCE_DIR}/kolourpaint/COPYING kolourpaintlicense.h)

set(libkolourpaintwidgets_SRCS
	${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpcolorsimilaritycube.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpcolorsimilaritydialog.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpcolortoolbar.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpresizesignallinglabel.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpsqueezedtextlabel.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolpreviewdialog.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptooltoolbar.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetbase.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetbrush.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgeterasersize.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetfillstyle.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetlinewidth.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetopaqueortransparent.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kptoolwidgetspraycansize.cpp
 )

set(libkolourpaintviews_SRCS 
	${CMAKE_SOURCE_DIR}/kolourpaint/views/kpthumbnailview.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpunzoomedthumbnailview.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpzoomedthumbnailview.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpzoomedview.cpp
)

set(libkolourpainttools_SRCS 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolautocrop.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolbrush.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolclear.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolcolorpicker.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolcolorwasher.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolconverttograyscale.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolcrop.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolcurve.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolflowbase.cpp
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolflowcommand.cpp
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolflowpixmapbase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolellipse.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolellipticalselection.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptooleraser.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolflip.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolfloodfill.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolfreeformselection.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolline.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolpen.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolpolygon.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolpolyline.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolrectangle.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolrectselection.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolresizescale.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolrotate.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolroundedrectangle.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolselection.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolspraycan.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolskew.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptooltext.cpp
)

set(libkolourpaintimagelib_SRCS
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpcolor.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpcoloreffect.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpdocumentmetainfo.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectbalance.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectblursharpen.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectemboss.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectflatten.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectinvert.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectreducecolors.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpeffectsdialog.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpfloodfill.cpp 
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpimage.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kppainter.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kptextstyle.cpp 
)

set(libkolourpaintcursors_SRCS 
	${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpcursorlightcross.cpp 
	${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpcursorprovider.cpp
	)


set(kolourpaint_SRCS ${libkolourpaintcursors_SRCS} ${libkolourpaintimagelib_SRCS} ${libkolourpainttools_SRCS} ${libkolourpaintwidgets_SRCS} ${libkolourpaintviews_SRCS}
   kolourpaint.cpp 
   kpdocument.cpp 
   kpdocumentsaveoptions.cpp 
   kpdocumentsaveoptionswidget.cpp 
   kpview.cpp 
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
   kppixmapfx.cpp
   kpselection.cpp 
   kpselectiondrag.cpp 
   kpselectiontransparency.cpp 
   kptemppixmap.cpp 
   kpthumbnail.cpp 
   kptool.cpp 
   kptoolaction.cpp 
   kpviewmanager.cpp 
   kpviewscrollablecontainer.cpp 
   kpwidgetmapper.cpp 
   compat/kpbug.cpp )

kde4_automoc(${kolourpaint_SRCS})

kde4_add_executable(kolourpaint ${kolourpaint_SRCS})

target_link_libraries(kolourpaint  ${KDE4_KDECORE_LIBS} kdeprint )

install(TARGETS kolourpaint  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kolourpaint.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kolourpaintui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kolourpaint )




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
