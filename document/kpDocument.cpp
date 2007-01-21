
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qsize.h>
#include <qmatrix.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>  // TODO: isn't this in KIO?
#include <ktemporaryfile.h>

#include <kpcolor.h>
#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpdocumentsaveoptions.h>
#include <kpdocumentmetainfo.h>
#include <kpEffectReduceColors.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptooltoolbar.h>
#include <kpviewmanager.h>


kpDocument::kpDocument (int w, int h, kpMainWindow *mainWindow)
    : m_constructorWidth (w), m_constructorHeight (h),
      m_mainWindow (mainWindow),
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
    kDebug () << "kpDocument::kpDocument (" << w << "," << h << ")" << endl;
#endif

    m_pixmap = new QPixmap (w, h);
    m_pixmap->fill (Qt::white);
}

kpDocument::~kpDocument ()
{
    delete d;

    delete m_pixmap;

    delete m_saveOptions;
    delete m_metaInfo;

    delete m_selection;
}


kpMainWindow *kpDocument::mainWindow () const
{
    return m_mainWindow;
}

void kpDocument::setMainWindow (kpMainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}


// public
bool kpDocument::savedAtLeastOnceBefore () const
{
    return m_savedAtLeastOnceBefore;
}

// public
KUrl kpDocument::url () const
{
    return m_url;
}

// public
void kpDocument::setURL (const KUrl &url, bool isFromURL)
{
    m_url = url;
    m_isFromURL = isFromURL;
}

// public
bool kpDocument::isFromURL (bool checkURLStillExists) const
{
    if (!m_isFromURL)
        return false;

    if (!checkURLStillExists)
        return true;

    return (!m_url.isEmpty () &&
            KIO::NetAccess::exists (m_url, true/*open*/, m_mainWindow));
}


// static
QString kpDocument::prettyURLForURL (const KUrl &url)
{
    if (url.isEmpty ())
        return i18n ("Untitled");
    else
        return url.pathOrUrl ();
}

QString kpDocument::prettyUrl () const
{
    return prettyURLForURL (m_url);
}


// static
QString kpDocument::prettyFilenameForURL (const KUrl &url)
{
    if (url.isEmpty ())
        return i18n ("Untitled");
    else if (url.fileName ().isEmpty ())
        return prettyURLForURL (url);  // better than the name ""
    else
        return url.fileName ();
}

QString kpDocument::prettyFilename () const
{
    return prettyFilenameForURL (m_url);
}


// public
const kpDocumentSaveOptions *kpDocument::saveOptions () const
{
    return m_saveOptions;
}

// public
void kpDocument::setSaveOptions (const kpDocumentSaveOptions &saveOptions)
{
    *m_saveOptions = saveOptions;
}


// public
const kpDocumentMetaInfo *kpDocument::metaInfo () const
{
    return m_metaInfo;
}

// public
void kpDocument::setMetaInfo (const kpDocumentMetaInfo &metaInfo)
{
    *m_metaInfo = metaInfo;
}


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

bool kpDocument::isModified () const
{
    return m_modified;
}

bool kpDocument::isEmpty () const
{
    return url ().isEmpty () && !isModified ();
}


int kpDocument::constructorWidth () const
{
    return m_constructorWidth;
}

int kpDocument::width (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->width ();
    else
        return m_pixmap->width ();
}

int kpDocument::oldWidth () const
{
    return m_oldWidth;
}

void kpDocument::setWidth (int w, const kpColor &backgroundColor)
{
    resize (w, height (), backgroundColor);
}


int kpDocument::constructorHeight () const
{
    return m_constructorHeight;
}

int kpDocument::height (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->height ();
    else
        return m_pixmap->height ();
}

int kpDocument::oldHeight () const
{
    return m_oldHeight;
}

void kpDocument::setHeight (int h, const kpColor &backgroundColor)
{
    resize (width (), h, backgroundColor);
}

QRect kpDocument::rect (bool ofSelection) const
{
    if (ofSelection && m_selection)
        return m_selection->boundingRect ();
    else
        return m_pixmap->rect ();
}


/*
 * Pixmap access
 */

// public
QPixmap kpDocument::getPixmapAt (const QRect &rect) const
{
    return kpPixmapFX::getPixmapAt (*m_pixmap, rect);
}

// public
void kpDocument::setPixmapAt (const QPixmap &pixmap, const QPoint &at)
{
#if DEBUG_KP_DOCUMENT && 0
    kDebug () << "kpDocument::setPixmapAt (pixmap (w="
               << pixmap.width ()
               << ",h=" << pixmap.height ()
               << "), x=" << at.x ()
               << ",y=" << at.y ()
               << endl;
#endif

    kpPixmapFX::setPixmapAt (m_pixmap, at, pixmap);
    slotContentsChanged (QRect (at.x (), at.y (), pixmap.width (), pixmap.height ()));
}

// public
void kpDocument::paintPixmapAt (const QPixmap &pixmap, const QPoint &at)
{
    kpPixmapFX::paintPixmapAt (m_pixmap, at, pixmap);
    slotContentsChanged (QRect (at.x (), at.y (), pixmap.width (), pixmap.height ()));
}


// public
QPixmap *kpDocument::pixmap (bool ofSelection) const
{
    if (ofSelection)
    {
        if (m_selection && m_selection->pixmap ())
            return m_selection->pixmap ();
        else
            return 0;
    }
    else
        return m_pixmap;
}

// public
void kpDocument::setPixmap (const QPixmap &pixmap)
{
    m_oldWidth = width (), m_oldHeight = height ();

    *m_pixmap = pixmap;

    if (m_oldWidth == width () && m_oldHeight == height ())
        slotContentsChanged (pixmap.rect ());
    else
        slotSizeChanged (width (), height ());
}

// public
void kpDocument::setPixmap (bool ofSelection, const QPixmap &pixmap)
{
    if (ofSelection)
    {
        // Have to have a selection in order to set its pixmap.
        Q_ASSERT (m_selection);
        
        m_selection->setPixmap (pixmap);
    }
    else
        setPixmap (pixmap);
}


/*
 * Transformations
 */

void kpDocument::fill (const kpColor &color)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::fill ()" << endl;
#endif

    kpPixmapFX::fill (m_pixmap, color);
    slotContentsChanged (m_pixmap->rect ());
}

void kpDocument::resize (int w, int h, const kpColor &backgroundColor)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::resize (" << w << "," << h << ")" << endl;
#endif

    m_oldWidth = width (), m_oldHeight = height ();

#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "\toldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << endl;
#endif

    if (w == m_oldWidth && h == m_oldHeight)
        return;

    kpPixmapFX::resize (m_pixmap, w, h, backgroundColor);

    slotSizeChanged (width (), height ());
}


/*
 * Slots
 */

void kpDocument::slotContentsChanged (const QRect &rect)
{
    setModified ();
    emit contentsChanged (rect);
}

void kpDocument::slotSizeChanged (int newWidth, int newHeight)
{
    setModified ();
    emit sizeChanged (newWidth, newHeight);
    emit sizeChanged (QSize (newWidth, newHeight));
}

void kpDocument::slotSizeChanged (const QSize &newSize)
{
    slotSizeChanged (newSize.width (), newSize.height ());
}

#include <kpDocument.moc>

