/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011 Martin Koller <kollix@aon.at>
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


#define DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG 0


#include "kpTransformResizeScaleDialog.h"


#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QSize>
#include <QToolButton>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <KSharedConfig>
#include <KConfigGroup>
#include "kpLogCategories.h"
#include <KLocalizedString>

#include "layers/selections/kpAbstractSelection.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "layers/selections/text/kpTextSelection.h"
#include "tools/kpTool.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"

//---------------------------------------------------------------------

#define kpSettingResizeScaleLastKeepAspect "Resize Scale - Last Keep Aspect"
#define kpSettingResizeScaleScaleType "Resize Scale - ScaleType"

//---------------------------------------------------------------------

#define SET_VALUE_WITHOUT_SIGNAL_EMISSION(knuminput_instance,value)    \
{                                                                      \
    knuminput_instance->blockSignals (true);                           \
    knuminput_instance->setValue (value);                              \
    knuminput_instance->blockSignals (false);                          \
}

#define IGNORE_KEEP_ASPECT_RATIO(cmd) \
{                                     \
    m_ignoreKeepAspectRatio++;        \
    cmd;                              \
    m_ignoreKeepAspectRatio--;        \
}

//---------------------------------------------------------------------

kpTransformResizeScaleDialog::kpTransformResizeScaleDialog (
        kpTransformDialogEnvironment *_env, QWidget *parent)
    : QDialog (parent),
      m_environ (_env),
      m_ignoreKeepAspectRatio (0),
      m_lastType(kpTransformResizeScaleCommand::Resize)
{
    setWindowTitle (i18nc ("@title:window", "Resize / Scale"));
    QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel, this);
    connect (buttons, &QDialogButtonBox::accepted, this, &kpTransformResizeScaleDialog::accept);
    connect (buttons, &QDialogButtonBox::rejected, this, &kpTransformResizeScaleDialog::reject);

    QWidget *baseWidget = new QWidget (this);

    auto *dialogLayout = new QVBoxLayout (this);
    dialogLayout->addWidget (baseWidget);
    dialogLayout->addWidget (buttons);

    QWidget *actOnBox = createActOnBox(baseWidget);
    QGroupBox *operationGroupBox = createOperationGroupBox(baseWidget);
    QGroupBox *dimensionsGroupBox = createDimensionsGroupBox(baseWidget);

    auto *baseLayout = new QVBoxLayout (baseWidget);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    baseLayout->addWidget(actOnBox);
    baseLayout->addWidget(operationGroupBox);
    baseLayout->addWidget(dimensionsGroupBox);

    KConfigGroup cfg(KSharedConfig::openConfig(), kpSettingsGroupGeneral);
    setKeepAspectRatio(cfg.readEntry(kpSettingResizeScaleLastKeepAspect, false));
    m_lastType = static_cast<kpTransformResizeScaleCommand::Type>
                   (cfg.readEntry(kpSettingResizeScaleScaleType,
                                  static_cast<int>(kpTransformResizeScaleCommand::Resize)));

    slotActOnChanged ();

    m_newWidthInput->setFocus ();

    //enableButtonOk (!isNoOp ());
}

//---------------------------------------------------------------------
// private

kpDocument *kpTransformResizeScaleDialog::document () const
{
    return m_environ->document ();
}

//---------------------------------------------------------------------
// private

kpAbstractSelection *kpTransformResizeScaleDialog::selection () const
{
    Q_ASSERT (document ());
    return document ()->selection ();
}

//---------------------------------------------------------------------
// private

kpTextSelection *kpTransformResizeScaleDialog::textSelection () const
{
    Q_ASSERT (document ());
    return document ()->textSelection ();
}

//---------------------------------------------------------------------
// private

