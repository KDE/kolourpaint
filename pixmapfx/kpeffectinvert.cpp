
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

#define DEBUG_KP_EFFECT_INVERT 0


#include <kpeffectinvert.h>

#include <qcheckbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kppixmapfx.h>


//
// kpEffectInvertCommand
//

kpEffectInvertCommand::kpEffectInvertCommand (int channels,
                                              bool actOnSelection,
                                              kpMainWindow *mainWindow)
    : kpColorEffectCommand (channels == RGB ?
                                i18n ("Invert Colors") : i18n ("Invert"),
                            actOnSelection, mainWindow),
      m_channels (channels)
{
}

kpEffectInvertCommand::kpEffectInvertCommand (bool actOnSelection,
                                              kpMainWindow *mainWindow)
    : kpColorEffectCommand (i18n ("Invert Colors"), actOnSelection, mainWindow),
      m_channels (RGB)
{
}

kpEffectInvertCommand::~kpEffectInvertCommand ()
{
}


// public static
void kpEffectInvertCommand::apply (QPixmap *destPixmapPtr, int channels)
{
    QImage image = kpPixmapFX::convertToImage (*destPixmapPtr);
    apply (&image, channels);
    *destPixmapPtr = kpPixmapFX::convertToPixmap (image);
}

// public static
QPixmap kpEffectInvertCommand::apply (const QPixmap &pm, int channels)
{
    QImage image = kpPixmapFX::convertToImage (pm);
    apply (&image, channels);
    return kpPixmapFX::convertToPixmap (image);
}

// public static
void kpEffectInvertCommand::apply (QImage *destImagePtr, int channels)
{
    QRgb mask = qRgba ((channels & Red) ? 0xFF : 0,
                       (channels & Green) ? 0xFF : 0,
                       (channels & Blue) ? 0xFF : 0,
                       0/*don't invert alpha*/);
#if DEBUG_KP_EFFECT_INVERT
    kdDebug () << "kpEffectInvertCommand::apply(channels=" << channels
               << ") mask=" << (int *) mask
               << endl;
#endif

    if (destImagePtr->depth () > 8)
    {
    #if 0
        // SYNC: TODO: Qt BUG - invertAlpha argument is inverted!!!
        destImagePtr->invertPixels (true/*no invert alpha (Qt 3.2)*/);
    #else
        // Above version works for Qt 3.2 at least.
        // But this version will always work (slower, though) and supports
        // inverting particular channels.
        for (int y = 0; y < destImagePtr->height (); y++)
        {
            for (int x = 0; x < destImagePtr->width (); x++)
            {
                destImagePtr->setPixel (x, y, destImagePtr->pixel (x, y) ^ mask);
            }
        }
    #endif
    }
    else
    {
        for (int i = 0; i < destImagePtr->numColors (); i++)
        {
            destImagePtr->setColor (i, destImagePtr->color (i) ^ mask);
        }
    }
}

// public static
QImage kpEffectInvertCommand::apply (const QImage &img, int channels)
{
    QImage retImage = img;
    apply (&retImage, channels);
    return retImage;
}


//
// kpEffectInvertCommand implements kpColorEffectCommand interface
//

// protected virtual [base kpColorEffectCommand]
QPixmap kpEffectInvertCommand::applyColorEffect (const QPixmap &pixmap)
{
    return apply (pixmap, m_channels);
}


//
// kpEffectInvertWidget
//

