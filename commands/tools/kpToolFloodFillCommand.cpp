
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


#define DEBUG_KP_TOOL_FLOOD_FILL 0


#include <kpToolFloodFillCommand.h>

#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpimage.h>
#include <kppixmapfx.h>


struct kpToolFloodFillCommandPrivate
{
    kpImage oldImage;
    bool fillEntirePixmap;
};

kpToolFloodFillCommand::kpToolFloodFillCommand (int x, int y,
        const kpColor &color, int processedColorSimilarity,
        kpMainWindow *mainWindow)
        
    : kpCommand (mainWindow),
      kpFloodFill (document ()->pixmap (), x, y, color, processedColorSimilarity),
      d (new kpToolFloodFillCommandPrivate ())
{
    d->fillEntirePixmap = false;
}

kpToolFloodFillCommand::~kpToolFloodFillCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
QString kpToolFloodFillCommand::name () const
{
    return i18n ("Flood Fill");
}

// public virtual [base kpCommand]
int kpToolFloodFillCommand::size () const
{
    return kpFloodFill::size () + kpPixmapFX::pixmapSize (d->oldImage);
}


// public
void kpToolFloodFillCommand::setFillEntirePixmap (bool yes)
{
    d->fillEntirePixmap = yes;
}


// protected virtual [base kpCommand]
void kpToolFloodFillCommand::execute ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    kDebug () << "kpToolFloodFillCommand::execute() fillEntirePixmap="
              << d->fillEntirePixmap << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    if (d->fillEntirePixmap)
    {
        doc->fill (kpFloodFill::color ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            QApplication::setOverrideCursor (Qt::WaitCursor);
            {
                d->oldImage = doc->getPixmapAt (rect);

                kpFloodFill::fill ();
                doc->slotContentsChanged (rect);
            }
            QApplication::restoreOverrideCursor ();
        }
        else
        {
        #if DEBUG_KP_TOOL_FLOOD_FILL && 1
            kDebug () << "\tinvalid boundingRect - must be NOP case" << endl;
        #endif
        }
    }
}

// protected virtual [base kpCommand]
void kpToolFloodFillCommand::unexecute ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    kDebug () << "kpToolFloodFillCommand::unexecute() fillEntirePixmap="
              << d->fillEntirePixmap << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    if (d->fillEntirePixmap)
    {
        doc->fill (kpFloodFill::colorToChange ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            doc->setPixmapAt (d->oldImage, rect.topLeft ());

            d->oldImage = kpImage ();

            doc->slotContentsChanged (rect);
        }
    }
}


#include <kpToolFloodFillCommand.moc>
