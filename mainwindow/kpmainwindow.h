
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef KP_MAIN_WINDOW_H
#define KP_MAIN_WINDOW_H


#define DEBUG_KP_MAIN_WINDOW 0

#include <qpoint.h>
#include <qlist.h>
#include <qsize.h>

#include <kmainwindow.h>
#include <kurl.h>

#include <kpdefs.h>
#include <kpdocumentsaveoptions.h>
#include <kppixmapfx.h>


class QActionGroup;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMoveEvent;
class QPainter;
class QPixmap;
class QPoint;
class QRect;
class QSize;
class QStringList;
class QUrl;

class QAction;
class KFontAction;
class KFontSizeAction;
class KSelectAction;
class KToggleAction;
class KToolBar;
class KPrinter;
class KRecentFilesAction;
class KToggleFullScreenAction;
class KScanDialog;

class kpColor;
class kpColorToolBar;
class kpCommand;
class kpCommandHistory;
class kpDocument;
class kpDocumentMetaInfo;
class kpDocumentSaveOptions;
class kpViewManager;
class kpViewScrollableContainer;
class kpSelection;
class kpSelectionTransparency;
class kpSqueezedTextLabel;
class kpTextStyle;
class kpThumbnail;
class kpThumbnailView;
class kpTool;
class kpToolText;
class kpToolToolBar;
class kpZoomedView;


class kpMainWindow : public KMainWindow
{
Q_OBJECT

public:
    // Opens a new window with a blank document.
    kpMainWindow ();

    // Opens a new window with the document specified by <url>
    // or creates a blank document if <url> could not be opened.
    kpMainWindow (const KUrl &url);

    // Opens a new window with the document <newDoc>
    // (<newDoc> can be 0 although this would result in a new
    //  window without a document at all).
    kpMainWindow (kpDocument *newDoc);

public:
    double configColorSimilarity () const;
    void configSetColorSimilarity (double val);

private:
    bool m_configFirstTime;
    bool m_configShowGrid;
    bool m_configShowPath;
    double m_configColorSimilarity;

    bool m_configThumbnailShown;
    QRect m_configThumbnailGeometry;
    bool m_configZoomedThumbnail;

    void readGeneralSettings ();
    void readThumbnailSettings ();
    void init ();

public:
    ~kpMainWindow ();

private:
    bool m_isFullyConstructed;

public:
    kpDocument *document () const;
    kpViewManager *viewManager () const;
    kpColorToolBar *colorToolBar () const;
    kpToolToolBar *toolToolBar () const;
    kpCommandHistory *commandHistory () const;

private:
    kpViewScrollableContainer *m_scrollView;
    kpZoomedView *m_mainView;
    kpThumbnail *m_thumbnail;
    kpThumbnailView *m_thumbnailView;
    kpDocument *m_document;
    kpViewManager *m_viewManager;
    kpColorToolBar *m_colorToolBar;
    kpToolToolBar *m_toolToolBar;
    kpCommandHistory *m_commandHistory;

private:
    void setupActions ();
    void enableDocumentActions (bool enable = true);

private:
    void setDocument (kpDocument *newDoc);

    virtual bool queryClose ();

    virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dropEvent (QDropEvent *e);

private slots:
    void slotScrollViewAboutToScroll ();
    void slotScrollViewAfterScroll ();

private:
    virtual void moveEvent (QMoveEvent *e);

private slots:
    void slotUpdateCaption ();
    void slotDocumentRestored ();


    /*
     * Tools
     */

private:
    void setupToolActions ();
    void createToolBox ();
    void enableToolsDocumentActions (bool enable = true);

private slots:
    void updateToolOptionPrevNextActionsEnabled ();

private:
    kpTool *m_toolSpraycan, *m_toolBrush,
           *m_toolColorEraser, *m_toolColorPicker,
           *m_toolCurve, *m_toolEllipse,
           *m_toolEllipticalSelection, *m_toolEraser,
           *m_toolFloodFill, *m_toolFreeFormSelection,
           *m_toolLine, *m_toolPen, *m_toolPolygon,
           *m_toolPolyline, *m_toolRectangle, *m_toolRectSelection,
           *m_toolRoundedRectangle;
    kpToolText *m_toolText;

    QList <kpTool *> m_tools;
    int m_lastToolNumber;

    bool m_toolActionsEnabled;
    QAction *m_actionPrevToolOptionGroup1,
            *m_actionNextToolOptionGroup1,
            *m_actionPrevToolOptionGroup2,
            *m_actionNextToolOptionGroup2;

    int m_settingSelectionTransparency;