kpEffectInvertWidget::kpEffectInvertWidget (bool actOnSelection,
                                            kpMainWindow *mainWindow,
                                            QWidget *parent,
                                            const char *name)
    : kpColorEffectWidget (actOnSelection, mainWindow, parent, name)
{
    QVBoxLayout *topLevelLay = new QVBoxLayout (this, marginHint (), spacingHint ());


    QWidget *centerWidget = new QWidget (this);
    topLevelLay->addWidget (centerWidget, 0/*stretch*/, Qt::AlignCenter);


    QVBoxLayout *centerWidgetLay = new QVBoxLayout (centerWidget,
                                                    0/*margin*/,
                                                    spacingHint ());


    m_redCheckBox = new QCheckBox (i18n ("&Red"), centerWidget);
    m_greenCheckBox = new QCheckBox (i18n ("&Green"), centerWidget);
    m_blueCheckBox = new QCheckBox (i18n ("&Blue"), centerWidget);

    QWidget *spaceWidget = new QWidget (centerWidget);
    spaceWidget->setFixedSize (1, spacingHint ());

    m_allCheckBox = new QCheckBox (i18n ("&All"), centerWidget);


    m_redCheckBox->setChecked (false);
    m_greenCheckBox->setChecked (false);
    m_blueCheckBox->setChecked (false);

    m_allCheckBox->setChecked (false);


    centerWidgetLay->addWidget (m_redCheckBox);
    centerWidgetLay->addWidget (m_greenCheckBox);
    centerWidgetLay->addWidget (m_blueCheckBox);

    centerWidgetLay->addWidget (spaceWidget);

    centerWidgetLay->addWidget (m_allCheckBox);


    m_inSignalHandler = false;
    connect (m_redCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotRGBCheckBoxToggled ()));
    connect (m_greenCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotRGBCheckBoxToggled ()));
    connect (m_blueCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotRGBCheckBoxToggled ()));

    connect (m_allCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotAllCheckBoxToggled ()));
}

kpEffectInvertWidget::~kpEffectInvertWidget ()
{
}


// public
int kpEffectInvertWidget::channels () const
{
#if DEBUG_KP_EFFECT_INVERT
    kdDebug () << "kpEffectInvertWidget::channels()"
               << " isChecked: r=" << m_redCheckBox->isChecked ()
               << " g=" << m_greenCheckBox->isChecked ()
               << " b=" << m_blueCheckBox->isChecked ()
               << endl;
#endif

    int channels = 0;


    if (m_redCheckBox->isChecked ())
        channels |= kpEffectInvertCommand::Red;

    if (m_greenCheckBox->isChecked ())
        channels |= kpEffectInvertCommand::Green;

    if (m_blueCheckBox->isChecked ())
        channels |= kpEffectInvertCommand::Blue;


#if DEBUG_KP_EFFECT_INVERT
    kdDebug () << "\treturning channels=" << (int *) channels << endl;
#endif
    return channels;
}


//
// kpEffectInvertWidget implements kpColorEffectWidget interface
//

// public virtual [base kpColorEffectWidget]
QString kpEffectInvertWidget::caption () const
{
    return i18n ("Channels");
}


// public virtual [base kpColorEffectWidget]
bool kpEffectInvertWidget::isNoOp () const
{
    return (channels () == kpEffectInvertCommand::None);
}

// public virtual [base kpColorEffectWidget]
QPixmap kpEffectInvertWidget::applyColorEffect (const QPixmap &pixmap)
{
    return kpEffectInvertCommand::apply (pixmap, channels ());
}


// public virtual [base kpColorEffectWidget]
kpColorEffectCommand *kpEffectInvertWidget::createCommand () const
{
    return new kpEffectInvertCommand (channels (),
                                      m_actOnSelection,
                                      m_mainWindow);
}


// protected slots
void kpEffectInvertWidget::slotRGBCheckBoxToggled ()
{
    if (m_inSignalHandler)
        return;

    m_inSignalHandler = true;

    //blockSignals (true);
    m_allCheckBox->setChecked (m_redCheckBox->isChecked () &&
                               m_blueCheckBox->isChecked () &&
                               m_greenCheckBox->isChecked ());
    //blockSignals (false);

    emit settingsChanged ();

    m_inSignalHandler = false;
}

// protected slot
void kpEffectInvertWidget::slotAllCheckBoxToggled ()
{
    if (m_inSignalHandler)
        return;

    m_inSignalHandler = true;

    //blockSignals (true);
    m_redCheckBox->setChecked (m_allCheckBox->isChecked ());
    m_greenCheckBox->setChecked (m_allCheckBox->isChecked ());
    m_blueCheckBox->setChecked (m_allCheckBox->isChecked ());
    //blockSignals (false);

    emit settingsChanged ();

    m_inSignalHandler = false;
}


#include <kpeffectinvert.moc>

