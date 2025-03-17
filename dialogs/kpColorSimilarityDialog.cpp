
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpColorSimilarityDialog.h"

#include "widgets/colorSimilarity/kpColorSimilarityFrame.h"

#include "../widgets/imagelib/effects/kpNumInput.h"
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWhatsThis>

kpColorSimilarityDialog::kpColorSimilarityDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Color Similarity"));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &kpColorSimilarityDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &kpColorSimilarityDialog::reject);

    auto *baseWidget = new QWidget(this);

    auto *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(baseWidget);
    dialogLayout->addWidget(buttons);

    auto *cubeGroupBox = new QGroupBox(i18n("Preview"), baseWidget);

    m_colorSimilarityFrame = new kpColorSimilarityFrame(cubeGroupBox);
    m_colorSimilarityFrame->setMinimumSize(240, 180);

    auto *updatePushButton = new QPushButton(i18n("&Update"), cubeGroupBox);

    auto *cubeLayout = new QVBoxLayout(cubeGroupBox);
    cubeLayout->addWidget(m_colorSimilarityFrame, 1 /*stretch*/);
    cubeLayout->addWidget(updatePushButton, 0 /*stretch*/, Qt::AlignHCenter);

    connect(updatePushButton, &QPushButton::clicked, this, &kpColorSimilarityDialog::slotColorSimilarityValueChanged);

    auto *inputGroupBox = new QGroupBox(i18n("&RGB Color Cube Distance"), baseWidget);

    m_colorSimilarityInput = new kpIntNumInput(inputGroupBox);
    m_colorSimilarityInput->setRange(0, int(kpColorSimilarityHolder::MaxColorSimilarity * 100 + 0.1 /*don't floor below target int*/), 5 /*step*/);
    m_colorSimilarityInput->setSpecialValueText(i18nc("@label:spinbox", "Exact match"));

    // TODO: We have a good handbook section on this, which we should
    //       somehow link to.
    m_whatIsLabel = new QLabel(i18n("<a href=\"dummy_to_make_link_clickable\">"
                                    "What is Color Similarity?</a>"),
                               inputGroupBox);
    m_whatIsLabel->setAlignment(Qt::AlignHCenter);
    connect(m_whatIsLabel, &QLabel::linkActivated, this, &kpColorSimilarityDialog::slotWhatIsLabelClicked);

    auto *inputLayout = new QVBoxLayout(inputGroupBox);

    inputLayout->addWidget(m_colorSimilarityInput);
    inputLayout->addWidget(m_whatIsLabel);

    // COMPAT: This is not firing properly when the user is typing in a
    //         new value.
    connect(m_colorSimilarityInput, &kpIntNumInput::valueChanged, this, &kpColorSimilarityDialog::slotColorSimilarityValueChanged);

    auto *baseLayout = new QVBoxLayout(baseWidget);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    baseLayout->addWidget(cubeGroupBox, 1 /*stretch*/);
    baseLayout->addWidget(inputGroupBox);
}

kpColorSimilarityDialog::~kpColorSimilarityDialog() = default;

// public
double kpColorSimilarityDialog::colorSimilarity() const
{
    return m_colorSimilarityFrame->colorSimilarity();
}

// public
void kpColorSimilarityDialog::setColorSimilarity(double similarity)
{
    m_colorSimilarityInput->setValue(qRound(similarity * 100));
}

// private slot
void kpColorSimilarityDialog::slotColorSimilarityValueChanged()
{
    m_colorSimilarityFrame->setColorSimilarity(double(m_colorSimilarityInput->value()) / 100);
}

// private slot
void kpColorSimilarityDialog::slotWhatIsLabelClicked()
{
    QWhatsThis::showText(QCursor::pos(), m_colorSimilarityFrame->whatsThis(), this);
}

#include "moc_kpColorSimilarityDialog.cpp"
