
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

#include <qlabel.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kptoolskew.h>


/*
 * kpToolSkewCommand
 */

kpToolSkewCommand::kpToolSkewCommand (kpDocument *document, kpViewManager *viewManager,
                                      double hangle, double vangle)
    : m_document (document), m_viewManager (viewManager),
      m_hangle (hangle), m_vangle (vangle),
      m_oldPixmapPtr (0)
{
}

// virtual
QString kpToolSkewCommand::name () const
{
    if (m_hangle && m_vangle)
    {
        return i18n ("Skew %1 degrees horizontally, %2 degrees vertically")
                    .arg (m_hangle)
                    .arg (m_vangle);
    }
    else if (m_hangle)
    {
        return i18n ("Skew %1 degrees horizontally").arg (m_hangle);
    }
    else if (m_vangle)
    {
        return i18n ("Skew %1 degrees vertically").arg (m_vangle);
    }
    else
    {
        kdError (KP_AREA) << "kpToolSkewCommand::name() for pointless no-op skew" << endl;
        return i18n ("Pointless Skew");
    }
}

kpToolSkewCommand::~kpToolSkewCommand ()
{
}

// virtual
void kpToolSkewCommand::execute ()
{
    // OPT: can be avoided
    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *m_document->pixmap ();

    // (kpDocument skews horizontally in a convenient but unexpected way for positive hangle's)
    m_document->skew (-m_hangle, m_vangle);
}

void kpToolSkewCommand::unexecute ()
{
    m_document->setPixmap (*m_oldPixmapPtr);
    delete m_oldPixmapPtr;
    m_oldPixmapPtr = 0;
}


/*
 * kpToolSkewDialog
 */

kpToolSkewDialog::kpToolSkewDialog (QWidget *parent)
    : KDialogBase (parent, 0/*name*/, true/*modal*/, i18n ("Skew"),
                   KDialogBase::Ok | KDialogBase::Cancel)
{
    QWidget *page = new QWidget (this);
    setMainWidget (page);

    QGridLayout *lay = new QGridLayout (page, 4, 3, marginHint (), spacingHint ());

    m_inpHorizontalAngle = new KIntNumInput (page);
    lay->addWidget (m_inpHorizontalAngle, 0, 1);

    QLabel *lbHPixels = new QLabel (i18n ("degrees"), page);
    lay->addWidget (lbHPixels, 0, 2);

    m_inpVerticalAngle = new KIntNumInput (page);
    lay->addWidget (m_inpVerticalAngle, 2, 1);

    QLabel *lbVPixels = new QLabel (i18n ("degrees"), page);
    lay->addWidget (lbVPixels, 2, 2);
}

kpToolSkewDialog::~kpToolSkewDialog ()
{
}


double kpToolSkewDialog::horizontalAngle () const
{
    return m_inpHorizontalAngle->value ();
}

double kpToolSkewDialog::verticalAngle () const
{
    return m_inpVerticalAngle->value ();
}

bool kpToolSkewDialog::isNoop () const
{
    return (horizontalAngle () == 0) && (verticalAngle () == 0);
}

#include <kptoolskew.moc>