QWidget *kpTransformResizeScaleDialog::createActOnBox(QWidget *baseWidget)
{
    QWidget *actOnBox = new QWidget (baseWidget);


    auto *actOnLabel = new QLabel (i18n ("Ac&t on:"), actOnBox);
    m_actOnCombo = new QComboBox (actOnBox);


    actOnLabel->setBuddy (m_actOnCombo);

    m_actOnCombo->insertItem (Image, i18n ("Entire Image"));
    if (selection ())
    {
        QString selName = i18n ("Selection");

        if (textSelection ()) {
            selName = i18n ("Text Box");
        }

        m_actOnCombo->insertItem (Selection, selName);
        m_actOnCombo->setCurrentIndex (Selection);
    }
    else
    {
        actOnLabel->setEnabled (false);
        m_actOnCombo->setEnabled (false);
    }

    auto *lay = new QHBoxLayout (actOnBox);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget (actOnLabel);
    lay->addWidget (m_actOnCombo, 1);

    connect (m_actOnCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
             this, &kpTransformResizeScaleDialog::slotActOnChanged);

    return actOnBox;
}

//---------------------------------------------------------------------

static void toolButtonSetLook (QToolButton *button,
                               const QString &iconName,
                               const QString &name)
{
    QPixmap icon;
    const QString qrcPath = QStringLiteral(":/icons/") + iconName;
    if (!icon.load (qrcPath)) {
        qWarning() << qrcPath << "not found";
    } else {
        button->setIconSize (QSize (icon.width (), icon.height ()));
        button->setIcon (icon);
    }

    button->setToolButtonStyle (Qt::ToolButtonTextUnderIcon);
    button->setText (name);
    button->setFocusPolicy (Qt::StrongFocus);
    button->setCheckable (true);
}

//---------------------------------------------------------------------
// private

QGroupBox *kpTransformResizeScaleDialog::createOperationGroupBox (QWidget *baseWidget)
{
    QGroupBox *operationGroupBox = new QGroupBox (i18n ("Operation"), baseWidget);
    operationGroupBox->setWhatsThis(
        i18n ("<qt>"
              "<ul>"
                  "<li><b>Resize</b>: The size of the picture will be"
                  " increased"
                  " by creating new areas to the right and/or bottom"
                  " (filled in with the background color) or"
                  " decreased by cutting"
                  " it at the right and/or bottom.</li>"

                  "<li><b>Scale</b>: The picture will be expanded"
                  " by duplicating pixels or squashed by dropping pixels.</li>"

                  "<li><b>Smooth Scale</b>: This is the same as"
                  " <i>Scale</i> except that it blends neighboring"
                  " pixels to produce a smoother looking picture.</li>"
              "</ul>"
              "</qt>"));

    m_resizeButton = new QToolButton (operationGroupBox);
    toolButtonSetLook (m_resizeButton,
                       QStringLiteral ("resize"),
                       i18n ("&Resize"));

    m_scaleButton = new QToolButton (operationGroupBox);
    toolButtonSetLook (m_scaleButton,
                       QStringLiteral ("scale"),
                       i18n ("&Scale"));

    m_smoothScaleButton = new QToolButton (operationGroupBox);
    toolButtonSetLook (m_smoothScaleButton,
                       QStringLiteral ("smooth_scale"),
                       i18n ("S&mooth Scale"));

    auto *resizeScaleButtonGroup = new QButtonGroup (baseWidget);
    resizeScaleButtonGroup->addButton (m_resizeButton);
    resizeScaleButtonGroup->addButton (m_scaleButton);
    resizeScaleButtonGroup->addButton (m_smoothScaleButton);


    auto *operationLayout = new QGridLayout (operationGroupBox );
    operationLayout->addWidget (m_resizeButton, 0, 0, Qt::AlignCenter);
    operationLayout->addWidget (m_scaleButton, 0, 1, Qt::AlignCenter);
    operationLayout->addWidget (m_smoothScaleButton, 0, 2, Qt::AlignCenter);

    connect (m_resizeButton, &QToolButton::toggled,
             this, &kpTransformResizeScaleDialog::slotTypeChanged);
    connect (m_scaleButton, &QToolButton::toggled,
             this, &kpTransformResizeScaleDialog::slotTypeChanged);
    connect (m_smoothScaleButton, &QToolButton::toggled,
             this, &kpTransformResizeScaleDialog::slotTypeChanged);

    return operationGroupBox;
}

