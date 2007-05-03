
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
#include <kpSelection.h>


// public static
const char * const kpSelectionDrag::selectionMimeType = "application/x-kolourpaint-selection";


kpSelectionDrag::kpSelectionDrag (QWidget *dragSource, const char *name)
    : Q3ImageDrag (dragSource, name)
{
}

kpSelectionDrag::kpSelectionDrag (const QImage &image, QWidget *dragSource, const char *name)
    : Q3ImageDrag (image, dragSource, name)
{
}

kpSelectionDrag::kpSelectionDrag (const kpSelection &sel, QWidget *dragSource, const char *name)
    : Q3ImageDrag (dragSource, name)
{
    setSelection (sel);
}

kpSelectionDrag::~kpSelectionDrag ()
{
}


// public
void kpSelectionDrag::setSelection (const kpSelection &sel)
{
#if DEBUG_KP_SELECTION_DRAG && 1
    kDebug () << "kpSelectionDrag::setSelection() w=" << sel.width ()
               << " h=" << sel.height ()
               << " pm=" << sel.pixmap ()
               << endl;
#endif

    if (!sel.pixmap ())
    {
        kError () << "kpSelectionDrag::setSelection() without pixmap" << endl;
        return;
    }

    m_selection = sel;

    // OPT: an awful waste of memory storing image in both selection and QImage

    // HACK: need to set image else QImageDrag::format() lies
    const QImage image = kpPixmapFX::convertToImage (*m_selection.pixmap ());
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
    return bool (m_selection.pixmap ());
}


// public virtual [base QMimeSource]
const char *kpSelectionDrag::format (int which) const
{
#if DEBUG_KP_SELECTION_DRAG && 0
    kDebug () << "kpSelectionDrag::format(" << which << ")" << endl;
#endif
    const char *ret = Q3ImageDrag::format (which);
    if (ret)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\tQImageDrag reports " << ret << endl;
    #endif
        return ret;
    }

    int i;
    for (i = 0; Q3ImageDrag::format (i); i++)
        ;

#if DEBUG_KP_SELECTION_DRAG && 0
    kDebug () << "\tend of QImageDrag format list at " << i << endl;
#endif

    if (i == which)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\treturning own mimetype" << endl;
    #endif
        return kpSelectionDrag::selectionMimeType;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kDebug () << "\treturning non-existent" << endl;
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
    kDebug () << "kpSelectionDrag::provides(" << mimeType << ")" << endl;
#endif

    if (!mimeType)
        return false;

    const bool ret = (!strcmp (mimeType, kpSelectionDrag::selectionMimeType) ||
            Q3ImageDrag::provides (mimeType));
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "\treturning " << ret << endl;
#endif
    return ret;
}

// public virtual [base QMimeSource]
QByteArray kpSelectionDrag::encodedData (const char *mimeType) const
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::encodedData(" << mimeType << ")" << endl;
#endif

    if (!mimeType)
        return QByteArray ();

    if (!strcmp (mimeType, kpSelectionDrag::selectionMimeType))
    {
        QByteArray ba;
        QDataStream stream (&ba, QIODevice::WriteOnly);

    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\twant it as kpSelection in QByteArray" << endl;
    #endif

        if (holdsSelection ())
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\t\thave selection - return it" << endl;
        #endif
            stream << m_selection;
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\t\thave image - call kpSelectionDrag::decode(QImage)" << endl;
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

                QPixmap pixmap = kpPixmapFX::convertToPixmapAsLosslessAsPossible (image);

                stream << kpSelection (kpSelection::Rectangle,
                                       QRect (0, 0, pixmap.width (), pixmap.height ()),
                                       pixmap);
            }
            else
            {
                kError () << "kpSelectionDrag::encodedData(" << mimeType << ")"
                           << " kpSelectionDrag(QImage) could not decode data into QImage"
                           << endl;
                stream << kpSelection ();
            }
        }

        return ba;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\twant it as QImage in QByteArray" << endl;
    #endif

        QByteArray ba = Q3ImageDrag::encodedData (mimeType);
 
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\t\tba.size=" << ba.size () << endl;
    #endif
        return ba;
    }
}


// public static
bool kpSelectionDrag::canDecode (const QMimeSource *e)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::canDecode()" << endl;
#endif

    if (!e)
        return false;

    return (e->provides (kpSelectionDrag::selectionMimeType) ||
            Q3ImageDrag::canDecode (e));
}


// public static
bool kpSelectionDrag::decode (const QMimeSource *e, QImage &img)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::decode(QImage)" << endl;
#endif
    if (!e)
        return false;

    return (Q3ImageDrag::canDecode (e) &&  // prevents X errors, jumps based on unitialised values...
            Q3ImageDrag::decode (e, img/*ref*/));
}

// public static
bool kpSelectionDrag::decode (const QMimeSource *e, kpSelection &sel,
                              const kpPixmapFX::WarnAboutLossInfo &wali)
{
#if DEBUG_KP_SELECTION_DRAG
    kDebug () << "kpSelectionDrag::decode(kpSelection)" << endl;
#endif
    if (!e)
        return false;

    if (e->provides (kpSelectionDrag::selectionMimeType))
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\tmimeSource provides selection - just return it in QByteArray" << endl;
    #endif
        QByteArray data = e->encodedData (kpSelectionDrag::selectionMimeType);
        QDataStream stream (&data, QIODevice::ReadOnly);

        // (no need for wali as kpSelection's by definition only support QPixmap's)
        stream >> sel;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG
        kDebug () << "\tmimeSource doesn't provide selection - try image" << endl;
    #endif

        QImage image;
        if (kpSelectionDrag::decode (e, image/*ref*/))
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "\tok w=" << image.width () << " h=" << image.height () << endl;
        #endif

            sel = kpSelection (kpSelection::Rectangle,
                               QRect (0, 0, image.width (), image.height ()),
                               kpPixmapFX::convertToPixmapAsLosslessAsPossible (image, wali));
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kDebug () << "kpSelectionDrag::decode(kpSelection) mimeSource had no sel "
                          "and could not decode to image" << endl;
        #endif
            return false;
        }
    }

    return true;
}

#include <kpSelectionDrag.moc>