    int m_docResizeWidth, m_docResizeHeight;
    bool m_docResizeToBeCompleted;

public:
    QActionGroup *toolsActionGroup ();

    kpTool *tool () const;

    bool toolHasBegunShape () const;
    bool toolIsASelectionTool (bool includingTextTool = true) const;
    bool toolIsTextTool () const;

    kpSelectionTransparency selectionTransparency () const;
    void setSelectionTransparency (const kpSelectionTransparency &transparency,
                                   bool forceColorChange = false);
    int settingSelectionTransparency () const;

private slots:
    void slotToolSelected (kpTool *tool);

private:
    void readLastTool ();
    int toolNumber () const;
    void saveLastTool ();

private:
    bool maybeDragScrollingMainView () const;
private slots:
    bool slotDragScroll (const QPoint &docPoint,
                         const QPoint &docLastPoint,
                         int zoomLevel,
                         bool *didSomething);
    bool slotEndDragScroll ();

private slots:
    void slotBeganDocResize ();
    void slotContinuedDocResize (const QSize &size);
    void slotCancelledDocResize ();
    void slotEndedDocResize (const QSize &size);

    void slotDocResizeMessageChanged (const QString &string);

private slots:
    void slotActionPrevToolOptionGroup1 ();
    void slotActionNextToolOptionGroup1 ();
    void slotActionPrevToolOptionGroup2 ();
    void slotActionNextToolOptionGroup2 ();

public slots:
    void slotToolSpraycan ();
    void slotToolBrush ();
    void slotToolColorEraser ();
    void slotToolColorPicker ();
    void slotToolCurve ();
    void slotToolEllipse ();
    void slotToolEllipticalSelection ();
    void slotToolEraser ();
    void slotToolFloodFill ();
    void slotToolFreeFormSelection ();
    void slotToolLine ();
    void slotToolPen ();
    void slotToolPolygon ();
    void slotToolPolyline ();
    void slotToolRectangle ();
    void slotToolRectSelection ();
    void slotToolRoundedRectangle ();
    void slotToolText ();


    /*
     * File Menu
     */

private:
    void setupFileMenuActions ();
    void enableFileMenuDocumentActions (bool enable = true);

    QAction *m_actionNew, *m_actionOpen;
    KRecentFilesAction *m_actionOpenRecent;
    QAction *m_actionSave, *m_actionSaveAs, *m_actionExport, *m_actionScan,
            *m_actionReload,
            *m_actionPrint, *m_actionPrintPreview,
            *m_actionMail,
            *m_actionSetAsWallpaperTiled, *m_actionSetAsWallpaperCentered,
            *m_actionClose, *m_actionQuit;

    KUrl m_lastExportURL;
    kpDocumentSaveOptions m_lastExportSaveOptions;
    bool m_exportFirstTime;
    KScanDialog *m_scanDialog;

private:
    void addRecentURL (const KUrl &url);
    void setRecentURLs (const QStringList &items);

private slots:
    void slotNew ();
    void slotScanned( const QImage &image, int );

private:
    QSize defaultDocSize () const;
    void saveDefaultDocSize (const QSize &size);

private:
    bool open (const KUrl &url, bool newDocSameNameIfNotExist = false);
    KUrl::List askForOpenURLs (const QString &caption,
                               const QString &startURL,
                               bool allowMultipleURLs = true);

private slots:
    void slotOpen ();
    void slotOpenRecent (const KUrl &url);

    bool save (bool localOnly = false);
    bool slotSave ();

private:
    KUrl askForSaveURL (const QString &caption,
                        const QString &startURL,
                        const QPixmap &pixmapToBeSaved,
                        const kpDocumentSaveOptions &startSaveOptions,
                        const kpDocumentMetaInfo &docMetaInfo,
                        const QString &forcedSaveOptionsGroup,
                        bool localOnly,
                        kpDocumentSaveOptions *chosenSaveOptions,
                        bool isSavingForFirstTime,
                        bool *allowOverwritePrompt,
                        bool *allowLossyPrompt);

private slots:
    bool saveAs (bool localOnly = false);
    bool slotSaveAs ();

    bool slotExport ();
    void slotScan ();

    void slotEnableReload ();
    bool slotReload ();

private:
    void sendFilenameToPrinter (KPrinter *printer);
    void sendPixmapToPrinter (KPrinter *printer, bool showPrinterSetupDialog);

private slots:
    void slotPrint ();
    void slotPrintPreview ();

    void slotMail ();

private:
    void setAsWallpaper (bool centered);
private slots:
    void slotSetAsWallpaperCentered ();
    void slotSetAsWallpaperTiled ();

    void slotClose ();
    void slotQuit ();