//---------------------------------------------------------------------
// private

QGroupBox *kpTransformResizeScaleDialog::createDimensionsGroupBox(QWidget *baseWidget)
{
    QGroupBox *dimensionsGroupBox = new QGroupBox (i18n ("Dimensions"), baseWidget);

    auto *widthLabel = new QLabel (i18n ("Width:"), dimensionsGroupBox);
    widthLabel->setAlignment (widthLabel->alignment () | Qt::AlignHCenter);
    auto *heightLabel = new QLabel (i18n ("Height:"), dimensionsGroupBox);
    heightLabel->setAlignment (heightLabel->alignment () | Qt::AlignHCenter);

    auto *originalLabel = new QLabel (i18n ("Original:"), dimensionsGroupBox);
    m_originalWidthInput = new QSpinBox;
    m_originalWidthInput->setRange(1, INT_MAX);
    m_originalWidthInput->setValue(document()->width(static_cast<bool> (selection())));
    auto *xLabel0 = new QLabel (i18n ("x"), dimensionsGroupBox);
    m_originalHeightInput = new QSpinBox;
    m_originalHeightInput->setRange(1, INT_MAX);
    m_originalHeightInput->setValue(document()->height(static_cast<bool> (selection())));

    auto *newLabel = new QLabel (i18n ("&New:"), dimensionsGroupBox);
    m_newWidthInput = new QSpinBox;
    m_newWidthInput->setRange(1, INT_MAX);
    auto *xLabel1 = new QLabel (i18n ("x"), dimensionsGroupBox);
    m_newHeightInput = new QSpinBox;
    m_newHeightInput->setRange(1, INT_MAX);

    auto *percentLabel = new QLabel (i18n ("&Percent:"), dimensionsGroupBox);
    m_percentWidthInput = new QDoubleSpinBox;
    m_percentWidthInput->setRange(0.01, 1000000);
    m_percentWidthInput->setValue(100);
    m_percentWidthInput->setSingleStep(1);
    m_percentWidthInput->setDecimals(2);
    m_percentWidthInput->setSuffix(i18n("%"));

    auto *xLabel2 = new QLabel (i18n ("x"), dimensionsGroupBox);

    m_percentHeightInput = new QDoubleSpinBox;
    m_percentHeightInput->setRange(0.01, 1000000);
    m_percentHeightInput->setValue(100);
    m_percentHeightInput->setSingleStep(1);
    m_percentHeightInput->setDecimals(2);
    m_percentHeightInput->setSuffix(i18n("%"));

    m_keepAspectRatioCheckBox = new QCheckBox (i18n ("Keep &aspect ratio"),
                                               dimensionsGroupBox);


    m_originalWidthInput->setEnabled (false);
    m_originalHeightInput->setEnabled (false);
    originalLabel->setBuddy (m_originalWidthInput);
    newLabel->setBuddy (m_newWidthInput);
    m_percentWidthInput->setValue (100);
    m_percentHeightInput->setValue (100);
    percentLabel->setBuddy (m_percentWidthInput);


    auto *dimensionsLayout = new QGridLayout (dimensionsGroupBox);
    dimensionsLayout->setColumnStretch (1/*column*/, 1);
    dimensionsLayout->setColumnStretch (3/*column*/, 1);


    dimensionsLayout->addWidget (widthLabel, 0, 1);
    dimensionsLayout->addWidget (heightLabel, 0, 3);

    dimensionsLayout->addWidget (originalLabel, 1, 0);
    dimensionsLayout->addWidget (m_originalWidthInput, 1, 1);
    dimensionsLayout->addWidget (xLabel0, 1, 2);
    dimensionsLayout->addWidget (m_originalHeightInput, 1, 3);

    dimensionsLayout->addWidget (newLabel, 2, 0);
    dimensionsLayout->addWidget (m_newWidthInput, 2, 1);
    dimensionsLayout->addWidget (xLabel1, 2, 2);
    dimensionsLayout->addWidget (m_newHeightInput, 2, 3);

    dimensionsLayout->addWidget (percentLabel, 3, 0);
    dimensionsLayout->addWidget (m_percentWidthInput, 3, 1);
    dimensionsLayout->addWidget (xLabel2, 3, 2);
    dimensionsLayout->addWidget (m_percentHeightInput, 3, 3);

    dimensionsLayout->addWidget (m_keepAspectRatioCheckBox, 4, 0, 1, 4);
    dimensionsLayout->setRowStretch (4/*row*/, 1);
    dimensionsLayout->setRowMinimumHeight (4/*row*/, dimensionsLayout->rowMinimumHeight (4) * 2);



    connect (m_newWidthInput, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpTransformResizeScaleDialog::slotWidthChanged);

    connect (m_newHeightInput, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpTransformResizeScaleDialog::slotHeightChanged);

    // COMPAT: KDoubleNumInput only fires valueChanged(double) once per
    //         edit.  It should either fire:
    //
    //             1. At the end of the edit (triggered by clicking or tabbing
    //                away), like with KDE 3.
    //
    //             OR
    //
    //             2. Once per keystroke.
    //
    //         Bug in KDoubleNumInput.
    connect (m_percentWidthInput,
             static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             this, &kpTransformResizeScaleDialog::slotPercentWidthChanged);

    connect (m_percentHeightInput,
             static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             this, &kpTransformResizeScaleDialog::slotPercentHeightChanged);

    connect (m_keepAspectRatioCheckBox, &QCheckBox::toggled,
             this, &kpTransformResizeScaleDialog::setKeepAspectRatio);

    return dimensionsGroupBox;
}

