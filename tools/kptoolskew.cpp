
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

#define DEBUG_KP_TOOL_SKEW 0
#define DEBUG_KP_TOOL_SKEW_DIALOG 0


#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptoolskew.h>


/*
 * kpToolSkewCommand
 */

kpToolSkewCommand::kpToolSkewCommand (bool actOnSelection,
                                      int hangle, int vangle,
                                      kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_hangle (hangle), m_vangle (vangle),
      m_mainWindow (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor (actOnSelection) : kpColor::invalid),
      m_oldPixmapPtr (0)
{
}

// public virtual [base KCommand]
QString kpToolSkewCommand::name () const
{
    QString opName = i18n ("Skew");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}

kpToolSkewCommand::~kpToolSkewCommand ()
{
    delete m_oldPixmapPtr;
}


// private
kpDocument *kpToolSkewCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolSkewCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    QApplication::setOverrideCursor (Qt::waitCursor);


    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::skew (*doc->pixmap (m_actOnSelection),
                                          kpToolSkewDialog::horizontalAngleForPixmapFX (m_hangle),
                                          kpToolSkewDialog::verticalAngleForPixmapFX (m_vangle),
                                          m_backgroundColor);

    if (m_actOnSelection)
    {
        kpSelection *sel = doc->selection ();

        // Save old selection
        m_oldSelection = *sel;


        // Calculate skewed points
        QPointArray currentPoints = sel->points ();
        currentPoints.translate (-currentPoints.boundingRect ().x (),
                                 -currentPoints.boundingRect ().y ());
        QWMatrix skewMatrix = kpPixmapFX::skewMatrix (
            *doc->pixmap (m_actOnSelection),
            kpToolSkewDialog::horizontalAngleForPixmapFX (m_hangle),
            kpToolSkewDialog::verticalAngleForPixmapFX (m_vangle));
        currentPoints = skewMatrix.map (currentPoints);
        currentPoints.translate (-currentPoints.boundingRect ().x () + m_oldSelection.x (),
                                 -currentPoints.boundingRect ().y () + m_oldSelection.y ());


        if (currentPoints.boundingRect ().width () == newPixmap.width () &&
            currentPoints.boundingRect ().height () == newPixmap.height ())
        {
            doc->setSelection (kpSelection (currentPoints, newPixmap,
                                            m_oldSelection.transparency ()));
        }
        else
        {
            // TODO: fix the latter "victim of" problem in kpSelection by
            //       allowing the border width & height != pixmap width & height
            //       Or maybe autocrop?
        #if DEBUG_KP_TOOL_SKEW
            kdDebug () << "kpToolSkewCommand::execute() currentPoints.boundingRect="
                       << currentPoints.boundingRect ()
                       << " newPixmap: w=" << newPixmap.width ()
                       << " h=" << newPixmap.height ()
                       << " (victim of rounding error and/or skewed-a-(rectangular)-pixmap-that-was-transparent-in-the-corners-making-sel-uselessly-bigger-than-needs-be))"
                       << endl;
        #endif
            doc->setSelection (kpSelection (kpSelection::Rectangle,
                                            QRect (currentPoints.boundingRect ().x (),
                                                   currentPoints.boundingRect ().y (),
                                                   newPixmap.width (),
                                                   newPixmap.height ()),
                                            newPixmap,
                                            m_oldSelection.transparency ()));
        }

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    else
    {
        doc->setPixmap (newPixmap);
    }


    QApplication::restoreOverrideCursor ();
}

// public virtual [base KCommand]
void kpToolSkewCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    QApplication::setOverrideCursor (Qt::waitCursor);


    QPixmap oldPixmap = *m_oldPixmapPtr;
    delete m_oldPixmapPtr; m_oldPixmapPtr = 0;


    if (!m_actOnSelection)
        doc->setPixmap (oldPixmap);
    else
    {
        kpSelection oldSelection = m_oldSelection;
        doc->setSelection (oldSelection);

        if (m_mainWindow->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }


    QApplication::restoreOverrideCursor ();
}


/*
 * kpToolSkewDialog
 */


// private static
int kpToolSkewDialog::s_lastHorizontalAngle = 0,
    kpToolSkewDialog::s_lastVerticalAngle = 0;


kpToolSkewDialog::kpToolSkewDialog (bool actOnSelection, kpMainWindow *parent,
                                    const char *name)
    : kpToolPreviewDialog (i18n ("Skew"), actOnSelection, parent, name)
{
    // Too confusing - disable for now
    s_lastHorizontalAngle = s_lastVerticalAngle = 0;

    createAngleGroupBox ();
}

kpToolSkewDialog::~kpToolSkewDialog ()
{
}


// private
void kpToolSkewDialog::createAngleGroupBox ()
{
    QGroupBox *angleGroupBox = new QGroupBox (i18n ("Angle"), mainWidget ());
    addCustomWidget (angleGroupBox);


    QLabel *horizontalSkewPixmapLabel = new QLabel (angleGroupBox);
    horizontalSkewPixmapLabel->setPixmap (UserIcon ("image_skew_horizontal"));

    QLabel *horizontalSkewLabel = new QLabel (i18n ("&Horizontal:"), angleGroupBox);
    m_horizontalSkewInput = new KIntNumInput (s_lastHorizontalAngle, angleGroupBox);
    m_horizontalSkewInput->setMinValue (-89);
    m_horizontalSkewInput->setMaxValue (+89);

    QLabel *horizontalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    QLabel *verticalSkewPixmapLabel = new QLabel (angleGroupBox);
    verticalSkewPixmapLabel->setPixmap (UserIcon ("image_skew_vertical"));

    QLabel *verticalSkewLabel = new QLabel (i18n ("&Vertical:"), angleGroupBox);
    m_verticalSkewInput = new KIntNumInput (s_lastVerticalAngle, angleGroupBox);
    m_verticalSkewInput->setMinValue (-89);
    m_verticalSkewInput->setMaxValue (+89);

    QLabel *verticalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    QWidget *verticalSpaceWidget = new QWidget (angleGroupBox);
    verticalSpaceWidget->setMinimumSize (1, spacingHint ());
    QPushButton *updatePreviewPushButton = new QPushButton (i18n ("Update &Preview"), angleGroupBox);


    horizontalSkewLabel->setBuddy (m_horizontalSkewInput);
    verticalSkewLabel->setBuddy (m_verticalSkewInput);


    QGridLayout *angleLayout = new QGridLayout (angleGroupBox, 4, 4,
                                                marginHint () * 2, spacingHint ());

    angleLayout->addWidget (horizontalSkewPixmapLabel, 0, 0);
    angleLayout->addWidget (horizontalSkewLabel, 0, 1);
    angleLayout->addWidget (m_horizontalSkewInput, 0, 2);
    angleLayout->addWidget (horizontalSkewDegreesLabel, 0, 3);

    angleLayout->addWidget (verticalSkewPixmapLabel, 1, 0);
    angleLayout->addWidget (verticalSkewLabel, 1, 1);
    angleLayout->addWidget (m_verticalSkewInput, 1, 2);
    angleLayout->addWidget (verticalSkewDegreesLabel, 1, 3);

    angleLayout->addMultiCellWidget (verticalSpaceWidget, 2, 2, 0, 3);
    angleLayout->addMultiCellWidget (updatePreviewPushButton, 3, 3, 0, 3, Qt::AlignRight);


    connect (m_horizontalSkewInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotUpdate ()));
    connect (m_verticalSkewInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotUpdate ()));

    connect (updatePreviewPushButton, SIGNAL (clicked ()),
             this, SLOT (slotUpdate ()));

    slotUpdate ();
}


