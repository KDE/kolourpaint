
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

// TODO: maybe merge with kpSelection?


#ifndef __kp_temp_pixmap_h__
#define __kp_temp_pixmap_h__


#include <qpoint.h>
#include <qpixmap.h>

class kpViewManager;


class kpTempPixmap
{
public:
    enum RenderMode
    {
        SetPixmap,
        PaintPixmap,
        PaintMaskTransparentWithBrush
    };

    /*
     * <isBrush>    Specifies that its visibility is dependent on whether or
     *              not the mouse cursor is inside a view.  If false, the
     *              pixmap is always displayed.
     */
    kpTempPixmap (bool isBrush, RenderMode renderMode, const QPoint &topLeft, const QPixmap &pixmap);
    kpTempPixmap (const kpTempPixmap &rhs);
    kpTempPixmap &operator= (const kpTempPixmap &rhs);
    ~kpTempPixmap ();

    bool isBrush () const;
    RenderMode renderMode () const;
    QPoint topLeft () const;
    QPixmap pixmap () const;

    bool isVisible (const kpViewManager *vm) const;
    QRect rect () const;
    int width () const;
    int height () const;


    // Returns whether a call to paint() may change <*destPixmap>'s mask
    bool mayChangeDocumentMask () const;

    /*
     * Draws itself onto <*destPixmap>, given that <*destPixmap> represents
     * the unzoomed <docRect> of the kpDocument.  You should check for
     * visibility before calling this function.
     */
    void paint (QPixmap *destPixmap, const QRect &docRect) const;

private:
    bool m_isBrush;
    RenderMode m_renderMode;
    QPoint m_topLeft;
    QPixmap m_pixmap;
};


#endif  // __kp_temp_pixmap_h__