//---------------------------------------------------------------------
// private

void kpTransformResizeScaleDialog::widthFitHeightToAspectRatio ()
{
    if (m_keepAspectRatioCheckBox->isChecked () && !m_ignoreKeepAspectRatio)
    {
        // width / height = oldWidth / oldHeight
        // height = width * oldHeight / oldWidth
        const int newHeight = qRound (double (imageWidth ()) * double (originalHeight ())
                                      / double (originalWidth ()));
        IGNORE_KEEP_ASPECT_RATIO (m_newHeightInput->setValue (newHeight));
    }
}

//---------------------------------------------------------------------
// private

void kpTransformResizeScaleDialog::heightFitWidthToAspectRatio ()
{
    if (m_keepAspectRatioCheckBox->isChecked () && !m_ignoreKeepAspectRatio)
    {
        // width / height = oldWidth / oldHeight
        // width = height * oldWidth / oldHeight
        const int newWidth = qRound (double (imageHeight ()) * double (originalWidth ())
                                     / double (originalHeight ()));
        IGNORE_KEEP_ASPECT_RATIO (m_newWidthInput->setValue (newWidth));
    }
}

//---------------------------------------------------------------------
// private

bool kpTransformResizeScaleDialog::resizeEnabled () const
{
    return (!actOnSelection () ||
            (actOnSelection () && textSelection ()));
}

//---------------------------------------------------------------------
// private

bool kpTransformResizeScaleDialog::scaleEnabled () const
{
    return (!(actOnSelection () && textSelection ()));
}

//---------------------------------------------------------------------
// private

