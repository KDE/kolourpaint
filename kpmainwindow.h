
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


#ifndef __kpmainwindow_h__
#define __kpmainwindow_h__

#define DEBUG_KP_MAIN_WINDOW 1

#include <qptrlist.h>
#include <qvaluevector.h>
#include <kmainwindow.h>
#include <kurl.h>

class QPainter;
class QPoint;
class QRect;
class QScrollView;

class KCommand;
class KSelectAction;
class KToggleAction;
class KAction;
class KPrinter;
class KRecentFilesAction;

class kpColor;
class kpColorToolBar;
class kpCommandHistory;
class kpDocument;
class kpView;
class kpViewManager;
class kpThumbnail;
class kpTool;
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

private:
    void initGUI ();

public:

    ~kpMainWindow ();

private:
    bool m_configFirstTime;
    bool m_alive;

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

    void setDocument (kpDocument *newDoc);
    
    virtual bool queryClose ();
    
    virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dropEvent (QDropEvent *e);

protected:
    virtual void moveEvent (QMoveEvent *e);

public:
    void drawTransparentBackground (QPainter *painter,
                                    int viewWidth, int viewHeight,
                                    const QRect &rect,
                                    bool isPreview = false);

private slots:
    void slotUpdateCaption ();
    void slotDocumentRestored ();

private:
    bool legalDocPoint (const QPoint &point) const;

    enum
    {
        StatusBarItemDocInfo,
        StatusBarItemShapeEndPoints,
        StatusBarItemShapeSize
    };
    
private slots:
    void slotUpdateStatusBar ();
    void slotUpdateStatusBar (int docWidth, int docHeight);
    void slotUpdateStatusBar (int docColorDepth);
    void slotUpdateStatusBar (int docWidth, int docHeight, int docColorDepth);
    void slotUpdateStatusBar (const QPoint &point);
    void slotUpdateStatusBar (const QRect &srect);

    
    /*
     * Tools
     */

private:
    void setupTools ();
    void enableToolsDocumentActions (bool enable = true);
    
    kpTool *m_toolAirSpray, *m_toolBrush, *m_toolColorPicker,
           *m_toolColorWasher, *m_toolCurve, *m_toolEllipse,
           *m_toolEllipticalSelection, *m_toolEraser,
           *m_toolFloodFill, *m_toolFreeFormSelection, 
           *m_toolLine, *m_toolPen, *m_toolPolygon,
           *m_toolPolyline, *m_toolRectangle, *m_toolRectSelection,
           *m_toolRoundedRectangle, *m_toolText;

    QPtrList <kpTool> m_tools;

public:
    kpTool *tool () const;
    bool toolHasBegunShape () const;
    bool toolIsASelectionTool () const;
    
private slots:
    void slotToolSelected (kpTool *tool);

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

    QString m_configDefaultOutputMimetype;
    
private:
    bool shouldOpenInNewWindow () const;
    void addRecentURL (const KURL &url);

private slots:
    void slotNew (const KURL &url = KURL ());
    
    bool open (const KURL &url, bool newDocSameNameIfNotExist = false);
    void slotOpen ();
    void slotOpenRecent (const KURL &url);
    
    bool save (bool localOnly = false);
    bool slotSave ();
    
    bool saveAs (bool localOnly = false);
    bool slotSaveAs ();
    
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
    void paste (const QPixmap &pixmap);
private slots:
    void slotPaste ();
    void slotDelete ();

    void slotSelectAll ();
public slots:
    void slotDeselect ();

 
    /*
     * View Menu
     */

private:
    void setupViewMenuActions ();
    void enableViewMenuDocumentActions (bool enable = true);
    
    KAction *m_actionFullScreen,
            *m_actionActualSize,
            *m_actionFitToPage, *m_actionFitToWidth, *m_actionFitToHeight,
            *m_actionZoomIn, *m_actionZoomOut;
    KSelectAction *m_actionZoom;
    KToggleAction *m_actionShowGrid, *m_actionShowThumbnail;
    
    QValueVector <int> m_zoomList;
    bool m_configShowGrid;

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
    
    void slotShowGrid ();
    void slotActionShowGridToggled (bool on);

private:
    QRect mapToGlobal (const QRect &rect) const;
    QRect mapFromGlobal (const QRect &rect) const;

private slots:
    void slotDestroyThumbnailIfNotVisible (bool tnIsVisible);
    void slotDestroyThumbnail ();

public:
    void notifyThumbnailGeometryChanged ();
    
private slots:
    void slotShowThumbnail ();
    
    
    /*
     * Image Menu
     */
     
private:
    void setupImageMenuActions ();
    void enableImageMenuDocumentActions (bool enable = true);
    
    KAction *m_actionResizeScale,
            *m_actionCrop, *m_actionAutoCrop,
            *m_actionFlip, *m_actionRotate, *m_actionSkew,
            *m_actionConvertToBlackAndWhite, *m_actionConvertToGrayscale,
            *m_actionInvertColors, *m_actionClear;

private slots:
    void slotImageMenuUpdateDueToSelection ();

public:
    kpColor backgroundColor () const;
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


    /*
     * Settings Menu
     */

private:
    void setupSettingsMenuActions ();
    void enableSettingsMenuDocumentActions (bool enable = true);
    
    KToggleAction *m_actionShowPath;
    KAction *m_actionKeyBindings, *m_actionConfigureToolbars, *m_actionConfigure;
    
    bool m_configShowPath;

private slots:
    void slotShowPath ();
    void slotActionShowPathToggled (bool on);

    void slotConfigure ();
    void slotKeyBindings ();
    void slotConfigureToolBars ();
};

#endif  // __kpmainwindow_h__
