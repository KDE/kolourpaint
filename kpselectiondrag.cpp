
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

#define DEBUG_KP_SELECTION_DRAG 0


#include <kpselectiondrag.h>

#include <qdatastream.h>

#include <kdebug.h>

#include <kppixmapfx.h>
#include <kpselection.h>


// public static
const char * const kpSelectionDrag::selectionMimeType = "application/x-kolourpaint-selection";


kpSelectionDrag::kpSelectionDrag (QWidget *dragSource, const char *name)
    : QImageDrag (dragSource, name)
{
}

kpSelectionDrag::kpSelectionDrag (const QImage &image, QWidget *dragSource, const char *name)
    : QImageDrag (image, dragSource, name)
{
}

kpSelectionDrag::kpSelectionDrag (const kpSelection &sel, QWidget *dragSource, const char *name)
    : QImageDrag (dragSource, name)
{
    setSelection (sel);
}

kpSelectionDrag::~kpSelectionDrag ()
{
}


// public
void kpSelectionDrag::setSelection (const kpSelection &sel)
{
    if (!sel.pixmap ())
    {
        kdError () << "kpSelectionDrag::setSelection() without pixmap" << endl;
        return;
    }

    m_selection = sel;

    // OPT: an awful waste of memory storing image in both selection and QImage

    // HACK: need to set image else QImageDrag::format() lies
    setImage (kpPixmapFX::convertToImage (*m_selection.pixmap ()));
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
    kdDebug () << "kpSelectionDrag::format(" << which << ")" << endl;
#endif
    const char *ret = QImageDrag::format (which);
    if (ret)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kdDebug () << "\tQImageDrag reports " << ret << endl;
    #endif
        return ret;
    }

    int i;
    for (i = 0; QImageDrag::format (i); i++)
        ;

#if DEBUG_KP_SELECTION_DRAG && 0
    kdDebug () << "\tend of QImageDrag format list at " << i << endl;
#endif

    if (i == which)
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kdDebug () << "\treturning own mimetype" << endl;
    #endif
        return kpSelectionDrag::selectionMimeType;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG && 0
        kdDebug () << "\treturning non-existent" << endl;
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
    kdDebug () << "kpSelectionDrag::provides(" << mimeType << ")" << endl;
#endif

    if (!mimeType)
        return false;

    return (!strcmp (mimeType, kpSelectionDrag::selectionMimeType) ||
            QImageDrag::provides (mimeType));
}

// public virtual [base QMimeSource]
QByteArray kpSelectionDrag::encodedData (const char *mimeType) const
{
#if DEBUG_KP_SELECTION_DRAG
    kdDebug () << "kpSelectionDrag::encodedData(" << mimeType << ")" << endl;
#endif

    if (!mimeType)
        return QByteArray ();

    if (!strcmp (mimeType, kpSelectionDrag::selectionMimeType))
    {
        QByteArray ba;
        QDataStream stream (ba, IO_WriteOnly);

    #if DEBUG_KP_SELECTION_DRAG
        kdDebug () << "\twant it as kpSelection in QByteArray" << endl;
    #endif

        if (holdsSelection ())
        {
        #if DEBUG_KP_SELECTION_DRAG
            kdDebug () << "\t\thave selection - return it" << endl;
        #endif
            stream << m_selection;
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kdDebug () << "\t\thave image - call kpSelectionDrag::decode(QImage)" << endl;
        #endif
            QImage image;
            if (kpSelectionDrag::decode (this, image/*ref*/))
            {
            #if DEBUG_KP_SELECTION_DRAG
                kdDebug () << "\t\t\tok - returning sel with image w="
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
                kdError () << "kpSelectionDrag::encodedData(" << mimeType << ")"
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
        kdDebug () << "\twant it as QImage in QByteArray" << endl;
    #endif

        return QImageDrag::encodedData (mimeType);
    }
}


// public static
bool kpSelectionDrag::canDecode (const QMimeSource *e)
{
#if DEBUG_KP_SELECTION_DRAG
    kdDebug () << "kpSelectionDrag::canDecode()" << endl;
#endif

    if (!e)
        return false;

    return (e->provides (kpSelectionDrag::selectionMimeType) ||
            QImageDrag::canDecode (e));
}


// public static
bool kpSelectionDrag::decode (const QMimeSource *e, QImage &img)
{
#if DEBUG_KP_SELECTION_DRAG
    kdDebug () << "kpSelectionDrag::decode(QImage)" << endl;
#endif
    if (!e)
        return false;

    return (QImageDrag::canDecode (e) &&  // prevents X errors, jumps based on unitialised values...
            QImageDrag::decode (e, img/*ref*/));
}

// public static
bool kpSelectionDrag::decode (const QMimeSource *e, kpSelection &sel,
                              const kpPixmapFX::WarnAboutLossInfo &wali)
{
#if DEBUG_KP_SELECTION_DRAG
    kdDebug () << "kpSelectionDrag::decode(kpSelection)" << endl;
#endif
    if (!e)
        return false;

    if (e->provides (kpSelectionDrag::selectionMimeType))
    {
    #if DEBUG_KP_SELECTION_DRAG
        kdDebug () << "\tmimeSource provides selection - just return it in QByteArray" << endl;
    #endif
        QByteArray data = e->encodedData (kpSelectionDrag::selectionMimeType);
        QDataStream stream (data, IO_ReadOnly);

        // (no need for wali as kpSelection's by definition only support QPixmap's)
        stream >> sel;
    }
    else
    {
    #if DEBUG_KP_SELECTION_DRAG
        kdDebug () << "\tmimeSource doesn't provide selection - try image" << endl;
    #endif

        QImage image;
        if (kpSelectionDrag::decode (e, image/*ref*/))
        {
        #if DEBUG_KP_SELECTION_DRAG
            kdDebug () << "\tok w=" << image.width () << " h=" << image.height () << endl;
        #endif

            sel = kpSelection (kpSelection::Rectangle,
                               QRect (0, 0, image.width (), image.height ()),
                               kpPixmapFX::convertToPixmapAsLosslessAsPossible (image, wali));
        }
        else
        {
        #if DEBUG_KP_SELECTION_DRAG
            kdDebug () << "kpSelectionDrag::decode(kpSelection) mimeSource had no sel "
                          "and could not decode to image" << endl;
        #endif
            return false;
        }
    }

    return true;
}

#include <kpselectiondrag.moc>
