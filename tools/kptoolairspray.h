
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

#ifndef __kptoolairspray_h__
#define __kptoolairspray_h__

#include <qpen.h>
#include <kcommand.h>
#include <kptool.h>

class QPixmap;
class QPoint;
class QRect;
class QTimer;

class kpMainWindow;
class kpToolAirSprayCommand;
class kpToolWidgetSpraycanSize;
class kpViewManager;

class kpToolAirSpray : public kpTool
{
Q_OBJECT

public:
    kpToolAirSpray (kpMainWindow *);
    virtual ~kpToolAirSpray ();

    virtual void begin ();
    virtual void end ();

private slots:
    void slotSpraycanSizeChanged (int size);
    
public:
    virtual void beginDraw ();
    virtual void draw (const QPoint &thisPoint, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void endDraw (const QPoint &, const QRect &);

public slots:
    void actuallyDraw ();

private:
    kpToolWidgetSpraycanSize *m_toolWidgetSpraycanSize;
    kpToolAirSprayCommand *m_currentCommand;
    QTimer *m_timer;
    int m_size;
};

class kpToolAirSprayCommand : public KCommand
{
public:
    kpToolAirSprayCommand (const QPen &pen, int size,
                           kpDocument *document, kpViewManager *viewManager);
    virtual ~kpToolAirSprayCommand ();

    virtual void execute ();
    virtual void unexecute ();

    virtual QString name () const;

    // interface for KToolAirSpray
    void addPoints (const QPointArray &points);
    void finalize ();
    void cancel ();

private:
    QPen m_pen;
    int m_size;

    kpDocument *m_document;
    kpViewManager *m_viewManager;

    QPixmap *m_newPixmapPtr;
    QPixmap m_oldPixmap;
    QRect m_boundingRect;
};

#endif  // __kptoolairspray_h__
