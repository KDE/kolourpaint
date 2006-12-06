
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#include <kptoolskew.h>

#include <qapplication.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmatrix.h>
#include <qpixmap.h>
#include <qpolygon.h>

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


/*
 * kpTransformSkewCommand
 */

kpTransformSkewCommand::kpTransformSkewCommand (bool actOnSelection,
                                      int hangle, int vangle,
                                      kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_hangle (hangle), m_vangle (vangle),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor (actOnSelection) : kpColor::Invalid),
      m_oldPixmapPtr (0)
{
}

kpTransformSkewCommand::~kpTransformSkewCommand ()
{
    delete m_oldPixmapPtr;
}


// public virtual [base kpCommand]
QString kpTransformSkewCommand::name () const
{
    QString opName = i18n ("Skew");

    if (m_actOnSelection)
        return i18n ("Selection: %1", opName);
    else
        return opName;
}


// public virtual [base kpCommand]
int kpTransformSkewCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmapPtr) +
           m_oldSelection.size ();
}


// public virtual [base kpCommand]
void kpTransformSkewCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::skew (*doc->pixmap (m_actOnSelection),
                                          kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
                                          kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle),
                                          m_backgroundColor);

    if (m_actOnSelection)
    {
        kpSelection *sel = doc->selection ();

        // Save old selection
        m_oldSelection = *sel;


        // Calculate skewed points
        QPolygon currentPoints = sel->points ();
        currentPoints.translate (-currentPoints.boundingRect ().x (),
                                 -currentPoints.boundingRect ().y ());
        QMatrix skewMatrix = kpPixmapFX::skewMatrix (
            *doc->pixmap (m_actOnSelection),
            kpTransformSkewDialog::horizontalAngleForPixmapFX (m_hangle),
            kpTransformSkewDialog::verticalAngleForPixmapFX (m_vangle));
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
            kDebug () << "kpTransformSkewCommand::execute() currentPoints.boundingRect="
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

        Q_ASSERT (m_mainWindow->tool ());
        m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    else
    {
        doc->setPixmap (newPixmap);
    }


    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpTransformSkewCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QApplication::setOverrideCursor (Qt::WaitCursor);


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
 * kpTransformSkewDialog
 */


// private static
int kpTransformSkewDialog::s_lastWidth = -1,
    kpTransformSkewDialog::s_lastHeight = -1;

// private static
int kpTransformSkewDialog::s_lastHorizontalAngle = 0,
    kpTransformSkewDialog::s_lastVerticalAngle = 0;


kpTransformSkewDialog::kpTransformSkewDialog (bool actOnSelection, kpMainWindow *parent)
    : kpTransformPreviewDialog (kpTransformPreviewDialog::AllFeatures,
                           false/*don't reserve top row*/,
                           actOnSelection ? i18n ("Skew Selection") : i18n ("Skew Image"),
                           i18n ("After Skew:"),
                           actOnSelection, parent)
{
    // Too confusing - disable for now
    s_lastHorizontalAngle = s_lastVerticalAngle = 0;


    createAngleGroupBox ();


    if (s_lastWidth > 0 && s_lastHeight > 0)
        resize (s_lastWidth, s_lastHeight);


    slotUpdate ();
}

kpTransformSkewDialog::~kpTransformSkewDialog ()
{
    s_lastWidth = width (), s_lastHeight = height ();
}


// private
void kpTransformSkewDialog::createAngleGroupBox ()
{
    QGroupBox *angleGroupBox = new QGroupBox (i18n ("Angle"), mainWidget ());
    addCustomWidget (angleGroupBox);


    QLabel *horizontalSkewPixmapLabel = new QLabel (angleGroupBox);
    horizontalSkewPixmapLabel->setPixmap (UserIcon ("image_skew_horizontal"));

    QLabel *horizontalSkewLabel = new QLabel (i18n ("&Horizontal:"), angleGroupBox);
    m_horizontalSkewInput = new KIntNumInput (s_lastHorizontalAngle, angleGroupBox);
    m_horizontalSkewInput->setMinimum (-89);
    m_horizontalSkewInput->setMaximum (+89);

    QLabel *horizontalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    QLabel *verticalSkewPixmapLabel = new QLabel (angleGroupBox);
    verticalSkewPixmapLabel->setPixmap (UserIcon ("image_skew_vertical"));

    QLabel *verticalSkewLabel = new QLabel (i18n ("&Vertical:"), angleGroupBox);
    m_verticalSkewInput = new KIntNumInput (s_lastVerticalAngle, angleGroupBox);
    m_verticalSkewInput->setMinimum (-89);
    m_verticalSkewInput->setMaximum (+89);

    QLabel *verticalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    horizontalSkewLabel->setBuddy (m_horizontalSkewInput);
    verticalSkewLabel->setBuddy (m_verticalSkewInput);


    QGridLayout *angleLayout = new QGridLayout (angleGroupBox);
    angleLayout->setMargin (marginHint () * 2);
    angleLayout->setSpacing (spacingHint ());

    angleLayout->addWidget (horizontalSkewPixmapLabel, 0, 0);
    angleLayout->addWidget (horizontalSkewLabel, 0, 1);
    angleLayout->addWidget (m_horizontalSkewInput, 0, 2);
    angleLayout->addWidget (horizontalSkewDegreesLabel, 0, 3);

    angleLayout->addWidget (verticalSkewPixmapLabel, 1, 0);
    angleLayout->addWidget (verticalSkewLabel, 1, 1);
    angleLayout->addWidget (m_verticalSkewInput, 1, 2);
    angleLayout->addWidget (verticalSkewDegreesLabel, 1, 3);


    connect (m_horizontalSkewInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotUpdate ()));
    connect (m_verticalSkewInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotUpdate ()));
}


