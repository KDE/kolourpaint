project(kolourpaint)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

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
${CMAKE_SOURCE_DIR}/kolourpaint/document
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
${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/flow/kpToolFlowCommand.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/kpToolColorPickerCommand.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/kpToolFloodFillCommand.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/polygonal/kpToolPolygonalCommand.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/commands/tools/rectangular/kpToolRectangularCommand.cpp
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
${CMAKE_SOURCE_DIR}/kolourpaint/compat/kpBug.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpCursorLightCross.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/cursors/kpCursorProvider.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/effects/kpEffectsDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformFlipDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformPreviewDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformResizeScaleDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformRotateDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/imagelib/transforms/kpTransformSkewDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/kpColorSimilarityDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/dialogs/kpDocumentSaveOptionsPreviewDialog.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/document/kpDocument.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/document/kpDocument_Open.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/document/kpDocument_Save.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/document/kpDocumentSaveOptions.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/document/kpDocument_Selection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/generic/kpSetOverrideCursorSaver.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/generic/kpWidgetMapper.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectBalance.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectBlurSharpen.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectEmboss.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectFlatten.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectGrayscale.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectInvert.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/effects/kpEffectReduceColors.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpColor.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpDocumentMetaInfo.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpFloodFill.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpImage.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpPainter.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/kpTextStyle.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/transforms/kpTransformAutoCrop.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/imagelib/transforms/kpTransformCrop.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kolourpaint.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpCommandHistory.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpPixmapFX.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpSelection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpSelectionDrag.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpSelectionTransparency.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpTempPixmap.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpThumbnail.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpToolAction.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpViewManager.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/kpViewScrollableContainer.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Edit.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_File.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Help.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Image.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Settings.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_StatusBar.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Text.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_Tools.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/mainwindow/kpMainWindow_View.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolBrush.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolColorEraser.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolEraser.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolFlowBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolFlowPixmapBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolPen.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/flow/kpToolSpraycan.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpToolColorPicker.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_Drawing.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpToolFloodFill.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_KeyboardEvents.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_MouseEvents.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_OtherEvents.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_UserNotifications.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/kpTool_Utilities.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolCurve.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolLine.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolPolygonalBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolPolygon.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/polygonal/kpToolPolyline.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kpToolEllipse.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kpToolRectangle.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kpToolRectangularBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/rectangular/kpToolRoundedRectangle.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kpToolEllipticalSelection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kpToolFreeFormSelection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kpToolRectSelection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kpToolSelection.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/tools/selection/kpToolText.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/views/kpThumbnailView.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/views/kpUnzoomedThumbnailView.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/views/kpView.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/views/kpZoomedThumbnailView.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/views/kpZoomedView.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/generic/kpResizeSignallingLabel.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/generic/kpSqueezedTextLabel.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectBalanceWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectBlurSharpenWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectEmbossWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectFlattenWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectInvertWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectReduceColorsWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/imagelib/effects/kpEffectWidgetBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpColorCells.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpColorPalette.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpColorSimilarityCube.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpColorSimilarityToolBarItem.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpDocumentSaveOptionsWidget.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpDualColorButton.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/kpTransparentColorCell.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/kpColorToolBar.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/kpToolToolBar.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetBase.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetBrush.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetEraserSize.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetFillStyle.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetLineWidth.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.cpp
${CMAKE_SOURCE_DIR}/kolourpaint/widgets/toolbars/options/kpToolWidgetSpraycanSize.cpp

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
