
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
        : selection (0)
    {
    }

    kpAbstractImageSelection *selection;
};

kpSelectionDrag::kpSelectionDrag (QWidget *dragSource, const char *name)
    : Q3ImageDrag (dragSource, name),
      d (new kpSelectionDragPrivate ())
{
}

kpSelectionDrag::kpSelectionDrag (const QImage &image, QWidget *dragSource,
        const char *name)
    : Q3ImageDrag (image, dragSource, name),
      d (new kpSelectionDragPrivate ())
{
}

kpSelectionDrag::kpSelectionDrag (const kpAbstractImageSelection &sel,
        QWidget *dragSource, const char *name)
    : Q3ImageDrag (dragSource, name),
      d (new kpSelectionDragPrivate ())
{
    setSelection (sel);
}

kpSelectionDrag::~kpSelectionDrag ()
{
    delete d->selection;
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

    delete d->selection;
    d->selection = sel.clone ();

    // OPT: an awful waste of memory storing image in both selection and QImage

    // HACK: need to set image else QImageDrag::format() lies
    const QImage image = kpPixmapFX::convertToImage (d->selection->baseImage ());
#if DEBUG_KP_SELECTION_DRAG && 1
    kDebug () << "\timage: w=" << image.width ()
               << " h=" << image.height ()
               << endl;
#endif
    if (image.isNull ())
    {
        kError () << "kpSelectionDrag::setSelection() could not convert to image"
                   << endl;
    }
    setImage (image);
}

// protected
bool kpSelectionDrag::holdsSelection () const
{
    return (d->selection != 0);
}


// public virtual [base QMimeSource]
const char *kpSelectionDrag::format (int which) const
{
#if DEBUG_KP_SELECTION_DRAG && 0
    kDebug () << "kpSelectionDrag::format(" << which << ")";
#endif
    const char *ret = Q3ImageDrag::format (which);
    if (ret)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\tQImageDrag reports " << ret;
    #endif
        return ret;
    }

    int i;
    for (i = 0; Q3ImageDrag::format (i); i++)
        ;

#if DEBUG_KP_SELECTION_DRAG && 0
    kDebug () << "\tend of QImageDrag format list at " << i;
#endif

    if (i == which)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\treturning own mimetype";
    #endif
        return kpSelectionDrag::SelectionMimeType;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\treturning non-existent";
    #endif
        return 0;
    }
}

// public virtual [base QMimeSource]
// Don't need to override in Qt 3.2 (the QMimeSource base will work)
// but we do so just in case QImageDrag later decides to override it.
bool kpSelectionDrag::provides (const char *mimeType) const
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::provides(" << mimeType << ")";
#endif

    Q_ASSERT (mimeType);

    const bool ret = (!strcmp (mimeType, kpSelectionDrag::SelectionMimeType) ||
        Q3ImageDrag::provides (mimeType));
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "\treturning " << ret;
#endif
    return ret;
}

// public virtual [base QMimeSource]
QByteArray kpSelectionDrag::encodedData (const char *mimeType) const
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::encodedData(" << mimeType << ")";
#endif

    Q_ASSERT (mimeType);

    // We want the output as a selection?
    if (!strcmp (mimeType, kpSelectionDrag::SelectionMimeType))
    {
        QByteArray ba;
        QDataStream stream (&ba, QIODevice::WriteOnly);

    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\twant it as kpAbstractSelection in QByteArray";
    #endif

        if (holdsSelection ())
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\t\thave selection - return it";
        #endif
            stream << *d->selection;
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\t\thave image - call kpSelectionDrag::decode(QImage)";
        #endif
            QImage image;
            if (kpSelectionDrag::decode (this, image/*ref*/))
            {
            #if DEBUG_KP_SELECTION_DRAG
                kDebug () << "\t\t\tok - returning sel with image w="
                           << image.width ()
                           << " h="
                           << image.height ()
                           << endl;
            #endif

                // TODO: I think this needs kpPixmapFX::WarnAboutLossInfo.
                QPixmap pixmap = kpPixmapFX::convertToPixmapAsLosslessAsPossible (image);

                stream << kpRectangularImageSelection (
                    QRect (0, 0, pixmap.width (), pixmap.height ()),
                    pixmap);
            }
            else
            {
                kError () << "kpSelectionDrag::encodedData(" << mimeType << ")"
                           << " kpSelectionDrag(QImage) could not decode data into QImage"
                           << endl;
                stream << kpRectangularImageSelection ();
            }
        }

        return ba;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\twant it as QImage in QByteArray";
    #endif

        QByteArray ba = Q3ImageDrag::encodedData (mimeType);

    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\t\tba.size=" << ba.size ();
    #endif
        return ba;
    }
}


// public static
bool kpSelectionDrag::canDecode (const QMimeSource *e)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::canDecode()";
#endif

    Q_ASSERT (e);

    return (e->provides (kpSelectionDrag::SelectionMimeType) ||
            Q3ImageDrag::canDecode (e));
}


// public static
bool kpSelectionDrag::decode (const QMimeSource *e, QImage &img)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::decode(QImage)";
#endif
    Q_ASSERT (e);

    return (Q3ImageDrag::canDecode (e) &&  // prevents X errors, jumps based on unitialised values...
            Q3ImageDrag::decode (e, img/*ref*/));
}

// public static
kpAbstractImageSelection *kpSelectionDrag::decode (const QMimeSource *e,
        const kpPixmapFX::WarnAboutLossInfo &wali)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::decode(kpAbstractSelection)";
#endif
    Q_ASSERT (e);

    if (e->provides (kpSelectionDrag::SelectionMimeType))
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\tmimeSource provides selection - just return it in QByteArray";
    #endif
        QByteArray data = e->encodedData (kpSelectionDrag::SelectionMimeType);
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

        QImage image;
        if (kpSelectionDrag::decode (e, image/*ref*/))
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