// private virtual [base kpTransformPreviewDialog]
QSize kpTransformSkewDialog::newDimensions () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    QMatrix skewMatrix = kpPixmapFX::skewMatrix (*doc->pixmap (),
                                                  horizontalAngleForPixmapFX (),
                                                  verticalAngleForPixmapFX ());
    QRect skewRect = skewMatrix.mapRect (doc->rect (m_actOnSelection));

    return QSize (skewRect.width (), skewRect.height ());
}

// private virtual [base kpTransformPreviewDialog]
QPixmap kpTransformSkewDialog::transformPixmap (const QPixmap &pixmap,
                                           int targetWidth, int targetHeight) const
{
    return kpPixmapFX::skew (pixmap,
                             horizontalAngleForPixmapFX (),
                             verticalAngleForPixmapFX (),
                             m_mainWindow ? m_mainWindow->backgroundColor (m_actOnSelection) : kpColor::Invalid,
                             targetWidth,
                             targetHeight);
}


// private
void kpTransformSkewDialog::updateLastAngles ()
{
    s_lastHorizontalAngle = horizontalAngle ();
    s_lastVerticalAngle = verticalAngle ();
}

// private slot virtual [base kpTransformPreviewDialog]
void kpTransformSkewDialog::slotUpdate ()
{
    updateLastAngles ();
    kpTransformPreviewDialog::slotUpdate ();
}


// public
int kpTransformSkewDialog::horizontalAngle () const
{
    return m_horizontalSkewInput->value ();
}

// public
int kpTransformSkewDialog::verticalAngle () const
{
    return m_verticalSkewInput->value ();
}


// public static
int kpTransformSkewDialog::horizontalAngleForPixmapFX (int hangle)
{
    return -hangle;
}

// public static
int kpTransformSkewDialog::verticalAngleForPixmapFX (int vangle)
{
    return -vangle;
}


// public
int kpTransformSkewDialog::horizontalAngleForPixmapFX () const
{
    return kpTransformSkewDialog::horizontalAngleForPixmapFX (horizontalAngle ());
}

// public
int kpTransformSkewDialog::verticalAngleForPixmapFX () const
{
    return kpTransformSkewDialog::verticalAngleForPixmapFX (verticalAngle ());
}


// public virtual [base kpTransformPreviewDialog]
bool kpTransformSkewDialog::isNoOp () const
{
    return (horizontalAngle () == 0) && (verticalAngle () == 0);
}


// private slot virtual [base QDialog]
void kpTransformSkewDialog::accept ()
{
    KLocalizedString message;
    QString caption, continueButtonText;

    if (document ()->selection ())
    {
        if (!document ()->selection ()->isText ())
        {
            message =
                ki18n ("<qt><p>Skewing the selection to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure want to skew the selection?</p></qt>");

            caption = i18n ("Skew Selection?");
            continueButtonText = i18n ("Sk&ew Selection");
        }
    }
    else
    {
        message =
            ki18n ("<qt><p>Skewing the image to %1x%2"
                  " may take a substantial amount of memory."
                  " This can reduce system"
                  " responsiveness and cause other application resource"
                  " problems.</p>"

                  "<p>Are you sure want to skew the image?</p></qt>");

        caption = i18n ("Skew Image?");
        continueButtonText = i18n ("Sk&ew Image");
    }


    const int newWidth = newDimensions ().width ();
    const int newHeight = newDimensions ().height ();

    if (kpTool::warnIfBigImageSize (m_oldWidth,
            m_oldHeight,
            newWidth, newHeight,
            message.subs (newWidth).subs (newHeight).toString (),
            caption,
            continueButtonText,
            this))
    {
        KDialog::accept ();
    }
}


#include <kptoolskew.moc>
