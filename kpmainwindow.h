
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

#include <qcolor.h>
#include <qstringlist.h>
#include <kmainwindow.h>
#include <kurl.h>

class ButtonState;
class QPoint;
class QScrollView;

class KSelectAction;
class KToggleAction;
class KAction;
class KCommandHistory;
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

private:
    void setupActions ();

    void setupFileMenuActions ();
    KAction *m_actionNew, *m_actionOpen;
    KRecentFilesAction *m_actionOpenRecent;
    KAction *m_actionSave, *m_actionSaveAs, *m_actionRevert,
            *m_actionPrint, *m_actionPrintPreview,
            *m_actionSetAsWallpaperTiled, *m_actionSetAsWallpaperCentered,
            *m_actionClose, *m_actionQuit;

    void setupEditMenuActions ();
    KCommandHistory *m_commandHistory;
    KAction *m_actionCut, *m_actionCopy, *m_actionPaste,
            *m_actionSelectAll, *m_actionDeselect;

    void setupViewMenuActions ();
    KAction *m_actionFullScreen,
            *m_actionZoomIn, *m_actionZoomOut;
    KSelectAction *m_actionZoom;
    KToggleAction *m_actionShowGrid;

    void setupImageMenuActions ();
    KAction *m_actionProperties,
            *m_actionResizeScale, *m_actionFlip, *m_actionRotate, *m_actionSkew,
            *m_actionConvertToGrayscale, *m_actionInvertColors, *m_actionClear;

    void setupSettingsMenuActions ();
    KToggleAction *m_actionShowPath;
    KAction *m_actionKeyBindings, *m_actionConfigureToolbars, *m_actionConfigure;

private:
    void setupTools ();
/*    KAction *m_actionToolPen, *m_actionToolLine, *m_actionToolEllipse, *m_actionToolColorPicker,
            *m_actionToolAirSpray, *m_actionToolRectangle, *m_actionToolRoundedRectangle,
            *m_actionToolPolygon, *m_actionToolFloodFill, *m_actionToolRotate, *m_actionToolBrush,
            *m_actionToolEraser, *m_actionToolRectSelection, *m_actionToolEllipticalSelection, *m_actionToolFreeFormSelection, *m_actionToolColorWasher;*/

public:
    ~kpMainWindow ();

public:
    kpDocument *document ();
    kpViewManager *viewManager ();
    kpColorToolBar *colorToolBar ();
    kpToolToolBar *toolToolBar () { return m_toolToolBar; }
    KCommandHistory *commandHistory ();
    QColor color (int which) const;
    QColor color (const ButtonState &buttonState) const;
    kpTool *tool ();

    void switchToPreviousTool ();

signals:

public slots:
    void slotUpdateCaption ();
    void slotDocumentRestored ();

    void slotNew (const KURL &url = KURL ());
    bool slotOpen ();
    bool open (const QString &, bool newDocSameNameIfNotExist = false);
    bool open (const KURL &, bool newDocSameNameIfNotExist = false);
    bool slotOpenRecent (const KURL &);
    bool slotSave ();
    bool save (bool localOnly = false);
    bool slotSaveAs ();
    bool saveAs (bool localOnly = false);
    bool slotRevert ();
    void slotPrint ();
    void slotPrintPreview ();
    void slotClose ();
    void slotQuit ();
    void slotSetAsWallpaperCentered ();
    void slotSetAsWallpaperTiled ();

    void slotCut ();
    void slotCopy ();
    void slotPaste ();
    void slotSelectAll ();
    void slotDeselect ();

    void slotZoomIn ();
    void slotZoomOut ();
    void slotZoom ();
    void slotShowGrid ();

    void slotShowPath ();
    void slotConfigure ();
    void slotKeyBindings ();
    void slotConfigureToolBars ();

    void slotToolPen ();
    void slotToolLine ();
    void slotToolEllipse ();
    void slotToolColorPicker ();
    void slotToolAirSpray ();
    void slotToolRectangle ();
    void slotToolRoundedRectangle ();
    void slotToolPolygon ();
    void slotToolFloodFill ();
    void slotToolRotate ();
    void slotToolBrush ();
    void slotToolEraser ();
    void slotToolRectSelection ();
    void slotToolEllipticalSelection ();
    void slotToolFreeFormSelection ();
    void slotToolColorWasher ();

    void slotProperties ();
    void slotResizeScale ();
    void slotFlip ();
    void slotRotate ();
    void slotSkew ();
    void slotConvertToGrayscale ();
    void slotInvertColors ();
    void slotClear ();

    void slotUpdateStatusBar ();
    void slotUpdateStatusBar (int docWidth, int docHeight);
    void slotUpdateStatusBar (int docColorDepth);
    void slotUpdateStatusBar (int docWidth, int docHeight, int docColorDepth);
    void slotUpdateStatusBar (const QPoint &point);
    void slotUpdateStatusBar (const QRect &srect);

    // HACK
    QScrollView *scrollView () const { return m_scrollView; }

private slots:
    void slotActionShowGridToggled (bool on);
    void slotActionShowPathToggled (bool on);

    void switchToTool (kpTool *);
    void switchToTool (int which);

private:
    virtual bool queryClose ();
    void setMainView (kpView *view);

    bool legalDocPoint (const QPoint &point) const;

    void setAsWallpaper (bool centered);

    virtual void dragEnterEvent (QDragEnterEvent *e);
    virtual void dropEvent (QDropEvent *e);

    QStringList m_zoomList;

    QScrollView *m_scrollView;
    kpView *m_mainView;
    kpDocument *m_document;
    kpViewManager *m_viewManager;
    kpColorToolBar *m_colorToolBar;
    kpToolToolBar *m_toolToolBar;


    kpTool *m_tools [32];
    int m_toolNum, m_previousToolNum;
    int m_numTools;

    bool m_configFirstTime;
    bool m_configShowGrid;
    bool m_configShowPath;
    QString m_configDefaultOutputMimetype;
};

#endif  // __kpmainwindow_h__
