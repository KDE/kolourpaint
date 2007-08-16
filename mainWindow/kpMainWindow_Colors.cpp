
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


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <kpColorCollection.h>
#include <KSelectAction>

#include <kpColorCells.h>
#include <kpColorToolBar.h>


static QStringList KDEColorCollectionNames ()
{
    return kpColorCollection::installedCollections ();
}


// private
void kpMainWindow::setupColorsMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    d->actionColorsDefault = ac->addAction ("colors_default");
    d->actionColorsDefault->setText (i18n ("Use KolourPaint Defaults"));
    connect (d->actionColorsDefault, SIGNAL (triggered (bool)),
        SLOT (slotColorsDefault ()));

    d->actionColorsKDE = ac->add <KSelectAction> ("colors_kde");
    d->actionColorsKDE->setText (i18n ("Use KDE's"));
    connect (d->actionColorsKDE, SIGNAL (triggered (QAction *)),
        SLOT (slotColorsKDE ()));
    foreach (QString colName, ::KDEColorCollectionNames ())
        d->actionColorsKDE->addAction (colName);
    // TODO: Add refresh submenu item.

    d->actionColorsOpen = ac->addAction ("colors_open");
    d->actionColorsOpen->setText (i18n ("Use File..."));
    connect (d->actionColorsOpen, SIGNAL (triggered (bool)),
        SLOT (slotColorsOpen ()));


    d->actionColorsSaveAs = ac->addAction ("colors_save_as");
    d->actionColorsSaveAs->setText (i18n ("Save As..."));
    connect (d->actionColorsSaveAs, SIGNAL (triggered (bool)),
        SLOT (slotColorsSaveAs ()));


    d->actionColorsAppendRow = ac->addAction ("colors_append_row");
    d->actionColorsAppendRow->setText (i18n ("Append Row"));
    connect (d->actionColorsAppendRow, SIGNAL (triggered (bool)),
        SLOT (slotColorsAppendRow ()));

    d->actionColorsDeleteRow = ac->addAction ("colors_delete_row");
    d->actionColorsDeleteRow->setText (i18n ("Delete Last Row"));
    connect (d->actionColorsDeleteRow, SIGNAL (triggered (bool)),
        SLOT (slotColorsDeleteRow ()));


    enableColorsMenuDocumentActions (false);
}

// private
void kpMainWindow::enableColorsMenuDocumentActions (bool enable)
{
    d->actionColorsDefault->setEnabled (enable);
    d->actionColorsKDE->setEnabled (enable);
    d->actionColorsOpen->setEnabled (enable);

    d->actionColorsSaveAs->setEnabled (enable);

    d->actionColorsAppendRow->setEnabled (enable);
    d->actionColorsDeleteRow->setEnabled (enable);
}


// private
void kpMainWindow::deselectActionColorsKDE ()
{
    d->actionColorsKDE->setCurrentItem (-1);
}


// private slot
void kpMainWindow::slotColorsDefault ()
{
    // Call just in case.
    toolEndShape ();

    colorCells ()->setColorCollection (
        kpColorCells::DefaultColorCollection ());

    deselectActionColorsKDE ();
}

// private slot
void kpMainWindow::slotColorsKDE ()
{
    // Call in case an error dialog appears.
    toolEndShape ();

    const QStringList colNames = ::KDEColorCollectionNames ();
    const int selected = d->actionColorsKDE->currentItem ();
    Q_ASSERT (selected >= 0 && selected < colNames.size ());

    kpColorCollection colorCol (colNames [selected]);
    // TODO: Failure?

    // TODO: modified?
    colorCells ()->setColorCollection (colorCol);
}

// private slot
void kpMainWindow::slotColorsOpen ()
{
    // Call due to dialog.
    toolEndShape ();

    KFileDialog fd (colorCells ()->url (), QString::null/*filter*/, this);	//krazy:exclude=nullstrassign for old broken gcc
    fd.setCaption (i18n ("Open Colors"));
    fd.setOperationMode (KFileDialog::Opening);

    if (fd.exec ())
    {
        // TODO: modified

        colorCells ()->openColorCollection (fd.selectedUrl ());

        deselectActionColorsKDE ();
    }
}


// private slot
void kpMainWindow::slotColorsSaveAs ()
{
    // Call due to dialog.
    toolEndShape ();

    KFileDialog fd (colorCells ()->url (), QString::null/*filter*/, this);	//krazy:exclude=nullstrassign for old broken gcc
    fd.setCaption (i18n ("Save Colors As"));
    fd.setOperationMode (KFileDialog::Saving);

    if (fd.exec ())
    {
        colorCells ()->saveColorCollectionAs (fd.selectedUrl ());

        // We're definitely using our own color collection now.
        deselectActionColorsKDE ();
    }
}


// private slot
void kpMainWindow::slotColorsAppendRow ()
{
    // Call just in case.
    toolEndShape ();

    kpColorCells *colorCells = d->colorToolBar->colorCells ();
    colorCells->appendRow ();
}

// private slot
void kpMainWindow::slotColorsDeleteRow ()
{
    // Call just in case.
    toolEndShape ();

    kpColorCells *colorCells = d->colorToolBar->colorCells ();
    colorCells->deleteLastRow ();
}
