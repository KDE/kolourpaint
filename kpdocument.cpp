
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_DOCUMENT 1

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
#include <ktempfile.h>

#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptool.h>
#include <kpviewmanager.h>


kpDocument::kpDocument (int w, int h, int colorDepth, kpMainWindow *mainWindow)
    : m_oldWidth (-1), m_oldHeight (-1),
      m_colorDepth (colorDepth), m_oldColorDepth (-1),
      m_mainWindow (mainWindow),
      m_modified (false)
{
#if DEBUG_KP_DOCUMENT && 0
    kdDebug () << "kpDocument::kpDocument (" << w << "," << h << "," << colorDepth << ")" << endl;
#endif

    m_pixmap = new QPixmap (w, h);
    m_pixmap->fill (KP_BLANK_DOCUMENT_COLOR);
}

kpDocument::~kpDocument ()
{
    delete m_pixmap;
}


/*
 * File I/O
 */

void kpDocument::openNew (const KURL &url)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "KpDocument::openNew (" << url << ")" << endl;
#endif

    m_pixmap->fill (KP_BLANK_DOCUMENT_COLOR);

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

    bool errorOccurred = false;

    QString tempFile;
    if (!url.isEmpty () && KIO::NetAccess::download (url, tempFile, m_mainWindow))
    {
        // sync: remember to "KIO::NetAccess::removeTempFile (tempFile)" in all exit paths

        QString mimetype = KImageIO::mimeType (tempFile);

    #if DEBUG_KP_DOCUMENT
        kdDebug () << "\ttempFile=" << tempFile << endl;
        kdDebug () << "\tmimetype=" << mimetype << endl;
        kdDebug () << "\tsrc=" << url.path () << endl;
        kdDebug () << "\tmimetype of src=" << KImageIO::mimeType (url.path ()) << endl;
    #endif

        if (mimetype.isEmpty ())
        {
            KMessageBox::sorry (m_mainWindow,
                                i18n ("Could not open \"%1\" - unknown mimetype.")
                                    .arg (kpDocument::prettyFilenameForURL (url)));
            errorOccurred = true;
        }
        else
        {
            QImage image (tempFile);
            if (image.isNull ())
            {
                KMessageBox::sorry (m_mainWindow,
                                    i18n ("Could not open \"%1\" - unsupported image format.\n"
                                          "The file may be corrupt.")
                                        .arg (kpDocument::prettyFilenameForURL (url)));
                errorOccurred = true;
            }
            else
            {
            #if DEBUG_KP_DOCUMENT
                kdDebug () << "\timage: depth=" << image.depth ()
                           << " (X display=" << QColor::numBitPlanes () << ")"
                           << " hasAlphaBuffer=" << image.hasAlphaBuffer ()
                           << endl;
            #endif

                bool warned = false;
                
                // TODO: port warnings to cut & paste (in case remote program manipulates
                // QImage for e.g.) 
                
                if (!warned && image.hasAlphaBuffer ())
                {
                    // SYNC: remove 1.0 reference after impl transparency
                    errorOccurred = (KMessageBox::warningContinueCancel (m_mainWindow,
                        i18n ("The image \"%1\" has an Alpha Channel.\n"
                              "This is not fully supported by KolourPaint "
                              "(1-bit transparency will be supported by 1.0). "
                              "If you open this file, some of its colors and opacity "
                              "may be incorrect "
                              "and this will also adversely affect future save operations.\n"
                              "Do you really want to open this file?")
                            .arg (kpDocument::prettyFilenameForURL (url)),
                        i18n ("Loss of Color and/or Opacity Information"),
                        KStdGuiItem::open (),
                        "DoNotAskAgain_OpenLossOfColorAndOpacity") != KMessageBox::Continue);
                    warned = true;
                }
                
                if (!warned && image.depth () > QColor::numBitPlanes ())
                {
                    errorOccurred = (KMessageBox::warningContinueCancel (m_mainWindow,
                        i18n ("The image \"%1\" has a higher color depth (%2-bit) "
                              "than the display (%3-bit).\n"
                              "If you open this file, some of its colors may be incorrect "
                              "and this will also adversely affect future save operations.\n"
                              "Do you really want to open this file?")
                            .arg (kpDocument::prettyFilenameForURL (url))
                            .arg (image.depth ())
                            .arg (QColor::numBitPlanes ()),
                        i18n ("Loss of Color Information"),
                        KStdGuiItem::open (),
                        "DoNotAskAgain_OpenLossOfColor") != KMessageBox::Continue);
                    warned = true;
                }
                
                if (!errorOccurred)
                {
                    QPixmap *newPixmap = new QPixmap ();
                    *newPixmap = kpPixmapFX::convertToPixmap (image, true/*pretty*/);
                    
                    if (newPixmap->isNull ())
                    {
                        kdError () << "could not convert from QImage" << endl;
                        delete newPixmap;
                        errorOccurred = true;
                    }
                    else
                    {
                    #if DEBUG_KP_DOCUMENT
                        kdDebug () << "\tpixmap: depth=" << newPixmap->depth ()
                                   << " hasAlphaChannelOrMask=" << newPixmap->hasAlpha ()
                                   << " hasAlphaChannel=" << newPixmap->hasAlphaChannel ()
                                   << endl;
                    #endif
                    
                        KIO::NetAccess::removeTempFile (tempFile);
        
                        delete m_pixmap;
                        m_pixmap = newPixmap;
        
                        m_url = url;
                        m_mimetype = mimetype;
                        m_modified = false;
        
                        emit documentOpened ();
                        return true;
                    }
                }
            }
        }
        
        // --- if we are here, the file format was unrecognised --- //

        KIO::NetAccess::removeTempFile (tempFile);
    }

    if (newDocSameNameIfNotExist)
    {
        if (errorOccurred ||
            url.isEmpty () ||
            // maybe it was a permission error?
            KIO::NetAccess::exists (url, true/*open*/, m_mainWindow))
        {
            openNew (KURL ());
        }
        else
        {
            openNew (url);
        }

        return true;
    }
    else
    {
        if (!errorOccurred)
        {
            KMessageBox::sorry (m_mainWindow,
                                i18n ("Could not open \"%1\".")
                                    .arg (kpDocument::prettyFilenameForURL (url)));
        }

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

bool kpDocument::saveAs (const KURL &url, const QString &mimetype, bool overwritePrompt)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::saveAs (" << url << "," << mimetype << ")" << endl;
#endif

    if (overwritePrompt && KIO::NetAccess::exists (url, false/*write*/, m_mainWindow))
    {
        int result = KMessageBox::warningContinueCancel (m_mainWindow,
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
            KMessageBox::error (m_mainWindow,
                                i18n ("Could not save image - unable to create temporary file."));
            return false;
        }
    }
    else
        filename = url.path ();

    QString type = KImageIO::typeForMime (mimetype);
#if DEBUG_KP_DOCUMENT
    kdDebug () << "\tmimetype=" << mimetype << " type=" << type << endl;
#endif
    if (!pixmapWithSelection ().save (filename, type.latin1 ()))
    {
        KMessageBox::error (m_mainWindow,
                            i18n ("Could not save as \"%1\".")
                                .arg (kpDocument::prettyFilenameForURL (url)));
        return false;
    }

    if (!url.isLocalFile ())
    {
        if (!KIO::NetAccess::upload (filename, url, m_mainWindow))
        {
            KMessageBox::error (m_mainWindow,
                                i18n ("Could not save image - failed to upload."));
            return false;
        }
    }

    m_url = url;
    m_mimetype = mimetype;
    m_modified = false;

    emit documentSaved ();
    return true;
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

int kpDocument::width () const
{
    return m_pixmap->width ();
}

int kpDocument::oldWidth () const
{
    return m_oldWidth;
}

void kpDocument::setWidth (int w, const QColor &backgroundColor)
{
    resize (w, height (), backgroundColor);
}

int kpDocument::height () const
{
    return m_pixmap->height ();
}

int kpDocument::oldHeight () const
{
    return m_oldHeight;
}

void kpDocument::setHeight (int h, const QColor &backgroundColor)
{
    resize (width (), h, backgroundColor);
}

QRect kpDocument::rect () const
{
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

    // TODO

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
QPixmap *kpDocument::pixmap () const
{
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
QPixmap kpDocument::pixmapWithSelection () const
{
#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "kpDocument::pixmapWithSelection()" << endl;
#endif

    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    if (vm && vm->selectionActive ())
    {
    #if DEBUG_KP_DOCUMENT && 1
        kdDebug () << "\tselection @ " << vm->tempPixmapRect () << endl;
    #endif
        QPixmap output = *m_pixmap;
        
        QPainter painter (&output);
        painter.drawPixmap (vm->tempPixmapRect (), vm->tempPixmap ());
        painter.end ();
        
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

void kpDocument::fill (const QColor &color)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::fill ()" << endl;
#endif

    if (kpTool::isColorOpaque (color))
    {
        m_pixmap->setMask (QBitmap ());  // no mask = opaque
        m_pixmap->fill (color);
    }
    else
    {
        kpPixmapFX::ensureTransparentAt (m_pixmap, m_pixmap->rect ());
    }

    slotContentsChanged (m_pixmap->rect ());
}

void kpDocument::resize (int w, int h, const QColor &backgroundColor, bool fillNewAreas)
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

#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "\tbefore resize: hasMask=" << (bool) m_pixmap->mask () << endl;
#endif
    m_pixmap->resize (w, h);
#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "\tafter resize: hasMask=" << (bool) m_pixmap->mask () << endl;
#endif

    if (fillNewAreas && (w > m_oldWidth || h > m_oldHeight))
    {
    #if DEBUG_KP_DOCUMENT && 1
        kdDebug () << "\tfilling in new areas" << endl;
    #endif
        QBitmap maskBitmap;
        QPainter painter, maskPainter;
        
        if (kpTool::isColorOpaque (backgroundColor))
        {
            painter.begin (m_pixmap);
            painter.setPen (backgroundColor);
            painter.setBrush (backgroundColor);
        }
        
        if (kpTool::isColorTransparent (backgroundColor) || m_pixmap->mask ())
        {
            maskBitmap = kpPixmapFX::getNonNullMask (*m_pixmap);
            maskPainter.begin (&maskBitmap);
            if (kpTool::isColorTransparent (backgroundColor))
            {
                maskPainter.setPen (Qt::color0/*transparent*/);
                maskPainter.setBrush (Qt::color0/*transparent*/);
            }
            else
            {
                maskPainter.setPen (Qt::color1/*opaque*/);
                maskPainter.setBrush (Qt::color1/*opaque*/);
            }
        }

    #define PAINTER_CALL(cmd)         \
    {                                 \
        if (painter.isActive ())      \
            painter . cmd ;           \
                                      \
        if (maskPainter.isActive ())  \
            maskPainter . cmd ;       \
    }
        if (w > m_oldWidth)
            PAINTER_CALL (drawRect (m_oldWidth, 0, w - m_oldWidth, m_oldHeight));

        if (h > m_oldHeight)
            PAINTER_CALL (drawRect (0, m_oldHeight, w, h - m_oldHeight));
    #undef PAINTER_CALL

        if (maskPainter.isActive ())
            maskPainter.end ();

        if (painter.isActive ())
            painter.end ();

        if (!maskBitmap.isNull ())
            m_pixmap->setMask (maskBitmap);
    }

    slotSizeChanged (width (), height ());
}

bool kpDocument::scale (int w, int h)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::scale (" << w << "," << h << ")" << endl;
#endif

    m_oldWidth = width (), m_oldHeight = height ();

    if (w == m_oldWidth && h == m_oldHeight)
        return true;

#if 0  // slow but smooth, requiring change to QImage (like QPixmap::xForm()?)
    QImage image = (kpPixmapFX::convertToImage (*m_pixmap)).smoothScale (w, h);

    if (!kpPixmapFX::convertToPixmap (image, true/*pretty*/))
    {
        kdError () << "kpDocument::scale() could not convertToPixmap()" << endl;
        return false;
    }
#else
    QWMatrix matrix;
    matrix.scale (double (w) / double (width ()), double (h) / double (height ()));
    
    QPixmap newPixmap = m_pixmap->xForm (matrix);

    *m_pixmap = newPixmap;
#endif

    slotSizeChanged (width (), height ());
    return true;
}

bool kpDocument::skew (double hangle, double vangle, const QColor &backgroundColor)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::skew (" << hangle << "," << vangle << ")" << endl;
#endif

    if (hangle == 0 && vangle == 0)
        return true;  // no-op

    // make sure -90 < hangle/vangle < 90 degrees:
    // if (abs (hangle) >= 90 || abs (vangle) >= 90) {
    if (90 - fabs (hangle) < KP_EPSILON || 90 - fabs (vangle) < KP_EPSILON)
    {
        kdError () << "kpDocument::skew() passed hangle and/or vangle out of range (-90 < x < 90)" << endl;
        return false;
    }

    m_oldWidth = width (), m_oldHeight = height ();

    /* Diagram for completeness :)
     *
     *       |---------- w ----------|
     *     (0,0)
     *  _     _______________________ (w,0)
     *  |    |\~_ va                 |
     *  |    | \ ~_                  |
     *  |    |ha\  ~__               |
     *       |   \    ~__            | dy
     *  h    |    \      ~___        |
     *       |     \         ~___    |
     *  |    |      \            ~___| (w,w*tan(va)=dy)
     *  |    |       \         *     \
     *  _    |________\________|_____|\                                     vertical shear factor
     *     (0,h) dx   ^~_      |       \                                             |
     *                |  ~_    \________\________ General Point (x,y)                V
     *                |    ~__           \        Skewed Point (x + y*tan(ha),y + x*tan(va))
     *      (h*tan(ha)=dx,h)  ~__         \                             ^
     *                           ~___      \                            |
     *                               ~___   \                   horizontal shear factor
     *   Key:                            ~___\
     *    ha = hangle                         (w + h*tan(ha)=w+dx,h + w*tan(va)=w+dy)
     *    va = vangle
     *
     * Skewing really just twists a rectangle into a parallelogram.
     *
     */

    //QWMatrix matrix (1, tan (KP_DEGREES_TO_RADIANS (vangle)), tan (KP_DEGREES_TO_RADIANS (hangle)), 1, 0, 0);
    // I think this is clearer than above :)
    QWMatrix matrix;
    matrix.shear (tan (KP_DEGREES_TO_RADIANS (hangle)),
                  tan (KP_DEGREES_TO_RADIANS (vangle)));

    QRect newRect = matrix.mapRect (rect ());
#if DEBUG_KP_DOCUMENT
    kdDebug () << "\tnewRect=" << newRect << endl;
#endif

    QWMatrix translatedMatrix (matrix.m11 (), matrix.m12 (), matrix.m21 (), matrix.m22 (),
                               -newRect.left (), -newRect.top ());

    QPixmap newPixmap (newRect.width (), newRect.height ());
    QBitmap newBitmapMask;

    if (kpTool::isColorOpaque (backgroundColor))
        newPixmap.fill (backgroundColor);
        
    if (kpTool::isColorTransparent (backgroundColor) || m_pixmap->mask ())
    {
        newBitmapMask.resize (newRect.width (), newRect.height ());
        newBitmapMask.fill (kpTool::isColorTransparent (backgroundColor)
                                ?
                            Qt::color0/*transparent*/
                                :
                            Qt::color1/*opaque*/);
    }
    
    QPainter painter (&newPixmap);
#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "\tmatrix: m11=" << matrix.m11 ()
            << " m12=" << matrix.m12 ()
            << " m21=" << matrix.m21 ()
            << " m22=" << matrix.m22 ()
            << " dx=" << matrix.dx ()
            << " dy=" << matrix.dy ()
            << endl;
#endif
    painter.setWorldMatrix (translatedMatrix);
#if DEBUG_KP_DOCUMENT && 1
    kdDebug () << "\ttranslate top=" << painter.xForm (QPoint (0, 0)) << endl;
    kdDebug () << "\tmatrix: m11=" << painter.worldMatrix ().m11 ()
               << " m12=" << painter.worldMatrix ().m12 ()
               << " m21=" << painter.worldMatrix ().m21 ()
               << " m22=" << painter.worldMatrix ().m22 ()
               << " dx=" << painter.worldMatrix ().dx ()
               << " dy=" << painter.worldMatrix ().dy ()
               << endl;
#endif
    painter.drawPixmap (QPoint (0, 0), *m_pixmap);
    painter.end ();
    
    if (!newBitmapMask.isNull ())
    {
        QPainter maskPainter (&newBitmapMask);
        maskPainter.setWorldMatrix (translatedMatrix);
        maskPainter.drawPixmap (QPoint (0, 0), kpPixmapFX::getNonNullMask (*m_pixmap));
        maskPainter.end ();
        newPixmap.setMask (newBitmapMask);
    }
    
    setPixmap (newPixmap);

    return true;
}

bool kpDocument::flip (bool horz, bool vert)
{
    if (!horz && !vert)
        return true;

    QImage image = (kpPixmapFX::convertToImage (*m_pixmap)).mirror (horz, vert);
    *m_pixmap = kpPixmapFX::convertToPixmap (image);

    slotContentsChanged (m_pixmap->rect ());
    return true;
}

bool kpDocument::rotate (double angle, const QColor &backgroundColor)
{
#if DEBUG_KP_DOCUMENT
    kdDebug () << "kpDocument::rotate (" << angle << ") rect=" << rect () << endl;
#endif

    if (angle == 0) return true;

    if (angle < 0 || angle >= 360)
    {
        kdError () << "kpDocument::rotate() passed angle ! >= 0 && < 360" << endl;
        return false;
    }

    if (isLosslessRotation (angle))
    {
        QPixmap newPixmap;
        
        QWMatrix matrix;
        matrix.rotate (angle);
        
        newPixmap = m_pixmap->xForm (matrix);
            
        setPixmap (newPixmap);
    }
    else
    {
        QWMatrix matrix;
        matrix.rotate (angle);  //360.0 - angle);  // TODO: not counterclockwise???
        
        // calculate size of new pixmap (allowing for rounding error)
        QRect newRect = matrix.mapRect (rect ());
        newRect = QRect (0, 0, newRect.width () + 4, newRect.height () + 4);    
    #if DEBUG_KP_DOCUMENT
        kdDebug () << "\tnewRect=" << newRect << endl;
    #endif
    
        // recalculate matrix - this time rotating old pixmap centred at the
        // middle of the new (probably bigger) pixmap
        matrix.reset ();
        matrix.translate (newRect.width () / 2, newRect.height () / 2);
        matrix.rotate (angle);
        matrix.translate (-newRect.width () / 2, -newRect.height () / 2);

        QRect srcRect ((newRect.width () - m_pixmap->width ()) / 2,
                       (newRect.height () - m_pixmap->height ()) / 2,
                       m_pixmap->width (),
                       m_pixmap->height ());
        
        QPixmap newPixmap (newRect.width (), newRect.height ());
        QBitmap newBitmapMask;
    
        if (kpTool::isColorOpaque (backgroundColor))
            newPixmap.fill (backgroundColor);
            
        if (kpTool::isColorTransparent (backgroundColor) || m_pixmap->mask ())
        {
            newBitmapMask.resize (newRect.width (), newRect.height ());
            newBitmapMask.fill (kpTool::isColorTransparent (backgroundColor)
                                    ?
                                Qt::color0/*transparent*/
                                    :
                                Qt::color1/*opaque*/);
        }
    
        QPoint drawPoint = srcRect.topLeft ();
    
        QPainter painter (&newPixmap);
        painter.setWorldMatrix (matrix);
    #if DEBUG_KP_DOCUMENT
        kdDebug () << "\tsrcRect=" << srcRect << endl;
    #endif
        QRect destRect = painter.xForm (srcRect);
    #if DEBUG_KP_DOCUMENT
        kdDebug () << "\tdestRect=" << destRect << endl;
    #endif

        painter.drawPixmap (drawPoint, *m_pixmap);
        painter.end ();
    
        if (!newBitmapMask.isNull ())
        {
            QPainter maskPainter (&newBitmapMask);
            maskPainter.setWorldMatrix (matrix);
            maskPainter.drawPixmap (drawPoint, kpPixmapFX::getNonNullMask (*m_pixmap));
            maskPainter.end ();
            newPixmap.setMask (newBitmapMask);
        }
    
        // get rid of extra border (that allowed for rounding errors)
        setPixmap (kpPixmapFX::getPixmapAt (newPixmap, destRect));
    }

    return true;
}

// static
bool kpDocument::isLosslessRotation (double angle)
{
    return (qRound (angle) % 90 == 0);
}

static QRgb toGray (QRgb rgb)
{
    // naive way that doesn't preserve brightness
    // int gray = (qRed (rgb) + qGreen (rgb) + qBlue (rgb)) / 3;
            
    // over-exaggerates red & blue
    // int gray = qGray (rgb);

    int gray = (212671 * qRed (rgb) + 715160 * qGreen (rgb) + 72169 * qBlue (rgb)) / 1000000;
    return qRgba (gray, gray, gray, qAlpha (rgb));
}

bool kpDocument::convertToGrayscale ()
{
    QImage image = kpPixmapFX::convertToImage (*m_pixmap);

    if (image.depth () > 8)
    {
        // hmm, why not just write to the pixmap directly???

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                image.setPixel (x, y, toGray (image.pixel (x, y)));
            }
        }
    }
    else
    {
        // 1- & 8- bit images use a color table
        for (int i = 0; i < image.numColors (); i++)
            image.setColor (i, toGray (image.color (i)));
    }

    *m_pixmap = kpPixmapFX::convertToPixmap (image, true/*pretty*/);

    slotContentsChanged (m_pixmap->rect ());
    return true;
}

bool kpDocument::invertColors ()
{
    kpPixmapFX::invertColors (m_pixmap);

    slotContentsChanged (m_pixmap->rect ());
    return true;
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
}

#include <kpdocument.moc>
