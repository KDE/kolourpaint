
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#ifndef __kp_main_window_h__
#define __kp_main_window_h__

#define DEBUG_KP_MAIN_WINDOW 0

#include <qptrlist.h>
#include <qvaluevector.h>

#include <kmainwindow.h>
#include <kurl.h>

#include <kpdefs.h>
#include <kppixmapfx.h>


class QPainter;
class QPoint;
class QRect;
class QScrollView;
class QStringList;

class KAction;
class KFontAction;
class KFontSizeAction;
class KCommand;
class KSelectAction;
class KToggleAction;
class KToolBar;
class KPrinter;
class KRecentFilesAction;

class kpColor;
class kpColorToolBar;
class kpCommandHistory;
class kpDocument;
class kpView;
class kpViewManager;
class kpSelection;
class kpSelectionTransparency;
class kpSingleKeyTriggersAction;
class kpSqueezedTextLabel;
class kpTextStyle;
class kpThumbnail;
class KToggleFullScreenAction;
class kpTool;
class kpToolText;
class kpToolToolBar;


class kpMainWindow : public KMainWindow
{
Q_OBJECT

public:
    // Opens a new window with a blank document.
    kpMainWindow ();

    // Opens a new window with the document specified by <url>
    // or creates a blank document if <url> could not be opened.
    kpMainWindow (const KURL &url);

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
    QScrollView *m_scrollView;
    kpView *m_mainView;
    kpThumbnail *m_thumbnail;
    kpView *m_thumbnailView;
    kpDocument *m_document;
    kpViewManager *m_viewManager;
    kpColorToolBar *m_colorToolBar;
    kpToolToolBar *m_toolToolBar;
    kpCommandHistory *m_commandHistory;

private:
    void setupActions ();
    void enableDocumentActions (bool enable = true);

public:
    bool actionsSingleKeyTriggersEnabled () const;
    void enableActionsSingleKeyTriggers (bool enable = true);

private:
    void setDocument (kpDocument *newDoc);

    virtual bool queryClose ();

    virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dropEvent (QDropEvent *e);

private slots:
    void slotScrollViewAboutToScroll ();

private:
    virtual void moveEvent (QMoveEvent *e);

public:
    void drawTransparentBackground (QPainter *painter,
                                    int viewWidth, int viewHeight,
                                    const QRect &rect,
                                    bool isPreview = false);

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
    kpTool *m_toolAirSpray, *m_toolBrush, *m_toolColorPicker,
           *m_toolColorWasher, *m_toolCurve, *m_toolEllipse,
           *m_toolEllipticalSelection, *m_toolEraser,
           *m_toolFloodFill, *m_toolFreeFormSelection,
           *m_toolLine, *m_toolPen, *m_toolPolygon,
           *m_toolPolyline, *m_toolRectangle, *m_toolRectSelection,
           *m_toolRoundedRectangle;
    kpToolText *m_toolText;

    QPtrList <kpTool> m_tools;

    int m_settingSelectionTransparency;

public:
    kpTool *tool () const;
    bool toolHasBegunShape () const;
    bool toolIsASelectionTool (bool includingTextTool = true) const;
    bool toolIsTextTool () const;

    kpSelectionTransparency selectionTransparency () const;
    void setSelectionTransparency (const kpSelectionTransparency &transparency, bool forceColorChange = false);
    int settingSelectionTransparency () const;

private slots:
    void slotToolSelected (kpTool *tool);

private:
    void readLastTool ();
    int toolNumber () const;
    void saveLastTool ();

private slots:
    void slotActionPrevToolOptionGroup1 ();
    void slotActionNextToolOptionGroup1 ();
    void slotActionPrevToolOptionGroup2 ();
    void slotActionNextToolOptionGroup2 ();

public slots:
    void slotToolAirSpray ();
    void slotToolBrush ();
    void slotToolColorPicker ();
    void slotToolColorWasher ();
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

    KAction *m_actionNew, *m_actionOpen;
    KRecentFilesAction *m_actionOpenRecent;
    KAction *m_actionSave, *m_actionSaveAs, *m_actionReload,
            *m_actionPrint, *m_actionPrintPreview,
            *m_actionMail,
            *m_actionSetAsWallpaperTiled, *m_actionSetAsWallpaperCentered,
            *m_actionClose, *m_actionQuit;

private:
    bool shouldOpenInNewWindow () const;
    void addRecentURL (const KURL &url);
    void setRecentURLs (const QStringList &items);

private slots:
    void slotNew ();

private:
    QSize defaultDocSize () const;
    void saveDefaultDocSize (const QSize &size);

private:
    bool open (const KURL &url, bool newDocSameNameIfNotExist = false);
    KURL::List askForOpenURLs (const QString &caption,
                               const QString &startURL,
                               bool allowMultipleURLs = true);

private slots:
    void slotOpen ();
    void slotOpenRecent (const KURL &url);

