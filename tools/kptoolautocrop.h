
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


#ifndef __kptoolautocrop_h__
#define __kptoolautocrop_h__

#include <qrect.h>

#include <kpcommandhistory.h>

#include <kpcolor.h>
#include <kpselection.h>

class QPixmap;
class kpDocument;
class kpMainWindow;
class kpViewManager;


// (returns true on success (even if it did nothing) or false on error)
bool kpToolAutoCrop (kpMainWindow *mainWindow);


class kpToolAutoCropBorder
{
public:
    kpToolAutoCropBorder (const QPixmap *pixmapPtr, int processedColorSimilarity);

    int size () const;

    const QPixmap *pixmap () const;
    int processedColorSimilarity () const;
    QRect rect () const;
    int left () const;
    int right () const;
    int top () const;
    int bottom () const;
    kpColor referenceColor () const;
    kpColor averageColor () const;
    bool isSingleColor () const;

    // (returns true on success (even if no rect) or false on error)
    bool calculate (int isX, int dir);

    bool fillsEntirePixmap () const;
    bool exists () const;
    void invalidate ();

private:
    const QPixmap *m_pixmapPtr;
    int m_processedColorSimilarity;

    QRect m_rect;
    kpColor m_referenceColor;
    int m_redSum, m_greenSum, m_blueSum;
    bool m_isSingleColor;
};


class kpToolAutoCropCommand : public kpNamedCommand
{
public:
    kpToolAutoCropCommand (bool actOnSelection,
                           const kpToolAutoCropBorder &leftBorder,
                           const kpToolAutoCropBorder &rightBorder,
                           const kpToolAutoCropBorder &topBorder,
                           const kpToolAutoCropBorder &botBorder,
                           kpMainWindow *mainWindow);
    virtual ~kpToolAutoCropCommand ();

    enum NameOptions
    {
        DontShowAccel = 0,
        ShowAccel = 1
    };

    static QString name (bool actOnSelection, int options);

    virtual int size () const;

private:
    void getUndoPixmap (const kpToolAutoCropBorder &border, QPixmap **pixmap);
    void getUndoPixmaps ();
    void deleteUndoPixmaps ();

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    QRect contentsRect () const;

    bool m_actOnSelection;
    kpToolAutoCropBorder m_leftBorder, m_rightBorder, m_topBorder, m_botBorder;
    QPixmap *m_leftPixmap, *m_rightPixmap, *m_topPixmap, *m_botPixmap;

    QRect m_contentsRect;
    int m_oldWidth, m_oldHeight;
    kpSelection m_oldSelection;
};

#endif  // __kptoolautocrop_h__
