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

#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "kpDefs.h"
#include "environments/document/kpDocumentEnvironment.h"
#include "document/kpDocumentSaveOptions.h"
#include "imagelib/kpDocumentMetaInfo.h"
#include "imagelib/effects/kpEffectReduceColors.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "lgpl/generic/kpUrlFormatter.h"
#include "views/manager/kpViewManager.h"


#include <QColor>
#include <QImage>
#include <QMimeDatabase>
#include <QImageReader>
#include <QBuffer>

#include <KJobWidgets>
#include "kpLogCategories.h"
#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KMessageBox>

//---------------------------------------------------------------------

void kpDocument::getDataFromImage(const QImage &image,
                                  kpDocumentSaveOptions &saveOptions,
                                  kpDocumentMetaInfo &metaInfo)
{
  saveOptions.setColorDepth(image.depth());
  saveOptions.setDither(false);  // avoid double dithering when saving

  metaInfo.setDotsPerMeterX(image.dotsPerMeterX());
  metaInfo.setDotsPerMeterY(image.dotsPerMeterY());
  metaInfo.setOffset(image.offset());

  QStringList keys = image.textKeys();
  for (int i = 0; i < keys.count(); i++) {
      metaInfo.setText(keys[i], image.text(keys[i]));
  }
}

//---------------------------------------------------------------------

// public static
QImage kpDocument::getPixmapFromFile(const QUrl &url, bool suppressDoesntExistDialog,
                                     QWidget *parent,
                                     kpDocumentSaveOptions *saveOptions,
                                     kpDocumentMetaInfo *metaInfo)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::getPixmapFromFile(" << url << "," << parent << ")";
#endif

    if (saveOptions) {
        *saveOptions = kpDocumentSaveOptions ();
    }

    if (metaInfo) {
        *metaInfo = kpDocumentMetaInfo ();
    }

    if (url.isEmpty ()) {
        return {};
    }

    KIO::StoredTransferJob *job = KIO::storedGet (url);
    KJobWidgets::setWindow(job, parent);

    if (!job->exec())
    {
        if (!suppressDoesntExistDialog)
        {
            // TODO: Use "Cannot" instead of "Could not" in all dialogs in KolourPaint.
            //       Or at least choose one consistently.
            //
            // TODO: Have captions for all dialogs in KolourPaint.
            KMessageBox::sorry (parent,
                                i18n ("Could not open \"%1\".",
                                      kpUrlFormatter::PrettyFilename (url)));
        }

        return {};
    }
    QByteArray data = job->data();

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFileNameAndData(url.fileName(), data);

    if (saveOptions) {
        saveOptions->setMimeType(mimeType.name());
    }

#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tmimetype=" << mimeType.name();
    qCDebug(kpLogDocument) << "\tsrc=" << url.path ();
#endif

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);

    // Do *NOT* convert to
    // QImage image = reader.read();
    // this variant is more lenient on errors and we may get something that we would not otherwise
    // e.g. image from https://bugs.kde.org/show_bug.cgi?id=441554
    QImage image;
    reader.read(&image);

    if (image.isNull ())
    {
        KMessageBox::sorry (parent,
                            i18n ("Could not open \"%1\" - unsupported image format.\n"
                                  "The file may be corrupt.",
                                  kpUrlFormatter::PrettyFilename (url)));
        return {};
    }

#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tpixmap: depth=" << image.depth ()
                << " hasAlphaChannel=" << image.hasAlphaChannel ();
#endif

    if ( saveOptions && metaInfo ) {
        getDataFromImage(image, *saveOptions, *metaInfo);
    }

    // make sure we always have Format_ARGB32_Premultiplied as this is the fastest to draw on
    // and Qt can not draw onto Format_Indexed8 (Qt-4.7)
    if ( image.format() != QImage::Format_ARGB32_Premultiplied ) {
      image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    return image;
}

//---------------------------------------------------------------------

void kpDocument::openNew (const QUrl &url)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::openNew (" << url << ")";
#endif

    m_image->fill(QColor(Qt::white).rgb());

    setURL (url, false/*not from url*/);

    *m_saveOptions = kpDocumentSaveOptions ();

    if ( !url.isEmpty() )
    {
      //  guess the mimetype from url's filename extension.
      //
      //  That way "kolourpaint doesnotexist.bmp" automatically
      //  selects the BMP file format when the save dialog comes up for
      //  the first time.

      QMimeDatabase mimeDb;
      m_saveOptions->setMimeType(mimeDb.mimeTypeForUrl(url).name());
    }

    *m_metaInfo = kpDocumentMetaInfo ();
    m_modified = false;

    emit documentOpened ();
}

//---------------------------------------------------------------------

bool kpDocument::open (const QUrl &url, bool newDocSameNameIfNotExist)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::open (" << url << ")";
#endif

    kpDocumentSaveOptions newSaveOptions;
    kpDocumentMetaInfo newMetaInfo;
    QImage newPixmap = kpDocument::getPixmapFromFile (url,
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
        if (urlExists (url)) // not just a permission error?
        {
            openNew (url);
        }
        else
        {
            openNew (QUrl ());
        }

        return true;
    }

    return false;

}

//---------------------------------------------------------------------
