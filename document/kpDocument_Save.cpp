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


#include <QColor>
#include <QBitmap>
#include <QBrush>
#include <QFile>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QRect>
#include <QSaveFile>
#include <QSize>
#include <QTemporaryFile>
#include <QTransform>
#include <QMimeDatabase>

#include "kpLogCategories.h"
#include <KJobWidgets>
#include <KIO/FileCopyJob>
#include <KLocalizedString>
#include <KMessageBox>

#include "imagelib/kpColor.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "kpDefs.h"
#include "environments/document/kpDocumentEnvironment.h"
#include "document/kpDocumentSaveOptions.h"
#include "imagelib/kpDocumentMetaInfo.h"
#include "imagelib/effects/kpEffectReduceColors.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "lgpl/generic/kpUrlFormatter.h"
#include "views/manager/kpViewManager.h"


bool kpDocument::save (bool lossyPrompt)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::save("
               << ",lossyPrompt=" << lossyPrompt
               << ") url=" << m_url
               << " savedAtLeastOnceBefore=" << savedAtLeastOnceBefore ();
#endif

    // TODO: check feels weak
    if (m_url.isEmpty () || m_saveOptions->mimeType ().isEmpty ())
    {
        KMessageBox::detailedError (d->environ->dialogParent (),
            i18n ("Could not save image - insufficient information."),
            i18n ("URL: %1\n"
                  "Mimetype: %2",
                  prettyUrl (),
                  m_saveOptions->mimeType ().isEmpty () ?
                          i18n ("<empty>") :
                          m_saveOptions->mimeType ()),
            i18nc ("@title:window", "Internal Error"));
        return false;
    }

    return saveAs (m_url, *m_saveOptions,
                   lossyPrompt);
}

//---------------------------------------------------------------------

// public static
bool kpDocument::lossyPromptContinue (const QImage &pixmap,
                                      const kpDocumentSaveOptions &saveOptions,
                                      QWidget *parent)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::lossyPromptContinue()";
#endif

#define QUIT_IF_CANCEL(messageBoxCommand)            \
{                                                    \
    if (messageBoxCommand != KMessageBox::Continue)  \
    {                                                \
        return false;                                \
    }                                                \
}

    const int lossyType = saveOptions.isLossyForSaving (pixmap);
    if (lossyType & (kpDocumentSaveOptions::MimeTypeMaximumColorDepthLow |
                     kpDocumentSaveOptions::Quality))
    {
        QMimeDatabase db;

        QUIT_IF_CANCEL (
            KMessageBox::warningContinueCancel (parent,
                i18n ("<qt><p>The <b>%1</b> format may not be able"
                      " to preserve all of the image's color information.</p>"

                      "<p>Are you sure you want to save in this format?</p></qt>",
                      db.mimeTypeForName(saveOptions.mimeType()).comment()),
                // TODO: caption misleading for lossless formats that have
                //       low maximum colour depth
                i18nc ("@title:window", "Lossy File Format"),
                KStandardGuiItem::save (),
                KStandardGuiItem::cancel(),
                QLatin1String ("SaveInLossyMimeTypeDontAskAgain")));
    }
    else if (lossyType & kpDocumentSaveOptions::ColorDepthLow)
    {
        QUIT_IF_CANCEL (
            KMessageBox::warningContinueCancel (parent,
                i18n ("<qt><p>Saving the image at the low color depth of %1-bit"
                        " may result in the loss of color information."

                        // TODO: It looks like 8-bit QImage's now support alpha.
                        //       Update kpDocumentSaveOptions::isLossyForSaving()
                        //       and change "might" to "will".
                        " Any transparency might also be removed.</p>"

                        "<p>Are you sure you want to save at this color depth?</p></qt>",
                      saveOptions.colorDepth ()),
                i18nc ("@title:window", "Low Color Depth"),
                KStandardGuiItem::save (),
                KStandardGuiItem::cancel(),
                QLatin1String ("SaveAtLowColorDepthDontAskAgain")));
    }
#undef QUIT_IF_CANCEL

    return true;
}

//---------------------------------------------------------------------

