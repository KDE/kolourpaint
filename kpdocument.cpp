
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

#include <math.h>

#include <qcolor.h>
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

#include <kpdefs.h>
#include <kpdocument.h>

#define DEBUG_KPDOCUMENT 1


kpDocument::kpDocument ()
    : m_oldWidth (-1), m_oldHeight (-1),
      m_colorDepth (-1), m_oldColorDepth (-1),
      m_modified (false)
{
    m_pixmap = new QPixmap ();
}

kpDocument::kpDocument (int w, int h, int colorDepth)
    : m_oldWidth (-1), m_oldHeight (-1),
      m_colorDepth (colorDepth), m_oldColorDepth (-1),
      m_modified (false)
{
#if DEBUG_KPDOCUMENT
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
#if DEBUG_KPDOCUMENT
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
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::open (" << url << ")" << endl;
#endif

    if (url.isEmpty ())
    {
        openNew (url);
        return true;
    }

    QString tempFile;
    if (KIO::NetAccess::download (url, tempFile))
    {
        QString mimetype = KImageIO::mimeType (tempFile);

    #if DEBUG_KPDOCUMENT
        kdDebug () << "\tmimetype=" << mimetype << endl;
    #endif

        if (mimetype.isNull ())
        {
            KMessageBox::sorry (0, i18n ("Could not open \"%1\" - unsupported image format")
                                       .arg (kpDocument::filenameForURL (url)));
            return false;
        }

        QPixmap *newPixmap = new QPixmap (tempFile);
        if (newPixmap->isNull ())
        {
            KMessageBox::sorry (0, i18n ("Could not open \"%1\" - unsupported image format")
                                       .arg (kpDocument::filenameForURL (url)));
            delete newPixmap;
            return false;
        }

        KIO::NetAccess::removeTempFile (tempFile);

        delete m_pixmap;
        m_pixmap = newPixmap;

        m_url = url;
        m_mimetype = mimetype;
        m_modified = false;

        emit documentOpened ();
        return true;
    }
    else
    {
        if (newDocSameNameIfNotExist && !KIO::NetAccess::exists (url))
        {
            openNew (url);
            return true;
        }
        else
        {
            KMessageBox::sorry (0, i18n ("Could not open \"%1\".")
                                    .arg (kpDocument::filenameForURL (url)));
            return false;
        }
    }
}

