
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kpdocument.h>

#include <math.h>

#include <qcolor.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qsize.h>
#include <ktempfile.h>

#include <kpcolor.h>
#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kpviewmanager.h>


kpDocument::kpDocument (int w, int h, int colorDepth, kpMainWindow *mainWindow)
    : m_selection (0),
      m_oldWidth (-1), m_oldHeight (-1),
      m_colorDepth (colorDepth), m_oldColorDepth (-1),
      m_mainWindow (mainWindow),
      m_modified (false)
{
#if DEBUG_KP_DOCUMENT && 0
    kdDebug () << "kpDocument::kpDocument (" << w << "," << h << "," << colorDepth << ")" << endl;
#endif

    m_pixmap = new QPixmap (w, h);
    m_pixmap->fill (Qt::white);
}

kpDocument::~kpDocument ()
{
    delete m_pixmap;
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


/*
 * File I/O
 */

// public static
QPixmap kpDocument::getPixmapFromFile (const KURL &url, bool suppressDoesntExistDialog,
                                       QWidget *parent,
                                       QString &mimeType)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::getPixmapFromFile(" << url << "," << parent << ")" << endl;
#endif

    mimeType = QString::null;


    QString tempFile;
    if (url.isEmpty () || !KIO::NetAccess::download (url, tempFile, parent))
    {
        if (!suppressDoesntExistDialog)
        {
            KMessageBox::sorry (parent,
                                i18n ("Could not open \"%1\".")
                                    .arg (kpDocument::prettyFilenameForURL (url)));
        }

        return QPixmap ();
    }


    // sync: remember to "KIO::NetAccess::removeTempFile (tempFile)" in all exit paths

    mimeType = KImageIO::mimeType (tempFile);

#if DEBUG_KP_DOCUMENT
    kdDebug () << "\ttempFile=" << tempFile << endl;
    kdDebug () << "\tmimetype=" << mimetype << endl;
    kdDebug () << "\tsrc=" << url.path () << endl;
    kdDebug () << "\tmimetype of src=" << KImageIO::mimeType (url.path ()) << endl;
#endif

    if (mimeType.isEmpty ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - unknown mimetype.")
                                .arg (kpDocument::prettyFilenameForURL (url)));
        KIO::NetAccess::removeTempFile (tempFile);
        return QPixmap ();
    }


    QImage image (tempFile);
    if (image.isNull ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - unsupported image format.\n"
                                  "The file may be corrupt.")
                                .arg (kpDocument::prettyFilenameForURL (url)));
        KIO::NetAccess::removeTempFile (tempFile);
        return QPixmap ();
    }

#if DEBUG_KP_DOCUMENT
    kdDebug () << "\timage: depth=" << image.depth ()
                << " (X display=" << QColor::numBitPlanes () << ")"
                << " hasAlphaBuffer=" << image.hasAlphaBuffer ()
                << endl;
#endif

#if DEBUG_KP_DOCUMENT && 0
    kdDebug () << "Image dump:" << endl;

    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            const QRgb rgb = image.pixel (x, y);
            //fprintf (stderr, " %08X", rgb);
        }
        //fprintf (stderr, "\n");
    }
#endif


    QPixmap newPixmap;
    newPixmap = kpPixmapFX::convertToPixmap (image,
        false/*no dither*/,
        kpPixmapFX::WarnAboutLossInfo (parent,
            i18n ("image \"%1\"").arg (prettyFilenameForURL (url)),
            "docOpen"));

    if (newPixmap.isNull ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - out of graphics memory.")
                                .arg (kpDocument::prettyFilenameForURL (url)));
        KIO::NetAccess::removeTempFile (tempFile);
        return QPixmap ();
    }

#if DEBUG_KP_DOCUMENT
    kdDebug () << "\tpixmap: depth=" << newPixmap.depth ()
                << " hasAlphaChannelOrMask=" << newPixmap.hasAlpha ()
                << " hasAlphaChannel=" << newPixmap.hasAlphaChannel ()
                << endl;
#endif


    KIO::NetAccess::removeTempFile (tempFile);
    return newPixmap;
}

void kpDocument::openNew (const KURL &url)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "KpDocument::openNew (" << url << ")" << endl;
#endif

    m_pixmap->fill (Qt::white);

    m_url = url;
    m_mimetype = QString::null;
    m_modified = false;

    emit documentOpened ();
}

