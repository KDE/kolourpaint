
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kpmainwindow_h__
#define __kpmainwindow_h__

#define DEBUG_KPMAINWINDOW 1

#include <qptrlist.h>
#include <qstringlist.h>
#include <kmainwindow.h>
#include <kurl.h>

// Disabling all the actions (e.g. Undo/Redo, Open, Save) when a tool is active
// would look a little weird so we just ignore all calls to user-triggerable
// slots instead.
//
// Syntax examples:
//  KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;        // void return
//  KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE false;  // to return false
//
#define KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE if (toolHasBegunDraw ()) return

class QColor;
class QPoint;
class QScrollView;

class KSelectAction;
class KToggleAction;
class KAction;
class KCommandHistory;
class KPrinter;
class KRecentFilesAction;

class kpView;
class kpColorToolBar;
class kpDocument;
class kpViewManager;
class kpTool;
class kpToolToolBar;

class kpMainWindow : public KMainWindow
{
Q_OBJECT

public:
    kpMainWindow (const KURL &url = KURL ());
    ~kpMainWindow ();

private:
    bool m_configFirstTime;

public:
    kpDocument *document () const;
    kpViewManager *viewManager () const;
    kpColorToolBar *colorToolBar () const;
    kpToolToolBar *toolToolBar () const;
    KCommandHistory *commandHistory () const;

private:
    QScrollView *m_scrollView;
    kpView *m_mainView, *m_thumbnailView;
    kpDocument *m_document;
    kpViewManager *m_viewManager;
    kpColorToolBar *m_colorToolBar;
    kpToolToolBar *m_toolToolBar;

private:
    void setupActions ();

    void setMainView (kpView *view);
    
    virtual bool queryClose ();
    
    virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dropEvent (QDropEvent *e);

private slots:
    void slotUpdateCaption ();

private:
    bool legalDocPoint (const QPoint &point) const;
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
    
    kpTool *m_toolAirSpray, *m_toolBrush, *m_toolColorPicker,
           *m_toolColorWasher, *m_toolCurve, *m_toolEllipse,
           *m_toolEllipticalSelection, *m_toolEraser,
           *m_toolFloodFill, *m_toolFreeFormSelection, 
           *m_toolLine, *m_toolPen, *m_toolPolygon,
           *m_toolPolyline, *m_toolRectangle, *m_toolRectSelection,
           *m_toolRoundedRectangle, *m_toolText;

    QPtrList <kpTool> m_tools;
    kpTool *m_currentTool, *m_previousTool;

public:
    kpTool *tool () const;
    bool toolHasBegunDraw () const;
    
public slots:
    void switchToTool (kpTool *);
    void switchToPreviousTool ();

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
    
    KAction *m_actionNew, *m_actionOpen;
    KRecentFilesAction *m_actionOpenRecent;
    KAction *m_actionSave, *m_actionSaveAs, *m_actionRevert,
            *m_actionPrint, *m_actionPrintPreview,
            *m_actionSetAsWallpaperTiled, *m_actionSetAsWallpaperCentered,
            *m_actionClose, *m_actionQuit;

    QString m_configDefaultOutputMimetype;
    
private slots:
    void slotNew (const KURL &url = KURL ());
    void slotDocumentRestored ();
    
    bool open (const QString &, bool newDocSameNameIfNotExist = false);
    bool open (const KURL &, bool newDocSameNameIfNotExist = false);
    bool slotOpen ();
    bool slotOpenRecent (const KURL &);
    
    bool save (bool localOnly = false);
    bool slotSave ();
    
    bool saveAs (bool localOnly = false);
    bool slotSaveAs ();
    
    bool slotRevert ();

private:       
    void sendFilenameToPrinter (KPrinter *printer);
    void sendPixmapToPrinter (KPrinter *printer);

private slots:
    void slotPrint ();
    void slotPrintPreview ();
    
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
    KCommandHistory *m_commandHistory;
    KAction *m_actionUndo, *m_actionRedo,
            *m_actionCut, *m_actionCopy, *m_actionPaste, *m_actionDelete,
            *m_actionSelectAll, *m_actionDeselect;

private slots:
    void slotUndo ();
    void slotRedo ();
    
    void slotCut ();
    void slotCopy ();
    void slotPaste ();
    void slotDelete ();

    void slotSelectAll ();
    void slotDeselect ();

 
    /*
     * View Menu
     */

private:
    void setupViewMenuActions ();
    
    KAction *m_actionFullScreen,
            *m_actionZoomIn, *m_actionZoomOut;
    KSelectAction *m_actionZoom;
    KToggleAction *m_actionShowGrid;
    
    QStringList m_zoomList;
    bool m_configShowGrid;

private slots:
    void slotZoomIn ();
    void slotZoomOut ();
    void slotZoom ();
    
    void slotShowGrid ();
    void slotActionShowGridToggled (bool on);
    
    
    /*
     * Image Menu
     */
     
private:
    void setupImageMenuActions ();
    
    KAction *m_actionResizeScale, *m_actionFlip, *m_actionRotate, *m_actionSkew,
            *m_actionConvertToGrayscale, *m_actionInvertColors, *m_actionClear;

private slots:
    void slotResizeScale ();
    void slotFlip ();
    void slotRotate ();
    void slotSkew ();
    void slotConvertToGrayscale ();
    void slotInvertColors ();
    void slotClear ();


    /*
     * Settings Menu
     */

private:
    void setupSettingsMenuActions ();
    
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
