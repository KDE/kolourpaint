
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

#ifndef __kptoolpolygon_h__
#define __kptoolpolygon_h__

#include <qbrush.h>
#include <qpen.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>

#include <kcommand.h>

#include <kptool.h>
#include <kptoolwidgetfillstyle.h>

class QMouseEvent;
class QPen;
class QPoint;
class QRect;

class kpView;
class kpDocument;
class kpMainWindow;

class kpToolWidgetFillStyle;
class kpToolWidgetLineStyle;
class kpToolWidgetLineWidth;
class kpViewManager;

class kpToolPolygon : public kpTool
{
Q_OBJECT

public:
    kpToolPolygon (kpMainWindow *);
    virtual ~kpToolPolygon ();

    enum Mode
    {
        Polygon, Polyline, Line, Curve
    };
    
    void setMode (Mode mode);
    
    virtual bool careAboutModifierState () const { return true; }

    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void draw (const QPoint &, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void endDraw (const QPoint &, const QRect &);
    virtual void endShape (const QPoint & = QPoint (), const QRect & = QRect ());

    virtual bool hasBegunShape () const;

public slots:
    void slotLineStyleChanged (Qt::PenStyle lineStyle);
    void slotLineWidthChanged (int width);
    void slotFillStyleChanged (kpToolWidgetFillStyle::FillStyle fillStyle);
    
protected slots:
    virtual void slotForegroundColorChanged (const QColor &);
    virtual void slotBackgroundColorChanged (const QColor &);

private slots:
    void updateShape ();
    
private:
    Mode m_mode;

    kpToolWidgetFillStyle *m_toolWidgetFillStyle;
    
    Qt::PenStyle m_lineStyle;
    kpToolWidgetLineStyle *m_toolWidgetLineStyle;

    int m_lineWidth;
    kpToolWidgetLineWidth *m_toolWidgetLineWidth;

    int m_originatingMouseButton;

    void applyModifiers ();

    QPoint m_toolLineStartPoint, m_toolLineEndPoint;
    QRect m_toolLineRect;
    
    QPointArray m_points;
};

class kpToolPolygonCommand : public KCommand
{
public:
    kpToolPolygonCommand (kpViewManager *m_viewManager, kpDocument *m_document,
                          const QString &toolText,
                          const QPointArray &points,
                          const QRect &normalizedRect,
                          const QColor &foregroundColor, const QColor &backgroundColor,
                          int lineWidth, Qt::PenStyle lineStyle,
                          kpToolWidgetFillStyle *toolWidgetFillStyle,
                          const QPixmap &originalArea,
                          kpToolPolygon::Mode mode);
    virtual ~kpToolPolygonCommand ();

    virtual void execute ();
    virtual void unexecute ();

    virtual QString name () const;

private:
    kpViewManager *m_viewManager;
    kpDocument *m_document;
    
    QString m_name;

    QPointArray m_points;
    QRect m_normalizedRect;
    
    QColor m_foregroundColor, m_backgroundColor;
    int m_lineWidth;
    Qt::PenStyle m_lineStyle;
    kpToolWidgetFillStyle *m_toolWidgetFillStyle;
    
    QPixmap m_originalArea;
    kpToolPolygon::Mode m_mode;
};

#endif  // __kptoolpolygon_h__