bool kpDocument::open (const KURL &url, bool newDocSameNameIfNotExist)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::open (" << url << ")" << endl;
#endif

    QString newMimeType;
    QPixmap newPixmap = kpDocument::getPixmapFromFile (url,
        newDocSameNameIfNotExist/*suppress "doesn't exist" dialog*/,
        m_mainWindow,
        newMimeType/*ref*/);

    if (!newPixmap.isNull ())
    {
        delete m_pixmap;
        m_pixmap = new QPixmap (newPixmap);

        m_url = url;
        m_mimetype = newMimeType;
        m_modified = false;

        emit documentOpened ();
        return true;
    }

    if (newDocSameNameIfNotExist)
    {
        if (!url.isEmpty () &&
            // not just a permission error?
            !KIO::NetAccess::exists (url, true/*open*/, m_mainWindow))
        {
            openNew (url);
        }
        else
        {
            openNew (KURL ());
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool kpDocument::save ()
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::save [" << m_url << "," << m_mimetype << "]" << endl;
#endif

    if (m_url.isEmpty () || m_mimetype.isEmpty ())
    {
        KMessageBox::detailedError (m_mainWindow,
            i18n ("Could not save image - insufficient information."),
            i18n ("URL: %1\n"
                  "Mimetype: %2")
                .arg (prettyURL ())
                .arg (m_mimetype.isEmpty () ? i18n ("<empty>") : m_mimetype),
            i18n ("Internal Error"));
        return false;
    }

    return saveAs (m_url, m_mimetype, false);
}

static QPixmap pixmapWithDefinedTransparentPixels (const QPixmap &pixmap,
                                                   const QColor &transparentColor)
{
    if (!pixmap.mask ())
        return pixmap;

    QPixmap retPixmap (pixmap.width (), pixmap.height ());
    retPixmap.fill (transparentColor);

    QPainter p (&retPixmap);
    p.drawPixmap (QPoint (0, 0), pixmap);
    p.end ();

    retPixmap.setMask (*pixmap.mask ());
    return retPixmap;
}

// public static
bool kpDocument::savePixmapToFile (const QPixmap &pixmap,
                                   const KURL &url, const QString &mimeType,
                                   bool overwritePrompt, QWidget *parent)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::savePixmapToFile ("
               << url << "," << mimetype
               << ",overwritePrompt=" << overWritePrompt << ")"
               << endl;
#endif

    if (overwritePrompt && KIO::NetAccess::exists (url, false/*write*/, parent))
    {
        int result = KMessageBox::warningContinueCancel (parent,
            i18n ("A document called \"%1\" already exists.\n"
                  "Do you want to overwrite it?")
                .arg (prettyFilenameForURL (url)),
            QString::null,
            i18n ("Overwrite"));

        if (result != KMessageBox::Continue)
        {
        #if DEBUG_KP_DOCUMENT
            kdDebug () << "\tuser doesn't want to overwrite" << endl;
        #endif

            return false;
        }
    }


    KTempFile tempFile;
    tempFile.setAutoDelete (true);

    QString filename;

    if (!url.isLocalFile ())
    {
        filename = tempFile.name ();
        if (filename.isEmpty ())
        {
            KMessageBox::error (parent,
                                i18n ("Could not save image - unable to create temporary file."));
            return false;
        }
    }
    else
        filename = url.path ();


    QString type = KImageIO::typeForMime (mimeType);
#if DEBUG_KP_DOCUMENT
    kdDebug () << "\tmimeType=" << mimeType << " type=" << type << endl;
#endif


    QPixmap pixmapToSave =
        pixmapWithDefinedTransparentPixels (pixmap,
                                            Qt::white);  // CONFIG

    if (!pixmapToSave.save (filename, type.latin1 ()))
    {
        KMessageBox::error (parent,
                            i18n ("Could not save as \"%1\".")
                                .arg (kpDocument::prettyFilenameForURL (url)));
        return false;
    }


    if (!url.isLocalFile ())
    {
        if (!KIO::NetAccess::upload (filename, url, parent))
        {
            KMessageBox::error (parent,
                                i18n ("Could not save image - failed to upload."));
            return false;
        }
    }


    return true;
}

bool kpDocument::saveAs (const KURL &url, const QString &mimetype, bool overwritePrompt)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::saveAs (" << url << "," << mimetype << ")" << endl;
#endif

    if (kpDocument::savePixmapToFile (pixmapWithSelection (),
                                      url, mimetype,
                                      overwritePrompt,
                                      m_mainWindow))
    {
        m_url = url;
        m_mimetype = mimetype;
        m_modified = false;

        emit documentSaved ();
        return true;
    }
    else
    {
        return false;
    }
}