    /*
     * Edit Menu
     */

private:
    kpPixmapFX::WarnAboutLossInfo pasteWarnAboutLossInfo ();
    void setupEditMenuActions ();
    void enableEditMenuDocumentActions (bool enable = true);

    bool m_editMenuDocumentActionsEnabled;

    QAction *m_actionUndo, *m_actionRedo,
            *m_actionCut, *m_actionCopy,
            *m_actionPaste, *m_actionPasteInNewWindow,
            *m_actionDelete,
            *m_actionSelectAll, *m_actionDeselect,
            *m_actionCopyToFile, *m_actionPasteFromFile;

    KUrl m_lastPasteFromURL;

    KUrl m_lastCopyToURL;
    kpDocumentSaveOptions m_lastCopyToSaveOptions;
    bool m_copyToFirstTime;

public:
    QMenu *selectionToolRMBMenu ();

private slots:
    void slotCut ();
    void slotCopy ();
    void slotEnablePaste ();
private:
    QRect calcUsefulPasteRect (int pixmapWidth, int pixmapHeight);
    void paste (const kpSelection &sel,
                bool forceTopLeft = false);
public:
    // (<forceNewTextSelection> is ignored if <text> is empty)
    void pasteText (const QString &text,
                    bool forceNewTextSelection = false,
                    const QPoint &newTextSelectionTopLeft = KP_INVALID_POINT);
    void pasteTextAt (const QString &text, const QPoint &point,
                      // Allow tiny adjustment of <point> so that mouse
                      // pointer is not exactly on top of the topLeft of
                      // any new text selection (so that it doesn't look
                      // weird by being on top of a resize handle just after
                      // a paste).
                      bool allowNewTextSelectionPointShift = false);
public slots:
    void slotPaste ();
private slots:
    void slotPasteInNewWindow ();
public slots:
    void slotDelete ();

    void slotSelectAll ();
private:
    void addDeselectFirstCommand (kpCommand *cmd);
public slots:
    void slotDeselect ();
private slots:
    void slotCopyToFile ();
    void slotPasteFromFile ();


    /*
     * View Menu
     */

private:
    bool m_viewMenuDocumentActionsEnabled;

    void setupViewMenuActions ();
    bool viewMenuDocumentActionsEnabled () const;
    void enableViewMenuDocumentActions (bool enable = true);
    void actionShowGridUpdate ();

    QAction *m_actionActualSize,
            *m_actionFitToPage, *m_actionFitToWidth, *m_actionFitToHeight,
            *m_actionZoomIn, *m_actionZoomOut;
    KSelectAction *m_actionZoom;
    KToggleAction *m_actionShowGrid,
                  *m_actionShowThumbnail, *m_actionZoomedThumbnail;

    QList <int> m_zoomList;

private:
    void sendZoomListToActionZoom ();
    int zoomLevelFromString (const QString &string);
    QString zoomLevelToString (int zoomLevel);
    void zoomTo (int zoomLevel, bool centerUnderCursor = false);

private slots:
    void finishZoomTo ();

private slots:
    void slotActualSize ();
    void slotFitToPage ();
    void slotFitToWidth ();
    void slotFitToHeight ();

public:
    void zoomIn (bool centerUnderCursor = false);
    void zoomOut (bool centerUnderCursor = false);

public slots:
    void slotZoomIn ();
    void slotZoomOut ();

private:
    void zoomAccordingToZoomAction (bool centerUnderCursor = false);

private slots:
    void slotZoom ();

    void slotShowGridToggled ();
private:
    void updateMainViewGrid ();

private:
    QRect mapToGlobal (const QRect &rect) const;
    QRect mapFromGlobal (const QRect &rect) const;

private slots:
    void slotDestroyThumbnail ();
    void slotDestroyThumbnailInitatedByUser ();
    void slotCreateThumbnail ();

private:
    QTimer *m_thumbnailSaveConfigTimer;

public:
    void notifyThumbnailGeometryChanged ();

private slots:
    void slotSaveThumbnailGeometry ();
    void slotShowThumbnailToggled ();
    void updateThumbnailZoomed ();
    void slotZoomedThumbnailToggled ();
    void slotThumbnailShowRectangleToggled ();

private:
    void enableViewZoomedThumbnail (bool enable = true);
    void enableViewShowThumbnailRectangle (bool enable = true);
    void enableThumbnailOptionActions (bool enable = true);
    void createThumbnailView ();
    void destroyThumbnailView ();
    void updateThumbnail ();


    /*
     * Image Menu
     */

private:
    bool isSelectionActive () const;
    bool isTextSelection () const;

    QString autoCropText () const;

    void setupImageMenuActions ();
    void enableImageMenuDocumentActions (bool enable = true);

