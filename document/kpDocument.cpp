
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


#define DEBUG_KP_DOCUMENT 0


#include <kpDocument.h>
#include <kpDocumentPrivate.h>

#include <math.h>

#include <qcolor.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qfile.h>
#include <qimage.h>
#include <qlist.h>
#include <QImage>
#include <qpainter.h>
#include <qrect.h>
#include <qsize.h>
#include <qmatrix.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktemporaryfile.h>

#include <kpAbstractSelection.h>
#include <kpAbstractImageSelection.h>
#include <kpColor.h>
#include <kpColorToolBar.h>
#include <kpDefs.h>
#include <kpDocumentEnvironment.h>
#include <kpDocumentSaveOptions.h>
#include <kpDocumentMetaInfo.h>
#include <kpEffectReduceColors.h>
#include <kpPixmapFX.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpUrlFormatter.h>

//---------------------------------------------------------------------

kpDocument::kpDocument (int w, int h,
        kpDocumentEnvironment *environ)
    : QObject (),
      m_constructorWidth (w), m_constructorHeight (h),
      m_isFromURL (false),
      m_savedAtLeastOnceBefore (false),
      m_saveOptions (new kpDocumentSaveOptions ()),
      m_metaInfo (new kpDocumentMetaInfo ()),
      m_modified (false),
      m_selection (0),
      m_oldWidth (-1), m_oldHeight (-1),
      d (new kpDocumentPrivate ())
{
#if DEBUG_KP_DOCUMENT && 0
    kDebug () << "kpDocument::kpDocument (" << w << "," << h << ")";
#endif

    m_image = new kpImage(w, h, QImage::Format_ARGB32_Premultiplied);
    m_image->fill(QColor(Qt::white).rgb());

    d->environ = environ;
}

//---------------------------------------------------------------------

kpDocument::~kpDocument ()
{
    delete d;

    delete m_image;

    delete m_saveOptions;
    delete m_metaInfo;

    delete m_selection;
}

//---------------------------------------------------------------------

// public
kpDocumentEnvironment *kpDocument::environ () const
{
    return d->environ;
}

//---------------------------------------------------------------------

// public
void kpDocument::setEnviron (kpDocumentEnvironment *environ)
{
    d->environ = environ;
}

//---------------------------------------------------------------------

// public
bool kpDocument::savedAtLeastOnceBefore () const
{
    return m_savedAtLeastOnceBefore;
}

//---------------------------------------------------------------------

// public
KUrl kpDocument::url () const
{
    return m_url;
}

//---------------------------------------------------------------------

// public
void kpDocument::setURL (const KUrl &url, bool isFromURL)
{
    m_url = url;
    m_isFromURL = isFromURL;
}

//---------------------------------------------------------------------

// public
bool kpDocument::isFromURL (bool checkURLStillExists) const
{
    if (!m_isFromURL)
        return false;

    if (!checkURLStillExists)
        return true;

    return (!m_url.isEmpty () &&
            KIO::NetAccess::exists (m_url, KIO::NetAccess::SourceSide/*open*/,
                d->environ->dialogParent ()));
}

//---------------------------------------------------------------------

// public
QString kpDocument::prettyUrl () const
{
    return kpUrlFormatter::PrettyUrl (m_url);
}

//---------------------------------------------------------------------

// public
QString kpDocument::prettyFilename () const
{
    return kpUrlFormatter::PrettyFilename (m_url);
}

//---------------------------------------------------------------------

// public
const kpDocumentSaveOptions *kpDocument::saveOptions () const
{
    return m_saveOptions;
}

//---------------------------------------------------------------------

// public
void kpDocument::setSaveOptions (const kpDocumentSaveOptions &saveOptions)
{
    *m_saveOptions = saveOptions;
}

//---------------------------------------------------------------------

// public
const kpDocumentMetaInfo *kpDocument::metaInfo () const
{
    return m_metaInfo;
}

//---------------------------------------------------------------------

// public
void kpDocument::setMetaInfo (const kpDocumentMetaInfo &metaInfo)
{
    *m_metaInfo = metaInfo;
}

//---------------------------------------------------------------------

/*
 * Properties
 */

void kpDocument::setModified (bool yes)
{
    if (yes == m_modified)
        return;

    m_modified = yes;

    if (yes)
        emit documentModified ();
}

//---------------------------------------------------------------------

bool kpDocument::isModified () const
{
    return m_modified;
}

//---------------------------------------------------------------------