KURL kpDocument::url () const
{
    return m_url;
}


// static
QString kpDocument::prettyURLForURL (const KURL &url)
{
    if (url.isEmpty ())
        return i18n ("Untitled");
    else
        return url.prettyURL (0, KURL::StripFileProtocol);
}

QString kpDocument::prettyURL () const
{
    return prettyURLForURL (m_url);
}


// static
QString kpDocument::prettyFilenameForURL (const KURL &url)
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


QString kpDocument::mimetype () const
{
    return m_mimetype;
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

int kpDocument::colorDepth () const
{
    return m_pixmap->depth ();
}

int kpDocument::oldColorDepth () const
{
    return m_colorDepth;
}

bool kpDocument::setColorDepth (int)
{
    m_oldColorDepth = colorDepth ();

    // SYNC: Limitation of X/QPixmap - changing colour depth yet to be implemented
    //
    //       Not really a major problem though - how could you possibly edit an
    //       image at a higher depth than what your screen is set at
    //       (accurately)?

    emit colorDepthChanged (colorDepth ());
    return true;
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
    kdDebug () << "kpDocument::setPixmapAt (pixmap (w="
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
        if (!m_selection)
        {
            kdError () << "kpDocument::setPixmap(ofSelection=true) without sel" << endl;
            return;
        }

        m_selection->setPixmap (pixmap);
    }
    else
        setPixmap (pixmap);
}


// public
kpSelection *kpDocument::selection () const
{
    return m_selection;
}

// public
void kpDocument::setSelection (const kpSelection &selection)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::setSelection() sel boundingRect="
               << selection.boundingRect ()
               << endl;
#endif

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;
    if (vm)
        vm->setQueueUpdates ();

    bool hadSelection = (bool) m_selection;


    const bool isTextChanged = (m_mainWindow->toolIsTextTool () !=
                                (selection.type () == kpSelection::Text));

    // (we don't change the Selection Tool if the new selection's
    //  shape is different to the tool's because all the Selection
    //  Tools act the same, except for what would be really irritating
    //  if it kept changing whenever you paste an image - drawing the
    //  selection region)
    if (m_mainWindow &&
        (!m_mainWindow->toolIsASelectionTool () || isTextChanged))
    {
        // Switch to the appropriately shaped selection tool
        // _before_ we change the selection
        // (all selection tool's ::end() functions nuke the current selection)
        switch (selection.type ())
        {
        case kpSelection::Rectangle:
            m_mainWindow->slotToolRectSelection ();
            break;
        case kpSelection::Ellipse:
            m_mainWindow->slotToolEllipticalSelection ();
            break;
        case kpSelection::Points:
            m_mainWindow->slotToolFreeFormSelection ();
            break;
        case kpSelection::Text:
            m_mainWindow->slotToolText ();
            break;
        default:
            break;
        }
    }


    if (m_selection)
    {
        if (m_selection->pixmap ())
            slotContentsChanged (m_selection->boundingRect ());
        else
            vm->updateViews (m_selection->boundingRect ());
        delete m_selection;
    }

    m_selection = new kpSelection (selection);

    // TODO: this coupling is bad, careless and lazy
    if (m_mainWindow)
    {
        if (!m_selection->isText ())
        {
            if (m_selection->transparency () != m_mainWindow->selectionTransparency ())
            {
                kdDebug () << "kpDocument::setSelection() sel's transparency differs "
                              "from mainWindow's transparency - setting mainWindow's transparency "
                              "to sel"
                           << endl;
                kdDebug () << "\tisOpaque: sel=" << m_selection->transparency ().isOpaque ()
                           << " mainWindow=" << m_mainWindow->selectionTransparency ().isOpaque ()
                           << endl;
                m_mainWindow->setSelectionTransparency (m_selection->transparency ());
            }
        }
        else
        {
            if (m_selection->textStyle () != m_mainWindow->textStyle ())
            {
                kdDebug () << "kpDocument::setSelection() sel's textStyle differs "
                              "from mainWindow's textStyle - setting mainWindow's textStyle "
                              "to sel"
                           << endl;
                m_mainWindow->setTextStyle (m_selection->textStyle ());
            }
        }
    }

