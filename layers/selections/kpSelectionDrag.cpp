
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


#define DEBUG_KP_SELECTION_DRAG 0


#include <kpSelectionDrag.h>

#include <qdatastream.h>
#include <qpixmap.h>

#include <kdebug.h>

#include <kpPixmapFX.h>
#include <kpAbstractSelection.h>
#include <kpAbstractImageSelection.h>
#include <kpRectangularImageSelection.h>
#include <kpSelectionFactory.h>


// public static
const char * const kpSelectionDrag::SelectionMimeType =
    "application/x-kolourpaint-selection-400";


struct kpSelectionDragPrivate
{
    kpSelectionDragPrivate ()
    {
    }
};

kpSelectionDrag::kpSelectionDrag ()
    : QMimeData (),
      d (new kpSelectionDragPrivate ())
{
}

kpSelectionDrag::kpSelectionDrag (const kpAbstractImageSelection &sel)
    : QMimeData (),
      d (new kpSelectionDragPrivate ())
{
    setSelection (sel);
}

kpSelectionDrag::~kpSelectionDrag ()
{
    delete d;
}


// public
void kpSelectionDrag::setSelection (const kpAbstractImageSelection &sel)
{
#if DEBUG_KP_SELECTION_DRAG && 1
    kDebug () << "kpSelectionDrag::setSelection() w=" << sel.width ()
               << " h=" << sel.height ()
               << endl;
#endif

    Q_ASSERT (sel.hasContent ());


    // Store as selection.
    QByteArray ba;
    {
        QDataStream stream (&ba, QIODevice::WriteOnly);
        stream << sel;
    }
    setData (kpSelectionDrag::SelectionMimeType, ba);


    // Store as image (so that QMimeData::hasImage()) works).
    // OPT: an awful waste of memory storing image in both selection and QImage
    const QImage image = kpPixmapFX::convertToQImage (sel.baseImage ());
#if DEBUG_KP_SELECTION_DRAG && 1
    kDebug () << "\timage: w=" << image.width ()
               << " h=" << image.height ()
               << endl;
#endif
    if (image.isNull ())
    {
        // TODO: proper error handling.
        kError () << "kpSelectionDrag::setSelection() could not convert to image"
                   << endl;
    }
    else
        setImageData (image);
}


// public static
bool kpSelectionDrag::canDecode (const QMimeData *e)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::canDecode()"
              << "hasSel=" << e->hasFormat (kpSelectionDrag::SelectionMimeType)
              << "hasImage=" << e->hasImage ();
#endif

    Q_ASSERT (e);

    return (e->hasFormat (kpSelectionDrag::SelectionMimeType) ||
            e->hasImage ());
}

// public static
kpAbstractImageSelection *kpSelectionDrag::decode (const QMimeData *e,
        const kpPixmapFX::WarnAboutLossInfo &wali)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::decode(kpAbstractSelection)";
#endif
    Q_ASSERT (e);

    if (e->hasFormat (kpSelectionDrag::SelectionMimeType))
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\tmimeSource hasFormat selection - just return it in QByteArray";
    #endif
        QByteArray data = e->data (kpSelectionDrag::SelectionMimeType);
        QDataStream stream (&data, QIODevice::ReadOnly);

        // Don't pass <wali> so that we suppress excessive warnings when copying
        // and pasting selections.
        //
        // The marshalled selection must have come from another -- or the same --
        // KolourPaint instance, running on the same display, with the same
        // screen depth.  That other KolourPaint would have converted its
        // selection QPixmap to QImage losslessly, and placed it in the
        // clipboard.  Note that the original QPixmap could not have had an
        // alpha channel as that's a KolourPaint invaraint.
        //
        // kpSelectionFactory::FromStream() will now convert the QImage back to
        // a QPixmap _without_ dithering, so there should be no data loss
        // anyway.
        return kpSelectionFactory::FromStream (stream);
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\tmimeSource doesn't provide selection - try image";
    #endif

        QImage image = qvariant_cast <QImage> (e->imageData ());
        if (!image.isNull ())
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\tok w=" << image.width () << " h=" << image.height ();
        #endif

            return new kpRectangularImageSelection (
                QRect (0, 0, image.width (), image.height ()),
                kpPixmapFX::convertToPixmapAsLosslessAsPossible (image, wali));
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "kpSelectionDrag::decode(kpAbstractSelection) mimeSource had no sel "
                          "and could not decode to image" << endl;
        #endif
            return 0;
        }
    }
}


#include <kpSelectionDrag.moc>
