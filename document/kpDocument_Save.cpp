
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
#include <kmessagebox.h>
#include <kmimetype.h>  // TODO: isn't this in KIO?
#include <KSaveFile>
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
#include <kpUrlFormatter.h>
#include <kpViewManager.h>


bool kpDocument::save (bool overwritePrompt, bool lossyPrompt)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::save("
               << "overwritePrompt=" << overwritePrompt
               << ",lossyPrompt=" << lossyPrompt
               << ") url=" << m_url
               << " savedAtLeastOnceBefore=" << savedAtLeastOnceBefore ()
               << endl;
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
                   overwritePrompt,
                   lossyPrompt);
}

//---------------------------------------------------------------------

// public static
bool kpDocument::lossyPromptContinue (const QImage &pixmap,
                                      const kpDocumentSaveOptions &saveOptions,
                                      QWidget *parent)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::lossyPromptContinue()";
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
        QUIT_IF_CANCEL (
            KMessageBox::warningContinueCancel (parent,
                i18n ("<qt><p>The <b>%1</b> format may not be able"
                      " to preserve all of the image's color information.</p>"

                      "<p>Are you sure you want to save in this format?</p></qt>",
                      KMimeType::mimeType (saveOptions.mimeType ())->comment ()),
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

    QStringList types = KImageIO::typeForMime (saveOptions.mimeType ());
#if DEBUG_KP_DOCUMENT
    kDebug () << "\ttypes=" << types;
#endif
    if (types.isEmpty ())
        return false;

    // It's safe to arbitrarily choose the 0th type as any type in the list
    // should invoke the same KImageIO image loader.
    const QString type = types [0];

#if DEBUG_KP_DOCUMENT
    kDebug () << "\tmimeType=" << saveOptions.mimeType ()
               << " type=" << type << endl;
#endif

    if (lossyPrompt && !lossyPromptContinue (image, saveOptions, parent))
    {
        if (userCancelled)
            *userCancelled = true;

    #if DEBUG_KP_DOCUMENT
        kDebug () << "\treturning false because of lossyPrompt";
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
    kDebug () << "\tuseSaveOptionsColorDepth=" << useSaveOptionsColorDepth
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

    QList <QString> keyList = metaInfo.textKeys ();
    for (QList <QString>::const_iterator it = keyList.constBegin ();
         it != keyList.constEnd ();
         ++it)
    {
        imageToSave.setText (*it, metaInfo.text (*it));
    }


    //
    // Save at required quality
    //

    int quality = -1;  // default

    if (useSaveOptionsQuality)
        quality = saveOptions.quality ();

#if DEBUG_KP_DOCUMENT
    kDebug () << "\tsaving";
#endif
    if (!imageToSave.save (device, type.toLatin1 (), quality))
    {
    #if DEBUG_KP_DOCUMENT
        kDebug () << "\tQImage::save() returned false";
    #endif
        return false;
    }


#if DEBUG_KP_DOCUMENT
    kDebug () << "\tsave OK";
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

static void CouldNotSaveDialog (const KUrl &url, QWidget *parent)
{
    // TODO: use file.errorString()
    KMessageBox::error (parent,
                        i18n ("Could not save as \"%1\".",
                              kpUrlFormatter::PrettyFilename (url)));
}

//---------------------------------------------------------------------

// public static
bool kpDocument::savePixmapToFile (const QImage &pixmap,
                                   const KUrl &url,
                                   const kpDocumentSaveOptions &saveOptions,
                                   const kpDocumentMetaInfo &metaInfo,
                                   bool overwritePrompt,
                                   bool lossyPrompt,
                                   QWidget *parent)
{
    // TODO: Use KIO::NetAccess:mostLocalURL() for accessing home:/ (and other
    //       such local URLs) for efficiency and because only local writes
    //       are atomic.
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::savePixmapToFile ("
               << url
               << ",overwritePrompt=" << overwritePrompt
               << ",lossyPrompt=" << lossyPrompt
               << ")" << endl;
    saveOptions.printDebug (QLatin1String ("\tsaveOptions"));
    metaInfo.printDebug (QLatin1String ("\tmetaInfo"));
#endif

    if (overwritePrompt &&
        KIO::NetAccess::exists (url, KIO::NetAccess::DestinationSide/*write*/, parent))
    {
        int result = KMessageBox::warningContinueCancel (parent,
            i18n ("A document called \"%1\" already exists.\n"
                  "Do you want to overwrite it?",
                  kpUrlFormatter::PrettyFilename (url)),
            QString(),
            KGuiItem(i18n ("Overwrite")));

        if (result != KMessageBox::Continue)
        {
        #if DEBUG_KP_DOCUMENT
            kDebug () << "\tuser doesn't want to overwrite";
        #endif

            return false;
        }
    }


    if (lossyPrompt && !lossyPromptContinue (pixmap, saveOptions, parent))
    {
    #if DEBUG_KP_DOCUMENT
        kDebug () << "\treturning false because of lossyPrompt";
    #endif
        return false;
    }


    // Local file?
    if (url.isLocalFile ())
    {
        const QString filename = url.toLocalFile ();

        // sync: All failure exit paths _must_ call KSaveFile::abort() or
        //       else, the KSaveFile destructor will overwrite the file,
        //       <filename>, despite the failure.
        KSaveFile atomicFileWriter (filename);
        {
            if (!atomicFileWriter.open ())
            {
                // We probably don't need this as <filename> has not been
                // opened.
                atomicFileWriter.abort ();

            #if DEBUG_KP_DOCUMENT
                kDebug () << "\treturning false because could not open KSaveFile"
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
                atomicFileWriter.abort ();

            #if DEBUG_KP_DOCUMENT
                kDebug () << "\treturning false because could not save pixmap to device"
                          << endl;
            #endif
                ::CouldNotSaveDialog (url, parent);
                return false;
            }

            // Atomically overwrite local file with the temporary file
            // we saved to.
            if (!atomicFileWriter.finalize ())
            {
                atomicFileWriter.abort ();

            #if DEBUG_KP_DOCUMENT
                kDebug () << "\tcould not close KSaveFile";
            #endif
                ::CouldNotSaveDialog (url, parent);
                return false;
            }
        }  // sync KSaveFile.abort()
    }
    // Remote file?
    else
    {
        // Create temporary file that is deleted when the variable goes
        // out of scope.
        KTemporaryFile tempFile;
        if (!tempFile.open ())
        {
        #if DEBUG_KP_DOCUMENT
            kDebug () << "\treturning false because could not open tempFile";
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
            kDebug () << "\treturning false because could not save pixmap to device"
                        << endl;
        #endif
            ::CouldNotSaveDialog (url, parent);
            return false;
        }

        // Collect name of temporary file now, as QTemporaryFile::fileName()
        // stops working after close() is called.
        const QString tempFileName = tempFile.fileName ();
    #if DEBUG_KP_DOCUMENT
            kDebug () << "\ttempFileName='" << tempFileName << "'";
    #endif
        Q_ASSERT (!tempFileName.isEmpty ());

        tempFile.close ();
        if (tempFile.error () != QFile::NoError)
        {
        #if DEBUG_KP_DOCUMENT
            kDebug () << "\treturning false because could not close";
        #endif
            ::CouldNotSaveDialog (url, parent);
            return false;
        }

        // Copy local temporary file to overwrite remote.
        // TODO: No one seems to know how to do this atomically
        //       [http://lists.kde.org/?l=kde-core-devel&m=117845162728484&w=2].
        //       At least, fish:// (ssh) is definitely not atomic.
        if (!KIO::NetAccess::upload (tempFileName, url, parent))
        {
        #if DEBUG_KP_DOCUMENT
            kDebug () << "\treturning false because could not upload";
        #endif
            KMessageBox::error (parent,
                                i18n ("Could not save image - failed to upload."));
            return false;
        }
    }


    return true;
}

//---------------------------------------------------------------------

bool kpDocument::saveAs (const KUrl &url,
                         const kpDocumentSaveOptions &saveOptions,
                         bool overwritePrompt,
                         bool lossyPrompt)
{
#if DEBUG_KP_DOCUMENT
    kDebug () << "kpDocument::saveAs (" << url << ","
               << saveOptions.mimeType () << ")" << endl;
#endif

    if (kpDocument::savePixmapToFile (imageWithSelection (),
                                      url,
                                      saveOptions, *metaInfo (),
                                      overwritePrompt,
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
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------