#if DEBUG_KP_DOCUMENT && 0
    kdDebug () << "\tcheck sel " << (int *) m_selection
               << " boundingRect=" << m_selection->boundingRect ()
               << endl;
#endif
    if (m_selection->pixmap ())
        slotContentsChanged (m_selection->boundingRect ());
    else
        vm->updateViews (m_selection->boundingRect ());

    connect (m_selection, SIGNAL (changed (const QRect &)),
             this, SLOT (slotContentsChanged (const QRect &)));


    if (!hadSelection)
        emit selectionEnabled (true);

    if (isTextChanged)
        emit selectionIsTextChanged (selection.type () == kpSelection::Text);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public
QBitmap kpDocument::selectionGetMask () const
{
    kpSelection *sel = selection ();

    // must have a selection region
    if (!sel)
    {
        kdError () << "kpDocument::selectionGetMask() no sel region" << endl;
        return QBitmap ();
    }

    // easy if we already have it :)
    if (sel->pixmap ())
        return kpPixmapFX::getNonNullMask (*sel->pixmap ());


    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
    {
        kdError () << "kpDocument::selectionGetMask() boundingRect invalid" << endl;
        return QBitmap ();
    }


    QBitmap maskBitmap (boundingRect.width (), boundingRect.height ());
    if (sel->type () == kpSelection::Rectangle)
    {
        maskBitmap.fill (Qt::color1/*opaque*/);
        return maskBitmap;
    }


    maskBitmap.fill (Qt::color0/*transparent*/);

    QPainter painter;
    painter.begin (&maskBitmap);
    painter.setPen (Qt::color1)/*opaque*/;
    painter.setBrush (Qt::color1/*opaque*/);

    if (sel->type () == kpSelection::Ellipse)
        painter.drawEllipse (0, 0, boundingRect.width (), boundingRect.height ());
    else if (sel->type () == kpSelection::Points)
    {
        QPointArray points = sel->points ();
        points.detach ();
        points.translate (-boundingRect.x (), -boundingRect.y ());

        painter.drawPolygon (points, false/*even-odd algo*/);
    }

    painter.end ();


    return maskBitmap;
}

// public
QPixmap kpDocument::getSelectedPixmap (const QBitmap &maskBitmap_) const
{
    kpSelection *sel = selection ();

    // must have a selection region
    if (!sel)
    {
        kdError () << "kpDocument::getSelectedPixmap() no sel region" << endl;
        return QPixmap ();
    }

    // easy if we already have it :)
    if (sel->pixmap ())
        return *sel->pixmap ();


    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
    {
        kdError () << "kpDocument::getSelectedPixmap() boundingRect invalid" << endl;
        return QPixmap ();
    }


    QBitmap maskBitmap = maskBitmap_;
    if (sel->type () != kpSelection::Rectangle &&
        maskBitmap.isNull ())
    {
        maskBitmap = selectionGetMask ();

        if (maskBitmap.isNull ())
        {
            kdError () << "kpDocument::getSelectedPixmap() could not get mask" << endl;
            return QPixmap ();
        }
    }


    QPixmap selPixmap = getPixmapAt (boundingRect);

    if (!maskBitmap.isNull ())
    {
        // Src Dest = Result
        // -----------------
        //  0   0       0
        //  0   1       0
        //  1   0       0
        //  1   1       1
        QBitmap selMaskBitmap = kpPixmapFX::getNonNullMask (selPixmap);
        bitBlt (&selMaskBitmap,
                QPoint (0, 0),
                &maskBitmap,
                QRect (0, 0, maskBitmap.width (), maskBitmap.height ()),
                Qt::AndROP);
        selPixmap.setMask (selMaskBitmap);
    }

    return selPixmap;
}

