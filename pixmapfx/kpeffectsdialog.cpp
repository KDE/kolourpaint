
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#include <kpeffectsdialog.h>

#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpeffectbalance.h>
#include <kpeffectblursharpen.h>
#include <kpeffectemboss.h>
#include <kpeffectflatten.h>
#include <kpeffectinvert.h>
#include <kpeffectreducecolors.h>
#include <kppixmapfx.h>


// protected static
int kpEffectsDialog::s_lastWidth = 640;
int kpEffectsDialog::s_lastHeight = 620;


kpEffectsDialog::kpEffectsDialog (bool actOnSelection,
                                  kpMainWindow *parent,
                                  const char *name)
    : kpToolPreviewDialog (kpToolPreviewDialog::Preview,
                           true/*reserve top row*/,
                           QString::null/*caption*/,
                           QString::null/*afterActionText (no Dimensions Group Box)*/,
                           actOnSelection,
                           parent,
                           name),
      m_delayedUpdateTimer (new QTimer (this)),
      m_effectsComboBox (0),
      m_settingsGroupBox (0),
      m_settingsLayout (0),
      m_colorEffectWidget (0)
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::kpEffectsDialog()" << endl;
#endif

    if (actOnSelection)
        setCaption (i18n ("More Image Effects (Selection)"));
    else
        setCaption (i18n ("More Image Effects"));


    connect (m_delayedUpdateTimer, SIGNAL (timeout ()),
             this, SLOT (slotUpdateWithWaitCursor ()));


    QHBox *effectContainer = new QHBox (mainWidget ());
    effectContainer->setSpacing (spacingHint () * 4
                                 /*need more space for QGroupBox titles*/);
    effectContainer->setMargin (0);

    QLabel *label = new QLabel (i18n ("&Effect:"), effectContainer);

    m_effectsComboBox = new KComboBox (effectContainer);
    m_effectsComboBox->insertItem (i18n ("Balance"));
    m_effectsComboBox->insertItem (i18n ("Emboss"));
    m_effectsComboBox->insertItem (i18n ("Flatten"));
    m_effectsComboBox->insertItem (i18n ("Invert"));
    m_effectsComboBox->insertItem (i18n ("Reduce Colors"));
    m_effectsComboBox->insertItem (i18n ("Soften & Sharpen"));

    label->setBuddy (m_effectsComboBox);
    effectContainer->setStretchFactor (m_effectsComboBox, 1);

    addCustomWidgetToFront (effectContainer);


    m_settingsGroupBox = new QGroupBox (mainWidget ());
    m_settingsLayout = new QVBoxLayout (m_settingsGroupBox,
                                        marginHint () * 2,
                                        spacingHint ());
    addCustomWidgetToBack (m_settingsGroupBox);


    connect (m_effectsComboBox, SIGNAL (activated (int)),
             this, SLOT (selectEffect (int)));
    selectEffect (0);


    resize (s_lastWidth, s_lastHeight);


#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "\tabout to slotUpdate()" << endl;
#endif
    slotUpdate ();
}

kpEffectsDialog::~kpEffectsDialog ()
{
    s_lastWidth = width ();
    s_lastHeight = height ();
}


// public virtual [base kpToolPreviewDialog]
bool kpEffectsDialog::isNoOp () const
{
    if (!m_colorEffectWidget)
        return true;

    return m_colorEffectWidget->isNoOp ();
}

// public
kpColorEffectCommand *kpEffectsDialog::createCommand () const
{
    if (!m_colorEffectWidget)
        return 0;

    return m_colorEffectWidget->createCommand ();
}


// protected virtual [base kpToolPreviewDialog]
QSize kpEffectsDialog::newDimensions () const
{
    kpDocument *doc = document ();
    if (!doc)
        return QSize ();

    return QSize (doc->width (m_actOnSelection),
                  doc->height (m_actOnSelection));
}

// protected virtual [base kpToolPreviewDialog]
QPixmap kpEffectsDialog::transformPixmap (const QPixmap &pixmap,
                                          int targetWidth, int targetHeight) const
{
    QPixmap pixmapWithEffect;

    if (m_colorEffectWidget)
        pixmapWithEffect = m_colorEffectWidget->applyColorEffect (pixmap);
    else
        pixmapWithEffect = pixmap;

    return kpPixmapFX::scale (pixmapWithEffect, targetWidth, targetHeight);
}


// public
int kpEffectsDialog::selectedEffect () const
{
    return m_effectsComboBox->currentItem ();
}

