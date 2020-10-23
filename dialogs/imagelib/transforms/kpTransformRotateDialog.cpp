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

#define DEBUG_KP_TOOL_ROTATE 0


#include "kpTransformRotateDialog.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"
#include "views/manager/kpViewManager.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>
#include <QPolygon>
#include <QPushButton>
#include <QRadioButton>
#include <QTransform>


// private static
int kpTransformRotateDialog::s_lastWidth = -1,
    kpTransformRotateDialog::s_lastHeight = -1;

// private static
bool kpTransformRotateDialog::s_lastIsClockwise = true;
int kpTransformRotateDialog::s_lastAngleCustom = 0;


kpTransformRotateDialog::kpTransformRotateDialog (bool actOnSelection,
        kpTransformDialogEnvironment *_env, QWidget *parent)
    : kpTransformPreviewDialog (kpTransformPreviewDialog::AllFeatures,
        false/*don't reserve top row*/,
        actOnSelection ? i18nc ("@title:window", "Rotate Selection") : i18nc ("@title:window", "Rotate Image"),
        i18n ("After rotate:"),
        actOnSelection,
        _env, parent)
{
    s_lastAngleCustom = 0;


    createDirectionGroupBox ();
    createAngleGroupBox ();


    if (s_lastWidth > 0 && s_lastHeight > 0) {
        resize (s_lastWidth, s_lastHeight);
    }


    slotAngleCustomRadioButtonToggled (m_angleCustomRadioButton->isChecked ());
    slotUpdate ();
}

kpTransformRotateDialog::~kpTransformRotateDialog ()
{
    s_lastWidth = width ();
    s_lastHeight = height ();
}


// private
void kpTransformRotateDialog::createDirectionGroupBox ()
{
    auto *directionGroupBox = new QGroupBox (i18n ("Direction"), mainWidget ());
    addCustomWidget (directionGroupBox);


    auto *antiClockwisePixmapLabel = new QLabel (directionGroupBox);
    antiClockwisePixmapLabel->setPixmap (QStringLiteral(":/icons/image_rotate_anticlockwise"));

    auto *clockwisePixmapLabel = new QLabel (directionGroupBox);
    clockwisePixmapLabel->setPixmap (QStringLiteral(":/icons/image_rotate_clockwise"));


    m_antiClockwiseRadioButton = new QRadioButton (i18n ("Cou&nterclockwise"), directionGroupBox);
    m_clockwiseRadioButton = new QRadioButton (i18n ("C&lockwise"), directionGroupBox);


    m_antiClockwiseRadioButton->setChecked (!s_lastIsClockwise);
    m_clockwiseRadioButton->setChecked (s_lastIsClockwise);


    auto *directionLayout = new QGridLayout (directionGroupBox );
    directionLayout->addWidget (antiClockwisePixmapLabel, 0, 0, Qt::AlignCenter);
    directionLayout->addWidget (clockwisePixmapLabel, 0, 1, Qt::AlignCenter);
    directionLayout->addWidget (m_antiClockwiseRadioButton, 1, 0, Qt::AlignCenter);
    directionLayout->addWidget (m_clockwiseRadioButton, 1, 1, Qt::AlignCenter);


    connect (m_antiClockwiseRadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);

    connect (m_clockwiseRadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);
}

