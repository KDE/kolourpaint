
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#ifndef __kptoolpolygon_h__
#define __kptoolpolygon_h__

#include <qbrush.h>
#include <qpen.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>

#include <kpcommandhistory.h>

#include <kpcolor.h>
#include <kptool.h>
#include <kptoolwidgetfillstyle.h>

class QMouseEvent;
class QPen;
class QPoint;
class QRect;
class QString;

class kpView;
class kpDocument;
class kpMainWindow;

class kpToolWidgetFillStyle;
class kpToolWidgetLineWidth;
class kpViewManager;

class kpToolPolygon : public kpTool
{
Q_OBJECT

public:
    enum Mode
    {
        Polygon, Polyline, Line, Curve
    };

    kpToolPolygon (Mode mode, const QString &text, const QString &description,
                   int key,
                   kpMainWindow *mainWindow, const char *name);
    kpToolPolygon (kpMainWindow *mainWindow);
    virtual ~kpToolPolygon ();

    void setMode (Mode mode);

    virtual bool careAboutModifierState () const { return true; }

private:
    QString haventBegunShapeUserMessage () const;

public:
    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void draw (const QPoint &, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &, const QRect &);
    virtual void endShape (const QPoint & = QPoint (), const QRect & = QRect ());

    virtual bool hasBegunShape () const;

public slots:
    void slotLineWidthChanged (int width);
    void slotFillStyleChanged (kpToolWidgetFillStyle::FillStyle fillStyle);

protected slots:
    virtual void slotForegroundColorChanged (const kpColor &);
    virtual void slotBackgroundColorChanged (const kpColor &);

private slots:
    void updateShape ();

private:
    Mode m_mode;

    kpToolWidgetFillStyle *m_toolWidgetFillStyle;

    int m_lineWidth;
    kpToolWidgetLineWidth *m_toolWidgetLineWidth;

    int m_originatingMouseButton;

    void applyModifiers ();

    QPoint m_toolLineStartPoint, m_toolLineEndPoint;
    QRect m_toolLineRect;

    QPointArray m_points;
};

class kpToolPolygonCommand : public kpNamedCommand
{
public:
    kpToolPolygonCommand (const QString &name,
                          const QPointArray &points,
                          const QRect &normalizedRect,
                          const kpColor &foregroundColor, const kpColor &backgroundColor,
                          int lineWidth, Qt::PenStyle lineStyle,
                          kpToolWidgetFillStyle *toolWidgetFillStyle,
                          const QPixmap &originalArea,
                          kpToolPolygon::Mode mode,
                          kpMainWindow *mainWindow);
    virtual ~kpToolPolygonCommand ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

private:
    QPointArray m_points;
    QRect m_normalizedRect;

    kpColor m_foregroundColor, m_backgroundColor;
    int m_lineWidth;
    Qt::PenStyle m_lineStyle;
    kpToolWidgetFillStyle *m_toolWidgetFillStyle;

    QPixmap m_originalArea;
    kpToolPolygon::Mode m_mode;
};

#endif  // __kptoolpolygon_h__
