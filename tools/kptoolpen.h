
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

#ifndef __kptoolpen_h__
#define __kptoolpen_h__

#include <qrect.h>
#include <kcommand.h>
#include <kptool.h>

class QPoint;
class kpMainWindow;
class kpToolPenCommand;
class kpToolWidgetBrush;
class kpToolWidgetEraserSize;
class kpViewManager;

class kpToolPen : public kpTool
{
Q_OBJECT

public:
    kpToolPen (kpMainWindow *mainWindow);
    virtual ~kpToolPen ();

    enum Mode
    {
        // tool properties
        DrawsPixels = (1 << 0), DrawsPixmaps = (1 << 1), WashesPixmaps = (1 << 2),
        NoBrushes = 0, SquareBrushes = (1 << 3), DiverseBrushes = (1 << 4),
        NormalColors = 0, SwappedColors = (1 << 5),

        // tools:
        //
        // Pen = draws pixels, "interpolates" by "sweeping" pixels along a line (no brushes)
        // Brush = draws pixmaps, "interpolates" by "sweeping" pixmaps along a line (interesting brushes)
        // Eraser = Brush but with foreground & background colors swapped (a few square brushes)
        // Color Washer = Brush that replaces/washes the background color with the foreground color
        //
        // (note the capitalization of "brush" here :))
        Pen = DrawsPixels | NoBrushes | NormalColors,
        Brush = DrawsPixmaps | DiverseBrushes | NormalColors,
        Eraser = DrawsPixmaps | SquareBrushes | SwappedColors,
        ColorWasher = WashesPixmaps | SquareBrushes | NormalColors
    };

    void setMode (Mode mode);

    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void hover (const QPoint &point);
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint, const QRect &);
    virtual void cancelDraw ();
    virtual void endDraw (const QPoint &, const QRect &);

private slots:
    virtual void slotForegroundColorChanged (const QColor &col);
    virtual void slotBackgroundColorChanged (const QColor &col);

    void slotBrushChanged (const QPixmap &pixmap, bool isDiagonalLine);
    void slotEraserSizeChanged (int size);

private:
    void wash (QImage *image, const QRect &imageRect, int plotx, int ploty);
    void wash (QImage *image, const QRect &imageRect, const QRect &drawRect);

    QColor color (int which);

    QPoint hotPoint () const;
    QPoint hotPoint (int x, int y) const;
    QPoint hotPoint (const QPoint &point) const;
    QRect hotRect () const;
    QRect hotRect (int x, int y) const;
    QRect hotRect (const QPoint &point) const;

    Mode m_mode;

    void updateBrushCursor (bool recalc = true);

    kpToolWidgetBrush *m_toolWidgetBrush;
    kpToolWidgetEraserSize *m_toolWidgetEraserSize;
    QPixmap m_brushPixmap [2];
    QPixmap m_cursorPixmap;
    bool m_brushIsDiagonalLine;

    kpToolPenCommand *m_currentCommand;
};

class kpToolPenCommand : public KCommand
{
public:
    kpToolPenCommand (const QString &name, kpDocument *document, kpViewManager *viewManager);
    virtual QString name () const { return m_name; }
    virtual ~kpToolPenCommand ();

    virtual void execute ();
    virtual void unexecute ();

    // interface for KToolPen
    void updateBoundingRect (const QPoint &point);
    void updateBoundingRect (const QRect &rect);
    void finalize ();
    void cancel ();

private:
    void swapOldAndNew ();

    QString m_name;
    kpDocument *m_document;
    kpViewManager *m_viewManager;

    QPixmap m_pixmap;
    QRect m_boundingRect;
};

#endif  // __kptoolpen_h__
