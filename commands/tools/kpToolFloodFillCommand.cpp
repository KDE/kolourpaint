
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


#define DEBUG_KP_TOOL_FLOOD_FILL 0


#include "kpToolFloodFillCommand.h"

#include "imagelib/kpColor.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "imagelib/kpImage.h"
#include "kpLogCategories.h"

#include <QApplication>

#include <KLocalizedString>

//---------------------------------------------------------------------

struct kpToolFloodFillCommandPrivate
{
    kpImage oldImage;
    bool fillEntireImage{false};
};

//---------------------------------------------------------------------

kpToolFloodFillCommand::kpToolFloodFillCommand (int x, int y,
        const kpColor &color, int processedColorSimilarity,
        kpCommandEnvironment *environ)

    : kpCommand (environ),
      kpFloodFill (document ()->imagePointer (), x, y, color, processedColorSimilarity),
      d (new kpToolFloodFillCommandPrivate ())
{
    d->fillEntireImage = false;
}

//---------------------------------------------------------------------

kpToolFloodFillCommand::~kpToolFloodFillCommand ()
{
    delete d;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
QString kpToolFloodFillCommand::name () const
{
    return i18n ("Flood Fill");
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolFloodFillCommand::size () const
{
    return kpFloodFill::size () + ImageSize (d->oldImage);
}

//---------------------------------------------------------------------

// public
void kpToolFloodFillCommand::setFillEntireImage (bool yes)
{
    d->fillEntireImage = yes;
}

//---------------------------------------------------------------------

// protected virtual [base kpCommand]
void kpToolFloodFillCommand::execute ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    qCDebug(kpLogCommands) << "kpToolFloodFillCommand::execute() fillEntireImage="
              << d->fillEntireImage;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    if (d->fillEntireImage)
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
                d->oldImage = doc->getImageAt (rect);

                kpFloodFill::fill ();
                doc->slotContentsChanged (rect);
            }
            QApplication::restoreOverrideCursor ();
        }
        else
        {
        #if DEBUG_KP_TOOL_FLOOD_FILL && 1
            qCDebug(kpLogCommands) << "\tinvalid boundingRect - must be NOP case";
        #endif
        }
    }
}

//---------------------------------------------------------------------

// protected virtual [base kpCommand]
void kpToolFloodFillCommand::unexecute ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    qCDebug(kpLogCommands) << "kpToolFloodFillCommand::unexecute() fillEntireImage="
              << d->fillEntireImage;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    if (d->fillEntireImage)
    {
        doc->fill (kpFloodFill::colorToChange ());
    }
    else
    {
        QRect rect = kpFloodFill::boundingRect ();
        if (rect.isValid ())
        {
            doc->setImageAt (d->oldImage, rect.topLeft ());

            d->oldImage = kpImage ();

            doc->slotContentsChanged (rect);
        }
    }
}

//---------------------------------------------------------------------
