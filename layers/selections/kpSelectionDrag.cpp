/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2011 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_SELECTION_DRAG 0

#include "kpSelectionDrag.h"

#include <QDataStream>
#include <QIODevice>
#include <QImage>
#include <QUrl>

#include "kpLogCategories.h"

#include "kpSelectionFactory.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/image/kpRectangularImageSelection.h"

//---------------------------------------------------------------------

// public static
const char *const kpSelectionDrag::SelectionMimeType = "application/x-kolourpaint-selection-400";

//---------------------------------------------------------------------

kpSelectionDrag::kpSelectionDrag(const kpAbstractImageSelection &sel)
{
#if DEBUG_KP_SELECTION_DRAG && 1
    qCDebug(kpLogLayers) << "kpSelectionDrag() w=" << sel.width() << " h=" << sel.height();
#endif

    Q_ASSERT(sel.hasContent());

    // Store as selection.
    QByteArray ba;
    {
        QDataStream stream(&ba, QIODevice::WriteOnly);
        stream << sel;
    }
    setData(QLatin1String(kpSelectionDrag::SelectionMimeType), ba);

    // Store as image (so that QMimeData::hasImage()) works).
    // OPT: an awful waste of memory storing image in both selection and QImage
    const QImage image = sel.baseImage();
#if DEBUG_KP_SELECTION_DRAG && 1
    qCDebug(kpLogLayers) << "\timage: w=" << image.width() << " h=" << image.height();
#endif
    if (image.isNull()) {
        // TODO: proper error handling.
        qCCritical(kpLogLayers) << "kpSelectionDrag::setSelection() could not convert to image";
    } else {
        setImageData(image);
    }
}

//---------------------------------------------------------------------
// public static

bool kpSelectionDrag::canDecode(const QMimeData *mimeData)
{
    Q_ASSERT(mimeData);

#if DEBUG_KP_SELECTION_DRAG
    qCDebug(kpLogLayers) << "kpSelectionDrag::canDecode()"
                         << "hasSel=" << mimeData->hasFormat(QLatin1String(kpSelectionDrag::SelectionMimeType)) << "hasImage=" << mimeData->hasImage();
#endif

    // mimeData->hasImage() would not check if the data is a valid image
    return mimeData->hasFormat(QLatin1String(kpSelectionDrag::SelectionMimeType)) || !qvariant_cast<QImage>(mimeData->imageData()).isNull();
}

//---------------------------------------------------------------------
// public static

kpAbstractImageSelection *kpSelectionDrag::decode(const QMimeData *mimeData)
{
#if DEBUG_KP_SELECTION_DRAG
    qCDebug(kpLogLayers) << "kpSelectionDrag::decode(kpAbstractSelection)";
#endif
    Q_ASSERT(mimeData);

    if (mimeData->hasFormat(QLatin1String(kpSelectionDrag::SelectionMimeType))) {
#if DEBUG_KP_SELECTION_DRAG
        qCDebug(kpLogLayers) << "\tmimeSource hasFormat selection - just return it in QByteArray";
#endif
        QByteArray data = mimeData->data(QLatin1String(kpSelectionDrag::SelectionMimeType));
        QDataStream stream(&data, QIODevice::ReadOnly);

        return kpSelectionFactory::FromStream(stream);
    }

#if DEBUG_KP_SELECTION_DRAG
    qCDebug(kpLogLayers) << "\tmimeSource doesn't provide selection - try image";
#endif

    QImage image = qvariant_cast<QImage>(mimeData->imageData());
    if (!image.isNull()) {
#if DEBUG_KP_SELECTION_DRAG
        qCDebug(kpLogLayers) << "\tok w=" << image.width() << " h=" << image.height();
#endif

        return new kpRectangularImageSelection(QRect(0, 0, image.width(), image.height()), image);
    }

    if (mimeData->hasUrls()) // no image, check for path to local image file
    {
        QList<QUrl> urls = mimeData->urls();

        if (urls.count() && urls[0].isLocalFile()) {
            image.load(urls[0].toLocalFile());

            if (!image.isNull()) {
                return new kpRectangularImageSelection(QRect(0, 0, image.width(), image.height()), image);
            }
        }
    }

#if DEBUG_KP_SELECTION_DRAG
    qCDebug(kpLogLayers) << "kpSelectionDrag::decode(kpAbstractSelection) mimeSource had no sel "
                            "and could not decode to image";
#endif
    return nullptr;
}

//---------------------------------------------------------------------

#include "moc_kpSelectionDrag.cpp"
