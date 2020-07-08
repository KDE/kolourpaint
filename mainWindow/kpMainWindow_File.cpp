/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2007 John Layt <john@layt.net>
   Copyright (c) 2007,2011,2015 Martin Koller <kollix@aon.at>
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


#include "kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include <QAction>
#include <QDataStream>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QPrinter>
#include <QPrintDialog>
#include <QScreen>
#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QMimeDatabase>
#include <QPrintPreviewDialog>

#include <KActionCollection>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KFileCustomDialog>
#include <KPluralHandlingSpinBox>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KStandardShortcut>
#include <KStandardAction>
#include <KToolInvocation>
#include <KLocalizedString>

#include "kpLogCategories.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "commands/imagelib/kpDocumentMetaInfoCommand.h"
#include "dialogs/imagelib/kpDocumentMetaInfoDialog.h"
#include "widgets/kpDocumentSaveOptionsWidget.h"
#include "pixmapfx/kpPixmapFX.h"
#include "widgets/kpPrintDialogPage.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"

#if HAVE_KSANE
#include "../scan/sanedialog.h"
#endif // HAVE_KSANE

// private
void kpMainWindow::setupFileMenuActions ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::setupFileMenuActions()";
#endif
    KActionCollection *ac = actionCollection ();

    d->actionNew = KStandardAction::openNew (this, SLOT (slotNew()), ac);
    d->actionOpen = KStandardAction::open (this, SLOT (slotOpen()), ac);

    d->actionOpenRecent = KStandardAction::openRecent(this, &kpMainWindow::slotOpenRecent, ac);
    connect(d->actionOpenRecent, &KRecentFilesAction::recentListCleared, this, &kpMainWindow::slotRecentListCleared);
    d->actionOpenRecent->loadEntries (KSharedConfig::openConfig ()->group (kpSettingsGroupRecentFiles));
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\trecent URLs=" << d->actionOpenRecent->items ();
#endif

    d->actionSave = KStandardAction::save (this, SLOT (slotSave()), ac);
    d->actionSaveAs = KStandardAction::saveAs (this, SLOT (slotSaveAs()), ac);

    d->actionExport = ac->addAction(QStringLiteral("file_export"));
    d->actionExport->setText (i18n ("E&xport..."));
    d->actionExport->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));
    connect (d->actionExport, &QAction::triggered, this, &kpMainWindow::slotExport);

    d->actionScan = ac->addAction(QStringLiteral("file_scan"));
    d->actionScan->setText(i18n ("Scan..."));
    d->actionScan->setIcon(QIcon::fromTheme("scanner"));
#if HAVE_KSANE
    connect (d->actionScan, &QAction::triggered, this, &kpMainWindow::slotScan);
#else
    d->actionScan->setEnabled(false);