bool kpTransformResizeScaleDialog::smoothScaleEnabled () const
{
    return scaleEnabled ();
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::slotActOnChanged ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::slotActOnChanged()";
#endif

    m_resizeButton->setEnabled (resizeEnabled ());
    m_scaleButton->setEnabled (scaleEnabled ());
    m_smoothScaleButton->setEnabled (smoothScaleEnabled ());

    // TODO: somehow share logic with (resize|*scale)Enabled()
    if (actOnSelection ())
    {
        if (textSelection ())
        {
            m_resizeButton->setChecked (true);
        }
        else
        {
            if (m_lastType == kpTransformResizeScaleCommand::Scale) {
                m_scaleButton->setChecked (true);
            }
            else {
                m_smoothScaleButton->setChecked (true);
            }
        }
    }
    else
    {
        if (m_lastType == kpTransformResizeScaleCommand::Resize) {
            m_resizeButton->setChecked (true);
        }
        else if (m_lastType == kpTransformResizeScaleCommand::Scale) {
            m_scaleButton->setChecked (true);
        }
        else {
            m_smoothScaleButton->setChecked (true);
        }
    }


    m_originalWidthInput->setValue (originalWidth ());
    m_originalHeightInput->setValue (originalHeight ());


    m_newWidthInput->blockSignals (true);
    m_newHeightInput->blockSignals (true);

    m_newWidthInput->setMinimum (actOnSelection () ?
                                      selection ()->minimumWidth () :
                                      1);
    m_newHeightInput->setMinimum (actOnSelection () ?
                                       selection ()->minimumHeight () :
                                       1);

    m_newWidthInput->blockSignals (false);
    m_newHeightInput->blockSignals (false);


    IGNORE_KEEP_ASPECT_RATIO (slotPercentWidthChanged (m_percentWidthInput->value ()));
    IGNORE_KEEP_ASPECT_RATIO (slotPercentHeightChanged (m_percentHeightInput->value ()));

    setKeepAspectRatio (m_keepAspectRatioCheckBox->isChecked ());
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::slotTypeChanged ()
{
    m_lastType = type ();
}

//---------------------------------------------------------------------

// public slot
void kpTransformResizeScaleDialog::slotWidthChanged (int width)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::slotWidthChanged("
               << width << ")" << endl;
#endif
    const double newPercentWidth = double (width) * 100 / double (originalWidth ());

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_percentWidthInput,newPercentWidth);

    widthFitHeightToAspectRatio ();

    //enableButtonOk (!isNoOp ());
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::slotHeightChanged (int height)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::slotHeightChanged("
               << height << ")" << endl;
#endif
    const double newPercentHeight = double (height) * 100 / double (originalHeight ());

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_percentHeightInput,newPercentHeight);

    heightFitWidthToAspectRatio ();

    //enableButtonOk (!isNoOp ());
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::slotPercentWidthChanged (double percentWidth)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::slotPercentWidthChanged("
               << percentWidth << ")";
#endif

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_newWidthInput,
                                       qRound (percentWidth * originalWidth () / 100.0));

    widthFitHeightToAspectRatio ();

    //enableButtonOk (!isNoOp ());
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::slotPercentHeightChanged (double percentHeight)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::slotPercentHeightChanged("
               << percentHeight << ")";
#endif

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_newHeightInput,
                                       qRound (percentHeight * originalHeight () / 100.0));

    heightFitWidthToAspectRatio ();

    //enableButtonOk (!isNoOp ());
}

//---------------------------------------------------------------------
// public slot

void kpTransformResizeScaleDialog::setKeepAspectRatio (bool on)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    qCDebug(kpLogDialogs) << "kpTransformResizeScaleDialog::setKeepAspectRatio("
               << on << ")";
#endif

    if (on != m_keepAspectRatioCheckBox->isChecked ()) {
        m_keepAspectRatioCheckBox->setChecked (on);
    }

    if (on) {
        widthFitHeightToAspectRatio ();
    }
}

//---------------------------------------------------------------------

#undef IGNORE_KEEP_ASPECT_RATIO
#undef SET_VALUE_WITHOUT_SIGNAL_EMISSION


//---------------------------------------------------------------------
// private

int kpTransformResizeScaleDialog::originalWidth () const
{
    return document ()->width (actOnSelection ());
}

//---------------------------------------------------------------------
// private

int kpTransformResizeScaleDialog::originalHeight () const
{
    return document ()->height (actOnSelection ());
}

//---------------------------------------------------------------------
// public

int kpTransformResizeScaleDialog::imageWidth () const
{
    return m_newWidthInput->value ();
}

//---------------------------------------------------------------------
// public

int kpTransformResizeScaleDialog::imageHeight () const
{
    return m_newHeightInput->value ();
}

//---------------------------------------------------------------------
// public

bool kpTransformResizeScaleDialog::actOnSelection () const
{
    return (m_actOnCombo->currentIndex () == Selection);
}