// private virtual [base kpToolPreviewDialog]
QSize kpToolSkewDialog::newDimensions () const
{
    kpDocument *doc = document ();
    if (!doc)
        return QSize ();

    QWMatrix skewMatrix = kpPixmapFX::skewMatrix (*doc->pixmap (),
                                                  horizontalAngleForPixmapFX (),
                                                  verticalAngleForPixmapFX ());
    QRect skewRect = skewMatrix.mapRect (doc->rect (m_actOnSelection));

    return QSize (skewRect.width (), skewRect.height ());
}

// private virtual [base kpToolPreviewDialog]
QPixmap kpToolSkewDialog::transformPixmap (const QPixmap &pixmap,
                                           int targetWidth, int targetHeight) const
{
    return kpPixmapFX::skew (pixmap,
                             horizontalAngleForPixmapFX (),
                             verticalAngleForPixmapFX (),
                             m_mainWindow ? m_mainWindow->backgroundColor (m_actOnSelection) : kpColor::invalid,
                             targetWidth,
                             targetHeight);
}


// private
void kpToolSkewDialog::updateLastAngles ()
{
    s_lastHorizontalAngle = horizontalAngle ();
    s_lastVerticalAngle = verticalAngle ();
}

// private slot virtual [base kpToolPreviewDialog]
void kpToolSkewDialog::slotUpdate ()
{
    updateLastAngles ();
    kpToolPreviewDialog::slotUpdate ();
}


// public
int kpToolSkewDialog::horizontalAngle () const
{
    return m_horizontalSkewInput->value ();
}

// public
int kpToolSkewDialog::verticalAngle () const
{
    return m_verticalSkewInput->value ();
}


// public static
int kpToolSkewDialog::horizontalAngleForPixmapFX (int hangle)
{
    return -hangle;
}

// public static
int kpToolSkewDialog::verticalAngleForPixmapFX (int vangle)
{
    return -vangle;
}


// public
int kpToolSkewDialog::horizontalAngleForPixmapFX () const
{
    return kpToolSkewDialog::horizontalAngleForPixmapFX (horizontalAngle ());
}

// public
int kpToolSkewDialog::verticalAngleForPixmapFX () const
{
    return kpToolSkewDialog::verticalAngleForPixmapFX (verticalAngle ());
}


// public virtual [base kpToolPreviewDialog]
bool kpToolSkewDialog::isNoOp () const
{
    return (horizontalAngle () == 0) && (verticalAngle () == 0);
}

#include <kptoolskew.moc>
