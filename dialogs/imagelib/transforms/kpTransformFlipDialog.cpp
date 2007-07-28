
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


#include <kpTransformFlipDialog.h>

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <klocale.h>
#include <kvbox.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpPixmapFX.h>
#include <kpTool.h>


// private static
bool kpTransformFlipDialog::s_lastIsVerticalFlip = true;


kpTransformFlipDialog::kpTransformFlipDialog (bool actOnSelection, QWidget *parent)
    : KDialog (parent)
{
    setCaption (actOnSelection ? i18n ("Flip Selection") : i18n ("Flip Image"));
    setButtons (KDialog::Ok | KDialog::Cancel);
    QGroupBox *groupBox = new QGroupBox (i18n ("Direction"), this);

    // I'm sure vert flipping is much more common than horiz flipping so make it come first
    m_verticalFlipRadioButton = new QRadioButton (i18n ("&Vertical (upside-down)"), groupBox);
    m_horizontalFlipRadioButton = new QRadioButton (i18n ("&Horizontal"), groupBox);

    m_verticalFlipRadioButton->setChecked (s_lastIsVerticalFlip);
    m_horizontalFlipRadioButton->setChecked (!s_lastIsVerticalFlip);

    connect (m_verticalFlipRadioButton, SIGNAL (toggled (bool)),
             this, SLOT (slotIsVerticalFlipChanged ()));
    connect (m_horizontalFlipRadioButton, SIGNAL (toggled (bool)),
             this, SLOT (slotIsVerticalFlipChanged ()));

    QVBoxLayout *groupBoxLayout = new QVBoxLayout (groupBox);
    groupBoxLayout->addWidget (m_verticalFlipRadioButton);
    groupBoxLayout->addWidget (m_horizontalFlipRadioButton);

    setMainWidget (groupBox);
}

kpTransformFlipDialog::~kpTransformFlipDialog ()
{
}


// public slot
void kpTransformFlipDialog::slotIsVerticalFlipChanged ()
{
    s_lastIsVerticalFlip = m_verticalFlipRadioButton->isChecked ();
}


// public
bool kpTransformFlipDialog::getHorizontalFlip () const
{
    return m_horizontalFlipRadioButton->isChecked ();
}

// public
bool kpTransformFlipDialog::getVerticalFlip () const
{
    return m_verticalFlipRadioButton->isChecked ();
}

// public
bool kpTransformFlipDialog::isNoOp () const
{
    return !getHorizontalFlip () && !getVerticalFlip ();
}


#include <kpTransformFlipDialog.moc>

