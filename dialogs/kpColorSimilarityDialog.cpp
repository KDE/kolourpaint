
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


#include <kpColorSimilarityDialog.h>

#include <qboxlayout.h>
#include <qdialogbuttonbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QWhatsThis>

#include <klocale.h>
#include <knuminput.h>

#include <kpColorSimilarityFrame.h>


kpColorSimilarityDialog::kpColorSimilarityDialog (QWidget *parent)
    : QDialog (parent)
{
    setWindowTitle (i18nc ("@title:window", "Color Similarity"));
    QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel, this);
    connect (buttons, SIGNAL (accepted()), this, SLOT (accept()));
    connect (buttons, SIGNAL (rejected()), this, SLOT (reject()));

    QWidget *baseWidget = new QWidget (this);

    QVBoxLayout *dialogLayout = new QVBoxLayout (this);
    dialogLayout->addWidget (baseWidget);
    dialogLayout->addWidget (buttons);

    QGroupBox *cubeGroupBox = new QGroupBox (i18n ("Preview"), baseWidget);

    m_colorSimilarityFrame = new kpColorSimilarityFrame(cubeGroupBox);
    m_colorSimilarityFrame->setMinimumSize (240, 180);

    QPushButton *updatePushButton = new QPushButton (i18n ("&Update"), cubeGroupBox);


    QVBoxLayout *cubeLayout = new QVBoxLayout (cubeGroupBox);
    cubeLayout->addWidget (m_colorSimilarityFrame, 1/*stretch*/);
    cubeLayout->addWidget (updatePushButton, 0/*stretch*/, Qt::AlignHCenter);


    connect (updatePushButton, SIGNAL (clicked ()),
             this, SLOT (slotColorSimilarityValueChanged ()));


    QGroupBox *inputGroupBox = new QGroupBox (i18n ("&RGB Color Cube Distance"),
        baseWidget);

    m_colorSimilarityInput = new KIntNumInput (inputGroupBox);
    m_colorSimilarityInput->setRange (0, int (kpColorSimilarityHolder::MaxColorSimilarity * 100 + .1/*don't floor below target int*/),
                                      5/*step*/);
    m_colorSimilarityInput->setSliderEnabled (true);
    m_colorSimilarityInput->setSuffix (i18n ("%"));
    m_colorSimilarityInput->setSpecialValueText (i18n ("Exact Match"));

    // TODO: We have a good handbook section on this, which we should
    //       somehow link to.
    m_whatIsLabel = new QLabel (
        i18n ("<a href=\"dummy_to_make_link_clickable\">"
              "What is Color Similarity?</a>"),
        inputGroupBox);
    m_whatIsLabel->setAlignment (Qt::AlignHCenter);
    connect (m_whatIsLabel, SIGNAL (linkActivated (const QString &)),
        SLOT (slotWhatIsLabelClicked ()));


    QVBoxLayout *inputLayout = new QVBoxLayout (inputGroupBox);

    inputLayout->addWidget (m_colorSimilarityInput);
    inputLayout->addWidget (m_whatIsLabel);


    // COMPAT: This is not firing properly when the user is typing in a
    //         new value.
    connect (m_colorSimilarityInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotColorSimilarityValueChanged ()));


    QVBoxLayout *baseLayout = new QVBoxLayout (baseWidget);
    baseLayout->setMargin (0);
    baseLayout->addWidget (cubeGroupBox, 1/*stretch*/);
    baseLayout->addWidget (inputGroupBox);
}

kpColorSimilarityDialog::~kpColorSimilarityDialog ()
{
}


// public
double kpColorSimilarityDialog::colorSimilarity () const
{
    return m_colorSimilarityFrame->colorSimilarity ();
}

// public
void kpColorSimilarityDialog::setColorSimilarity (double similarity)
{
    m_colorSimilarityInput->setValue (qRound (similarity * 100));
}


// private slot
void kpColorSimilarityDialog::slotColorSimilarityValueChanged ()
{
    m_colorSimilarityFrame->setColorSimilarity (double (m_colorSimilarityInput->value ()) / 100);
}


// private slot
void kpColorSimilarityDialog::slotWhatIsLabelClicked ()
{
  QWhatsThis::showText(QCursor::pos(), m_colorSimilarityFrame->whatsThis(), this);
}
