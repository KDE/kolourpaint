
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

#ifndef __kptoolline_h__
#define __kptoolline_h__

#include <qpen.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>

#include <kcommand.h>

#include <kptool.h>

class QMouseEvent;
class QPen;
class QPoint;
class QRect;

class kpView;
class kpDocument;
class kpMainWindow;

class kpToolWidgetLineStyle;
class kpToolWidgetLineWidth;
class kpViewManager;

class kpToolLine : public kpTool
{
Q_OBJECT

public:
    kpToolLine (kpMainWindow *);
    virtual ~kpToolLine ();

    virtual bool careAboutModifierState () const { return true; }

    virtual void begin ();
    virtual void end ();

    virtual void beginDraw ();
    virtual void draw (const QPoint &, const QPoint &, const QRect &);
    virtual void cancelDraw ();
    virtual void endDraw (const QPoint &, const QRect &);

public slots:
    void slotLineStyleChanged (Qt::PenStyle lineStyle);
    void slotLineWidthChanged (int width);

private:
    Qt::PenStyle m_lineStyle;
    kpToolWidgetLineStyle *m_toolWidgetLineStyle;

    int m_lineWidth;
    kpToolWidgetLineWidth *m_toolWidgetLineWidth;

    QPen pen () const;

    void applyModifiers ();

    QPoint m_toolLineStartPoint, m_toolLineEndPoint;
    QRect m_toolLineRect;
};

class kpToolLineCommand : public KCommand
{
public:
    kpToolLineCommand (kpViewManager *m_viewManager, kpDocument *m_document,
                        const QPoint &startPoint, const QPoint &m_endPoint,
                        const QRect &normalizedRect,
                        const QPen &pen,
                        const QPixmap &originalArea);
    virtual ~kpToolLineCommand ();

    virtual void execute ();
    virtual void unexecute ();

    virtual QString name () const;

private:
    kpViewManager *m_viewManager;
    kpDocument *m_document;

    QPoint m_startPoint, m_endPoint;
    QRect m_normalizedRect;
    QPen m_pen;

    QPixmap m_originalArea;
};

#endif  // __kptoolline_h__
