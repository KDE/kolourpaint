
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

#ifndef __kptoolselection_h__
#define __kptoolselection_h__

#include <kptool.h>

class QPoint;
class QRect;
class kpMainWindow;

class kpToolSelection : public kpTool
{
Q_OBJECT

public:
    kpToolSelection (kpMainWindow *mainWindow);
    virtual ~kpToolSelection ();

    enum Mode {Rectangle, Ellipse, FreeForm};
    void setMode (Mode mode) { m_mode = mode; }

    virtual void begin ();
    virtual void end ();

    virtual bool careAboutModifierState () const { return true; }

    virtual void beginDraw ();
    virtual void hover (const QPoint &point);
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                        const QRect &normalizedRect);
    virtual void cancelDraw ();
    virtual void endDraw (const QPoint &thisPoint, const QRect &normalizedRect);

private:
    void sendSelectionToViewManager (const QPixmap &pixmap, const QRect &rect, bool showBorder = true);
    void sendSelectionToViewManager (const QPixmap &pixmap, const QPoint &topLeft, bool showBorder = true);

    Mode m_mode;

    QPoint m_startDragFromSelectionTopLeft;
    int m_dragMoving;
};

#endif  // __kptoolselection_h__