#endif // HAVE_KSANE

    d->actionScreenshot = ac->addAction(QStringLiteral("file_screenshot"));
    d->actionScreenshot->setText(i18n("Acquire Screenshot"));
    connect (d->actionScreenshot, &QAction::triggered, this, &kpMainWindow::slotScreenshot);

    d->actionProperties = ac->addAction (QStringLiteral("file_properties"));
    d->actionProperties->setText (i18n ("Properties"));
    d->actionProperties->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    connect (d->actionProperties, &QAction::triggered, this, &kpMainWindow::slotProperties);

    //d->actionRevert = KStandardAction::revert (this, SLOT (slotRevert()), ac);
    d->actionReload = ac->addAction (QStringLiteral("file_revert"));
    d->actionReload->setText (i18n ("Reloa&d"));
    d->actionReload->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    connect (d->actionReload, &QAction::triggered, this, &kpMainWindow::slotReload);
    ac->setDefaultShortcuts (d->actionReload, KStandardShortcut::reload ());
    slotEnableReload ();

    d->actionPrint = KStandardAction::print (this, SLOT (slotPrint()), ac);
    d->actionPrintPreview = KStandardAction::printPreview (this, SLOT (slotPrintPreview()), ac);

    d->actionMail = KStandardAction::mail (this, SLOT (slotMail()), ac);

    d->actionClose = KStandardAction::close (this, SLOT (slotClose()), ac);
    d->actionQuit = KStandardAction::quit (this, SLOT (slotQuit()), ac);

    d->scanDialog = nullptr;

    enableFileMenuDocumentActions (false);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableFileMenuDocumentActions (bool enable)
{
    // d->actionNew
    // d->actionOpen

    // d->actionOpenRecent

    d->actionSave->setEnabled (enable);
    d->actionSaveAs->setEnabled (enable);

    d->actionExport->setEnabled (enable);

    // d->actionScan

    d->actionProperties->setEnabled (enable);

    // d->actionReload

    d->actionPrint->setEnabled (enable);
    d->actionPrintPreview->setEnabled (enable);

    d->actionMail->setEnabled (enable);

    d->actionClose->setEnabled (enable);
    // d->actionQuit->setEnabled (enable);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::addRecentURL (const QUrl &url_)
{
    // HACK: KRecentFilesAction::loadEntries() clears the KRecentFilesAction::d->urls
    //       map.
    //
    //       So afterwards, the URL ref, our method is given, points to an
    //       element in this now-cleared map (see KRecentFilesAction::urlSelected(QAction*)).
    //       Accessing it would result in a crash.
    //
    //       To avoid the crash, make a copy of it before calling
    //       loadEntries() and use this copy, instead of the to-be-dangling
    //       ref.
    const QUrl url = url_; // DO NOT MAKE IT A REFERENCE, THE CALL BELOW TO loadEntries DESTROYS url_

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::addRecentURL(" << url << ")";
#endif
    if (url.isEmpty ())
        return;


    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    cfg->reparseConfiguration ();

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\trecent URLs=" << d->actionOpenRecent->items ();
#endif
    // HACK: Something might have changed interprocess.
    // If we could PROPAGATE: interprocess, then this wouldn't be required.
    d->actionOpenRecent->loadEntries (cfg->group (kpSettingsGroupRecentFiles));
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tafter loading config=" << d->actionOpenRecent->items ();
#endif

    d->actionOpenRecent->addUrl (url);

    d->actionOpenRecent->saveEntries (cfg->group (kpSettingsGroupRecentFiles));
    cfg->sync ();

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tnew recent URLs=" << d->actionOpenRecent->items ();
#endif


    // TODO: PROPAGATE: interprocess
    // TODO: Is this loop safe since a KMainWindow later along in the list,
    //       could be closed as the code in the body almost certainly re-enters
    //       the event loop?  Problem for KDE 3 as well, I think.
    for (auto *kmw : KMainWindow::memberList ())
    {
        Q_ASSERT (dynamic_cast <kpMainWindow *> (kmw));
        auto *mw = dynamic_cast <kpMainWindow *> (kmw);

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tmw=" << mw;
    #endif

        if (mw != this)
        {
            // WARNING: Do not use KRecentFilesAction::setItems()
            //          - it does not work since only its superclass,
            //          KSelectAction, implements setItems() and can't
            //          update KRecentFilesAction's URL list.

            // Avoid URL memory leak in KRecentFilesAction::loadEntries().
            mw->d->actionOpenRecent->clear ();

            mw->d->actionOpenRecent->loadEntries (cfg->group (kpSettingsGroupRecentFiles));
        #if DEBUG_KP_MAIN_WINDOW
            qCDebug(kpLogMainWindow) << "\t\t\tcheck recent URLs="
                        << mw->d->actionOpenRecent->items ();
        #endif
        }
    }
}

//---------------------------------------------------------------------


// private slot
// TODO: Disable action if
//       (d->configOpenImagesInSameWindow && d->document && d->document->isEmpty())
//       as it does nothing if this is true.
void kpMainWindow::slotNew ()
{
    toolEndShape ();

    if (d->document && !d->configOpenImagesInSameWindow)
    {
        // A document -- empty or otherwise -- is open.
        // Force open a new window.  In contrast, open() might not open
        // a new window in this case.
        auto *win = new kpMainWindow ();
        win->show ();
    }
    else
    {
        open (QUrl (), true/*create an empty doc*/);
    }
}

//---------------------------------------------------------------------


// private
QSize kpMainWindow::defaultDocSize () const
{
    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    KSharedConfig::openConfig ()->reparseConfiguration ();

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);

    QSize docSize = cfg.readEntry (kpSettingLastDocSize, QSize ());

    if (docSize.isEmpty ())
    {
        docSize = QSize (400, 300);
    }
    else
    {
        // Don't get too big or you'll thrash (or even lock up) the computer
        // just by opening a window
        docSize = QSize (qMin (2048, docSize.width ()),
                         qMin (2048, docSize.height ()));
    }

    return docSize;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::saveDefaultDocSize (const QSize &size)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tCONFIG: saving Last Doc Size = " << size;
#endif

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);

    cfg.writeEntry (kpSettingLastDocSize, size);
    cfg.sync ();
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::shouldOpen ()
{
    if (d->configOpenImagesInSameWindow)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\topenImagesInSameWindow";
    #endif
        // (this brings up a dialog and might save the current doc)
        if (!queryCloseDocument ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            qCDebug(kpLogMainWindow) << "\t\tqueryCloseDocument() aborts open";
        #endif
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setDocumentChoosingWindow (kpDocument *doc)
{
    // Want new window?
    if (d->document && !d->document->isEmpty () &&
        !d->configOpenImagesInSameWindow)
    {
        // Send doc to new window.
        auto *win = new kpMainWindow (doc);
        win->show ();
    }
    else
    {
        // (sets up views, doc signals)
        setDocument (doc);
    }
}

//---------------------------------------------------------------------

// private
kpDocument *kpMainWindow::openInternal (const QUrl &url,
        const QSize &fallbackDocSize,
        bool newDocSameNameIfNotExist)
{
    // If using OpenImagesInSameWindow mode, ask whether to close the
    // current document.
    if (!shouldOpen ())
        return nullptr;

    // Create/open doc.
    auto *newDoc = new kpDocument (fallbackDocSize.width (),
                                   fallbackDocSize.height (), documentEnvironment ());

    if (!newDoc->open (url, newDocSameNameIfNotExist))
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\topen failed";
    #endif
        delete newDoc;
        return nullptr;
    }

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\topen OK";
#endif
    // Send document to current or new window.
    setDocumentChoosingWindow (newDoc);

    return newDoc;
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::open (const QUrl &url, bool newDocSameNameIfNotExist)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::open(" << url
              << ",newDocSameNameIfNotExist=" << newDocSameNameIfNotExist
              << ")";
#endif

    kpDocument *newDoc = openInternal (url,
                                       defaultDocSize (),
                                       newDocSameNameIfNotExist);
    if (newDoc)
    {
        if (newDoc->isFromExistingURL ())
            addRecentURL (url);
        return true;
    }

    return false;
}

//---------------------------------------------------------------------

// private
QList<QUrl> kpMainWindow::askForOpenURLs(const QString &caption, bool allowMultipleURLs)
{
  QMimeDatabase db;
  QStringList filterList;
  QString filter;
  for (const auto &type : QImageReader::supportedMimeTypes())
  {
    if ( !filter.isEmpty() ) {
      filter += QLatin1Char(' ');
    }

    QMimeType mime(db.mimeTypeForName(QString::fromLatin1(type)));
    if ( mime.isValid() )
    {
      QString glob = mime.globPatterns().join(QLatin1Char(' '));

      filter += glob;

      // I want to show the mime comment AND the file glob pattern,
      // but to avoid that the "All Supported Files" entry shows ALL glob patterns,
      // I must add the pattern here a second time so that QFileDialog::HideNameFilterDetails
      // can hide the first pattern and I still see the second one
      filterList << mime.comment() + QStringLiteral(" (%1)(%2)").arg(glob).arg(glob);
    }
  }

  filterList.prepend(i18n("All Supported Files (%1)", filter));

  QFileDialog fd(this);
  fd.setNameFilters(filterList);
  fd.setOption(QFileDialog::HideNameFilterDetails);
  fd.setWindowTitle(caption);

  if ( allowMultipleURLs ) {
    fd.setFileMode(QFileDialog::ExistingFiles);
  }

  if ( fd.exec() ) {
    return fd.selectedUrls();
  }

  return {};
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotOpen ()
{
    toolEndShape ();

    const QList<QUrl> urls = askForOpenURLs(i18nc("@title:window", "Open Image"));

    for (const auto & url : urls)
    {
        open (url);
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotOpenRecent (const QUrl &url)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotOpenRecent(" << url << ")";
    qCDebug(kpLogMainWindow) << "\titems=" << d->actionOpenRecent->items ();
#endif

    toolEndShape ();

    open (url);

    // If the open is successful, addRecentURL() would have bubbled up the
    // URL in the File / Open Recent action.  As a side effect, the URL is
    // deselected.
    //
    // If the open fails, we should deselect the URL:
    //
    // 1. for consistency
    //
    // 2. because it has not been opened.
    //
    d->actionOpenRecent->setCurrentItem (-1);
}

//---------------------------------------------------------------------

void kpMainWindow::slotRecentListCleared()
{
  d->actionOpenRecent->saveEntries(KSharedConfig::openConfig()->group(kpSettingsGroupRecentFiles));
}

//---------------------------------------------------------------------

#if HAVE_KSANE
// private slot
void kpMainWindow::slotScan ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotScan() scanDialog=" << d->scanDialog;
#endif

    toolEndShape ();

    if (!d->scanDialog)
    {
        // Create scan dialog
        d->scanDialog = new SaneDialog(this);

        // No scanning support (kdegraphics/libkscan) installed?
        if (!d->scanDialog)
        {
            KMessageBox::sorry (this,
                                i18n("Failed to open scanning dialog."),
                                i18nc("@title:window", "Scanning Failed"));
            return;
        }

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tcreated scanDialog=" << d->scanDialog;
    #endif
        connect (d->scanDialog, &SaneDialog::finalImage, this, &kpMainWindow::slotScanned);
    }


    // If using OpenImagesInSameWindow mode, ask whether to close the
    // current document.
    //
    // Do this after scan support is detected.  Because if it's not, what
    // would be the point of closing the document?
    //
    // Ideally, we would do this after the user presses "Final Scan" in
    // the scan dialog and before the scan begins (if the user wants to
    // cancel the scan operation, it would be annoying to offer this choice
    // only after the slow scan is completed) but the KScanDialog API does
    // not allow this.  So we settle for doing this before any
    // scan dialogs are shown.  We don't do this between KScanDialog::setup()
    // and KScanDialog::exec() as it could be confusing alternating between
    // scanning and KolourPaint dialogs.
    if (!shouldOpen ()) {
        return;
    }


#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tcalling setup";
#endif
    // Bring up dialog to select scan device.
    // If there is no scanner, we find that this does not bring up a dialog
    // but still returns true.
    if (d->scanDialog->setup ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tOK - showing dialog";
    #endif
        // Called only if scanner configured/available.
        //
        // In reality, this seems to be called even if you press "Cancel" in
        // the KScanDialog::setup() dialog!
        //
        // We use exec() to make sure it's modal.  show() seems to work too
        // but better safe than sorry.
        d->scanDialog->exec ();
    }
    else
    {
        // Have never seen this code path execute even if "Cancel" is pressed.
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\tFAIL";
    #endif
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotScanned (const QImage &image, int)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotScanned() image.rect=" << image.rect ();
#endif

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\thiding dialog";
#endif
    // (KScanDialog does not close itself after a scan is made)
    //
    // Close the dialog, first thing:
    //
    // 1. This means that any dialogs we bring up won't be nested on top.
    //
    // 2. We don't want to return from this method but forget to close
    //    the dialog.  So do it before anything else.
    d->scanDialog->hide ();

    // (just in case there's some drawing between slotScan() exiting and
    //  us being called)
    toolEndShape ();


    // TODO: Maybe this code should be moved into kpdocument.cpp -
    //       since it resembles the responsibilities of kpDocument::open().

    kpDocumentSaveOptions saveOptions;
    kpDocumentMetaInfo metaInfo;

    kpDocument::getDataFromImage(image, saveOptions, metaInfo);

    // Create document from image and meta info.
    auto *doc = new kpDocument (image.width (), image.height (), documentEnvironment ());
    doc->setImage (image);
    doc->setSaveOptions (saveOptions);
    doc->setMetaInfo (metaInfo);

    // Send document to current or new window.
    setDocumentChoosingWindow (doc);
}
#endif // HAVE_KSANE

//---------------------------------------------------------------------

void kpMainWindow::slotScreenshot()
{
  toolEndShape();

  auto *dialog = new QDialog(this);
  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                   QDialogButtonBox::Cancel, dialog);
  connect (buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect (buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  auto *label = new QLabel(i18n("Snapshot Delay"));
  auto *seconds = new KPluralHandlingSpinBox;
  seconds->setRange(0, 99);
  seconds->setSuffix(ki18np(" second", " seconds"));
  seconds->setSpecialValueText(i18n("No delay"));

  auto *hideWindow = new QCheckBox(i18n("Hide Main Window"));
  hideWindow->setChecked(true);

  auto *vbox = new QVBoxLayout(dialog);
  vbox->addWidget(label);
  vbox->addWidget(seconds);
  vbox->addWidget(hideWindow);
  vbox->addWidget(buttons);

  if ( dialog->exec() == QDialog::Rejected )
  {
    delete dialog;
    return;
  }

  if ( hideWindow->isChecked() ) {
    hide();
  }

  // at least 1 seconds to make sure the window is hidden and the hide effect already stopped
  QTimer::singleShot((seconds->value() + 1) * 1000, this, &kpMainWindow::slotMakeScreenshot);

  delete dialog;
}

//---------------------------------------------------------------------

void kpMainWindow::slotMakeScreenshot()
{
  QCoreApplication::processEvents();
  QPixmap pixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());

  auto *doc = new kpDocument(pixmap.width(), pixmap.height(), documentEnvironment());
  doc->setImage(pixmap.toImage());

  // Send document to current or new window.
  setDocumentChoosingWindow(doc);

  show();  // in case we hid the mainwindow, show it again
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotProperties ()
{
    toolEndShape ();

    kpDocumentMetaInfoDialog dialog (document ()->metaInfo (), this);

    if (dialog.exec () && !dialog.isNoOp ())
    {
        commandHistory ()->addCommand (
            new kpDocumentMetaInfoCommand (
                i18n ("Document Properties"),
                dialog.metaInfo ()/*new*/, *document ()->metaInfo ()/*old*/,
                commandEnvironment ()));
    }
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::save (bool localOnly)
{
    if (d->document->url ().isEmpty () ||
        !QImageWriter::supportedMimeTypes()
            .contains(d->document->saveOptions ()->mimeType().toLatin1()) ||
        // SYNC: kpDocument::getPixmapFromFile() can't determine quality
        //       from file so it has been set initially to an invalid value.
        (d->document->saveOptions ()->mimeTypeHasConfigurableQuality () &&
            d->document->saveOptions ()->qualityIsInvalid ()) ||
        (localOnly && !d->document->url ().isLocalFile ()))
    {
        return saveAs (localOnly);
    }

    if (d->document->save (!d->document->savedAtLeastOnceBefore ()/*lossy prompt*/))
    {
        addRecentURL (d->document->url ());
        return true;
    }

    return false;
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotSave ()
{
    toolEndShape ();

    return save ();
}

//---------------------------------------------------------------------

// private
QUrl kpMainWindow::askForSaveURL (const QString &caption,
                                  const QString &startURL,
                                  const kpImage &imageToBeSaved,
                                  const kpDocumentSaveOptions &startSaveOptions,
                                  const kpDocumentMetaInfo &docMetaInfo,
                                  const QString &forcedSaveOptionsGroup,
                                  bool localOnly,
                                  kpDocumentSaveOptions *chosenSaveOptions,
                                  bool isSavingForFirstTime,
                                  bool *allowLossyPrompt)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::askForURL() startURL=" << startURL;
    startSaveOptions.printDebug ("\tstartSaveOptions");
#endif

    bool reparsedConfiguration = false;

    // KConfig::readEntry() does not actually reread from disk, hence doesn't
    // realize what other processes have done e.g. Settings / Show Path
    // so reparseConfiguration() must be called
#define SETUP_READ_CFG()                                                             \
    if (!reparsedConfiguration)                                                      \
    {                                                                                \
        KSharedConfig::openConfig ()->reparseConfiguration ();                       \
        reparsedConfiguration = true;                                                \
    }                                                                                \
                                                                                     \
    KConfigGroup cfg (KSharedConfig::openConfig (), forcedSaveOptionsGroup);


    if (chosenSaveOptions) {
        *chosenSaveOptions = kpDocumentSaveOptions ();
    }

    if (allowLossyPrompt) {
        *allowLossyPrompt = true;  // play it safe for now
    }


    kpDocumentSaveOptions fdSaveOptions = startSaveOptions;

    QStringList mimeTypes;
    for (const auto &type : QImageWriter::supportedMimeTypes()) {
      mimeTypes << QString::fromLatin1(type);
    }
#if DEBUG_KP_MAIN_WINDOW
    QStringList sortedMimeTypes = mimeTypes;
    sortedMimeTypes.sort ();
    qCDebug(kpLogMainWindow) << "\tmimeTypes=" << mimeTypes
               << "\tsortedMimeTypes=" << sortedMimeTypes;
#endif
    if (mimeTypes.isEmpty ())
    {
        qCCritical(kpLogMainWindow) << "No output mimetypes!";
        return {};
    }

#define MIME_TYPE_IS_VALID() (!fdSaveOptions.mimeTypeIsInvalid () &&                 \
                              mimeTypes.contains (fdSaveOptions.mimeType ()))
    if (!MIME_TYPE_IS_VALID ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tmimeType=" << fdSaveOptions.mimeType ()
                   << " not valid, get default";
    #endif

        SETUP_READ_CFG ();

        fdSaveOptions.setMimeType (kpDocumentSaveOptions::defaultMimeType (cfg));


        if (!MIME_TYPE_IS_VALID ())
        {
        #if DEBUG_KP_MAIN_WINDOW
            qCDebug(kpLogMainWindow) << "\tmimeType=" << fdSaveOptions.mimeType ()
                       << " not valid, get hardcoded";
        #endif
            if (mimeTypes.contains(QLatin1String("image/png"))) {
                fdSaveOptions.setMimeType (QStringLiteral("image/png"));
            }
            else if (mimeTypes.contains(QLatin1String("image/bmp"))) {
                fdSaveOptions.setMimeType (QStringLiteral("image/bmp"));
            }
            else {
                fdSaveOptions.setMimeType (mimeTypes.first ());
            }
        }
    }
#undef MIME_TYPE_IS_VALID

    if (fdSaveOptions.colorDepthIsInvalid ())
    {
        SETUP_READ_CFG ();

        fdSaveOptions.setColorDepth (kpDocumentSaveOptions::defaultColorDepth (cfg));
        fdSaveOptions.setDither (kpDocumentSaveOptions::defaultDither (cfg));
    }

    if (fdSaveOptions.qualityIsInvalid ())
    {
        SETUP_READ_CFG ();

        fdSaveOptions.setQuality (kpDocumentSaveOptions::defaultQuality (cfg));
    }
#if DEBUG_KP_MAIN_WINDOW
    fdSaveOptions.printDebug ("\tcorrected saveOptions passed to fileDialog");
#endif

    auto *saveOptionsWidget =
        new kpDocumentSaveOptionsWidget (imageToBeSaved,
            fdSaveOptions,
            docMetaInfo,
            this);

    KFileCustomDialog fd (QUrl (startURL), this);
    fd.setOperationMode (KFileWidget::Saving);
    fd.setWindowTitle (caption);
    fd.setCustomWidget (saveOptionsWidget);
    KFileWidget *fw = fd.fileWidget();
    fw->setConfirmOverwrite (true);
    fw->setMimeFilter (mimeTypes, fdSaveOptions.mimeType ());
    if (localOnly) {
        fw->setMode (KFile::File | KFile::LocalOnly);
    }

    saveOptionsWidget->setVisualParent (&fd);

    connect (fw, &KFileWidget::filterChanged,
             saveOptionsWidget, &kpDocumentSaveOptionsWidget::setMimeType);

    if ( fd.exec() == QDialog::Accepted )
    {
        kpDocumentSaveOptions newSaveOptions = saveOptionsWidget->documentSaveOptions ();
    #if DEBUG_KP_MAIN_WINDOW
        newSaveOptions.printDebug ("\tnewSaveOptions");
    #endif

        KConfigGroup cfg (KSharedConfig::openConfig (), forcedSaveOptionsGroup);

        // Save options user forced - probably want to use them in future
        kpDocumentSaveOptions::saveDefaultDifferences (cfg,
            fdSaveOptions, newSaveOptions);
        cfg.sync ();


        if (chosenSaveOptions) {
            *chosenSaveOptions = newSaveOptions;
        }

        const QList<QUrl> selectedUrls = fw->selectedUrls ();
        if (selectedUrls.isEmpty()) { // shouldn't happen
            return {};
        }
        const QUrl selectedUrl = selectedUrls.at(0);

        if (allowLossyPrompt)
        {
            // SYNC: kpDocumentSaveOptions elements - everything except quality
            //       (one quality setting is "just as lossy" as another so no
            //        need to continually warn due to quality change)
            *allowLossyPrompt =
                (isSavingForFirstTime ||
                 selectedUrl != QUrl (startURL) ||
                 newSaveOptions.mimeType () != startSaveOptions.mimeType () ||
                 newSaveOptions.colorDepth () != startSaveOptions.colorDepth () ||
                 newSaveOptions.dither () != startSaveOptions.dither ());
        #if DEBUG_KP_MAIN_WINDOW
            qCDebug(kpLogMainWindow) << "\tallowLossyPrompt=" << *allowLossyPrompt;
        #endif
        }


    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tselectedUrl=" << selectedUrl;
    #endif
        return selectedUrl;
    }

    return {};
#undef SETUP_READ_CFG
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::saveAs (bool localOnly)
{
    kpDocumentSaveOptions chosenSaveOptions;
    bool allowLossyPrompt;
    QUrl chosenURL = askForSaveURL (i18nc ("@title:window", "Save Image As"),
                                    d->document->url ().url (),
                                    d->document->imageWithSelection (),
                                    *d->document->saveOptions (),
                                    *d->document->metaInfo (),
                                    kpSettingsGroupFileSaveAs,
                                    localOnly,
                                    &chosenSaveOptions,
                                    !d->document->savedAtLeastOnceBefore (),
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ()) {
        return false;
    }


    if (!d->document->saveAs (chosenURL, chosenSaveOptions,
                             allowLossyPrompt))
    {
        return false;
    }


    addRecentURL (chosenURL);

    return true;
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotSaveAs ()
{
    toolEndShape ();

    return saveAs ();
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotExport ()
{
    toolEndShape ();

    kpDocumentSaveOptions chosenSaveOptions;
    bool allowLossyPrompt;
    QUrl chosenURL = askForSaveURL (i18nc ("@title:window", "Export"),
                                    d->lastExportURL.url (),
                                    d->document->imageWithSelection (),
                                    d->lastExportSaveOptions,
                                    *d->document->metaInfo (),
                                    kpSettingsGroupFileExport,
                                    false/*allow remote files*/,
                                    &chosenSaveOptions,
                                    d->exportFirstTime,
                                    &allowLossyPrompt);


    if (chosenURL.isEmpty ()) {
        return false;
    }

    if (!kpDocument::savePixmapToFile (d->document->imageWithSelection (),
                                       chosenURL,
                                       chosenSaveOptions, *d->document->metaInfo (),
                                       allowLossyPrompt,
                                       this))
    {
        return false;
    }


    addRecentURL (chosenURL);

    d->lastExportURL = chosenURL;
    d->lastExportSaveOptions = chosenSaveOptions;

    d->exportFirstTime = false;

    return true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotEnableReload ()
{
    d->actionReload->setEnabled (d->document);
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotReload ()
{
    toolEndShape ();

    Q_ASSERT (d->document);


    QUrl oldURL = d->document->url ();


    if (d->document->isModified ())
    {
        int result = KMessageBox::Cancel;

        if (d->document->isFromExistingURL () && !oldURL.isEmpty ())
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes since you last saved it.\n"
                               "Are you sure?",
                               d->document->prettyFilename ()),
                         QString()/*caption*/,
                         KGuiItem(i18n ("&Reload")));
        }
        else
        {
            result = KMessageBox::warningContinueCancel (this,
                         i18n ("The document \"%1\" has been modified.\n"
                               "Reloading will lose all changes.\n"
                               "Are you sure?",
                               d->document->prettyFilename ()),
                         QString()/*caption*/,
                         KGuiItem(i18n ("&Reload")));
        }

        if (result != KMessageBox::Continue) {
            return false;
        }
    }


    kpDocument *doc = nullptr;

    // If it's _supposed to_ come from an existing URL or it actually exists
    if (d->document->isFromExistingURL () ||
        d->document->urlExists (oldURL))
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "kpMainWindow::slotReload() reloading from disk!";
    #endif

        doc = new kpDocument (1, 1, documentEnvironment ());
        if (!doc->open (oldURL))
        {
            delete doc; doc = nullptr;
            return false;
        }

        addRecentURL (oldURL);
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "kpMainWindow::slotReload() create doc";
    #endif

        doc = new kpDocument (d->document->constructorWidth (),
                              d->document->constructorHeight (),
                              documentEnvironment ());
        doc->setURL (oldURL, false/*not from URL*/);
    }


    setDocument (doc);

    return true;
}


// private
void kpMainWindow::sendDocumentNameToPrinter (QPrinter *printer)
{
    QUrl url = d->document->url ();
    if (!url.isEmpty ())
    {
        int dot;

        QString fileName = url.fileName ();
        dot = fileName.lastIndexOf ('.');

        // file.ext but not .hidden-file?
        if (dot > 0) {
            fileName.truncate (dot);
        }

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "kpMainWindow::sendDocumentNameToPrinter() fileName="
                   << fileName
                   << " dir="
                   << url.path();
    #endif
        printer->setDocName (fileName);
    }
}

//--------------------------------------------------------------------------------

void kpMainWindow::sendPreviewToPrinter(QPrinter *printer)
{
  sendImageToPrinter(printer, false);
}

//--------------------------------------------------------------------------------
// private
void kpMainWindow::sendImageToPrinter (QPrinter *printer,
        bool showPrinterSetupDialog)
{
    // Get image to be printed.
    kpImage image = d->document->imageWithSelection ();


    // Get image DPI.
    auto imageDotsPerMeterX = double (d->document->metaInfo ()->dotsPerMeterX ());
    auto imageDotsPerMeterY = double (d->document->metaInfo ()->dotsPerMeterY ());
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::sendImageToPrinter() image:"
               << " width=" << image.width ()
               << " height=" << image.height ()
               << " dotsPerMeterX=" << imageDotsPerMeterX
               << " dotsPerMeterY=" << imageDotsPerMeterY;
#endif

    // Image DPI invalid (e.g. new image, could not read from file
    // or Qt3 doesn't implement DPI for JPEG)?
    if (imageDotsPerMeterX <= 0 || imageDotsPerMeterY <= 0)
    {
        // Even if just one DPI dimension is invalid, mutate both DPI
        // dimensions as we have no information about the intended
        // aspect ratio anyway (and other dimension likely to be invalid).

        // When rendering text onto a document, the fonts are rasterised
        // according to the screen's DPI.
        // TODO: I think we should use the image's DPI.  Technically
        //       possible?
        //
        //       So no matter what computer you draw text on, you get
        //       the same pixels.
        //
        // So we must print at the screen's DPI to get the right text size.
        //
        // Unfortunately, this means that moving to a different screen DPI
        // affects printing.  If you edited the image at a different screen
        // DPI than when you print, you get incorrect results.  Furthermore,
        // this is bogus if you don't have text in your image.  Worse still,
        // what if you have multiple screens connected to the same computer
        // with different DPIs?
        // TODO: mysteriously, someone else is setting this to 96dpi always.
        QPixmap arbitraryScreenElement(1, 1);
        const QPaintDevice *screenDevice = &arbitraryScreenElement;
        const auto dpiX = screenDevice->logicalDpiX ();
        const auto dpiY = screenDevice->logicalDpiY ();
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tusing screen dpi: x=" << dpiX << " y=" << dpiY;
    #endif

        imageDotsPerMeterX = dpiX * KP_INCHES_PER_METER;
        imageDotsPerMeterY = dpiY * KP_INCHES_PER_METER;
    }


    // Get page size (excluding margins).
    // Coordinate (0,0) is the X here:
    //     mmmmm
    //     mX  m
    //     m   m       m = margin
    //     m   m
    //     mmmmm
    const auto printerWidthMM = printer->widthMM ();
    const auto printerHeightMM = printer->heightMM ();

    auto dpiX = imageDotsPerMeterX / KP_INCHES_PER_METER;
    auto dpiY = imageDotsPerMeterY / KP_INCHES_PER_METER;

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\tprinter: widthMM=" << printerWidthMM
               << " heightMM=" << printerHeightMM;

    qCDebug(kpLogMainWindow) << "\timage: dpiX=" << dpiX << " dpiY=" << dpiY;
#endif


    //
    // If image doesn't fit on page at intended DPI, change the DPI.
    //

    const auto scaleDpiX =
        (image.width () / (printerWidthMM / KP_MILLIMETERS_PER_INCH)) / dpiX;
    const auto scaleDpiY =
        (image.height () / (printerHeightMM / KP_MILLIMETERS_PER_INCH)) / dpiY;
    const auto scaleDpi = qMax (scaleDpiX, scaleDpiY);

#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "\t\tscaleDpi: x=" << scaleDpiX << " y=" << scaleDpiY
               << " --> scale at " << scaleDpi << " to fit?";
#endif

    // Need to increase resolution to fit page?
    if (scaleDpi > 1.0)
    {
        dpiX *= scaleDpi;
        dpiY *= scaleDpi;
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\t\t\tto fit page, scaled to:"
                   << " dpiX=" << dpiX << " dpiY=" << dpiY;
    #endif
    }


    // Make sure DPIs are equal as that's all QPrinter::setResolution()
    // supports.  We do this in such a way that we only ever stretch an
    // image, to avoid losing information.  Don't antialias as the printer
    // will do that to translate our DPI to its physical resolution and
    // double-antialiasing looks bad.
    if (dpiX > dpiY)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tdpiX > dpiY; stretching image height to equalise DPIs to dpiX="
                   << dpiX;
    #endif
        kpPixmapFX::scale (&image,
             image.width (),
             qMax (1, qRound (image.height () * dpiX / dpiY)),
             false/*don't antialias*/);

        dpiY = dpiX;
    }
    else if (dpiY > dpiX)
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\tdpiY > dpiX; stretching image width to equalise DPIs to dpiY="
                   << dpiY;
    #endif
        kpPixmapFX::scale (&image,
             qMax (1, qRound (image.width () * dpiY / dpiX)),
             image.height (),
             false/*don't antialias*/);

        dpiX = dpiY;
    }

    Q_ASSERT (dpiX == dpiY);


    // QPrinter::setResolution() has to be called before QPrinter::setup().
    printer->setResolution (qMax (1, qRound (dpiX)));


    sendDocumentNameToPrinter (printer);


    if (showPrinterSetupDialog)
    {
        auto *optionsPage = new kpPrintDialogPage (this);
        optionsPage->setPrintImageCenteredOnPage (d->configPrintImageCenteredOnPage);

        QPrintDialog printDialog (printer, this);
        printDialog.setOptionTabs ({optionsPage});
        printDialog.setWindowTitle (i18nc ("@title:window", "Print Image"));

        // Display dialog.
        const bool wantToPrint = printDialog.exec ();

        if (optionsPage->printImageCenteredOnPage () !=
            d->configPrintImageCenteredOnPage)
        {
            // Save config option even if the dialog was cancelled.
            d->configPrintImageCenteredOnPage = optionsPage->printImageCenteredOnPage ();

            KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupGeneral);
            cfg.writeEntry (kpSettingPrintImageCenteredOnPage,
                           d->configPrintImageCenteredOnPage);
            cfg.sync ();
        }

        if (!wantToPrint) {
            return;
        }
    }


    // Send image to printer.
    QPainter painter;
    painter.begin(printer);

    double originX = 0, originY = 0;

    // Center image on page?
    if (d->configPrintImageCenteredOnPage)
    {
        originX = (printer->width() - image.width ()) / 2;
        originY = (printer->height() - image.height ()) / 2;
    }

    painter.drawImage(qRound(originX), qRound(originY), image);
    painter.end();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotPrint ()
{
    toolEndShape ();

    QPrinter printer;

    sendImageToPrinter (&printer, true/*showPrinterSetupDialog*/);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotPrintPreview ()
{
    toolEndShape ();

    QPrintPreviewDialog printPreview(this);
    connect(&printPreview, &QPrintPreviewDialog::paintRequested, this, &kpMainWindow::sendPreviewToPrinter);

    printPreview.exec ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotMail ()
{
    toolEndShape ();

    if (d->document->url ().isEmpty ()/*no name*/ ||
        !(d->document->isFromExistingURL () && d->document->urlExists (d->document->url ())) ||
        d->document->isModified ()/*needs to be saved*/)
    {
        int result = KMessageBox::questionYesNo (this,
                        i18n ("You must save this image before sending it.\n"
                              "Do you want to save it?"),
                        QString(),
                        KStandardGuiItem::save (), KStandardGuiItem::cancel ());

        if (result == KMessageBox::Yes)
        {
            if (!save ())
            {
                // save failed or aborted - don't email
                return;
            }
        }
        else
        {
            // don't want to save - don't email
            return;
        }
    }

    KToolInvocation::invokeMailer (
        QString()/*to*/,
        QString()/*cc*/,
        QString()/*bcc*/,
        d->document->prettyFilename()/*subject*/,
        QString()/*body*/,
        QString()/*messageFile*/,
        QStringList(d->document->url().url())/*attachments*/);
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::queryCloseDocument ()
{
    toolEndShape ();

    if (!d->document || !d->document->isModified ()) {
        return true;  // ok to close current doc
    }

    int result = KMessageBox::warningYesNoCancel (this,
                     i18n ("The document \"%1\" has been modified.\n"
                           "Do you want to save it?",
                           d->document->prettyFilename ()),
                    QString()/*caption*/,
                    KStandardGuiItem::save (), KStandardGuiItem::discard ());

    switch (result)
    {
    case KMessageBox::Yes:
        return slotSave ();  // close only if save succeeds
    case KMessageBox::No:
        return true;  // close without saving
    default:
        return false;  // don't close current doc
    }
}

//---------------------------------------------------------------------

// private virtual [base KMainWindow]
bool kpMainWindow::queryClose ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::queryClose()";
#endif
    toolEndShape ();

    if (!queryCloseDocument ()) {
        return false;
    }

    if (!queryCloseColors ()) {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotClose ()
{
    toolEndShape ();

    if (!queryCloseDocument ()) {
        return;
    }

    setDocument (nullptr);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotQuit ()
{
    toolEndShape ();

    close ();  // will call queryClose()
}

//---------------------------------------------------------------------