bool kpDocument::save ()
{
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::save [" << m_url << "," << m_mimetype << "]" << endl;
#endif

    if (m_url.isEmpty () || m_mimetype.isEmpty ())
    {
        KMessageBox::detailedError (0, i18n ("Could not save image - insufficient information."),
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
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::saveAs (" << url << "," << mimetype << ")" << endl;
#endif

    if (overwritePrompt && KIO::NetAccess::exists (url))
    {
        int result = KMessageBox::warningContinueCancel (0,
            i18n ("A document called \"%1\" already exists.\n"
                  "Do you want to overwrite it?")
                .arg (filenameForURL (url)),
            QString::null,
            i18n ("Overwrite"));

        if (result != KMessageBox::Continue)
        {
        #if DEBUG_KPDOCUMENT
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
            KMessageBox::error (0, i18n ("Could not save image - unable to create temporary file."));
            return false;
        }
    }
    else
        filename = url.path ();

    QString type = KImageIO::typeForMime (mimetype);
#if DEBUG_KPDOCUMENT
    kdDebug () << "\tmimetype=" << mimetype << " type=" << type << endl;
#endif
    if (!m_pixmap->save (filename, type.latin1 ()))
    {
        KMessageBox::error (0, i18n ("Could not save image as type <b>%1 (%2)</b>.").arg (mimetype).arg (type));
        return false;
    }

    if (!url.isLocalFile ())
    {
        if (!KIO::NetAccess::upload (filename, url))
        {
            KMessageBox::error (0, i18n ("Could not save image - failed to upload.."));
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
QString kpDocument::filenameForURL (const KURL &url)
{
    if (url.isEmpty ())
        return i18n ("Untitled");
    else
        return url.fileName ();
}

QString kpDocument::filename () const
{
    return filenameForURL (m_url);
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

void kpDocument::setWidth (int w)
{
    resize (w, height ());
}

int kpDocument::height () const
{
    return m_pixmap->height ();
}

int kpDocument::oldHeight () const
{
    return m_oldHeight;
}

void kpDocument::setHeight (int h)
{
    resize (width (), h);
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

// TODO: implement colorDepth properly
#if 0
    // pixmap --> image
    QImage image = m_pixmap->convertToImage ();

    // convert depth
    QImage imageNewDepth = image.convertDepth (depth);
    if (imageNewDepth.isNull ())
    {
        kdWarning () << "Could not convert depth from "
                     << oldDepth << " to " << depth << endl;
        return false;
    }

    // image --> pixmap
    if (!m_pixmap->convertFromImage (imageNewDepth))
    {
        kdWarning () << "Could not convert image back to pixmap" << endl;
        return false;
    }
#endif

    emit colorDepthChanged (colorDepth ());
    return true;
}


/*
 * Pixmap access
 */

QPixmap kpDocument::getPixmapAt (const QRect &rect) const
{
    QPixmap pixmap (rect.width (), rect.height ());
    QPainter painter (&pixmap);

    painter.drawPixmap (QPoint (0, 0), *m_pixmap, rect);

    return pixmap;
}

void kpDocument::setPixmapAt (const QPixmap &pixmap, const QPoint &at)
{
    kdDebug () << "kpDocument::setPixmapAt (pixmap (w="
               << pixmap.width ()
               << ",h=" << pixmap.height ()
               << "), x=" << at.x ()
               << ",y=" << at.y ()
               << endl;

    QPainter painter (m_pixmap);
    painter.drawPixmap (at, pixmap);

    slotContentsChanged (QRect (at.x (), at.y (), pixmap.width (), pixmap.height ()));
}

QPixmap *kpDocument::pixmap () const
{
    return m_pixmap;
}

void kpDocument::setPixmap (const QPixmap &pixmap)
{
    m_oldWidth = width (), m_oldHeight = height ();

    *m_pixmap = pixmap;

    if (m_oldWidth == width () && m_oldHeight == height ())
        slotContentsChanged (pixmap.rect ());
    else
        slotSizeChanged (width (), height ());
}


/*
 * Transformations
 */

void kpDocument::fill (const QColor &color)
{
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::fill ()" << endl;
#endif

    m_pixmap->fill (color);

    slotContentsChanged (m_pixmap->rect ());
}

void kpDocument::resize (int w, int h, bool fillNewAreas)
{
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::resize (" << w << "," << h << "," << fillNewAreas << ")" << endl;
#endif

    m_oldWidth = width (), m_oldHeight = height ();

    if (w == m_oldWidth && h == m_oldHeight)
        return;

    m_pixmap->resize (w, h);

    if (fillNewAreas)
    {
        QPainter painter (m_pixmap);
        QBrush brush (KP_BLANK_DOCUMENT_COLOR);

        if (w > m_oldWidth)
            painter.fillRect (m_oldWidth, 0, w - m_oldWidth, m_oldHeight, brush);

        if (h > m_oldHeight)
            painter.fillRect (0, m_oldHeight, w, h - m_oldHeight, brush);
    }

    slotSizeChanged (width (), height ());
}

bool kpDocument::scale (int w, int h)
{
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::scale (" << w << "," << h << ")" << endl;
#endif

    m_oldWidth = width (), m_oldHeight = height ();

    if (w == m_oldWidth && h == m_oldHeight)
        return true;

#if 0  // slow but smooth, requiring change to QImage (like QPixmap::xForm()?)
    QImage image = (m_pixmap->convertToImage ()).smoothScale (w, h);

    if (!m_pixmap->convertFromImage (image))
    {
        kdError () << "kpDocument::scale() could not convertFromImage()" << endl;
        return false;
    }
#else
    QPixmap *newPixmap = new QPixmap (w, h);
    QPainter painter;

    painter.begin (newPixmap);
    painter.scale (double (w) / double (width ()), double (h) / double (height ()));
    painter.drawPixmap (0, 0, *m_pixmap);
    painter.end ();

    delete m_pixmap;
    m_pixmap = newPixmap;
#endif

    slotSizeChanged (width (), height ());
    return true;
}

bool kpDocument::skew (double hangle, double vangle)
{
#if DEBUG_KPDOCUMENT
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
    *m_pixmap = m_pixmap->xForm (matrix);

    if (m_oldWidth == width () && m_oldHeight == height ())
        slotContentsChanged (rect ());
    else
        slotSizeChanged (width (), height ());
    return true;
}

bool kpDocument::flip (bool horz, bool vert)
{
    if (!horz && !vert)
        return true;

    QImage image = (m_pixmap->convertToImage ()).mirror (horz, vert);

    if (!m_pixmap->convertFromImage (image))
    {
        kdError () << "kpDocument::flip() could not convertFromImage()" << endl;
        return false;
    }

    slotContentsChanged (m_pixmap->rect ());
    return true;
}

bool kpDocument::rotate (double angle)
{
#if DEBUG_KPDOCUMENT
    kdDebug () << "kpDocument::rotate (" << angle << ")" << endl;
#endif

    if (angle == 0) return true;

    if (angle < 0 || angle >= 360)
    {
        kdError () << "kpDocument::rotate() passed angle ! >= 0 && < 360" << endl;
        return false;
    }

    m_oldWidth = width (), m_oldHeight = height ();

    QWMatrix matrix;
    matrix.rotate (angle);  //360.0 - angle);  // TODO: not counterclockwise???
    *m_pixmap = m_pixmap->xForm (matrix);

/*    QRect newRect = matrix.mapRect (QRect (0, 0, width (), height ()));

    QPixmap *newPixmap = new QPixmap (newRect.width (), newRect.height ());
    newPixmap->fill (KP_BLANK_DOCUMENT_COLOR);

    QPainter painter;
    painter.begin (newPixmap);
    painter.setWorldMatrix (matrix);
    painter.drawPixmap (QPoint (0, 0), *m_pixmap);
    painter.end ();

    delete m_pixmap;
    m_pixmap = newPixmap;*/

    if (m_oldWidth == width () && m_oldHeight == height ())
        slotContentsChanged (rect ());
    else
        slotSizeChanged (width (), height ());
    return true;
}

// static
bool kpDocument::isLosslessRotation (double angle)
{
    int closest90 = int (angle) / 90 * 90;

    // this should handle things like 89.9 and 90.1 correctly
    return (fabs (double (closest90) - angle) < KP_EPSILON ||
            fabs (double (closest90 + 90) - angle) < KP_EPSILON);
}

bool kpDocument::convertToGrayscale ()
{
    QImage image = m_pixmap->convertToImage ();

    for (int y = 0; y < image.height (); y++)
    {
        for (int x = 0; x < image.width (); x++)
        {
            QRgb rgb = image.pixel (x, y);

            // int gray = (qRed (rgb) + qGreen (rgb) + qBlue (rgb)) / 3;  // naive way that doesn't preserve brightness
            // int gray = qGray (rgb);  // over-exaggerates red & blue
            int gray = (212671 * qRed (rgb) + 715160 * qGreen (rgb) + 72169 * qBlue (rgb)) / 1000000;
            image.setPixel (x, y, qRgb (gray, gray, gray));
        }
    }

    if (!m_pixmap->convertFromImage (image))
    {
        kdError () << "kpDocument::convertToGrayscale() could not convertFromImage()" << endl;
        return false;
    }

    slotContentsChanged (m_pixmap->rect ());
    return true;
}

bool kpDocument::invertColors ()
{
     QImage image = m_pixmap->convertToImage ();

     image.invertPixels ();

     if (!m_pixmap->convertFromImage (image))
     {
         kdError () << "kpDocument::invertColors() could not convertFromImage()" << endl;
         return false;
     }

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
