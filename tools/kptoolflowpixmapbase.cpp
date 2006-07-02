
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#include <kptoolflowpixmapbase.h>

#include <qbitmap.h>

#include <kpcolor.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kptoolflowcommand.h>


kpToolFlowPixmapBase::kpToolFlowPixmapBase (const QString &text, const QString &description,
            int key,
            kpMainWindow *mainWindow, const char *name)
    : kpToolFlowBase (text, description, key, mainWindow, name)
{
}

kpToolFlowPixmapBase::~kpToolFlowPixmapBase ()
{
}


void kpToolFlowPixmapBase::drawPoint (const QPoint & /*point*/)
{            
    if (color (m_mouseButton).isOpaque ())
        document ()->paintPixmapAt (m_brushPixmap [m_mouseButton], hotPoint ());
    else
    {
        kpPixmapFX::paintMaskTransparentWithBrush (document ()->pixmap (),
            hotPoint (),
            kpPixmapFX::getNonNullMask (m_brushPixmap [m_mouseButton]));
        document ()->slotContentsChanged (hotRect ());
    }

    m_currentCommand->updateBoundingRect (hotRect ());
}



bool kpToolFlowPixmapBase::drawLine (QPixmap *pixmap,
    const QRect &docRect,
    const QPoint &thisPoint, const QPoint &lastPoint)
{
    QValueVector <QPoint> points = interpolatePoints (docRect,
        thisPoint, lastPoint);
        
    for (QValueVector <QPoint>::const_iterator pit = points.begin ();
         pit != points.end ();
         pit++)
    {
        const int XXX = (*pit).x (), YYY = (*pit).y ();
        
        if (!color (m_mouseButton).isTransparent ())
        {
            kpPixmapFX::paintPixmapAt (pixmap,
                hotPoint (XXX, YYY),
                m_brushPixmap [m_mouseButton]);
        }
        else
        {
            kpPixmapFX::paintMaskTransparentWithBrush (pixmap,
                hotPoint (XXX, YYY),
                kpPixmapFX::getNonNullMask (m_brushPixmap [m_mouseButton]));
        }
    }

    return true;
}

    
#include <kptoolflowpixmapbase.moc>