bool kpDocument::isEmpty () const
{
    return url ().isEmpty () && !isModified ();
}

//---------------------------------------------------------------------

int kpDocument::constructorWidth () const
{
    return m_constructorWidth;
}

//---------------------------------------------------------------------

int kpDocument::width (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->width ();
    else
        return m_image->width ();
}

//---------------------------------------------------------------------

int kpDocument::oldWidth () const
{
    return m_oldWidth;
}

//---------------------------------------------------------------------

void kpDocument::setWidth (int w, const kpColor &backgroundColor)
{
    resize (w, height (), backgroundColor);
}

//---------------------------------------------------------------------

int kpDocument::constructorHeight () const
{
    return m_constructorHeight;
}

//---------------------------------------------------------------------

int kpDocument::height (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->height ();
    else
        return m_image->height ();
}

//---------------------------------------------------------------------

int kpDocument::oldHeight () const
{
    return m_oldHeight;
}

//---------------------------------------------------------------------

void kpDocument::setHeight (int h, const kpColor &backgroundColor)
{
    resize (width (), h, backgroundColor);
}

//---------------------------------------------------------------------

QRect kpDocument::rect (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->boundingRect ();
    else
        return m_image->rect ();
}

//---------------------------------------------------------------------

// public
kpImage kpDocument::getImageAt (const QRect &rect) const
{
    return kpPixmapFX::getPixmapAt (*m_image, rect);
}

//---------------------------------------------------------------------

// public
void kpDocument::setImageAt (const kpImage &image, const QPoint &at)
{
#if DEBUG_KP_DOCUMENT && 0
    kDebug () << "kpDocument::setImageAt (image (w="
               << image.width ()
               << ",h=" << image.height ()
               << "), x=" << at.x ()
               << ",y=" << at.y ()
               << endl;
#endif

    kpPixmapFX::setPixmapAt (m_image, at, image);
    slotContentsChanged (QRect (at.x (), at.y (), image.width (), image.height ()));
}

//---------------------------------------------------------------------

// public
kpImage kpDocument::image (bool ofSelection) const
{
    kpImage ret;

    if (ofSelection)
    {
        kpAbstractImageSelection *imageSel = imageSelection ();
        Q_ASSERT (imageSel);

        ret = imageSel->baseImage ();
    }
    else
        ret = *m_image;

    return ret;
}

//---------------------------------------------------------------------

// public
kpImage *kpDocument::imagePointer () const
{
    return m_image;
}

//---------------------------------------------------------------------

// public
void kpDocument::setImage (const kpImage &image)
{
    m_oldWidth = width (), m_oldHeight = height ();

    *m_image = image;

    if (m_oldWidth == width () && m_oldHeight == height ())
        slotContentsChanged (image.rect ());
    else
        slotSizeChanged (width (), height ());
}

//---------------------------------------------------------------------

// public
void kpDocument::setImage (bool ofSelection, const kpImage &image)
{
    if (ofSelection)
    {
        kpAbstractImageSelection *imageSel = imageSelection ();

        // Have to have an image selection in order to set its pixmap.
        Q_ASSERT (imageSel);

        imageSel->setBaseImage (image);
    }
    else
        setImage (image);
}

//---------------------------------------------------------------------

void kpDocument::fill (const kpColor &color)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::fill ()";
#endif

    m_image->fill(color.toQRgb());
    slotContentsChanged (m_image->rect ());
}

//---------------------------------------------------------------------

void kpDocument::resize (int w, int h, const kpColor &backgroundColor)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::resize (" << w << "," << h << ")";
#endif

    m_oldWidth = width (), m_oldHeight = height ();

#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "\toldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << endl;
#endif

    if (w == m_oldWidth && h == m_oldHeight)
        return;

    kpPixmapFX::resize (m_image, w, h, backgroundColor);

    slotSizeChanged (width (), height ());
}

//---------------------------------------------------------------------

void kpDocument::slotContentsChanged (const QRect &rect)
{
    setModified ();
    emit contentsChanged (rect);
}

//---------------------------------------------------------------------

void kpDocument::slotSizeChanged (int newWidth, int newHeight)
{
    setModified ();
    emit sizeChanged (newWidth, newHeight);
    emit sizeChanged (QSize (newWidth, newHeight));
}

//---------------------------------------------------------------------

void kpDocument::slotSizeChanged (const QSize &newSize)
{
    slotSizeChanged (newSize.width (), newSize.height ());
}

//---------------------------------------------------------------------


#include <kpDocument.moc>

