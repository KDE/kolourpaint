
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_SELECTION 0

#include "kpSelectionFactory.h"

#include <QDataStream>

#include "layers/selections/image/kpEllipticalImageSelection.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"
#include "layers/selections/image/kpRectangularImageSelection.h"

//---------------------------------------------------------------------

// public static
// TODO: KolourPaint has not been tested against invalid or malicious
//       clipboard data [Bug #28].
kpAbstractImageSelection *kpSelectionFactory::FromStream(QDataStream &stream)
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
    switch (serialID) {
    case kpRectangularImageSelection::SerialID:
        imageSel = new kpRectangularImageSelection();
        break;

    case kpEllipticalImageSelection::SerialID:
        imageSel = new kpEllipticalImageSelection();
        break;

    case kpFreeFormImageSelection::SerialID:
        imageSel = new kpFreeFormImageSelection();
        break;
    }

    // Unknown selection type?
    if (imageSel == nullptr) {
        return nullptr;
    }

    if (!imageSel->readFromStream(stream)) {
        delete imageSel;
        return nullptr;
    }

    return imageSel;
}

//---------------------------------------------------------------------
