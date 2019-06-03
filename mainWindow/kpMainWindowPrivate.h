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


#include "document/kpDocumentSaveOptions.h"


class QAction;
class QActionGroup;
class QLabel;

class KSelectAction;
class KToggleAction;
class KSqueezedTextLabel;
class KRecentFilesAction;
class KFontAction;
class KFontSizeAction;
class KToggleFullScreenAction;
class kpCommandEnvironment;
class kpDocumentEnvironment;
class kpToolSelectionEnvironment;
class kpTransformDialogEnvironment;
class kpViewScrollableContainer;
class kpZoomedView;
class kpThumbnail;
class kpThumbnailView;
class kpDocument;
class kpViewManager;
class kpColorToolBar;
class kpToolToolBar;
class kpCommandHistory;
class kpTool;
class kpToolText;
class SaneDialog;


struct kpMainWindowPrivate
{
  kpMainWindowPrivate ()
    : isFullyConstructed(false),
      scrollView(nullptr),
      mainView(nullptr),
      thumbnail(nullptr),
      thumbnailView(nullptr),
      document(nullptr),
      viewManager(nullptr),
      colorToolBar(nullptr),
      toolToolBar(nullptr),
      commandHistory(nullptr),

      configFirstTime(false),
      configShowGrid(false),
      configShowPath(false),
      configThumbnailShown(false),
      configZoomedThumbnail(false),

      documentEnvironment(nullptr),
      commandEnvironment(nullptr),

      // Tools

      toolSelectionEnvironment(nullptr),
      toolsActionGroup(nullptr),

      toolSpraycan(nullptr),
      toolBrush(nullptr),
      toolColorEraser(nullptr),
      toolColorPicker(nullptr),
      toolCurve(nullptr),
      toolEllipse(nullptr),
      toolEllipticalSelection(nullptr),
      toolEraser(nullptr),
      toolFloodFill(nullptr),
      toolFreeFormSelection(nullptr),
      toolLine(nullptr),
      toolPen(nullptr),
      toolPolygon(nullptr),
      toolPolyline(nullptr),
      toolRectangle(nullptr),
      toolRectSelection(nullptr),
      toolRoundedRectangle(nullptr),
      toolZoom(nullptr),
      toolText(nullptr),

      lastToolNumber(0),
      toolActionsEnabled(false),
      actionPrevToolOptionGroup1(nullptr),
      actionNextToolOptionGroup1(nullptr),
      actionPrevToolOptionGroup2(nullptr),
      actionNextToolOptionGroup2(nullptr),

      settingImageSelectionTransparency(0),

      docResizeWidth(0),
      docResizeHeight(0),
      docResizeToBeCompleted(false),

      configOpenImagesInSameWindow(false),
      configPrintImageCenteredOnPage(false),

      actionNew(nullptr),
      actionOpen(nullptr),
      actionOpenRecent(nullptr),
      actionScan(nullptr),
      actionScreenshot(nullptr),
      actionProperties(nullptr),
      actionSave(nullptr),
      actionSaveAs(nullptr),
      actionExport(nullptr),
      actionReload(nullptr),
      actionPrint(nullptr),
      actionPrintPreview(nullptr),
      actionMail(nullptr),
      actionClose(nullptr),
      actionQuit(nullptr),

      scanDialog(nullptr),

      exportFirstTime(false),

      // Edit Menu

      editMenuDocumentActionsEnabled(false),

      actionUndo(nullptr),
      actionRedo(nullptr),
      actionCut(nullptr),
      actionCopy(nullptr),
      actionPaste(nullptr),
      actionPasteInNewWindow(nullptr),
      actionDelete(nullptr),
      actionSelectAll(nullptr),
      actionDeselect(nullptr),
      actionCopyToFile(nullptr),
      actionPasteFromFile(nullptr),

      copyToFirstTime(false),

      // View Menu

      configThumbnailShowRectangle(false),
      actionShowThumbnailRectangle(nullptr),

      viewMenuDocumentActionsEnabled(false),

      actionActualSize(nullptr),
      actionFitToPage(nullptr),
      actionFitToWidth(nullptr),
      actionFitToHeight(nullptr),
      actionZoomIn(nullptr),
      actionZoomOut(nullptr),
      actionZoom(nullptr),
      actionShowGrid(nullptr),
      actionShowThumbnail(nullptr),
      actionZoomedThumbnail(nullptr),

      thumbnailSaveConfigTimer(nullptr),

      // Image Menu

      transformDialogEnvironment(nullptr),

      imageMenuDocumentActionsEnabled(false),

      actionResizeScale(nullptr),
      actionCrop(nullptr),
      actionAutoCrop(nullptr),
      actionFlip(nullptr),
      actionMirror(nullptr),
      actionRotate(nullptr),
      actionRotateLeft(nullptr),
      actionRotateRight(nullptr),
      actionSkew(nullptr),
      actionConvertToBlackAndWhite(nullptr),
      actionConvertToGrayscale(nullptr),
      actionBlur(nullptr),
      actionMoreEffects(nullptr),
      actionInvertColors(nullptr),
      actionClear(nullptr),

      actionDrawOpaque(nullptr),
      actionDrawColorSimilarity(nullptr),

      moreEffectsDialogLastEffect(0),

      // Colors Menu

      colorMenuDocumentActionsEnabled(false),

      actionColorsDefault(nullptr),
      actionColorsKDE(nullptr),
      actionColorsOpen(nullptr),
      actionColorsReload(nullptr),
      actionColorsSave(nullptr),
      actionColorsSaveAs(nullptr),
      actionColorsAppendRow(nullptr),
      actionColorsDeleteRow(nullptr),

      // Settings Menu

      actionShowPath(nullptr),
      actionKeyBindings(nullptr),
      actionConfigureToolbars(nullptr),
      actionConfigure(nullptr),
      actionFullScreen(nullptr),

      // Status Bar

      statusBarCreated(false),
      statusBarMessageLabel(nullptr),
      statusBarShapeLastPointsInitialised(false),
      statusBarShapeLastSizeInitialised(false),

      // Text ToolBar

      actionTextFontFamily(nullptr),
      actionTextFontSize(nullptr),
      actionTextBold(nullptr),
      actionTextItalic(nullptr),
      actionTextUnderline(nullptr),
      actionTextStrikeThru(nullptr),
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
  QAction *actionPrevToolOptionGroup1,
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
  QAction *actionScan, *actionScreenshot, *actionProperties,
          *actionSave, *actionSaveAs, *actionExport,
          *actionReload,
          *actionPrint, *actionPrintPreview,
          *actionMail,
          *actionClose, *actionQuit;

  SaneDialog *scanDialog;

  QUrl lastExportURL;
  kpDocumentSaveOptions lastExportSaveOptions;
  bool exportFirstTime;

  //
  // Edit Menu
  //

  bool editMenuDocumentActionsEnabled;

  QAction *actionUndo, *actionRedo,
          *actionCut, *actionCopy,
          *actionPaste, *actionPasteInNewWindow,
          *actionDelete,
          *actionSelectAll, *actionDeselect,
          *actionCopyToFile, *actionPasteFromFile;

  QUrl lastCopyToURL;
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

  QAction *actionResizeScale,
          *actionCrop, *actionAutoCrop,
          *actionFlip, *actionMirror,
          *actionRotate, *actionRotateLeft, *actionRotateRight,
          *actionSkew,
          *actionConvertToBlackAndWhite, *actionConvertToGrayscale,
          *actionBlur, *actionMoreEffects,
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
  QList<QLabel *> statusBarLabels;

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
