/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2014      Martin Koller <kollix@aon.at>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef kpMainWindowPrivate_H
#define kpMainWindowPrivate_H


#define DEBUG_KP_MAIN_WINDOW 0


#include <kpDocumentSaveOptions.h>


class QAction;
class QActionGroup;

class KSelectAction;
class KToggleAction;
class KAction;
class KSqueezedTextLabel;

class kpCommandEnvironment;
class kpDocumentEnvironment;
class kpToolSelectionEnvironment;
class kpTransformDialogEnvironment;


struct kpMainWindowPrivate
{
  kpMainWindowPrivate ()
    : isFullyConstructed(false),
      scrollView(0),
      mainView(0),
      thumbnail(0),
      thumbnailView(0),
      document(0),
      viewManager(0),
      colorToolBar(0),
      toolToolBar(0),
      commandHistory(0),

      configFirstTime(false),
      configShowGrid(false),
      configShowPath(false),
      configThumbnailShown(false),
      configZoomedThumbnail(false),

      documentEnvironment(0),
      commandEnvironment(0),

      // Tools

      toolSelectionEnvironment(0),
      toolsActionGroup(0),

      toolSpraycan(0),
      toolBrush(0),
      toolColorEraser(0),
      toolColorPicker(0),
      toolCurve(0),
      toolEllipse(0),
      toolEllipticalSelection(0),
      toolEraser(0),
      toolFloodFill(0),
      toolFreeFormSelection(0),
      toolLine(0),
      toolPen(0),
      toolPolygon(0),
      toolPolyline(0),
      toolRectangle(0),
      toolRectSelection(0),
      toolRoundedRectangle(0),
      toolZoom(0),
      toolText(0),

      lastToolNumber(0),
      toolActionsEnabled(false),
      actionPrevToolOptionGroup1(0),
      actionNextToolOptionGroup1(0),
      actionPrevToolOptionGroup2(0),
      actionNextToolOptionGroup2(0),

      settingImageSelectionTransparency(0),

      docResizeWidth(0),
      docResizeHeight(0),
      docResizeToBeCompleted(false),

      configOpenImagesInSameWindow(false),
      configPrintImageCenteredOnPage(false),

      actionNew(0),
      actionOpen(0),
      actionOpenRecent(0),
      actionScan(0),
      actionScreenshot(0),
      actionProperties(0),
      actionSave(0),
      actionSaveAs(0),
      actionExport(0),
      actionReload(0),
      actionPrint(0),
      actionPrintPreview(0),
      actionMail(0),
      actionClose(0),
      actionQuit(0),

      scanDialog(0),

      exportFirstTime(false),

      // Edit Menu

      editMenuDocumentActionsEnabled(false),

      actionUndo(0),
      actionRedo(0),
      actionCut(0),
      actionCopy(0),
      actionPaste(0),
      actionPasteInNewWindow(0),
      actionDelete(0),
      actionSelectAll(0),
      actionDeselect(0),
      actionCopyToFile(0),
      actionPasteFromFile(0),

      copyToFirstTime(false),

      // View Menu

      configThumbnailShowRectangle(false),
      actionShowThumbnailRectangle(0),

      viewMenuDocumentActionsEnabled(false),

      actionActualSize(0),
      actionFitToPage(0),
      actionFitToWidth(0),
      actionFitToHeight(0),
      actionZoomIn(0),
      actionZoomOut(0),
      actionZoom(0),
      actionShowGrid(0),
      actionShowThumbnail(0),
      actionZoomedThumbnail(0),

      thumbnailSaveConfigTimer(0),

      // Image Menu

      transformDialogEnvironment(0),

      imageMenuDocumentActionsEnabled(false),

      actionResizeScale(0),
      actionCrop(0),
      actionAutoCrop(0),
      actionFlip(0),
      actionMirror(0),
      actionRotate(0),
      actionRotateLeft(0),
      actionRotateRight(0),
      actionSkew(0),
      actionConvertToBlackAndWhite(0),
      actionConvertToGrayscale(0),
      actionMoreEffects(0),
      actionInvertColors(0),
      actionClear(0),

      actionDrawOpaque(0),
      actionDrawColorSimilarity(0),

      moreEffectsDialogLastEffect(0),

      // Colors Menu

      colorMenuDocumentActionsEnabled(false),

      actionColorsDefault(0),
      actionColorsKDE(0),
      actionColorsOpen(0),
      actionColorsReload(0),
      actionColorsSave(0),
      actionColorsSaveAs(0),
      actionColorsAppendRow(0),
      actionColorsDeleteRow(0),

      // Settings Menu

      actionShowPath(0),
      actionKeyBindings(0),
      actionConfigureToolbars(0),
      actionConfigure(0),
      actionFullScreen(0),

      // Status Bar

      statusBarCreated(false),
      statusBarMessageLabel(0),
      statusBarShapeLastPointsInitialised(false),
      statusBarShapeLastSizeInitialised(false),

      // Text ToolBar

      actionTextFontFamily(0),
      actionTextFontSize(0),
      actionTextBold(0),
      actionTextItalic(0),
      actionTextUnderline(0),
      actionTextStrikeThru(0),
      settingTextStyle(0),
      textOldFontSize(0)
  {
  }

  bool isFullyConstructed;