// public static
bool kpDocument::savePixmapToDevice (const QImage &image,
                                     QIODevice *device,
                                     const kpDocumentSaveOptions &saveOptions,
                                     const kpDocumentMetaInfo &metaInfo,
                                     bool lossyPrompt,
                                     QWidget *parent,
                                     bool *userCancelled)
{
    if (userCancelled)
        *userCancelled = false;

    QString type = QMimeDatabase().mimeTypeForName (saveOptions.mimeType ()).preferredSuffix ();
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tmimeType=" << saveOptions.mimeType ()
               << " type=" << type;
#endif
    if (type.isEmpty ())
        return false;

    if (lossyPrompt && !lossyPromptContinue (image, saveOptions, parent))
    {
        if (userCancelled)
            *userCancelled = true;

    #if DEBUG_KP_DOCUMENT
        qCDebug(kpLogDocument) << "\treturning false because of lossyPrompt";
    #endif
        return false;
    }


    // TODO: fix dup with kpDocumentSaveOptions::isLossyForSaving()
    const bool useSaveOptionsColorDepth =
        (saveOptions.mimeTypeHasConfigurableColorDepth () &&
         !saveOptions.colorDepthIsInvalid ());

    const bool useSaveOptionsQuality =
        (saveOptions.mimeTypeHasConfigurableQuality () &&
         !saveOptions.qualityIsInvalid ());


    //
    // Reduce colors if required
    //

#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tuseSaveOptionsColorDepth=" << useSaveOptionsColorDepth
              << "current image depth=" << image.depth ()
              << "save options depth=" << saveOptions.colorDepth ();
#endif
    QImage imageToSave(image);

    if (useSaveOptionsColorDepth &&
        imageToSave.depth () != saveOptions.colorDepth ())
    {
        // TODO: I think this erases the mask!
        //
        //       I suspect this doesn't matter since this is only called to
        //       reduce color depth and QImage's with depth < 32 don't
        //       support masks anyway.
        //
        //       Later: I think the mask is preserved for 8-bit since Qt4
        //              seems to support it for QImage.
        imageToSave = kpEffectReduceColors::convertImageDepth (imageToSave,
                                           saveOptions.colorDepth (),
                                           saveOptions.dither ());
    }


    //
    // Write Meta Info
    //

    imageToSave.setDotsPerMeterX (metaInfo.dotsPerMeterX ());
    imageToSave.setDotsPerMeterY (metaInfo.dotsPerMeterY ());
    imageToSave.setOffset (metaInfo.offset ());

    foreach (const QString &key, metaInfo.textKeys())
      imageToSave.setText(key, metaInfo.text(key));

    //
    // Save at required quality
    //

    int quality = -1;  // default

    if (useSaveOptionsQuality)
      quality = saveOptions.quality();

#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tsaving";
#endif
    if (!imageToSave.save (device, type.toLatin1 (), quality))
    {
    #if DEBUG_KP_DOCUMENT
        qCDebug(kpLogDocument) << "\tQImage::save() returned false";
    #endif
        return false;
    }


#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "\tsave OK";
#endif
    return true;
}

//---------------------------------------------------------------------

static void CouldNotCreateTemporaryFileDialog (QWidget *parent)
{
    KMessageBox::error (parent,
                        i18n ("Could not save image - unable to create temporary file."));
}

//---------------------------------------------------------------------

static void CouldNotSaveDialog (const QUrl &url, const QString &error, QWidget *parent)
{
    KMessageBox::error (parent,
                        i18n ("Could not save as \"%1\": %2",
                              kpUrlFormatter::PrettyFilename (url),
                              error));
}

//---------------------------------------------------------------------

