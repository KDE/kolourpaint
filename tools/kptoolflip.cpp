
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kptoolflip.h>

#include <qapplication.h>
#include <qradiobutton.h>
#include <qvbox.h>
#include <qvbuttongroup.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kpmainwindow.h>


/*
 * kpToolFlipCommand
 */

kpToolFlipCommand::kpToolFlipCommand (bool actOnSelection,
                                      bool horiz, bool vert,
                                      kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_horiz (horiz), m_vert (vert)
{
}

kpToolFlipCommand::~kpToolFlipCommand ()
{
}


// public virtual [base kpCommand]
QString kpToolFlipCommand::name () const
{
    QString opName;

    
#if 1
    opName = i18n ("Flip");
#else  // re-enable when giving full descriptions for all actions
    if (m_horiz && m_vert)
        opName = i18n ("Flip horizontally and vertically");
    else if (m_horiz)
        opName = i18n ("Flip horizontally");
    else if (m_vert)
        opName = i18n ("Flip vertically");
    else
    {
        kdError () << "kpToolFlipCommand::name() not asked to flip" << endl;
        return QString::null;
    }
#endif


    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}


// public virtual [base kpCommand]
int kpToolFlipCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolFlipCommand::execute ()
{
    flip ();
}

// public virtual [base kpCommand]
void kpToolFlipCommand::unexecute ()
{
    flip ();
}


// private
void kpToolFlipCommand::flip ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    QApplication::setOverrideCursor (Qt::waitCursor);

    
    if (m_actOnSelection)
    {
        doc->selection ()->flip (m_horiz, m_vert);
        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    else
    {
        QPixmap newPixmap = kpPixmapFX::flip (*doc->pixmap (), m_horiz, m_vert);

        doc->setPixmap (newPixmap);
    }


    QApplication::restoreOverrideCursor ();
}


/*
 * kpToolFlipDialog
 */

// private static
bool kpToolFlipDialog::s_lastIsVerticalFlip = true;


kpToolFlipDialog::kpToolFlipDialog (bool actOnSelection, QWidget *parent)
    : KDialogBase (parent, 0/*name*/, true/*modal*/,
                   actOnSelection ? i18n ("Flip Selection") : i18n ("Flip Image"),
                   KDialogBase::Ok | KDialogBase::Cancel)
{
    QVBox *vbox = makeVBoxMainWidget ();

    if (!vbox)
    {
        kdError () << "kpToolFlipDialog::kpToolFlipDialog() received NULL vbox" << endl;
    }
    else
    {
        QVButtonGroup *buttonGroup = new QVButtonGroup (i18n ("Direction"), vbox);

        // I'm sure vert flipping is much more common than horiz flipping so make it come first
        m_verticalFlipRadioButton = new QRadioButton (i18n ("&Vertical (upside-down)"), buttonGroup);
        m_horizontalFlipRadioButton = new QRadioButton (i18n ("&Horizontal"), buttonGroup);

        m_verticalFlipRadioButton->setChecked (s_lastIsVerticalFlip);
        m_horizontalFlipRadioButton->setChecked (!s_lastIsVerticalFlip);

        connect (m_verticalFlipRadioButton, SIGNAL (toggled (bool)),
                 this, SLOT (slotIsVerticalFlipChanged ()));
        connect (m_horizontalFlipRadioButton, SIGNAL (toggled (bool)),
                 this, SLOT (slotIsVerticalFlipChanged ()));
    }
}

kpToolFlipDialog::~kpToolFlipDialog ()
{
}


// public slot
void kpToolFlipDialog::slotIsVerticalFlipChanged ()
{
    s_lastIsVerticalFlip = m_verticalFlipRadioButton->isChecked ();
}


// public
bool kpToolFlipDialog::getHorizontalFlip () const
{
    return m_horizontalFlipRadioButton->isChecked ();
}

// public
bool kpToolFlipDialog::getVerticalFlip () const
{
    return m_verticalFlipRadioButton->isChecked ();
}

// public
bool kpToolFlipDialog::isNoOp () const
{
    return !getHorizontalFlip () && !getVerticalFlip ();
}


#include <kptoolflip.moc>

