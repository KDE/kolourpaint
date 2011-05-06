
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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QWhatsThis>

#include <klocale.h>
#include <knuminput.h>

#include <kpColorSimilarityFrame.h>


kpColorSimilarityDialog::kpColorSimilarityDialog (QWidget *parent)
    : KDialog (parent)
{
    setCaption (i18nc ("@title:window", "Color Similarity"));
    setButtons (KDialog::Ok | KDialog::Cancel);
    QWidget *baseWidget = new QWidget (this);
    setMainWidget (baseWidget);


    QGroupBox *cubeGroupBox = new QGroupBox (i18n ("Preview"), baseWidget);

    m_colorSimilarityFrame = new kpColorSimilarityFrame(cubeGroupBox);
    m_colorSimilarityFrame->setMinimumSize (240, 180);

    QPushButton *updatePushButton = new QPushButton (i18n ("&Update"), cubeGroupBox);


    QVBoxLayout *cubeLayout = new QVBoxLayout (cubeGroupBox);
    cubeLayout->setSpacing(spacingHint ());
    cubeLayout->setMargin (marginHint () * 2);
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
    inputLayout->setSpacing (spacingHint () * 4);
    inputLayout->setMargin (marginHint () * 2);

    inputLayout->addWidget (m_colorSimilarityInput);
    inputLayout->addWidget (m_whatIsLabel);


    // COMPAT: This is not firing properly when the user is typing in a
    //         new value.
    connect (m_colorSimilarityInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotColorSimilarityValueChanged ()));


    QVBoxLayout *baseLayout = new QVBoxLayout (baseWidget);
    baseLayout->setSpacing (spacingHint () * 2);
    baseLayout->setMargin (0/*margin*/);
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
    QWhatsThis::showText (QCursor::pos (), m_colorSimilarityFrame->whatsThis (),
        this);

    // LOTODO: It looks weird with the focus rectangle.
    //         It's also very hard for the user to make it lose focus for some reason
    //         (you must click on the label - nowhere else will work).
    //
    //         This doesn't work - I don't know why:
    //             m_whatIsLabel->clearFocus ();
    //
    //         Maybe it's a weird kind of focus?
}


#include <kpColorSimilarityDialog.moc>