// private
void kpTransformRotateDialog::createAngleGroupBox ()
{
    auto *angleGroupBox = new QGroupBox (i18n ("Angle"), mainWidget ());
    addCustomWidget (angleGroupBox);


    m_angle90RadioButton = new QRadioButton (i18n ("90 &degrees"), angleGroupBox);
    m_angle180RadioButton = new QRadioButton (i18n ("180 d&egrees"), angleGroupBox);
    m_angle270RadioButton = new QRadioButton (i18n ("270 de&grees"), angleGroupBox);

    m_angleCustomRadioButton = new QRadioButton (i18n ("C&ustom:"), angleGroupBox);
    m_angleCustomInput = new QSpinBox;
    m_angleCustomInput->setMinimum(-359);
    m_angleCustomInput->setMaximum(+359);
    m_angleCustomInput->setValue(s_lastAngleCustom);
    auto *degreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    m_angleCustomRadioButton->setChecked (true);


    auto *angleLayout = new QGridLayout (angleGroupBox );

    angleLayout->addWidget (m_angle90RadioButton, 0, 0, 1, 3);
    angleLayout->addWidget (m_angle180RadioButton, 1, 0, 1, 3);
    angleLayout->addWidget (m_angle270RadioButton, 2, 0, 1, 3);

    angleLayout->addWidget (m_angleCustomRadioButton, 3, 0);
    angleLayout->addWidget (m_angleCustomInput, 3, 1);
    angleLayout->addWidget (degreesLabel, 3, 2);

    angleLayout->setColumnStretch (1, 2);  // Stretch Custom Angle Input



    connect (m_angle90RadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);
    connect (m_angle180RadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);
    connect (m_angle270RadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);

    connect (m_angleCustomRadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotAngleCustomRadioButtonToggled);
    connect (m_angleCustomRadioButton, &QRadioButton::toggled,
             this, &kpTransformRotateDialog::slotUpdate);

    connect (m_angleCustomInput,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpTransformRotateDialog::slotUpdate);
}


// public virtual [base kpTransformPreviewDialog]
bool kpTransformRotateDialog::isNoOp () const
{
    return (angle () == 0);
}

// public
int kpTransformRotateDialog::angle () const
{
    int retAngle;


    if (m_angle90RadioButton->isChecked ()) {
        retAngle = 90;
    }
    else if (m_angle180RadioButton->isChecked ()) {
        retAngle = 180;
    }
    else if (m_angle270RadioButton->isChecked ()) {
        retAngle = 270;
    }
    else { // if (m_angleCustomRadioButton->isChecked ())
        retAngle = m_angleCustomInput->value ();
    }


    if (m_antiClockwiseRadioButton->isChecked ()) {
        retAngle *= -1;
    }


    if (retAngle < 0) {
        retAngle += ((0 - retAngle) / 360 + 1) * 360;
    }

    if (retAngle >= 360) {
        retAngle -= ((retAngle - 360) / 360 + 1) * 360;
    }


    return retAngle;
}


// private virtual [base kpTransformPreviewDialog]
QSize kpTransformRotateDialog::newDimensions () const
{
    QTransform matrix = kpPixmapFX::rotateMatrix (m_oldWidth, m_oldHeight, angle ());
    QRect rect = matrix.mapRect (QRect (0, 0, m_oldWidth, m_oldHeight));
    return rect.size ();
}

// private virtual [base kpTransformPreviewDialog]
QImage kpTransformRotateDialog::transformPixmap (const QImage &image,
                                                 int targetWidth, int targetHeight) const
{
    return kpPixmapFX::rotate (image, angle (),
                               m_environ->backgroundColor (m_actOnSelection),
                               targetWidth, targetHeight);
}


// private slot
void kpTransformRotateDialog::slotAngleCustomRadioButtonToggled (bool isChecked)
{
    m_angleCustomInput->setEnabled (isChecked);

    if (isChecked) {
        m_angleCustomInput->setFocus();
    }
}

// private slot virtual [base kpTransformPreviewDialog]
void kpTransformRotateDialog::slotUpdate ()
{
    s_lastIsClockwise = m_clockwiseRadioButton->isChecked ();
    s_lastAngleCustom = m_angleCustomInput->value ();

    kpTransformPreviewDialog::slotUpdate ();
}


// private slot virtual [base QDialog]
void kpTransformRotateDialog::accept ()
{
    KLocalizedString message;
    QString caption, continueButtonText;

    if (document ()->selection ())
    {
        if (!document ()->textSelection ())
        {
            message =
                ki18n ("<qt><p>Rotating the selection to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to rotate the selection?</p></qt>");

            caption = i18nc ("@title:window", "Rotate Selection?");
            continueButtonText = i18n ("Rotat&e Selection");
        }
    }
    else
    {
        message =
            ki18n ("<qt><p>Rotating the image to %1x%2"
                  " may take a substantial amount of memory."
                  " This can reduce system"
                  " responsiveness and cause other application resource"
                  " problems.</p>"

                  "<p>Are you sure you want to rotate the image?</p></qt>");

        caption = i18nc ("@title:window", "Rotate Image?");
        continueButtonText = i18n ("Rotat&e Image");
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
        QDialog::accept ();
    }
}


