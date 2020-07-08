
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


#define DEBUG_KP_EFFECTS_DIALOG 0


#include "kpEffectsDialog.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "widgets/imagelib/effects/kpEffectBalanceWidget.h"
#include "widgets/imagelib/effects/kpEffectBlurSharpenWidget.h"
#include "widgets/imagelib/effects/kpEffectEmbossWidget.h"
#include "widgets/imagelib/effects/kpEffectFlattenWidget.h"
#include "widgets/imagelib/effects/kpEffectHSVWidget.h"
#include "widgets/imagelib/effects/kpEffectInvertWidget.h"
#include "widgets/imagelib/effects/kpEffectReduceColorsWidget.h"
#include "widgets/imagelib/effects/kpEffectToneEnhanceWidget.h"
#include "pixmapfx/kpPixmapFX.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"

#include <KConfig>
#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QImage>


// protected static
int kpEffectsDialog::s_lastWidth = 640;
int kpEffectsDialog::s_lastHeight = 620;


kpEffectsDialog::kpEffectsDialog (bool actOnSelection,
                                  kpTransformDialogEnvironment *_env,
                                  QWidget *parent,
                                  int defaultSelectedEffect)
    : kpTransformPreviewDialog (kpTransformPreviewDialog::Preview,
                           true/*reserve top row*/,
                           QString()/*caption*/,
                           QString()/*afterActionText (no Dimensions Group Box)*/,
                           actOnSelection,
                           _env,
                           parent),
      m_delayedUpdateTimer (new QTimer (this)),
      m_effectsComboBox (nullptr),
      m_settingsGroupBox (nullptr),
      m_settingsLayout (nullptr),
      m_effectWidget (nullptr)
{
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "kpEffectsDialog::kpEffectsDialog()";
#endif
    const bool e = updatesEnabled ();
    setUpdatesEnabled (false);


    if (actOnSelection) {
        setWindowTitle (i18nc ("@title:window", "More Image Effects (Selection)"));
    }
    else {
        setWindowTitle (i18nc ("@title:window", "More Image Effects"));
    }


    m_delayedUpdateTimer->setSingleShot (true);
    connect (m_delayedUpdateTimer, &QTimer::timeout,
             this, &kpEffectsDialog::slotUpdateWithWaitCursor);


    QWidget *effectContainer = new QWidget (mainWidget ());

    auto *containerLayout = new QHBoxLayout (effectContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel (i18n ("&Effect:"), effectContainer);

    m_effectsComboBox = new QComboBox (effectContainer);
    // Keep in alphabetical order.
    // TODO: What about translations?
    // sync: order in selectEffect().
    m_effectsComboBox->addItem (i18n ("Balance"));
    m_effectsComboBox->addItem (i18n ("Emboss"));
    m_effectsComboBox->addItem (i18n ("Flatten"));
    m_effectsComboBox->addItem (i18n ("Histogram Equalizer"));
    m_effectsComboBox->addItem (i18n ("Hue, Saturation, Value"));
    m_effectsComboBox->addItem (i18n ("Invert"));
    m_effectsComboBox->addItem (i18n ("Reduce Colors"));
    m_effectsComboBox->addItem (i18n ("Soften & Sharpen"));

    containerLayout->addWidget (label);
    containerLayout->addWidget (m_effectsComboBox, 1);

    label->setBuddy (m_effectsComboBox);

    addCustomWidgetToFront (effectContainer);


    m_settingsGroupBox = new QGroupBox (mainWidget ());
    m_settingsLayout = new QVBoxLayout ( m_settingsGroupBox );
    addCustomWidgetToBack (m_settingsGroupBox);


    connect (m_effectsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
             this, &kpEffectsDialog::selectEffect);


    selectEffect (defaultSelectedEffect);


    resize (s_lastWidth, s_lastHeight);


#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "about to setUpdatesEnabled()";
#endif
    // OPT: The preview pixmap gets recalculated here and then possibly
    //      again when QResizeEvent fires, when the dialog is shown.
    setUpdatesEnabled (e);
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << endl
              << endl
              << endl;
#endif
}

kpEffectsDialog::~kpEffectsDialog ()
{
    s_lastWidth = width ();
    s_lastHeight = height ();
}


// public virtual [base kpTransformPreviewDialog]
bool kpEffectsDialog::isNoOp () const
{
    if (!m_effectWidget) {
        return true;
    }

    return m_effectWidget->isNoOp ();
}

// public
kpEffectCommandBase *kpEffectsDialog::createCommand () const
{
    if (!m_effectWidget) {
        return nullptr;
    }

    return m_effectWidget->createCommand (m_environ->commandEnvironment ());
}


// protected virtual [base kpTransformPreviewDialog]
QSize kpEffectsDialog::newDimensions () const
{
    kpDocument *doc = document ();
    if (!doc) {
        return  {};
    }

    return  {doc->width (m_actOnSelection), doc->height (m_actOnSelection)};
}

// protected virtual [base kpTransformPreviewDialog]
QImage kpEffectsDialog::transformPixmap (const QImage &pixmap,
                                         int targetWidth, int targetHeight) const
{
    QImage pixmapWithEffect;

    if (m_effectWidget && !m_effectWidget->isNoOp ()) {
        pixmapWithEffect = m_effectWidget->applyEffect (pixmap);
    }
    else {
        pixmapWithEffect = pixmap;
    }

    return kpPixmapFX::scale (pixmapWithEffect, targetWidth, targetHeight);
}


// public
int kpEffectsDialog::selectedEffect () const
{
    return m_effectsComboBox->currentIndex ();
}

// public slot
void kpEffectsDialog::selectEffect (int which)
{
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "kpEffectsDialog::selectEffect(" << which << ")";
#endif

    if (which < 0 ||
        which >= m_effectsComboBox->count ())
    {
        return;
    }

    if (which != m_effectsComboBox->currentIndex ()) {
        m_effectsComboBox->setCurrentIndex (which);
    }


    delete m_effectWidget;
    m_effectWidget = nullptr;


    m_settingsGroupBox->setWindowTitle(QString());

#define CREATE_EFFECT_WIDGET(name)  \
    m_effectWidget = new name (m_actOnSelection, m_settingsGroupBox)
    // sync: order in constructor.
    switch (which)
    {
    case 0:
        CREATE_EFFECT_WIDGET (kpEffectBalanceWidget);
        break;

    case 1:
        CREATE_EFFECT_WIDGET (kpEffectEmbossWidget);
        break;

    case 2:
        CREATE_EFFECT_WIDGET (kpEffectFlattenWidget);
        break;
    
    case 3:
        CREATE_EFFECT_WIDGET (kpEffectToneEnhanceWidget);
        break;

    case 4:
        CREATE_EFFECT_WIDGET (kpEffectHSVWidget);
        break;

    case 5:
        CREATE_EFFECT_WIDGET (kpEffectInvertWidget);
        break;

    case 6:
        CREATE_EFFECT_WIDGET (kpEffectReduceColorsWidget);
        break;

    case 7:
        CREATE_EFFECT_WIDGET (kpEffectBlurSharpenWidget);
        break;
    }
#undef CREATE_EFFECT_WIDGET


    if (m_effectWidget)
    {
        const bool e = updatesEnabled ();
        setUpdatesEnabled (false);

    #if DEBUG_KP_EFFECTS_DIALOG
        qCDebug(kpLogDialogs) << "widget exists for effect #";
    #endif
        m_settingsGroupBox->setTitle (m_effectWidget->caption ());

        // Show widget.
        //
        // Don't resize the whole dialog when doing this.
        // This seems to work magically without any extra code with Qt4.
    #if DEBUG_KP_EFFECTS_DIALOG
        qCDebug(kpLogDialogs) << "addWidget";
    #endif
        m_settingsLayout->addWidget (m_effectWidget);
    #if DEBUG_KP_EFFECTS_DIALOG
        qCDebug(kpLogDialogs) << "show widget";
    #endif
        m_effectWidget->show ();

        connect (m_effectWidget, &kpEffectWidgetBase::settingsChangedNoWaitCursor,
                 this, &kpEffectsDialog::slotUpdate);
        connect (m_effectWidget, &kpEffectWidgetBase::settingsChanged,
                 this, &kpEffectsDialog::slotUpdateWithWaitCursor);
        connect (m_effectWidget, &kpEffectWidgetBase::settingsChangedDelayed,
                 this, &kpEffectsDialog::slotDelayedUpdate);

    #if DEBUG_KP_EFFECTS_DIALOG
        qCDebug(kpLogDialogs) << "about to setUpdatesEnabled()";
    #endif
        setUpdatesEnabled (e);
    }


#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "done"
              << endl
              << endl
              << endl;
#endif
}


// protected slot virtual [base kpTransformPreviewDialog]
void kpEffectsDialog::slotUpdate ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "kpEffectsDialog::slotUpdate()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif

    m_delayedUpdateTimer->stop ();

    kpTransformPreviewDialog::slotUpdate ();
}

// protected slot virtual [base kpTransformPreviewDialog]
void kpEffectsDialog::slotUpdateWithWaitCursor ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "kpEffectsDialog::slotUpdateWithWaitCursor()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif

    m_delayedUpdateTimer->stop ();

    kpTransformPreviewDialog::slotUpdateWithWaitCursor ();
}


// protected slot
void kpEffectsDialog::slotDelayedUpdate ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    qCDebug(kpLogDialogs) << "kpEffectsDialog::slotDelayedUpdate()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif
    m_delayedUpdateTimer->stop ();

    // (single shot)
    m_delayedUpdateTimer->start (400/*ms*/);
}


