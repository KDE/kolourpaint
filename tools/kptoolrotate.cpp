
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <knuminput.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptoolrotate.h>
#include <kpviewmanager.h>


kpToolRotateCommand::kpToolRotateCommand (bool actOnSelection,
                                          double angle,
                                          kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_angle (angle),
      m_mainWindow (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid),
      m_losslessRotation (kpPixmapFX::isLosslessRotation (angle))
{
}

// public virtual [base KCommand]
QString kpToolRotateCommand::name () const
{
    QString opName = i18n ("Rotate");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}

kpToolRotateCommand::~kpToolRotateCommand ()
{
}


// private
kpDocument *kpToolRotateCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

// private
kpViewManager *kpToolRotateCommand::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}


// public virtual [base KCommand]
void kpToolRotateCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    if (!m_losslessRotation)
        m_oldPixmap = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::rotate (*doc->pixmap (m_actOnSelection),
                                            m_angle,
                                            m_backgroundColor);


    if (m_actOnSelection)
    {
        kpViewManager *vm = viewManager ();
        vm->setQueueUpdates ();

        kpSelection *sel = doc->selection ();

        m_oldTopLeft = sel->boundingRect ().topLeft ();
        int oldWidth = sel->width ();
        int oldHeight = sel->height ();
        // TODO: with so many "/ 2"'s around, should use floating point
        QPoint center = m_oldTopLeft + QPoint (oldWidth / 2, oldHeight / 2);

        sel->setPixmap (newPixmap);
        sel->moveTo (center - QPoint (newPixmap.width () / 2, newPixmap.height () / 2));

        vm->restoreQueueUpdates ();
    }
    else
        doc->setPixmap (newPixmap);
}

// public virtual [base KCommand]
void kpToolRotateCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    kpSelection *sel = doc->selection ();

    kpViewManager *vm = viewManager ();
    if (!vm)
        return;

    vm->setQueueUpdates ();


    if (!m_losslessRotation)
    {
        doc->setPixmap (m_actOnSelection, m_oldPixmap);
    }
    else
    {
        QPixmap oldPixmap = kpPixmapFX::rotate (*doc->pixmap (m_actOnSelection),
                                                360 - m_angle,
                                                m_backgroundColor);
        doc->setPixmap (m_actOnSelection, oldPixmap);
    }


    // keep centre of selection
    if (m_actOnSelection)
        sel->moveTo (m_oldTopLeft);


    vm->restoreQueueUpdates ();
    m_oldPixmap.resize (0, 0);
}


kpToolRotateDialog::kpToolRotateDialog (QWidget *parent)
    : KDialogBase (parent, 0/*name*/, true/*modal*/, i18n ("Rotate Image"),
                   KDialogBase::Ok | KDialogBase::Cancel)
{
    QGroupBox *page = new QGroupBox (i18n ("Rotate"), this);
    setMainWidget (page);

    QGridLayout *lay = new QGridLayout (page, 4, 3, marginHint (), spacingHint ());

    QButtonGroup *buttonGroup = new QButtonGroup (this);
    buttonGroup->hide ();  // invisible

    m_rbRotateLeft = new QRadioButton (i18n ("&Left"), page);
    buttonGroup->insert (m_rbRotateLeft);
    lay->addMultiCellWidget (m_rbRotateLeft, 0, 0, 0, 2);

    m_rbRotateRight = new QRadioButton (i18n ("&Right"), page);
    buttonGroup->insert (m_rbRotateRight);
    lay->addMultiCellWidget (m_rbRotateRight, 1, 1, 0, 2);

    m_rbRotate180 = new QRadioButton (i18n ("180 &degrees"), page);
    buttonGroup->insert (m_rbRotate180);
    lay->addMultiCellWidget (m_rbRotate180, 2, 2, 0, 2);

    m_rbRotateArbitrary = new QRadioButton (i18n ("&Angle: "), page);
    buttonGroup->insert (m_rbRotateArbitrary);
    lay->addWidget (m_rbRotateArbitrary, 3, 0);

    m_inpAngle = new KDoubleNumInput (0 /*CONFIG*/, page);
    m_inpAngle->setRange (-359.99, 359.99, 1/*step*/, false/*no slider*/);
    m_inpAngle->setPrecision (2);
    lay->addWidget (m_inpAngle, 3, 1);

    QLabel *lbDegrees = new QLabel (i18n ("degrees"), page);
    lay->addWidget (lbDegrees, 3, 2);

    m_inpAngle->setEnabled (false);
    lbDegrees->setEnabled (false);

    connect (m_rbRotateArbitrary, SIGNAL (toggled (bool)),
             m_inpAngle, SLOT (setEnabled (bool)));
    connect (m_rbRotateArbitrary, SIGNAL (toggled (bool)),
             lbDegrees, SLOT (setEnabled (bool)));
    connect (m_rbRotateArbitrary, SIGNAL (toggled (bool)),
             SLOT (slotAngleChanged ()));
    connect (m_inpAngle, SIGNAL (valueChanged (double)),
             SLOT (slotAngleChanged ()));

    m_rbRotateLeft->setChecked (true);  // CONFIG
}

kpToolRotateDialog::~kpToolRotateDialog ()
{
}

void kpToolRotateDialog::slotAngleChanged ()
{
    enableButtonOK (m_inpAngle->value () != 0);
}

double kpToolRotateDialog::angle () const
{
    if (m_rbRotateLeft->isChecked ())
        return 270;
    else if (m_rbRotateRight->isChecked ())
        return 90;
    else if (m_rbRotate180->isChecked ())
        return 180;
    else // if (m_rbRotateArbitrary->isChecked ())
    {
        double angle = m_inpAngle->value ();

        if (angle < 0)
            angle += ((0 - angle) / 360 + 1) * 360;

        if (angle >= 360)
            angle -= ((angle - 360) / 360 + 1) * 360;

        return angle;
    }
}

bool kpToolRotateDialog::isNoopRotate () const
{
    return angle () == 0;
}

#include <kptoolrotate.moc>


/*


kpToolRotate::kpToolRotate (kpMainWindow *mainWindow)
    : kpTool (mainWindow)
{
}

kpToolRotate::~kpToolRotate ()
{
}

// virtual
void kpToolRotate::begin ()
{
    viewManager ()->setCursor (QCursor (SizeAllCursor));
    m_startAngle = angle ();
}

// virtual
void kpToolRotate::end ()
{
    m_oldPixmap.resize (0, 0);
    viewManager ()->unsetCursor ();
}

// virtual
void kpToolRotate::beginDraw ()
{
    m_oldPixmap = *document ()->pixmap ();
}

// virtual
void kpToolRotate::draw ()
{
    double newAngle = angle ();

    document ()->setPixmap (m_oldPixmap);
    m_lastAngle = newAngle;
}

// virtual
void kpToolRotate::cancelShape ()
{
    document ()->setPixmapAt (m_oldPixmap, QPoint (0, 0));
}

// virtual
void kpToolRotate::endDraw ()
{
    mainWindow ()->commandHistory ()->addCommand (
        new kpToolRotateCommand (document (), viewManager (), angle ()));
}
// static
double kpToolRotate

// returns in degrees
double kpToolRotate::angle () const
{
    int centerX = m_oldPixmap.width () / 2,
        centerY = m_oldPixmap.height () / 2;

    return
}
*/
