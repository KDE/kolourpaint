
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
#include <kpViewManager.h>


QPixmap kpDocument::convertToPixmapAsLosslessAsPossible (
        const QImage &image,
        const kpPixmapFX::WarnAboutLossInfo &wali,

        kpDocumentSaveOptions *saveOptions,
        kpDocumentMetaInfo *metaInfo)
{
    if (image.isNull ())
        return QPixmap ();

#if DEBUG_KP_DOCUMENT
    kDebug () << "\timage: depth=" << image.depth ()
                << " (X display=" << QPixmap::defaultDepth () << ")"
                << " hasAlphaChannel=" << image.hasAlphaChannel ()
                << endl;
#endif

    if (saveOptions)
    {
        saveOptions->setColorDepth (image.depth ());
        saveOptions->setDither (false);  // avoid double dithering when saving
    }

    if (metaInfo)
    {
        metaInfo->setDotsPerMeterX (image.dotsPerMeterX ());
        metaInfo->setDotsPerMeterY (image.dotsPerMeterY ());
        metaInfo->setOffset (image.offset ());

        QList <QString> keyList = image.textKeys ();
        for (QList <QString>::const_iterator it = keyList.begin ();
             it != keyList.end ();
             it++)
        {
            metaInfo->setText (*it, image.text (*it));
        }

    #if DEBUG_KP_DOCUMENT || 1
        metaInfo->printDebug ("\tmetaInfo");
    #endif
    }

#if DEBUG_KP_DOCUMENT && 1
{
    if (image.width () <= 16 && image.height () <= 16)
    {
        kDebug () << "Image dump:";

        for (int y = 0; y < image.height (); y++)
        {
            for (int x = 0; x < image.width (); x++)
            {
                const QRgb rgb = image.pixel (x, y);
                fprintf (stderr, " %08X", rgb);
            }
            fprintf (stderr, "\n");
        }
    }
}
#endif


    QPixmap newPixmap = kpPixmapFX::convertToPixmapAsLosslessAsPossible (image, wali);


#if DEBUG_KP_DOCUMENT && 1
{
    const QImage image2 = kpPixmapFX::convertToImage (newPixmap);
    kDebug () << "(Converted to pixmap) Image dump:";

    bool differsFromOrgImage = false;
    unsigned long hash = 0;
    int numDiff = 0;
    for (int y = 0; y < image2.height (); y++)
    {
        for (int x = 0; x < image2.width (); x++)
        {
            const QRgb rgb = image2.pixel (x, y);
            hash += ((x % 2) + 1) * rgb;
            if (rgb != image.pixel (x, y))
            {
                differsFromOrgImage = true;
                numDiff++;
            }
            if (image2.width () <= 16 && image2.height () <= 16)
                fprintf (stderr, " %08X", rgb);
        }
        if (image2.width () <= 16 && image2.height () <= 16)
            fprintf (stderr, "\n");
    }

    kDebug () << "\tdiffersFromOrgImage="
               << differsFromOrgImage
               << " numDiff="
               << numDiff
               << " hash=" << hash << endl;
}
#endif

    return newPixmap;
}

// public static
QPixmap kpDocument::getPixmapFromFile (const KUrl &url, bool suppressDoesntExistDialog,
                                       QWidget *parent,
                                       kpDocumentSaveOptions *saveOptions,
                                       kpDocumentMetaInfo *metaInfo)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::getPixmapFromFile(" << url << "," << parent << ")";
