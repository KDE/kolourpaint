
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


#include "kpDocument.h"
#include "kpDocumentPrivate.h"

#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "kpDefs.h"
#include "environments/document/kpDocumentEnvironment.h"
#include "document/kpDocumentSaveOptions.h"
#include "imagelib/kpDocumentMetaInfo.h"
#include "imagelib/effects/kpEffectReduceColors.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "lgpl/generic/kpUrlFormatter.h"


#include "kpLogCategories.h"
#include <KJobWidgets>
#include <KLocalizedString>
#include <KIO/StatJob>

#include <QColor>
#include <QBrush>
#include <QFile>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QTransform>

//---------------------------------------------------------------------

kpDocument::kpDocument (int w, int h,
        kpDocumentEnvironment *environ)
    : QObject (),
      m_constructorWidth (w), m_constructorHeight (h),
      m_isFromExistingURL (false),
      m_savedAtLeastOnceBefore (false),
      m_saveOptions (new kpDocumentSaveOptions ()),
      m_metaInfo (new kpDocumentMetaInfo ()),
      m_modified (false),
      m_selection (nullptr),
      m_oldWidth (-1), m_oldHeight (-1),
      d (new kpDocumentPrivate ())
{
#if DEBUG_KP_DOCUMENT && 0
    qCDebug(kpLogDocument) << "kpDocument::kpDocument (" << w << "," << h << ")";
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
QUrl kpDocument::url () const
{
    return m_url;
}

//---------------------------------------------------------------------

// public
void kpDocument::setURL (const QUrl &url, bool isFromExistingURL)
{
    m_url = url;
    m_isFromExistingURL = isFromExistingURL;
}

//---------------------------------------------------------------------

// public
bool kpDocument::isFromExistingURL () const
{
    return m_isFromExistingURL;
}

//---------------------------------------------------------------------

// public
bool kpDocument::urlExists (const QUrl &url) const
{
    if (url.isEmpty()) {
        return false;
    }
    KIO::StatJob *job = KIO::statDetails(url, KIO::StatJob::SourceSide, KIO::StatNoDetails);
    KJobWidgets::setWindow (job, d->environ->dialogParent ());
    return job->exec();
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
    if (yes == m_modified) {
        return;
    }

    m_modified = yes;

    if (yes) {
        emit documentModified ();
    }
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
    return (ofSelection && m_selection) ? m_selection->width() : m_image->width();
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
    return (ofSelection && m_selection) ? m_selection->height() : m_image->height();
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
    return (ofSelection && m_selection) ? m_selection->boundingRect() : m_image->rect();
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
    qCDebug(kpLogDocument) << "kpDocument::setImageAt (image (w="
               << image.width ()
               << ",h=" << image.height ()
               << "), x=" << at.x ()
               << ",y=" << at.y ();
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
    else {
        ret = *m_image;
    }

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
    m_oldWidth = width ();
    m_oldHeight = height ();

    *m_image = image;

    if (m_oldWidth == width () && m_oldHeight == height ()) {
        slotContentsChanged (image.rect ());
    }
    else {
        slotSizeChanged (QSize (width (), height ()));
    }
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
    else {
        setImage (image);
    }
}

//---------------------------------------------------------------------

void kpDocument::fill (const kpColor &color)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::fill ()";
#endif

    m_image->fill(color.toQRgb());
    slotContentsChanged (m_image->rect ());
}

//---------------------------------------------------------------------

void kpDocument::resize (int w, int h, const kpColor &backgroundColor)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::resize (" << w << "," << h << ")";
#endif

    m_oldWidth = width ();
    m_oldHeight = height ();

#if DEBUG_KP_DOCUMENT && 1
    qCDebug(kpLogDocument) << "\toldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight;
#endif

    if (w == m_oldWidth && h == m_oldHeight) {
        return;
    }

    kpPixmapFX::resize (m_image, w, h, backgroundColor);

    slotSizeChanged (QSize (width (), height ()));
}

//---------------------------------------------------------------------

void kpDocument::slotContentsChanged (const QRect &rect)
{
    setModified ();
    emit contentsChanged (rect);
}

//---------------------------------------------------------------------

void kpDocument::slotSizeChanged (const QSize &newSize)
{
    setModified ();
    emit sizeChanged (newSize.width(), newSize.height());
    emit sizeChanged (newSize);
}

//---------------------------------------------------------------------



