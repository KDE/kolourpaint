
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

#ifndef __kpviewmanager_h__
#define __kpviewmanager_h__

#include <qobject.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qrect.h>


class QCursor;
class QPixmap;
class QRect;
class kpView;
class kpMainWindow;

class kpViewManager : public QObject
{
Q_OBJECT

public:
    kpViewManager (kpMainWindow *mainWindow);
    ~kpViewManager ();

    // registering views
    void registerView (kpView *view);
    void unregisterView (kpView *view);
    void unregisterAllViews ();

    enum SelectionBorderType
    {
        NoBorder, Rectangle, Ellipse, FreeForm
    };

    void setSelectionBorderType (enum SelectionBorderType sb = Rectangle,
                                 bool update = true);
    enum SelectionBorderType selectionBorderType () const;

    enum TempPixmapType
    {
        NoPixmap, NormalPixmap, SelectionPixmap, BrushPixmap
    };

    // (you don't need to call invalidateTempPixmap first)
    void setTempPixmapAt (const QPixmap &pixmap, const QPoint &at,
                          enum TempPixmapType type = NormalPixmap,
                          enum SelectionBorderType selBorderType = NoBorder);
    void invalidateTempPixmap (const bool doUpdate = true);

    enum TempPixmapType tempPixmapType () /*const*/;
    bool tempPixmapActive () /*const*/;

    bool normalActive () /*const*/;
    bool selectionActive () /*const*/;
    bool brushActive () /*const*/;

    QRect tempPixmapRect () const;
    QPixmap tempPixmap () const;

    void setCursor (const QCursor &cursor);
    void unsetCursor ();

    kpView *viewUnderCursor () /*const*/;
    
    //
    // QWidget::hasMouse() is unreliable:
    //
    // "bool QWidget::hasMouse () const
    //  ... See the "underMouse" property for details.
    //         .
    //         .
    //         .
    //  bool underMouse
    //  ... This value is not updated properly during drag and drop operations."
    //
    // i.e. it's possible that hasMouse() returns false in a mousePressEvent()!
    //
    // This hack needs to be called from kpView so that viewUnderCursor() works
    // as a reasonable replacement (although there is at least one case where
    // it still won't work - just after a fake drag onto the view).
    //
    void setViewUnderCursor (kpView *view);
    
signals:
    void selectionEnabled (bool on);

public slots:
    void repaintBrushPixmap ();

    // updating views
    void updateViews ();
    void updateViews (const QRect &docRect);
    void updateViews (int x, int y, int w, int h);
    void resizeViews (int docWidth, int docHeight);

private:
    // don't use
    kpViewManager (const kpViewManager &);
    bool operator= (const kpViewManager &);

    kpDocument *document ();

    kpMainWindow *m_mainWindow;
    QPtrList <kpView> m_views;
    
    kpView *m_viewUnderCursor;

    QPixmap m_tempPixmap;
    QRect m_tempPixmapRect;
    enum SelectionBorderType m_selectionBorder;
    enum TempPixmapType m_tempPixmapType;
};

#endif  // __kpviewmanager_h__
