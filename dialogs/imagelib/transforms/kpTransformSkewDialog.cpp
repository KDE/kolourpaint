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


#include "dialogs/imagelib/transforms/kpTransformSkewDialog.h"

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTransform>
#include <QImage>
#include <QSpinBox>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"
#include "tools/kpTool.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"


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


    if (s_lastWidth > 0 && s_lastHeight > 0) {
        resize (s_lastWidth, s_lastHeight);
    }


    slotUpdate ();
    

    m_horizontalSkewInput->setFocus ();
}

kpTransformSkewDialog::~kpTransformSkewDialog ()
{
    s_lastWidth = width ();
    s_lastHeight = height ();
}


// private
void kpTransformSkewDialog::createAngleGroupBox ()
{
    auto *angleGroupBox = new QGroupBox (i18n ("Angle"), mainWidget ());
    addCustomWidget (angleGroupBox);


    auto *horizontalSkewPixmapLabel = new QLabel (angleGroupBox);
    horizontalSkewPixmapLabel->setPixmap (QStringLiteral(":/icons/image_skew_horizontal"));

    auto *horizontalSkewLabel = new QLabel (i18n ("&Horizontal:"), angleGroupBox);
    m_horizontalSkewInput = new QSpinBox;
    m_horizontalSkewInput->setValue(s_lastHorizontalAngle);
    m_horizontalSkewInput->setMinimum(-89);
    m_horizontalSkewInput->setMaximum(+89);

    auto *horizontalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    auto *verticalSkewPixmapLabel = new QLabel (angleGroupBox);
    verticalSkewPixmapLabel->setPixmap (QStringLiteral(":/icons/image_skew_vertical"));

    auto *verticalSkewLabel = new QLabel (i18n ("&Vertical:"), angleGroupBox);
    m_verticalSkewInput = new QSpinBox;
    m_verticalSkewInput->setValue(s_lastVerticalAngle);
    m_verticalSkewInput->setMinimum(-89);
    m_verticalSkewInput->setMaximum(+89);

    auto *verticalSkewDegreesLabel = new QLabel (i18n ("degrees"), angleGroupBox);


    horizontalSkewLabel->setBuddy (m_horizontalSkewInput);
    verticalSkewLabel->setBuddy (m_verticalSkewInput);


    auto *angleLayout = new QGridLayout (angleGroupBox);

    angleLayout->addWidget (horizontalSkewPixmapLabel, 0, 0);
    angleLayout->addWidget (horizontalSkewLabel, 0, 1);
    angleLayout->addWidget (m_horizontalSkewInput, 0, 2, Qt::AlignVCenter);
    angleLayout->addWidget (horizontalSkewDegreesLabel, 0, 3);

    angleLayout->addWidget (verticalSkewPixmapLabel, 1, 0);
    angleLayout->addWidget (verticalSkewLabel, 1, 1);
    angleLayout->addWidget (m_verticalSkewInput, 1, 2, Qt::AlignVCenter);
    angleLayout->addWidget (verticalSkewDegreesLabel, 1, 3);


    connect (m_horizontalSkewInput,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpTransformSkewDialog::slotUpdate);

    connect (m_verticalSkewInput,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpTransformSkewDialog::slotUpdate);
}


// private virtual [base kpTransformPreviewDialog]
QSize kpTransformSkewDialog::newDimensions () const
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    auto skewMatrix = kpPixmapFX::skewMatrix (doc->image (),
                                                  horizontalAngleForPixmapFX (),
                                                  verticalAngleForPixmapFX ());
    auto skewRect = skewMatrix.mapRect (doc->rect (m_actOnSelection));

    return  {skewRect.width (), skewRect.height ()};
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

                      "<p>Are you sure you want to skew the selection?</p></qt>");

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

                  "<p>Are you sure you want to skew the image?</p></qt>");

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
        QDialog::accept ();
    }
}


