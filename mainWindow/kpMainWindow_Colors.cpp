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


#include "kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include "widgets/kpColorCells.h"
#include "lgpl/generic/kpColorCollection.h"
#include "lgpl/generic/kpUrlFormatter.h"
#include "widgets/toolbars/kpColorToolBar.h"

#include <KActionCollection>
#include <KMessageBox>
#include <KSelectAction>
#include <KStandardGuiItem>
#include "kpLogCategories.h"

#include <QFileDialog>
#include <QAction>

//---------------------------------------------------------------------

static QStringList KDEColorCollectionNames ()
{
    return kpColorCollection::installedCollections ();
}

//---------------------------------------------------------------------

// private
void kpMainWindow::setupColorsMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    d->actionColorsDefault = ac->addAction (QStringLiteral("colors_default"));
    d->actionColorsDefault->setText (i18n ("Use KolourPaint Defaults"));
    connect (d->actionColorsDefault, &QAction::triggered,
             this, &kpMainWindow::slotColorsDefault);

    d->actionColorsKDE = ac->add <KSelectAction> (QStringLiteral("colors_kde"));
    d->actionColorsKDE->setText (i18nc ("@item:inmenu colors", "Use KDE's"));
    // TODO: Will this slot be called spuriously if there are no colors
    //       installed?
    connect (d->actionColorsKDE,
             static_cast<void (KSelectAction::*)(QAction*)>(&KSelectAction::triggered),
             this, &kpMainWindow::slotColorsKDE);

    for (const auto &colName : ::KDEColorCollectionNames ()) {
        d->actionColorsKDE->addAction (colName);
    }

    d->actionColorsOpen = ac->addAction (QStringLiteral("colors_open"));
    d->actionColorsOpen->setText (i18nc ("@item:inmenu colors", "&Open..."));
    connect (d->actionColorsOpen, &QAction::triggered, this, &kpMainWindow::slotColorsOpen);

    d->actionColorsReload = ac->addAction (QStringLiteral("colors_reload"));
    d->actionColorsReload->setText (i18nc ("@item:inmenu colors", "Reloa&d"));
    connect (d->actionColorsReload, &QAction::triggered,
             this, &kpMainWindow::slotColorsReload);

    d->actionColorsSave = ac->addAction (QStringLiteral("colors_save"));
    d->actionColorsSave->setText (i18nc ("@item:inmenu colors", "&Save"));
    connect (d->actionColorsSave, &QAction::triggered,
             this, &kpMainWindow::slotColorsSave);

    d->actionColorsSaveAs = ac->addAction (QStringLiteral("colors_save_as"));
    d->actionColorsSaveAs->setText (i18nc ("@item:inmenu colors", "Save &As..."));
    connect (d->actionColorsSaveAs, &QAction::triggered,
             this, &kpMainWindow::slotColorsSaveAs);

    d->actionColorsAppendRow = ac->addAction (QStringLiteral("colors_append_row"));
    d->actionColorsAppendRow->setText (i18nc ("@item:inmenu colors", "Add Row"));
    connect (d->actionColorsAppendRow, &QAction::triggered,
             this, &kpMainWindow::slotColorsAppendRow);

    d->actionColorsDeleteRow = ac->addAction (QStringLiteral("colors_delete_row"));
    d->actionColorsDeleteRow->setText (i18nc ("@item:inmenu colors", "Delete Last Row"));
    connect (d->actionColorsDeleteRow, &QAction::triggered,
             this, &kpMainWindow::slotColorsDeleteRow);


    enableColorsMenuDocumentActions (false);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::createColorBox ()
{
    d->colorToolBar = new kpColorToolBar (i18n ("Color Box"), this);

    // (needed for QMainWindow::saveState())
    d->colorToolBar->setObjectName ( QStringLiteral("Color Box" ));

    connect (colorCells (), &kpColorCells::rowCountChanged,
             this, &kpMainWindow::slotUpdateColorsDeleteRowActionEnabled);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::enableColorsMenuDocumentActions (bool enable)
{
    d->actionColorsDefault->setEnabled (enable);
    d->actionColorsKDE->setEnabled (enable);
    d->actionColorsOpen->setEnabled (enable);
    d->actionColorsReload->setEnabled (enable);

    d->actionColorsSave->setEnabled (enable);
    d->actionColorsSaveAs->setEnabled (enable);

    d->actionColorsAppendRow->setEnabled (enable);

    d->colorMenuDocumentActionsEnabled = enable;

    slotUpdateColorsDeleteRowActionEnabled ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotUpdateColorsDeleteRowActionEnabled ()
{
    // Currently, this is always enabled since kpColorCells guarantees that
    // there will be at least one row of cells (which might all be of the
    // invalid color).
    //
    // But this method is left here for future extensibility.
    d->actionColorsDeleteRow->setEnabled (
        d->colorMenuDocumentActionsEnabled && (colorCells ()->rowCount () > 0));
}

//---------------------------------------------------------------------


// Used in 2 situations:
//
// 1. User opens a color without using the "Use KDE's" submenu.
// 2. User attempts to open a color using the "Use KDE's" submenu but the
//    opening fails.
//
// TODO: Maybe we could put the 3 actions (for different ways of opening
//       colors) in an exclusive group -- this might eliminate the need for
//       this hack.
//
// private
void kpMainWindow::deselectActionColorsKDE ()
{
    d->actionColorsKDE->setCurrentItem (-1);
}

//---------------------------------------------------------------------


// private
bool kpMainWindow::queryCloseColors ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::queryCloseColors() colorCells.modified="
              << colorCells ()->isModified ();
#endif

    toolEndShape ();

    if (!colorCells ()->isModified ()) {
        return true;  // ok to close
    }

    int result = KMessageBox::Cancel;


    if (!colorCells ()->url ().isEmpty ())
    {
        result = KMessageBox::warningYesNoCancel (this,
            i18n ("The color palette \"%1\" has been modified.\n"
                  "Do you want to save it?",
                  kpUrlFormatter::PrettyFilename (colorCells ()->url ())),
            QString ()/*caption*/,
            KStandardGuiItem::save (), KStandardGuiItem::discard ());
    }
    else
    {
        const QString name = colorCells ()->colorCollection ()->name ();
        if (!name.isEmpty ())
        {
            result = KMessageBox::warningYesNoCancel (this,
                i18n ("The KDE color palette \"%1\" has been modified.\n"
                      "Do you want to save it to a file?",
                      name),
                QString ()/*caption*/,
                KStandardGuiItem::save (), KStandardGuiItem::discard ());
        }
        else
        {
            result = KMessageBox::warningYesNoCancel (this,
                i18n ("The default color palette has been modified.\n"
                      "Do you want to save it to a file?"),
                QString ()/*caption*/,
                KStandardGuiItem::save (), KStandardGuiItem::discard ());
        }
    }

    switch (result)
    {
    case KMessageBox::Yes:
        return slotColorsSave ();  // close only if save succeeds
    case KMessageBox::No:
        return true;  // close without saving
    default:
        return false;  // don't close current doc
    }
}

//---------------------------------------------------------------------


// private
void kpMainWindow::openDefaultColors ()
{
    colorCells ()->setColorCollection (
        kpColorCells::DefaultColorCollection ());
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsDefault ()
{
    // Call just in case.
    toolEndShape ();

    if (!queryCloseColors ()) {
        return;
    }

    openDefaultColors ();

    deselectActionColorsKDE ();
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::openKDEColors (const QString &name)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::openKDEColors(" << name << ")";
#endif

    kpColorCollection colorCol;
    if (colorCol.openKDE (name, this))
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "opened";
    #endif
        colorCells ()->setColorCollection (colorCol);
        return true;
    }
    else
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "failed to open";
    #endif
        return false;
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsKDE ()
{
    // Call in case an error dialog appears.
    toolEndShape ();

    const int curItem = d->actionColorsKDE->currentItem ();

    if (!queryCloseColors ())
    {
        deselectActionColorsKDE ();
        return;
    }

    // queryCloseColors() calls slotColorSave(), which can call
    // slotColorSaveAs(), which can call deselectActionColorsKDE().
    d->actionColorsKDE->setCurrentItem (curItem);

    const QStringList colNames = ::KDEColorCollectionNames ();
    const int selected = d->actionColorsKDE->currentItem ();
    Q_ASSERT (selected >= 0 && selected < colNames.size ());

    if (!openKDEColors (colNames [selected])) {
        deselectActionColorsKDE ();
    }
}

//---------------------------------------------------------------------

// private
bool kpMainWindow::openColors (const QUrl &url)
{
    return colorCells ()->openColorCollection (url);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsOpen ()
{
    // Call due to dialog.
    toolEndShape ();

    QFileDialog fd(this);
    fd.setDirectoryUrl(colorCells ()->url());
    fd.setWindowTitle(i18nc ("@title:window", "Open Color Palette"));

    if (fd.exec ())
    {
        if (!queryCloseColors ()) {
            return;
        }

        QList<QUrl> selected = fd.selectedUrls();
        if ( selected.count() && openColors(selected[0]) ) {
          deselectActionColorsKDE();
        }
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsReload ()
{
    toolEndShape ();

    if (colorCells ()->isModified ())
    {
        int result = KMessageBox::Cancel;

        if (!colorCells ()->url ().isEmpty ())
        {
            result = KMessageBox::warningContinueCancel (this,
                i18n ("The color palette \"%1\" has been modified.\n"
                      "Reloading will lose all changes since you last saved it.\n"
                      "Are you sure?",
                      kpUrlFormatter::PrettyFilename (colorCells ()->url ())),
                QString ()/*caption*/,
                KGuiItem(i18n ("&Reload")));
        }
        else
        {
            const QString name = colorCells ()->colorCollection ()->name ();
            if (!name.isEmpty ())
            {
                result = KMessageBox::warningContinueCancel (this,
                    i18n ("The KDE color palette \"%1\" has been modified.\n"
                          "Reloading will lose all changes.\n"
                          "Are you sure?",
                          colorCells ()->colorCollection ()->name ()),
                    QString ()/*caption*/,
                    KGuiItem (i18n ("&Reload")));
            }
            else
            {
                result = KMessageBox::warningContinueCancel (this,
                    i18n ("The default color palette has been modified.\n"
                          "Reloading will lose all changes.\n"
                          "Are you sure?"),
                    QString ()/*caption*/,
                    KGuiItem (i18n ("&Reload")));
            }
        }

    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "result=" << result
                  << "vs KMessageBox::Continue" << KMessageBox::Continue;
    #endif
        if (result != KMessageBox::Continue) {
            return;
        }
    }


    if (!colorCells ()->url ().isEmpty ())
    {
        openColors (colorCells ()->url ());
    }
    else
    {
        const QString name = colorCells ()->colorCollection ()->name ();
        if (!name.isEmpty ()) {
            openKDEColors (name);
        }
        else {
            openDefaultColors ();
        }
    }
}

//---------------------------------------------------------------------


// private slot
bool kpMainWindow::slotColorsSave ()
{
    // Call due to dialog.
    toolEndShape ();

    if (colorCells ()->url ().isEmpty ())
    {
        return slotColorsSaveAs ();
    }

    return colorCells ()->saveColorCollection ();
}

//---------------------------------------------------------------------

// private slot
bool kpMainWindow::slotColorsSaveAs ()
{
    // Call due to dialog.
    toolEndShape ();

    QFileDialog fd(this);
    fd.setDirectoryUrl(colorCells ()->url());
    fd.setWindowTitle(i18n("Save Color Palette As"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    // Note that QFileDialog takes care of asking the user to confirm overwriting.

    if (fd.exec ())
    {
        QList<QUrl> selected = fd.selectedUrls();
        if ( !selected.count() || !colorCells ()->saveColorCollectionAs(selected[0]) ) {
          return false;
        }

        // We're definitely using our own color collection now.
        deselectActionColorsKDE ();

        return true;
    }

    return false;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsAppendRow ()
{
    // Call just in case.
    toolEndShape ();

    kpColorCells *colorCells = d->colorToolBar->colorCells ();
    colorCells->appendRow ();
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::slotColorsDeleteRow ()
{
    // Call just in case.
    toolEndShape ();

    kpColorCells *colorCells = d->colorToolBar->colorCells ();
    colorCells->deleteLastRow ();
}