//---------------------------------------------------------------------
// public

kpTransformResizeScaleCommand::Type kpTransformResizeScaleDialog::type () const
{
    if (m_resizeButton->isChecked ()) {
        return kpTransformResizeScaleCommand::Resize;
    }

    if (m_scaleButton->isChecked ()) {
        return kpTransformResizeScaleCommand::Scale;
    }

    return kpTransformResizeScaleCommand::SmoothScale;
}

//---------------------------------------------------------------------
// public

bool kpTransformResizeScaleDialog::isNoOp () const
{
    return (imageWidth () == originalWidth () &&
            imageHeight () == originalHeight ());
}

//---------------------------------------------------------------------
// private slot virtual [base QDialog]

void kpTransformResizeScaleDialog::accept ()
{
    enum { eText, eSelection, eImage } actionTarget = eText;

    if (actOnSelection ())
    {
        if (textSelection ())
        {
            actionTarget = eText;
        }
        else
        {
            actionTarget = eSelection;
        }
    }
    else
    {
        actionTarget = eImage;
    }


    KLocalizedString message;
    QString caption, continueButtonText;

    // Note: If eText, can't Scale nor SmoothScale.
    //       If eSelection, can't Resize.

    switch (type ())
    {
    default:
    case kpTransformResizeScaleCommand::Resize:
        if (actionTarget == eText)
        {
            message =
                ki18n ("<qt><p>Resizing the text box to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to resize the text box?</p></qt>");

            caption = i18nc ("@title:window", "Resize Text Box?");
            continueButtonText = i18n ("R&esize Text Box");
        }
        else if (actionTarget == eImage)
        {
            message =
                ki18n ("<qt><p>Resizing the image to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to resize the image?</p></qt>");

            caption = i18nc ("@title:window", "Resize Image?");
            continueButtonText = i18n ("R&esize Image");
        }

        break;

    case kpTransformResizeScaleCommand::Scale:
        if (actionTarget == eImage)
        {
            message =
                ki18n ("<qt><p>Scaling the image to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to scale the image?</p></qt>");

            caption = i18nc ("@title:window", "Scale Image?");
            continueButtonText = i18n ("Scal&e Image");
        }
        else if (actionTarget == eSelection)
        {
            message =
                ki18n ("<qt><p>Scaling the selection to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to scale the selection?</p></qt>");

            caption = i18nc ("@title:window", "Scale Selection?");
            continueButtonText = i18n ("Scal&e Selection");
        }

        break;

    case kpTransformResizeScaleCommand::SmoothScale:
        if (actionTarget == eImage)
        {
            message =
                ki18n ("<qt><p>Smooth Scaling the image to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to smooth scale the image?</p></qt>");

            caption = i18nc ("@title:window", "Smooth Scale Image?");
            continueButtonText = i18n ("Smooth Scal&e Image");
        }
        else if (actionTarget == eSelection)
        {
            message =
                ki18n ("<qt><p>Smooth Scaling the selection to %1x%2"
                      " may take a substantial amount of memory."
                      " This can reduce system"
                      " responsiveness and cause other application resource"
                      " problems.</p>"

                      "<p>Are you sure you want to smooth scale the selection?</p></qt>");

            caption = i18nc ("@title:window", "Smooth Scale Selection?");
            continueButtonText = i18n ("Smooth Scal&e Selection");
        }

        break;
    }


    if (kpTool::warnIfBigImageSize (originalWidth (),
            originalHeight (),
            imageWidth (), imageHeight (),
            message.subs (imageWidth ()).subs (imageHeight ()).toString (),
            caption,
            continueButtonText,
            this))
    {
        QDialog::accept ();
    }

    // store settings
    KConfigGroup cfg(KSharedConfig::openConfig(), kpSettingsGroupGeneral);

    cfg.writeEntry(kpSettingResizeScaleLastKeepAspect, m_keepAspectRatioCheckBox->isChecked());
    cfg.writeEntry(kpSettingResizeScaleScaleType, static_cast<int>(m_lastType));
    cfg.sync();
}

//---------------------------------------------------------------------

