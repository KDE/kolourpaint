
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#ifndef KP_TOOL_SPRAYCAN_H
#define KP_TOOL_SPRAYCAN_H


#include "kpToolFlowBase.h"


class QPoint;
class QRect;
class QString;
class QTimer;

class kpToolWidgetSpraycanSize;


class kpToolSpraycan : public kpToolFlowBase
{
Q_OBJECT

public:
    kpToolSpraycan (kpToolEnvironment *environ, QObject *parent);

protected:
    QString haventBegunDrawUserMessage () const override;


public:
    void begin () override;
    void end () override;


public:
    void beginDraw () override;
protected:
    // (ASSUMPTION: <probability> is between 0.0 and 1.0 inclusive)
    QRect drawLineWithProbability (const QPoint &thisPoint,
         const QPoint &lastPoint,
         double probability);
public:
    QRect drawPoint (const QPoint &point) override;
    QRect drawLine (const QPoint &thisPoint, const QPoint &lastPoint) override;
    void cancelShape () override;
    void endDraw (const QPoint &thisPoint,
        const QRect &normalizedRect) override;

protected slots:
    void timeoutDraw ();


protected:
    int spraycanSize () const;
protected slots:
    void slotSpraycanSizeChanged (int size);


protected:
    QTimer *m_timer;
    kpToolWidgetSpraycanSize *m_toolWidgetSpraycanSize;
};


#endif  // KP_TOOL_SPRAYCAN_H