// public
bool kpDocument::selectionPullFromDocument (const kpColor &backgroundColor)
{
    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    kpSelection *sel = selection ();

    // must have a selection region
    if (!sel)
    {
        kdError () << "kpDocument::selectionPullFromDocument() no sel region" << endl;
        return false;
    }

    // should not already have a pixmap
    if (sel->pixmap ())
    {
        kdError () << "kpDocument::selectionPullFromDocument() already has pixmap" << endl;
        return false;
    }

    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
    {
        kdError () << "kpDocument::selectionPullFromDocument() boundingRect invalid" << endl;
        return false;
    }


    //
    // Figure out mask for non-rectangular selections
    //

    QBitmap maskBitmap;

    if (sel->type () == kpSelection::Ellipse || sel->type () == kpSelection::Points)
        maskBitmap = selectionGetMask ();


    //
    // Get selection pixmap from document
    //

    QPixmap selPixmap = getSelectedPixmap (maskBitmap);

    if (vm)
        vm->setQueueUpdates ();

    sel->setPixmap (selPixmap);


    //
    // Fill opaque bits of the hole in the document
    //

    // TODO: this assumes backgroundColor == sel->transparency ().transparentColor()
    const QPixmap selTransparentPixmap = sel->transparentPixmap ();

    if (backgroundColor.isOpaque ())
    {
        QPixmap erasePixmap (boundingRect.width (), boundingRect.height ());
        erasePixmap.fill (backgroundColor.toQColor ());

        if (selTransparentPixmap.mask ())
            erasePixmap.setMask (*selTransparentPixmap.mask ());

        paintPixmapAt (erasePixmap, boundingRect.topLeft ());
    }
    else
    {
        kpPixmapFX::paintMaskTransparentWithBrush (m_pixmap,
                                                   boundingRect.topLeft (),
                                                   kpPixmapFX::getNonNullMask (selTransparentPixmap));
        slotContentsChanged (boundingRect);
    }

    if (vm)
        vm->restoreQueueUpdates ();

    return true;
}

// public
bool kpDocument::selectionDelete ()
{
    kpSelection *sel = selection ();

    if (!sel)
        return false;

    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
        return false;

    bool selectionHadPixmap = m_selection ? (bool) m_selection->pixmap () : false;

    delete m_selection;
    m_selection = 0;

    // HACK to prevent document from being modified when
    //      user cancels dragging out a new selection
    if (selectionHadPixmap)
        slotContentsChanged (boundingRect);
    else
        emit contentsChanged (boundingRect);

    emit selectionEnabled (false);

    return true;
}

// public
bool kpDocument::selectionCopyOntoDocument (bool useTransparentPixmap)
{
    kpSelection *sel = selection ();

    // must have a pixmap already
    if (!sel)
        return false;

    // hasn't actually been lifted yet
    if (!sel->pixmap ())
        return true;

    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
        return false;

    paintPixmapAt (useTransparentPixmap ? sel->transparentPixmap () : sel->opaquePixmap (),
                   boundingRect.topLeft ());
    return true;
}

// public
bool kpDocument::selectionPushOntoDocument (bool useTransparentPixmap)
{
    return (selectionCopyOntoDocument (useTransparentPixmap) && selectionDelete ());
}

// public
QPixmap kpDocument::pixmapWithSelection () const
{
#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "kpDocument::pixmapWithSelection()" << endl;
#endif

    if (m_selection && m_selection->pixmap ())
    {
    #if DEBUG_KP_DOCUMENT && 1
        kdDebug () << "\tselection @ " << m_selection->boundingRect () << endl;
    #endif
        QPixmap output = *m_pixmap;

        kpPixmapFX::paintPixmapAt (&output,
                                   m_selection->topLeft (),
                                   *m_selection->pixmap ());
        return output;
    }
    else
    {
    #if DEBUG_KP_DOCUMENT && 1
        kdDebug () << "\tno selection" << endl;
    #endif
        return *m_pixmap;
    }
}


/*
 * Transformations
 */

void kpDocument::fill (const kpColor &color)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::fill ()" << endl;
#endif

    kpPixmapFX::fill (m_pixmap, color);
    slotContentsChanged (m_pixmap->rect ());
}

void kpDocument::resize (int w, int h, const kpColor &backgroundColor, bool fillNewAreas)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::resize (" << w << "," << h << "," << fillNewAreas << ")" << endl;
#endif

    m_oldWidth = width (), m_oldHeight = height ();

#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "\toldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << endl;
#endif

    if (w == m_oldWidth && h == m_oldHeight)
        return;

    kpPixmapFX::resize (m_pixmap, w, h, backgroundColor, fillNewAreas);

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

#include <kpdocument.moc>
