
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

#define DEBUG_KP_EFFECT_FLATTEN 0


#include <kpeffectflatten.h>

#include <qcheckbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qvbox.h>

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kimageeffect.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kppixmapfx.h>


//
// kpEffectFlattenCommand
//

kpEffectFlattenCommand::kpEffectFlattenCommand (const QColor &color1,
                                                const QColor &color2,
                                                bool actOnSelection,
                                                kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Flatten"), actOnSelection, mainWindow),
      m_color1 (color1), m_color2 (color2)
{
}

kpEffectFlattenCommand::~kpEffectFlattenCommand ()
{
}


// public static
void kpEffectFlattenCommand::apply (QPixmap *destPixmapPtr,
                                    const QColor &color1, const QColor &color2)
{
    if (!destPixmapPtr)
        return;

    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    apply (&image, color1, color2);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpEffectFlattenCommand::apply (const QPixmap &pm,
                                       const QColor &color1, const QColor &color2)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    apply (&image, color1, color2);
    return kpPixmapFX::convertToPixmap (image);
}

// public static
void kpEffectFlattenCommand::apply (QImage *destImagePtr,
                                    const QColor &color1, const QColor &color2)
{
    if (!destImagePtr)
        return;

    KImageEffect::flatten (*destImagePtr/*ref*/, color1, color2);
}

// public static
QImage kpEffectFlattenCommand::apply (const QImage &img,
                                      const QColor &color1, const QColor &color2)
{
    QImage retImage = img;
    apply (&retImage, color1, color2);
    return retImage;
}


//
// kpEffectFlattenCommand implements kpColorEffectCommand interface
//

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectFlattenCommand::applyColorEffect (const QPixmap &pixmap)
{
    return apply (pixmap, m_color1, m_color2);
}


//
// kpEffectFlattenWidget
//

// public static
// Don't initialise globally when we probably don't have a colour
// allocation context.  This way, the colours aren't sometimes invalid
// (e.g. at 8-bit).
QColor kpEffectFlattenWidget::s_lastColor1;
QColor kpEffectFlattenWidget::s_lastColor2;

kpEffectFlattenWidget::kpEffectFlattenWidget (bool actOnSelection,
                                              kpMainWindow *mainWindow,
                                              QWidget *parent,
                                              const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    if (!s_lastColor1.isValid () || !s_lastColor2.isValid ())
    {
        KConfigGroupSaver cfgGroupSaver (KGlobal::config (), kpSettingsGroupFlattenEffect);
        KConfigBase *cfg = cfgGroupSaver.config ();

        s_lastColor1 = cfg->readColorEntry (kpSettingFlattenEffectColor1);
        if (!s_lastColor1.isValid ())
            s_lastColor1 = Qt::red;

        s_lastColor2 = cfg->readColorEntry (kpSettingFlattenEffectColor2);
        if (!s_lastColor2.isValid ())
            s_lastColor2 = Qt::blue;
    }


    m_enableCheckBox = new QCheckBox (i18n ("E&nable"), this);

    QVBox *colorButtonContainer = new QVBox (this);
    colorButtonContainer->setMargin (KDialog::marginHint () / 2);
    colorButtonContainer->setSpacing (spacingHint ());
    m_color1Button = new KColorButton (s_lastColor1, colorButtonContainer);
    m_color2Button = new KColorButton (s_lastColor2, colorButtonContainer);


    m_color1Button->setEnabled (false);
    m_color2Button->setEnabled (false);


    QVBoxLayout *lay = new QVBoxLayout (this, marginHint (), spacingHint ());
    lay->addWidget (m_enableCheckBox);
    lay->addWidget (colorButtonContainer);


    connect (m_enableCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotEnableChanged (bool)));

    connect (m_color1Button, SIGNAL (changed (const QColor &)),
             this, SIGNAL (settingsChanged ()));
    connect (m_color2Button, SIGNAL (changed (const QColor &)),
             this, SIGNAL (settingsChanged ()));
}

kpEffectFlattenWidget::~kpEffectFlattenWidget ()
{
    s_lastColor1 = color1 ();
    s_lastColor2 = color2 ();


    KConfigGroupSaver cfgGroupSaver (KGlobal::config (), kpSettingsGroupFlattenEffect);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingFlattenEffectColor1, s_lastColor1);
    cfg->writeEntry (kpSettingFlattenEffectColor2, s_lastColor2);
    cfg->sync ();
}


// public
QColor kpEffectFlattenWidget::color1 () const
{
    return m_color1Button->color ();
}

// public
QColor kpEffectFlattenWidget::color2 () const
{
    return m_color2Button->color ();
}


//
// kpEffectFlattenWidget implements kpColorEffectWidget interface
//

// public virtual [base kpColorEffectWidget]
QString kpEffectFlattenWidget::caption () const
{
    return i18n ("Colors");
}


// public virtual [base kpColorEffectWidget]
bool kpEffectFlattenWidget::isNoOp () const
{
    return !m_enableCheckBox->isChecked ();
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectFlattenWidget::applyColorEffect (const QPixmap &pixmap)
{
#if DEBUG_KP_EFFECT_FLATTEN
    kdDebug () << "kpEffectFlattenWidget::applyColorEffect() nop="
               << isNoOp () << endl;
#endif

    if (isNoOp ())
        return pixmap;

    return kpEffectFlattenCommand::apply (pixmap, color1 (), color2 ());
}


// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectFlattenWidget::createCommand () const
{
    return new kpEffectFlattenCommand (color1 (), color2 (),
                                       m_actOnSelection,
                                       m_mainWindow);
}


// protected slot:
void kpEffectFlattenWidget::slotEnableChanged (bool enable)
{
#if DEBUG_KP_EFFECT_FLATTEN
    kdDebug () << "kpEffectFlattenWidget::slotEnableChanged(" << enable
               << ") enableButton=" << m_enableCheckBox->isChecked ()
               << endl;
#endif

    m_color1Button->setEnabled (enable);
    m_color2Button->setEnabled (enable);

    emit settingsChanged ();
}


#include <kpeffectflatten.moc>

