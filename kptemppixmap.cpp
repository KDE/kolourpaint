
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


#define DEBUG_KP_TEMP_PIXMAP 0


#include <kptemppixmap.h>

#include <kppixmapfx.h>
#include <kpviewmanager.h>


kpTempPixmap::kpTempPixmap (bool isBrush, RenderMode renderMode,
                            const QPoint &topLeft, const QPixmap &pixmap)
    : m_isBrush (isBrush),
      m_renderMode (renderMode),
      m_topLeft (topLeft),
      m_pixmap (pixmap)
{
}

kpTempPixmap::kpTempPixmap (const kpTempPixmap &rhs)
    : m_isBrush (rhs.m_isBrush),
      m_renderMode (rhs.m_renderMode),
      m_topLeft (rhs.m_topLeft),
      m_pixmap (rhs.m_pixmap)
{
}

kpTempPixmap &kpTempPixmap::operator= (const kpTempPixmap &rhs)
{
    if (this == &rhs)
        return *this;

    m_isBrush = rhs.m_isBrush;
    m_renderMode = rhs.m_renderMode;
    m_topLeft = rhs.m_topLeft;
    m_pixmap = rhs.m_pixmap;

    return *this;
}

kpTempPixmap::~kpTempPixmap ()
{
}


// public
bool kpTempPixmap::isBrush () const
{
    return m_isBrush;
}

// public
kpTempPixmap::RenderMode kpTempPixmap::renderMode () const
{
    return m_renderMode;
}

// public
QPoint kpTempPixmap::topLeft () const
{
    return m_topLeft;
}

// public
QPixmap kpTempPixmap::pixmap () const
{
    return m_pixmap;
}


// public
bool kpTempPixmap::isVisible (const kpViewManager *vm) const
{
    return m_isBrush ? (bool) vm->viewUnderCursor () : true;
}

// public
QRect kpTempPixmap::rect () const
{
    return QRect (m_topLeft.x (), m_topLeft.y (),
                  m_pixmap.width (), m_pixmap.height ());
}

// public
int kpTempPixmap::width () const
{
    return m_pixmap.width ();
}

// public
int kpTempPixmap::height () const
{
    return m_pixmap.height ();
}


// public
bool kpTempPixmap::mayChangeDocumentMask () const
{
    return (m_renderMode == SetPixmap ||
            m_renderMode == PaintMaskTransparentWithBrush);
}

// public
void kpTempPixmap::paint (QPixmap *destPixmap, const QRect &docRect) const
{
#define PARAMS destPixmap, m_topLeft - docRect.topLeft (), m_pixmap
    switch (m_renderMode)
    {
    case SetPixmap:
        kpPixmapFX::setPixmapAt (PARAMS);
        break;
    case PaintPixmap:
        kpPixmapFX::paintPixmapAt (PARAMS);
        break;
    case PaintMaskTransparentWithBrush:
        kpPixmapFX::paintMaskTransparentWithBrush (PARAMS);
        break;
    }
#undef PARAMS
}
