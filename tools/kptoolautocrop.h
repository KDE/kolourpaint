
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

#ifndef __kptoolautocrop_h__
#define __kptoolautocrop_h__

#include <qcolor.h>
#include <qrect.h>
#include <kcommand.h>

class QPixmap;
class kpDocument;
class kpMainWindow;
class kpViewManager;


// (returns true on success (even if it did nothing) or false on error)
bool kpToolAutoCrop (kpMainWindow *mainWindow);


struct kpToolAutoCropBorder
{
    kpToolAutoCropBorder (const QPixmap *pixmapPtr);

    // (returns true on success (even if no rect) or false on error)
    bool calculate (int isX, int dir);

    bool fillsEntirePixmap () const;
    bool exists () const;
    void invalidate ();

    const QPixmap *m_pixmapPtr;
    QRect m_rect;
    QColor m_color;
};


class kpToolAutoCropCommand : public KCommand
{
public:
    kpToolAutoCropCommand (bool actOnSelection,
                           const kpToolAutoCropBorder &leftBorder,
                           const kpToolAutoCropBorder &rightBorder,
                           const kpToolAutoCropBorder &topBorder,
                           const kpToolAutoCropBorder &botBorder,
                           kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolAutoCropCommand ();

private:
    kpDocument *document () const;
    kpViewManager *viewManager () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    QRect contentsRect () const;

    bool m_actOnSelection;
    kpToolAutoCropBorder m_leftBorder;
    kpToolAutoCropBorder m_rightBorder;
    kpToolAutoCropBorder m_topBorder;
    kpToolAutoCropBorder m_botBorder;
    kpMainWindow *m_mainWindow;

    QRect m_contentsRect;
    int m_oldWidth, m_oldHeight;
};

#endif  // __kptoolautocrop_h__