#endif

    if (saveOptions)
        *saveOptions = kpDocumentSaveOptions ();

    if (metaInfo)
        *metaInfo = kpDocumentMetaInfo ();


    QString tempFile;
    if (url.isEmpty () || !KIO::NetAccess::download (url, tempFile, parent))
    {
        if (!suppressDoesntExistDialog)
        {
            // TODO: Use "Cannot" instead of "Could not" in all dialogs in KolourPaint.
            //       Or at least choose one consistently.
            //
            // TODO: Have captions for all dialogs in KolourPaint.
            KMessageBox::sorry (parent,
                                i18n ("Could not open \"%1\".",
                                      kpDocument::prettyFilenameForURL (url)));
        }

        return QPixmap ();
    }


    QImage image;

    // sync: remember to "KIO::NetAccess::removeTempFile (tempFile)" in all exit paths
    {
    #if 0
        QString detectedMimeType = KImageIO::mimeType (tempFile);
    #else  // COMPAT: this is wrong - should be what QImage::QImage() loaded file as
        KMimeType::Ptr detectedMimeTypePtr = KMimeType::findByFileContent (tempFile);
        QString detectedMimeType =
            detectedMimeTypePtr != KMimeType::defaultMimeTypePtr () ?
                detectedMimeTypePtr->name () :
                QString::null;
    #endif
        if (saveOptions)
            saveOptions->setMimeType (detectedMimeType);

    #if DEBUG_KP_DOCUMENT
        kDebug () << "\ttempFile=" << tempFile;
        kDebug () << "\tmimetype=" << detectedMimeType;
        kDebug () << "\tsrc=" << url.path ();
        // COMPAT kDebug () << "\tmimetype of src=" << KImageIO::mimeType (url.path ());
    #endif

        if (detectedMimeType.isEmpty ())
        {
            KMessageBox::sorry (parent,
                                i18n ("Could not open \"%1\" - unknown mimetype.",
                                    kpDocument::prettyFilenameForURL (url)));
            KIO::NetAccess::removeTempFile (tempFile);
            return QPixmap ();
        }


        image = QImage (tempFile);
        KIO::NetAccess::removeTempFile (tempFile);
    }

    if (image.isNull ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - unsupported image format.\n"
                                  "The file may be corrupt.",
                                  kpDocument::prettyFilenameForURL (url)));
        return QPixmap ();
    }

    const QPixmap newPixmap = kpDocument::convertToPixmapAsLosslessAsPossible (image,
        kpPixmapFX::WarnAboutLossInfo (
             ki18n ("The image \"%1\""
                   " may have more colors than the current screen mode."
                   " In order to display it, some colors may be changed."
                   " Try increasing your screen depth to at least %2bpp."

                   "\nIt also"

                   " contains translucency which is not fully"
                   " supported. The translucency data will be"
                   " approximated with a 1-bit transparency mask.")
                   .subs (prettyFilenameForURL (url)),
             ki18n ("The image \"%1\""
                   " may have more colors than the current screen mode."
                   " In order to display it, some colors may be changed."
                   " Try increasing your screen depth to at least %2bpp.")
                   .subs (prettyFilenameForURL (url)),
             i18n ("The image \"%1\""
                   " contains translucency which is not fully"
                   " supported. The translucency data will be"
                   " approximated with a 1-bit transparency mask.",
                   prettyFilenameForURL (url)),
            "docOpen",
            parent),
        saveOptions,
        metaInfo);

    if (newPixmap.isNull ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - out of graphics memory.",
                                  kpDocument::prettyFilenameForURL (url)));
        return QPixmap ();
    }

#if DEBUG_KP_DOCUMENT
    kDebug () << "\tpixmap: depth=" << newPixmap.depth ()
                << " hasAlphaChannelOrMask=" << newPixmap.hasAlpha ()
                << " hasAlphaChannel=" << newPixmap.hasAlphaChannel ()
                << endl;
#endif


    return newPixmap;
}

void kpDocument::openNew (const KUrl &url)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::openNew (" << url << ")";
#endif

    m_image->fill (Qt::white);

    setURL (url, false/*not from url*/);
    // TODO: Maybe we should guess the mimetype from "url"'s filename
    //       extension.
    //
    //       That way "kolourpaint doesnotexist.bmp" would automatically
    //       select the BMP file format when the save dialog comes up for
    //       the first time.
    *m_saveOptions = kpDocumentSaveOptions ();
    *m_metaInfo = kpDocumentMetaInfo ();
    m_modified = false;

    emit documentOpened ();
}

bool kpDocument::open (const KUrl &url, bool newDocSameNameIfNotExist)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::open (" << url << ")";
#endif

    kpDocumentSaveOptions newSaveOptions;
    kpDocumentMetaInfo newMetaInfo;
    QPixmap newPixmap = kpDocument::getPixmapFromFile (url,
        newDocSameNameIfNotExist/*suppress "doesn't exist" dialog*/,
        d->environ->dialogParent (),
        &newSaveOptions,
        &newMetaInfo);

    if (!newPixmap.isNull ())
    {
        delete m_image;
        m_image = new kpImage (newPixmap);

        setURL (url, true/*is from url*/);
        *m_saveOptions = newSaveOptions;
        *m_metaInfo = newMetaInfo;
        m_modified = false;

        emit documentOpened ();
        return true;
    }

    if (newDocSameNameIfNotExist)
    {
        if (!url.isEmpty () &&
            // not just a permission error?
            !KIO::NetAccess::exists (url, KIO::NetAccess::SourceSide/*open*/, d->environ->dialogParent ()))
        {
            openNew (url);
        }
        else
        {
            openNew (KUrl ());
        }

        return true;
    }
    else
    {
        return false;
    }
}

