
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <kpconfigdialog.h>

kpConfigDialog::kpConfigDialog ()
    : KDialogBase (int (KDialogBase::IconList), i18n ("Configure"),
                   int (KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel),
                   KDialogBase::Ok)
{
    setupGeneralPage ();
    setupGridPage ();
    setupNotificationsPage ();
}

kpConfigDialog::~kpConfigDialog ()
{
}


/*
 * General Page
 */

void kpConfigDialog::setupGeneralPage ()
{
    QVBox *page = addVBoxPage (i18n ("General"));

    m_cbOpenDocSameWindow = new QCheckBox (i18n ("Open documents in the &same window"), page);

    m_inpRecentFiles = new KIntNumInput (page);
    m_inpRecentFiles->setLabel (i18n ("Number of &recent files:"));
    m_inpRecentFiles->setRange (0, 20);

    m_inpUndoRedoLimit = new KIntNumInput (page);
    m_inpUndoRedoLimit->setLabel (i18n ("&Undo/redo limit:"));
    m_inpUndoRedoLimit->setRange (0, 20);
    connect (m_inpUndoRedoLimit, SIGNAL (valueChanged (int)), SLOT (slotCheckHighUndoRedoLimit (int)));

    m_cbDoubleBuffer = new QCheckBox (i18n ("Double buffer to reduce flicker (uses a lot of memory)"), page);
}

void kpConfigDialog::slotCheckHighUndoRedoLimit (int limit)
{
    if (limit >= 5)
    {
        int result = KMessageBox::warningYesNo (this,
            i18n ("Storing %1 levels of undo/redo information for pixmaps may consume a huge amount of memory. "
                  "This can potentially cause other programs to run more slowly or more serious system malfunctions.\n"
                  "Are you sure want to set the limit this high?").arg (limit),
            QString::null,
            KStdGuiItem::yes (),
            KStdGuiItem::no (),
            QString ("DoNotAskAgainSetHighUndoRedoLimit"));
    }
}


/*
 * Grid Page
 */

void kpConfigDialog::setupGridPage ()
{
    QFrame *page = addPage (i18n ("Grid"));
    QGridLayout *grid = new QGridLayout (page, 1, 3, marginHint (), spacingHint ());

    QCheckBox *cbTiledGrid = new QCheckBox (i18n ("Tile grid every: "), page);
    grid->addWidget (cbTiledGrid, 0, 0);

    KNumInput *inpGridTileSize = new KIntNumInput (16, page);
    grid->addWidget (inpGridTileSize, 0, 1);

    QLabel *lbPixels = new QLabel (i18n ("pixels"), page);
    grid->addWidget (lbPixels, 0, 2);

    connect (cbTiledGrid, SIGNAL (toggled (bool)), inpGridTileSize, SLOT (setEnabled (bool)));
    cbTiledGrid->setChecked (false);
}

/*
 * Notifications Page
 */

void kpConfigDialog::setupNotificationsPage ()
{
    QVBox *page = addVBoxPage (i18n ("Notifications"));
    QLabel *label = new QLabel (i18n ("Warn me about:"), page);

    QCheckBox *cbHighUndoRedoLimit = new QCheckBox (i18n ("Setting the Undo Limit too high"), page);
    QCheckBox *cbOpenImageBitDepth = new QCheckBox (i18n ("Opening an image that has a higher bit depth than the display"), page);
    QCheckBox *cbSaveImageBitDepth = new QCheckBox (i18n ("Saving an image at a lower bit depth than what is being displayed"), page);
    QCheckBox *cbSaveLossy = new QCheckBox (i18n ("Saving an image in a lossy file format"), page);

}

#include <kpconfigdialog.moc>