// public slot
void kpEffectsDialog::selectEffect (int which)
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::selectEffect(" << which << ")" << endl;
#endif

    if (which < 0 ||
        which >= m_effectsComboBox->count ())
    {
        return;
    }

    if (which != m_effectsComboBox->currentItem ())
        m_effectsComboBox->setCurrentItem (which);


    delete m_colorEffectWidget;
    m_colorEffectWidget = 0;


    m_settingsGroupBox->setCaption (QString::null);

#define CREATE_EFFECT_WIDGET(name)                        \
    m_colorEffectWidget = new name (m_actOnSelection,     \
                                    m_mainWindow,         \
                                    m_settingsGroupBox)
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
        CREATE_EFFECT_WIDGET (kpEffectInvertWidget);
        break;

    case 4:
        CREATE_EFFECT_WIDGET (kpEffectReduceColorsWidget);
        break;

    case 5:
        CREATE_EFFECT_WIDGET (kpEffectBlurSharpenWidget);
        break;
    }
#undef CREATE_EFFECT_WIDGET


    if (m_colorEffectWidget)
    {
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\twidget exists for effect #" << endl;
    #endif
        m_settingsGroupBox->setTitle (m_colorEffectWidget->caption ());


        // Don't resize the preview when showing the widget:
        // TODO: actually work

        QSize previewGroupBoxMinSize = m_previewGroupBox->minimumSize ();
        QSize previewGroupBoxMaxSize = m_previewGroupBox->maximumSize ();
        QLayout::ResizeMode previewGroupBoxResizeMode =
            m_previewGroupBox->layout () ?
                m_previewGroupBox->layout ()->resizeMode () :
                QLayout::Auto;
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tpreviewGroupBox: minSize=" << previewGroupBoxMinSize
                   << " maxSize=" << previewGroupBoxMaxSize
                   << " size=" << m_previewGroupBox->size ()
                   << " layout=" << m_previewGroupBox->layout ()
                   << " resizeMode=" << previewGroupBoxResizeMode
                   << endl;
    #endif

        if (m_previewGroupBox->layout ())
            m_previewGroupBox->layout ()->setResizeMode (QLayout::FreeResize);
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter set resizeMode, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif
        m_previewGroupBox->setFixedSize (m_previewGroupBox->size ());
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter set fixedSize, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif

        // Show widget
        m_settingsLayout->addWidget (m_colorEffectWidget);
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter addWidget, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif
        m_colorEffectWidget->show ();
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter addWidget show, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif

        m_previewGroupBox->setMinimumSize (previewGroupBoxMinSize);
        m_previewGroupBox->setMaximumSize (previewGroupBoxMaxSize);
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter set fixedSize, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif
        if (m_previewGroupBox->layout ())
            m_previewGroupBox->layout ()->setResizeMode (previewGroupBoxResizeMode);
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter restore resizeMode, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif


        connect (m_colorEffectWidget, SIGNAL (settingsChangedNoWaitCursor ()),
                 this, SLOT (slotUpdate ()));
        connect (m_colorEffectWidget, SIGNAL (settingsChanged ()),
                 this, SLOT (slotUpdateWithWaitCursor ()));
        connect (m_colorEffectWidget, SIGNAL (settingsChangedDelayed ()),
                 this, SLOT (slotDelayedUpdate ()));
        slotUpdateWithWaitCursor ();
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\tafter slotUpdateWithWaitCursor, previewGroupBox.size="
                   << m_previewGroupBox->size () << endl;
    #endif
    }
}


// protected slot virtual [base kpToolPreviewDialog]
void kpEffectsDialog::slotUpdate ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::slotUpdate()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif

    m_delayedUpdateTimer->stop ();

    kpToolPreviewDialog::slotUpdate ();
}

// protected slot virtual [base kpToolPreviewDialog]
void kpEffectsDialog::slotUpdateWithWaitCursor ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::slotUpdateWithWaitCursor()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif

    m_delayedUpdateTimer->stop ();

    kpToolPreviewDialog::slotUpdateWithWaitCursor ();
}


// protected slot
void kpEffectsDialog::slotDelayedUpdate ()
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::slotDelayedUpdate()"
               << " timerActive=" << m_delayedUpdateTimer->isActive ()
               << endl;
#endif
    m_delayedUpdateTimer->stop ();

    m_delayedUpdateTimer->start (400/*ms*/, true/*single shot*/);
}


#include <kpeffectsdialog.moc>
