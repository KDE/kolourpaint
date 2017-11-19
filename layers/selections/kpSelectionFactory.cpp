
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


#define DEBUG_KP_SELECTION 0


#include "kpSelectionFactory.h"

#include <QDataStream>

#include "layers/selections/image/kpRectangularImageSelection.h"
#include "layers/selections/image/kpEllipticalImageSelection.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"

//---------------------------------------------------------------------

// public static
// TODO: KolourPaint has not been tested against invalid or malicious
//       clipboard data [Bug #28].
kpAbstractImageSelection *kpSelectionFactory::FromStream (QDataStream &stream)
{
#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "kpSelectionFactory::FromStream()";
#endif
    int serialID;
    stream >> serialID;

#if DEBUG_KP_SELECTION && 1
    qCDebug(kpLogLayers) << "\tserialID=" << serialID;
#endif

    // Only image selections are marshalled.
    //
    // Text selections are only ever seen in the clipboard as ordinary text,
    // not selections, since copying text formatting over the clipboard doesn't
    // seem compelling.
    kpAbstractImageSelection *imageSel = nullptr;
    switch (serialID)
    {
    case kpRectangularImageSelection::SerialID:
        imageSel = new kpRectangularImageSelection ();
        break;

    case kpEllipticalImageSelection::SerialID:
        imageSel = new kpEllipticalImageSelection ();
        break;

    case kpFreeFormImageSelection::SerialID:
        imageSel = new kpFreeFormImageSelection ();
        break;
    }

    // Unknown selection type?
    if (imageSel == nullptr)
    {
        return nullptr;
    }

    if (!imageSel->readFromStream (stream))
    {
        delete imageSel;
        return nullptr;
    }

    return imageSel;
}

//---------------------------------------------------------------------