    bool m_imageMenuDocumentActionsEnabled;

    QAction *m_actionResizeScale,
            *m_actionCrop, *m_actionAutoCrop,
            *m_actionFlip, *m_actionRotate, *m_actionSkew,
            *m_actionConvertToBlackAndWhite, *m_actionConvertToGrayscale,
            *m_actionMoreEffects,
            *m_actionInvertColors, *m_actionClear;

private slots:
    void slotImageMenuUpdateDueToSelection ();

public:
    kpColor backgroundColor (bool ofSelection = false) const;
    void addImageOrSelectionCommand (kpCommand *cmd,
                                     bool addSelCreateCmdIfSelAvail = true,
                                     bool addSelPullCmdIfSelAvail = true);

private slots:
    void slotResizeScale ();
public slots:
    void slotCrop ();
private slots:
    void slotAutoCrop ();
    void slotFlip ();

    void slotRotate ();
    void slotRotate270 ();
    void slotRotate90 ();

    void slotSkew ();
    void slotConvertToBlackAndWhite ();
    void slotConvertToGrayscale ();
    void slotInvertColors ();
    void slotClear ();
    void slotMoreEffects ();


    /*
     * Settings Menu
     */

private:
    void setupSettingsMenuActions ();
    void enableSettingsMenuDocumentActions (bool enable = true);

    KToggleAction *m_actionShowPath;
    QAction *m_actionKeyBindings, *m_actionConfigureToolbars, *m_actionConfigure;
    KToggleFullScreenAction *m_actionFullScreen;

private slots:
    void slotFullScreen ();

    void slotEnableSettingsShowPath ();
    void slotShowPathToggled ();

    void slotKeyBindings ();

    void slotConfigureToolBars ();
    void slotNewToolBarConfig ();

    void slotConfigure ();


    /*
     * Status Bar
     */

private:
    bool m_statusBarCreated;
    kpSqueezedTextLabel *m_statusBarMessageLabel;

    bool m_statusBarShapeLastPointsInitialised;
    QPoint m_statusBarShapeLastStartPoint, m_statusBarShapeLastEndPoint;
    bool m_statusBarShapeLastSizeInitialised;
    QSize m_statusBarShapeLastSize;

    enum
    {
        StatusBarItemMessage,
        StatusBarItemShapePoints,
        StatusBarItemShapeSize,
        StatusBarItemDocSize,
        StatusBarItemDocDepth,
        StatusBarItemZoom
    };

    void addPermanentStatusBarItem (int id, int maxTextLen);
    void createStatusBar ();

private slots:
    void setStatusBarMessage (const QString &message = QString::null);
    void setStatusBarShapePoints (const QPoint &startPoint = KP_INVALID_POINT,
                                  const QPoint &endPoint = KP_INVALID_POINT);
    void setStatusBarShapeSize (const QSize &size = KP_INVALID_SIZE);
    void setStatusBarDocSize (const QSize &size = KP_INVALID_SIZE);
    void setStatusBarDocDepth (int depth = 0);
    void setStatusBarZoom (int zoom = 0);

    void recalculateStatusBarMessage ();
    void recalculateStatusBarShape ();

    void recalculateStatusBar ();


    /*
     * Text ToolBar
     */

private:
    void setupTextToolBarActions ();
    void readAndApplyTextSettings ();

public:
    void enableTextToolBarActions (bool enable = true);

private slots:
    void slotTextFontFamilyChanged ();
    void slotTextFontSizeChanged ();
    void slotTextBoldChanged ();
    void slotTextItalicChanged ();
    void slotTextUnderlineChanged ();
    void slotTextStrikeThruChanged ();

public:
    KToolBar *textToolBar ();
    bool isTextStyleBackgroundOpaque () const;
    kpTextStyle textStyle () const;
    void setTextStyle (const kpTextStyle &textStyle_);
    int settingTextStyle () const;

private:
    KFontAction *m_actionTextFontFamily;
    KFontSizeAction *m_actionTextFontSize;
    KToggleAction *m_actionTextBold, *m_actionTextItalic,
                  *m_actionTextUnderline, *m_actionTextStrikeThru;

    int m_settingTextStyle;
    QString m_textOldFontFamily;
    int m_textOldFontSize;


    /*
     * Help Menu
     */
private:
    void setupHelpMenuActions ();
    void enableHelpMenuDocumentActions (bool enable = true);

private slots:
    void slotHelpTakingScreenshots ();
    void slotHelpTakingScreenshotsFollowLink (const QString &link);


private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpMainWindowPrivate *d;
};


#endif  // KP_MAIN_WINDOW_H
