
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_EFFECTS_DIALOG 1


#include <kpeffectsdialog.h>

#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpdocument.h>
#include <kpeffectcontrast.h>
#include <kpeffectswirl.h>
#include <kppixmapfx.h>


// protected static
int kpEffectsDialog::s_lastEffectSelected = 0;


kpEffectsDialog::kpEffectsDialog (bool actOnSelection,
                                  kpMainWindow *parent,
                                  const char *name)
    : kpToolPreviewDialog (kpToolPreviewDialog::Preview,
                           QString::null/*actionName*/,
                           actOnSelection,
                           parent,
                           name),
      m_effectsComboBox (0),
      m_settingsGroupBox (0),
      m_settingsLayout (),
      m_colorEffectWidget (0)
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::kpEffectsDialog()" << endl;
#endif

    if (actOnSelection)
        setCaption (i18n ("More Image Effects (Selection)"));
    else
        setCaption (i18n ("More Image Effects"));


    QHBox *effectContainer = new QHBox (mainWidget ());
    effectContainer->setSpacing (spacingHint ());
    effectContainer->setMargin (0);

    QLabel *label = new QLabel (i18n ("&Effect:"), effectContainer);

    m_effectsComboBox = new KComboBox (effectContainer);
    m_effectsComboBox->insertItem (i18n ("Contrast"));
    m_effectsComboBox->insertItem (i18n ("Swirl"));

    label->setBuddy (m_effectsComboBox);
    effectContainer->setStretchFactor (m_effectsComboBox, 1);

    addCustomWidgetToFront (effectContainer);


    m_settingsGroupBox = new QGroupBox (i18n ("&Settings"), mainWidget ());
    m_settingsLayout = new QVBoxLayout (m_settingsGroupBox,
                                        marginHint () * 2,
                                        spacingHint ());
    addCustomWidgetToBack (m_settingsGroupBox);


    connect (m_effectsComboBox, SIGNAL (activated (int)),
             this, SLOT (slotEffectSelected (int)));
    m_effectsComboBox->setCurrentItem (s_lastEffectSelected);
    slotEffectSelected (s_lastEffectSelected);


    // TODO: actually work
    setMinimumSize (500, 480);

    resize (500, 480);


#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "\tabout to slotUpdate()" << endl;
#endif
    slotUpdate ();
}

kpEffectsDialog::~kpEffectsDialog ()
{
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


// protected slot
void kpEffectsDialog::slotEffectSelected (int which)
{
#if DEBUG_KP_EFFECTS_DIALOG
    kdDebug () << "kpEffectsDialog::slotEffectSelected(" << which << ")" << endl;
#endif

    delete m_colorEffectWidget;
    m_colorEffectWidget = 0;


    switch (which)
    {
    case 0:
        m_colorEffectWidget = new kpEffectContrastWidget (m_actOnSelection,
                                                          m_mainWindow,
                                                          m_settingsGroupBox);
        break;

    case 1:
        m_colorEffectWidget = new kpEffectSwirlWidget (m_actOnSelection,
                                                       m_mainWindow,
                                                       m_settingsGroupBox);
        break;
    }


    if (m_colorEffectWidget)
    {
    #if DEBUG_KP_EFFECTS_DIALOG
        kdDebug () << "\twidget exists for effect #" << endl;
    #endif
        m_settingsLayout->addWidget (m_colorEffectWidget);
        m_colorEffectWidget->show ();


        connect (m_colorEffectWidget, SIGNAL (settingsChanged ()),
                 this, SLOT (slotUpdate ()));
        slotUpdate ();


        s_lastEffectSelected = which;
    }
}


#include <kpeffectsdialog.moc>