    bool save (bool localOnly = false);
    bool slotSave ();

private:
    KURL askForSaveURL (const QString &caption,
                        const QString &startURL,
                        const QString &startMimeType,
                        const char *lastOutputMimeTypeSettingsPrefix,
                        bool localOnly,
                        QString &chosenMimeType);
    void saveLastOutputMimeType (const QString &mimeType,
                                 const char *lastOutputMimeTypeSettingsPrefix);

private slots:
    bool saveAs (bool localOnly = false);
    bool slotSaveAs ();

    bool slotExport ();

    void slotEnableReload ();
    bool slotReload ();

private:
    void sendFilenameToPrinter (KPrinter *printer);
    void sendPixmapToPrinter (KPrinter *printer);

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

    KAction *m_actionUndo, *m_actionRedo,
            *m_actionCut, *m_actionCopy, *m_actionPaste, *m_actionDelete,
            *m_actionSelectAll, *m_actionDeselect;

private slots:
    void slotCut ();
    void slotCopy ();
    void slotEnablePaste ();
private:
    QRect calcUsefulPasteRect (int pixmapWidth, int pixmapHeight);
    void paste (const kpSelection &sel);
public:
    void pasteText (const QString &text, bool forceNewTextSelection = false);
    void pasteTextAt (const QString &text, const QPoint &point);
public slots:
    void slotPaste ();
private slots:
    void slotPasteInNewWindow ();
public slots:
    void slotDelete ();

    void slotSelectAll ();
private:
    void addDeselectFirstCommand (KCommand *cmd);
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

    KAction *m_actionFullScreenBIC,
            *m_actionActualSize,
            *m_actionFitToPage, *m_actionFitToWidth, *m_actionFitToHeight,
            *m_actionZoomIn, *m_actionZoomOut;
    KSelectAction *m_actionZoom;
    KToggleAction *m_actionShowGrid, *m_actionShowThumbnail;

    QValueVector <int> m_zoomList;

private:
    void sendZoomListToActionZoom ();
    int zoomLevelFromString (const QString &string);
    QString zoomLevelToString (int zoomLevel);
    void zoomTo (int zoomLevel);

private slots:
    void finishZoomTo ();

private slots:
    void slotActualSize ();
    void slotFitToPage ();
    void slotFitToWidth ();
    void slotFitToHeight ();

    void slotZoomIn ();
    void slotZoomOut ();

    void slotZoom ();

    void slotShowGridToggled ();
private:
    void updateMainViewGrid ();

private:
    QRect mapToGlobal (const QRect &rect) const;
    QRect mapFromGlobal (const QRect &rect) const;

private slots:
    void slotDestroyThumbnailIfNotVisible (bool tnIsVisible);
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
private:
    void updateThumbnail ();


    /*
     * Image Menu
     */

private:
    bool isSelectionActive () const;
    bool isTextSelection () const;
    QString actionResizeScaleText () const;

    void setupImageMenuActions ();
    void enableImageMenuDocumentActions (bool enable = true);

    bool m_imageMenuDocumentActionsEnabled;

    KAction *m_actionResizeScale,
            *m_actionCrop, *m_actionAutoCrop,
            *m_actionFlip, *m_actionRotate, *m_actionSkew,
            *m_actionConvertToBlackAndWhite, *m_actionConvertToGrayscale,
            *m_actionInvertColors, *m_actionClear;

private slots:
    void slotImageMenuUpdateDueToSelection ();

public:
    kpColor backgroundColor (bool ofSelection = false) const;
    void addImageOrSelectionCommand (KCommand *cmd, bool actOnSelectionIfAvail = true);

private slots:
    void slotResizeScale ();
    void slotCrop ();
    void slotAutoCrop ();
    void slotFlip ();
    void slotRotate ();
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
    KAction *m_actionKeyBindings, *m_actionConfigureToolbars, *m_actionConfigure;

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
    void slotUpdateStatusBarMessage (const QString &message = QString::null);
    void slotUpdateStatusBarShapePoints (const QPoint &startPoint = KP_INVALID_POINT,
                                         const QPoint &endPoint = KP_INVALID_POINT);
    void slotUpdateStatusBarShapeSize (const QSize &size = KP_INVALID_SIZE);
    void slotUpdateStatusBarDocSize (const QSize &size = KP_INVALID_SIZE);
    void slotUpdateStatusBarDocDepth (int depth = 0);
    void slotUpdateStatusBarZoom (int zoom = 0);
    void slotUpdateStatusBar ();


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

private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpMainWindowPrivate *d;
};

struct kpMainWindowPrivate
{
    KToggleFullScreenAction *m_actionFullScreen;
    KAction *m_actionExport;
    KAction *m_actionPasteInNewWindow;
    KAction *m_actionCopyToFile, *m_actionPasteFromFile;
    KURL m_lastPasteFromURL;
    KURL m_lastCopyToURL;
    QString m_lastCopyToMimeType;
    KURL m_lastExportURL;
    QString m_lastExportMimeType;
    int m_lastToolNumber;
    KAction *m_actionMoreEffects;

    bool m_toolActionsEnabled;
    kpSingleKeyTriggersAction *m_actionPrevToolOptionGroup1,
                              *m_actionNextToolOptionGroup1,
                              *m_actionPrevToolOptionGroup2,
                              *m_actionNextToolOptionGroup2;
};

#endif  // __kp_main_window_h__