// public static
bool kpDocument::savePixmapToFile (const QImage &pixmap,
                                   const QUrl &url,
                                   const kpDocumentSaveOptions &saveOptions,
                                   const kpDocumentMetaInfo &metaInfo,
                                   bool lossyPrompt,
                                   QWidget *parent)
{
    // TODO: Use KIO::NetAccess:mostLocalURL() for accessing home:/ (and other
    //       such local URLs) for efficiency and because only local writes
    //       are atomic.
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::savePixmapToFile ("
               << url
               << ",lossyPrompt=" << lossyPrompt
               << ")";
    saveOptions.printDebug (QLatin1String ("\tsaveOptions"));
    metaInfo.printDebug (QLatin1String ("\tmetaInfo"));
#endif

    if (lossyPrompt && !lossyPromptContinue (pixmap, saveOptions, parent))
    {
    #if DEBUG_KP_DOCUMENT
        qCDebug(kpLogDocument) << "\treturning false because of lossyPrompt";
    #endif
        return false;
    }


    // Local file?
    if (url.isLocalFile ())
    {
        const QString filename = url.toLocalFile ();

        // sync: All failure exit paths _must_ call QSaveFile::cancelWriting() or
        //       else, the QSaveFile destructor will overwrite the file,
        //       <filename>, despite the failure.
        QSaveFile atomicFileWriter (filename);
        {
            if (!atomicFileWriter.open (QIODevice::WriteOnly))
            {
                // We probably don't need this as <filename> has not been
                // opened.
                atomicFileWriter.cancelWriting ();

            #if DEBUG_KP_DOCUMENT
                qCDebug(kpLogDocument) << "\treturning false because could not open QSaveFile"
                          << " error=" << atomicFileWriter.error () << endl;
            #endif
                ::CouldNotCreateTemporaryFileDialog (parent);
                return false;
            }

            // Write to local temporary file.
            if (!savePixmapToDevice (pixmap, &atomicFileWriter,
                                     saveOptions, metaInfo,
                                     false/*no lossy prompt*/,
                                     parent))
            {
                atomicFileWriter.cancelWriting ();

            #if DEBUG_KP_DOCUMENT
                qCDebug(kpLogDocument) << "\treturning false because could not save pixmap to device"
                          << endl;
            #endif
                ::CouldNotSaveDialog (url, i18n("Error saving image"), parent);
                return false;
            }

            // Atomically overwrite local file with the temporary file
            // we saved to.
            if (!atomicFileWriter.commit ())
            {
                atomicFileWriter.cancelWriting ();

            #if DEBUG_KP_DOCUMENT
                qCDebug(kpLogDocument) << "\tcould not close QSaveFile";
            #endif
                ::CouldNotSaveDialog (url, atomicFileWriter.errorString(), parent);
                return false;
            }
        }  // sync QSaveFile.cancelWriting()
    }
    // Remote file?
    else
    {
        // Create temporary file that is deleted when the variable goes
        // out of scope.
        QTemporaryFile tempFile;
        if (!tempFile.open ())
        {
        #if DEBUG_KP_DOCUMENT
            qCDebug(kpLogDocument) << "\treturning false because could not open tempFile";
        #endif
            ::CouldNotCreateTemporaryFileDialog (parent);
            return false;
        }

        // Write to local temporary file.
        if (!savePixmapToDevice (pixmap, &tempFile,
                                 saveOptions, metaInfo,
                                 false/*no lossy prompt*/,
                                 parent))
        {
        #if DEBUG_KP_DOCUMENT
            qCDebug(kpLogDocument) << "\treturning false because could not save pixmap to device"
                        << endl;
        #endif
            ::CouldNotSaveDialog (url, i18n("Error saving image"), parent);
            return false;
        }

        // Collect name of temporary file now, as QTemporaryFile::fileName()
        // stops working after close() is called.
        const QString tempFileName = tempFile.fileName ();
    #if DEBUG_KP_DOCUMENT
            qCDebug(kpLogDocument) << "\ttempFileName='" << tempFileName << "'";
    #endif
        Q_ASSERT (!tempFileName.isEmpty ());

        tempFile.close ();
        if (tempFile.error () != QFile::NoError)
        {
        #if DEBUG_KP_DOCUMENT
            qCDebug(kpLogDocument) << "\treturning false because could not close";
        #endif
            ::CouldNotSaveDialog (url, tempFile.errorString(), parent);
            return false;
        }

        // Copy local temporary file to overwrite remote.
        // It's the kioslave's job to make this atomic (write to .part, then rename .part file)
        KIO::FileCopyJob *job = KIO::file_copy (QUrl::fromLocalFile (tempFileName),
                                                url,
                                                -1,
                                                KIO::Overwrite);
        KJobWidgets::setWindow (job, parent);
        if (!job->exec ())
        {
        #if DEBUG_KP_DOCUMENT
            qCDebug(kpLogDocument) << "\treturning false because could not upload";
        #endif
            KMessageBox::error (parent,
                                i18n ("Could not save image - failed to upload."));
            return false;
        }
    }


    return true;
}

//---------------------------------------------------------------------

bool kpDocument::saveAs (const QUrl &url,
                         const kpDocumentSaveOptions &saveOptions,
                         bool lossyPrompt)
{
#if DEBUG_KP_DOCUMENT
    qCDebug(kpLogDocument) << "kpDocument::saveAs (" << url << ","
               << saveOptions.mimeType () << ")" << endl;
#endif

    if (kpDocument::savePixmapToFile (imageWithSelection (),
                                      url,
                                      saveOptions, *metaInfo (),
                                      lossyPrompt,
                                      d->environ->dialogParent ()))
    {
        setURL (url, true/*is from url*/);
        *m_saveOptions = saveOptions;
        m_modified = false;

        m_savedAtLeastOnceBefore = true;

        emit documentSaved ();
        return true;
    }

    return false;
}

//---------------------------------------------------------------------
