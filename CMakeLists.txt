project(kolourpaint)

include_directories(
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/transforms
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/flow
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/polygonal
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/rectangular
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text
    ${CMAKE_SOURCE_DIR}/kolourpaint/compat
    ${CMAKE_SOURCE_DIR}/kolourpaint/cursors
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/effects
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms
    ${CMAKE_SOURCE_DIR}/kolourpaint/generic
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/transforms
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection
    ${CMAKE_SOURCE_DIR}/kolourpaint/views
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/generic
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDES}
    )

add_subdirectory( pics )


########### next target ###############
macro(CREATE_VERSION _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "static const char * const kpVersionText =")
    STRING(REGEX REPLACE "\n" "" _contents "${_contents}" )
    FILE(APPEND ${_out_FILE} "\"${_contents}\"")
    FILE(APPEND ${_out_FILE} ";\n")
endmacro(CREATE_VERSION)


macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintversion.h )

create_version(${CMAKE_SOURCE_DIR}/kolourpaint/VERSION ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintversion.h)


macro(CREATE_LICENSE _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "static const char * const kpLicenseText =")
    STRING(REGEX REPLACE "\n" "\\\\n\"\n\"" _contents "${_contents}" )
    FILE(APPEND ${_out_FILE} "\"${_contents}\"")
    FILE(APPEND ${_out_FILE} ";\n")
endmacro(CREATE_LICENSE)

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintlicense.h )

create_license(${CMAKE_SOURCE_DIR}/kolourpaint/COPYING ${CMAKE_CURRENT_BINARY_DIR}/kolourpaintlicense.h)


set(kolourpaint_SRCS
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectBalanceCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectBlurSharpenCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectClearCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectCommandBase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectEmbossCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectFlattenCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectGrayscaleCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectInvertCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/effects/kpEffectReduceColorsCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/transforms/kpTransformFlipCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/transforms/kpTransformResizeScaleCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/transforms/kpTransformRotateCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/imagelib/transforms/kpTransformSkewCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/flow/kptoolflowcommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/kpToolColorPickerCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/kpToolFloodFillCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/polygonal/kpToolPolygonalCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/rectangular/kptoolrectangularcommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionCreateCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionDestroyCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionMoveCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionPullFromDocumentCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionResizeScaleCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/kpToolSelectionTransparencyCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text/kpToolTextBackspaceCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text/kpToolTextChangeStyleCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text/kpToolTextDeleteCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text/kpToolTextEnterCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/selection/text/kpToolTextInsertCommand.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/compat/kpbug.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpcursorlightcross.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpcursorprovider.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/effects/kpEffectsDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformFlipDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformPreviewDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformResizeScaleDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformRotateDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformSkewDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/kpcolorsimilaritydialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/kpDocumentSaveOptionsPreviewDialog.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/generic/kpSetOverrideCursorSaver.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/generic/kpwidgetmapper.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectBalance.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectBlurSharpen.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectEmboss.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectFlatten.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectGrayscale.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectInvert.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectReduceColors.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpcolor.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpdocumentmetainfo.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpfloodfill.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpimage.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kppainter.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kptextstyle.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/transforms/kptoolautocrop.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/transforms/kptoolcrop.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kolourpaint.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpcommandhistory.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpdocument.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpdocumentsaveoptions.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kppixmapfx.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpselection.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpselectiondrag.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpselectiontransparency.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kptemppixmap.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpthumbnail.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kptoolaction.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpviewmanager.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/kpviewscrollablecontainer.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_edit.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_file.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_help.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_image.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_settings.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_statusbar.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_text.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_tools.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpmainwindow_view.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolbrush.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolcoloreraser.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptooleraser.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolflowbase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolflowpixmapbase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolpen.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kptoolspraycan.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolcolorpicker.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptool.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_Drawing.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kptoolfloodfill.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_KeyboardEvents.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_MouseEvents.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_OtherEvents.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_UserNotifications.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_Utilities.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kptoolcurve.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kptoolline.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolPolygonalBase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolPolygon.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kptoolpolyline.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kptoolellipse.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kptoolrectangle.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kptoolrectangularbase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kptoolroundedrectangle.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kptoolellipticalselection.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kptoolfreeformselection.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kptoolrectselection.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kptoolselection.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kptooltext.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpthumbnailview.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpunzoomedthumbnailview.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpview.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpzoomedthumbnailview.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/views/kpzoomedview.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/generic/kpresizesignallinglabel.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/generic/kpsqueezedtextlabel.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectBalanceWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectBlurSharpenWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectEmbossWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectFlattenWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectInvertWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectReduceColorsWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectWidgetBase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpcolorsimilaritycube.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpDocumentSaveOptionsWidget.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/kpcolortoolbar.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/kptooltoolbar.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetbase.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetbrush.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgeterasersize.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetfillstyle.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetlinewidth.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetopaqueortransparent.cpp
    ${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kptoolwidgetspraycansize.cpp
)  # set(kolourpaint_SRCS

kde4_automoc(${kolourpaint_SRCS})

kde4_add_executable(kolourpaint ${kolourpaint_SRCS})

target_link_libraries(kolourpaint ${KDE4_KDECORE_LIBS} ${KDE4_KDEPRINT_LIBS})

install(TARGETS kolourpaint DESTINATION ${BIN_INSTALL_DIR})


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