  kpViewScrollableContainer *scrollView;
  kpZoomedView *mainView;
  kpThumbnail *thumbnail;
  kpThumbnailView *thumbnailView;
  kpDocument *document;
  kpViewManager *viewManager;
  kpColorToolBar *colorToolBar;
  kpToolToolBar *toolToolBar;
  kpCommandHistory *commandHistory;

  bool configFirstTime;
  bool configShowGrid;
  bool configShowPath;

  bool configThumbnailShown;
  QRect configThumbnailGeometry;
  bool configZoomedThumbnail;

  kpDocumentEnvironment *documentEnvironment;
  kpCommandEnvironment *commandEnvironment;

  //
  // Tools
  //

  kpToolSelectionEnvironment *toolSelectionEnvironment;
  QActionGroup *toolsActionGroup;

  kpTool *toolSpraycan, *toolBrush,
         *toolColorEraser, *toolColorPicker,
         *toolCurve, *toolEllipse,
         *toolEllipticalSelection, *toolEraser,
         *toolFloodFill, *toolFreeFormSelection,
         *toolLine, *toolPen, *toolPolygon,
         *toolPolyline, *toolRectangle, *toolRectSelection,
         *toolRoundedRectangle, *toolZoom;
  kpToolText *toolText;

  QList <kpTool *> tools;
  int lastToolNumber;

  bool toolActionsEnabled;
  KAction *actionPrevToolOptionGroup1,
          *actionNextToolOptionGroup1,
          *actionPrevToolOptionGroup2,
          *actionNextToolOptionGroup2;

  int settingImageSelectionTransparency;

  int docResizeWidth, docResizeHeight;
  bool docResizeToBeCompleted;

  //
  // File Menu
  //

  bool configOpenImagesInSameWindow, configPrintImageCenteredOnPage;

  QAction *actionNew, *actionOpen;
  KRecentFilesAction *actionOpenRecent;
  KAction *actionScan, *actionScreenshot, *actionProperties,
          *actionSave, *actionSaveAs, *actionExport,
          *actionReload,
          *actionPrint, *actionPrintPreview,
          *actionMail,
          *actionClose, *actionQuit;

  KScanDialog *scanDialog;

  KUrl lastExportURL;
  kpDocumentSaveOptions lastExportSaveOptions;
  bool exportFirstTime;

  //
  // Edit Menu
  //

  bool editMenuDocumentActionsEnabled;

  KAction *actionUndo, *actionRedo,
          *actionCut, *actionCopy,
          *actionPaste, *actionPasteInNewWindow,
          *actionDelete,
          *actionSelectAll, *actionDeselect,
          *actionCopyToFile, *actionPasteFromFile;

  KUrl lastCopyToURL;
  kpDocumentSaveOptions lastCopyToSaveOptions;
  bool copyToFirstTime;

  //
  // View Menu
  //

  bool configThumbnailShowRectangle;
  KToggleAction *actionShowThumbnailRectangle;

  bool viewMenuDocumentActionsEnabled;

  QAction *actionActualSize,
          *actionFitToPage, *actionFitToWidth, *actionFitToHeight,
          *actionZoomIn, *actionZoomOut;
  KSelectAction *actionZoom;
  KToggleAction *actionShowGrid,
                *actionShowThumbnail, *actionZoomedThumbnail;

  QList <int> zoomList;

  QTimer *thumbnailSaveConfigTimer;

  //
  // Image Menu
  //

  kpTransformDialogEnvironment *transformDialogEnvironment;

  bool imageMenuDocumentActionsEnabled;

  KAction *actionResizeScale,
          *actionCrop, *actionAutoCrop,
          *actionFlip, *actionMirror,
          *actionRotate, *actionRotateLeft, *actionRotateRight,
          *actionSkew,
          *actionConvertToBlackAndWhite, *actionConvertToGrayscale,
          *actionMoreEffects,
          *actionInvertColors, *actionClear;

  // Implemented in kpMainWindow_Tools.cpp, not kpImageWindow_Image.cpp
  // since they're really setting tool options.
  KToggleAction *actionDrawOpaque;
  QAction *actionDrawColorSimilarity;

  int moreEffectsDialogLastEffect;

  //
  // Colors Menu
  //

  bool colorMenuDocumentActionsEnabled;

  QAction *actionColorsDefault;
  KSelectAction *actionColorsKDE;
  QAction *actionColorsOpen, *actionColorsReload;

  QAction *actionColorsSave, *actionColorsSaveAs;

  QAction *actionColorsAppendRow;
  QAction *actionColorsDeleteRow;

  //
  // Settings Menu
  //

  KToggleAction *actionShowPath;
  QAction *actionKeyBindings, *actionConfigureToolbars, *actionConfigure;
  KToggleFullScreenAction *actionFullScreen;

  //
  // Status Bar
  //

  bool statusBarCreated;
  KSqueezedTextLabel *statusBarMessageLabel;

  bool statusBarShapeLastPointsInitialised;
  QPoint statusBarShapeLastStartPoint, statusBarShapeLastEndPoint;
  bool statusBarShapeLastSizeInitialised;
  QSize statusBarShapeLastSize;

  //
  // Text ToolBar
  //

  KFontAction *actionTextFontFamily;
  KFontSizeAction *actionTextFontSize;
  KToggleAction *actionTextBold, *actionTextItalic,
                *actionTextUnderline, *actionTextStrikeThru;

  int settingTextStyle;
  QString textOldFontFamily;
  int textOldFontSize;
};


#endif  // kpMainWindowPrivate_H
