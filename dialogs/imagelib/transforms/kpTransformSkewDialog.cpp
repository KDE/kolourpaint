
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


#define DEBUG_KP_TOOL_SKEW 0
#define DEBUG_KP_TOOL_SKEW_DIALOG 0


#include <kpTransformSkewDialog.h>

#include <qapplication.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmatrix.h>
#include <QImage>
#include <qpolygon.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpPixmapFX.h>
#include <kpTool.h>
#include <kpTransformDialogEnvironment.h>


// private static
int kpTransformSkewDialog::s_lastWidth = -1,
    kpTransformSkewDialog::s_lastHeight = -1;

// private static
int kpTransformSkewDialog::s_lastHorizontalAngle = 0,
    kpTransformSkewDialog::s_lastVerticalAngle = 0;


kpTransformSkewDialog::kpTransformSkewDialog (bool actOnSelection,
        kpTransformDialogEnvironment *_env, QWidget *parent)
    : kpTransformPreviewDialog (kpTransformPreviewDialog::AllFeatures,
        false/*don't reserve top row*/,
        actOnSelection ? i18nc ("@title:window", "Skew Selection") : i18nc ("@title:window", "Skew Image"),
        i18n ("After skew:"),
        actOnSelection,
        _env, parent)
{
    // Too confusing - disable for now
    s_lastHorizontalAngle = s_lastVerticalAngle = 0;


    createAngleGroupBox ();


    if (s_lastWidth > 0 && s_lastHeight > 0)
        resize (s_lastWidth, s_lastHeight);


    slotUpdate ();
    

    m_horizontalSkewInput->setEditFocus ();
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
    angleLayout->addWidget (m_horizontalSkewInput, 0, 2, Qt::AlignVCenter);
    angleLayout->addWidget (horizontalSkewDegreesLabel, 0, 3);

    angleLayout->addWidget (verticalSkewPixmapLabel, 1, 0);
    angleLayout->addWidget (verticalSkewLabel, 1, 1);
    angleLayout->addWidget (m_verticalSkewInput, 1, 2, Qt::AlignVCenter);
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

    QMatrix skewMatrix = kpPixmapFX::skewMatrix (doc->image (),
                                                  horizontalAngleForPixmapFX (),
                                                  verticalAngleForPixmapFX ());
    QRect skewRect = skewMatrix.mapRect (doc->rect (m_actOnSelection));

    return QSize (skewRect.width (), skewRect.height ());
}

// private virtual [base kpTransformPreviewDialog]
QImage kpTransformSkewDialog::transformPixmap (const QImage &image,
                                           int targetWidth, int targetHeight) const
{
    return kpPixmapFX::skew (image,
                             horizontalAngleForPixmapFX (),
                             verticalAngleForPixmapFX (),
                             m_environ->backgroundColor (m_actOnSelection),
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
        if (!document ()->textSelection ())
        {
            message =
                ki18n ("<qt><p>Skewing the selection to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure want to skew the selection?</p></qt>");

            caption = i18nc ("@title:window", "Skew Selection?");
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

        caption = i18nc ("@title:window", "Skew Image?");
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


#include <kpTransformSkewDialog.moc>
