
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

#include <qradiobutton.h>
#include <qvbox.h>
#include <qvbuttongroup.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpviewmanager.h>

#include <kptoolflip.h>


/*
 * kpToolFlipCommand
 */

kpToolFlipCommand::kpToolFlipCommand (kpDocument *document, kpViewManager *viewManager,
                                      bool horiz, bool vert)
    : m_document (document), m_viewManager (viewManager),
      m_horiz (horiz), m_vert (vert)
{
}

// virtual
QString kpToolFlipCommand::name () const
{
    if (m_horiz && m_vert)
        return i18n ("Flip horizontally and vertically");
    else if (m_horiz)
        return i18n ("Flip horizontally");
    else if (m_vert)
        return i18n ("Flip vertically");
    else
    {
        kdError (KP_AREA) << "kpToolFlipCommand::name() not asked to flip" << endl;
        return QString::null;
    }
}

kpToolFlipCommand::~kpToolFlipCommand ()
{
}

// virtual
void kpToolFlipCommand::execute ()
{
    flip ();
}

// virtual
void kpToolFlipCommand::unexecute ()
{
    flip ();
}

// private
void kpToolFlipCommand::flip ()
{
    m_document->flip (m_horiz, m_vert);
}


/*
 * kpToolFlipDialog
 */

kpToolFlipDialog::kpToolFlipDialog (QWidget *parent)
    : KDialogBase (parent, 0/*name*/, true/*modal*/, i18n ("Flip Image"),
                   KDialogBase::Ok | KDialogBase::Cancel)
{
    QVBox *vbox = makeVBoxMainWidget ();

    if (!vbox)
    {
        kdError (KP_AREA) << "kpToolFlipDialog::kpToolFlipDialog() received NULL vbox" << endl;
    }
    else
    {
        QVButtonGroup *buttonGroup = new QVButtonGroup (i18n ("Flip"), vbox);

        // I'm sure vert flipping is much more common than horiz flipping so make it come first
        m_rbVertFlip = new QRadioButton (i18n ("&Vertically (upside-down)"), buttonGroup);
        m_rbVertFlip->setChecked (true);  // CONFIG
        m_rbHorizFlip = new QRadioButton (i18n ("&Horizontally"), buttonGroup);
    }
}

kpToolFlipDialog::~kpToolFlipDialog ()
{
}

bool kpToolFlipDialog::getHorizontalFlip () const
{
    return m_rbHorizFlip->isChecked ();
}

bool kpToolFlipDialog::getVerticalFlip () const
{
    return m_rbVertFlip->isChecked ();
}

bool kpToolFlipDialog::isNoopFlip () const
{
    return !getHorizontalFlip () && !getVerticalFlip ();
}

#include <kptoolflip.moc>
