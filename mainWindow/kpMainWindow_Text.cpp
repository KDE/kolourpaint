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


#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include <kwidgetsaddons_version.h>
#include <KActionCollection>
#include <KSharedConfig>
#include <KConfigGroup>
#include "kpLogCategories.h"
#include <KFontAction>
#include <KFontSizeAction>
#include <KToolBar>
#include <KToggleAction>
#include <KLocalizedString>

#include "widgets/toolbars/kpColorToolBar.h"
#include "kpDefs.h"
#include "layers/selections/text/kpTextStyle.h"
#include "tools/selection/text/kpToolText.h"
#include "widgets/toolbars/kpToolToolBar.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "views/kpZoomedView.h"


// private
void kpMainWindow::setupTextToolBarActions ()
{
    KActionCollection *ac = actionCollection ();

    d->actionTextFontFamily = ac->add<KFontAction> (QStringLiteral("text_font_family"));
    d->actionTextFontFamily->setText (i18n ("Font Family"));
    connect (d->actionTextFontFamily,
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 78, 0)
             &KSelectAction::textTriggered,
#else
             static_cast<void (KFontAction::*)(const QString&)>(&KFontAction::triggered),
#endif
             this, &kpMainWindow::slotTextFontFamilyChanged);

    d->actionTextFontSize = ac->add<KFontSizeAction> (QStringLiteral("text_font_size"));
    d->actionTextFontSize->setText (i18n ("Font Size"));
    connect (d->actionTextFontSize,
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 78, 0)
             &KSelectAction::indexTriggered,
#else
             static_cast<void (KFontSizeAction::*)(int)>(&KFontSizeAction::triggered),
#endif
             this, &kpMainWindow::slotTextFontSizeChanged);

    d->actionTextBold = ac->add<KToggleAction> (QStringLiteral("text_bold"));
    d->actionTextBold->setIcon(QIcon::fromTheme(QStringLiteral("format-text-bold")));
    d->actionTextBold->setText (i18n ("Bold"));
    connect (d->actionTextBold, &KToggleAction::triggered,
             this, &kpMainWindow::slotTextBoldChanged);

    d->actionTextItalic = ac->add<KToggleAction> (QStringLiteral("text_italic"));
    d->actionTextItalic->setIcon (QIcon::fromTheme(QStringLiteral("format-text-italic")));
    d->actionTextItalic->setText (i18n ("Italic"));
    connect (d->actionTextItalic, &KToggleAction::triggered,
             this, &kpMainWindow::slotTextItalicChanged);

    d->actionTextUnderline = ac->add<KToggleAction> (QStringLiteral("text_underline"));
    d->actionTextUnderline->setIcon (QIcon::fromTheme(QStringLiteral("format-text-underline")));
    d->actionTextUnderline->setText (i18n ("Underline"));
    connect (d->actionTextUnderline, &KToggleAction::triggered,
             this, &kpMainWindow::slotTextUnderlineChanged);

    d->actionTextStrikeThru = ac->add<KToggleAction> (QStringLiteral("text_strike_thru"));
    d->actionTextStrikeThru->setIcon(QIcon::fromTheme(QStringLiteral("format-text-strikethrough")));
    d->actionTextStrikeThru->setText (i18n ("Strike Through"));
    connect (d->actionTextStrikeThru, &KToggleAction::triggered,
             this, &kpMainWindow::slotTextStrikeThruChanged);


    readAndApplyTextSettings ();


    enableTextToolBarActions (false);
}

// private
void kpMainWindow::readAndApplyTextSettings ()
{
    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);

    const QString font (cfg.readEntry (kpSettingFontFamily, QStringLiteral ("Times")));
    d->actionTextFontFamily->setFont (font);
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "asked setFont to set to=" << font
              << "- got back=" << d->actionTextFontFamily->font ();
#endif
    d->actionTextFontSize->setFontSize (cfg.readEntry (kpSettingFontSize, 14));
    d->actionTextBold->setChecked (cfg.readEntry (kpSettingBold, false));
    d->actionTextItalic->setChecked (cfg.readEntry (kpSettingItalic, false));
    d->actionTextUnderline->setChecked (cfg.readEntry (kpSettingUnderline, false));
    d->actionTextStrikeThru->setChecked (cfg.readEntry (kpSettingStrikeThru, false));

    d->textOldFontFamily = d->actionTextFontFamily->font ();
    d->textOldFontSize = d->actionTextFontSize->fontSize ();
}


// public
void kpMainWindow::enableTextToolBarActions (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::enableTextToolBarActions(" << enable << ")";
#endif

    d->actionTextFontFamily->setEnabled (enable);
    d->actionTextFontSize->setEnabled (enable);
    d->actionTextBold->setEnabled (enable);
    d->actionTextItalic->setEnabled (enable);
    d->actionTextUnderline->setEnabled (enable);
    d->actionTextStrikeThru->setEnabled (enable);

    if (textToolBar ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        qCDebug(kpLogMainWindow) << "\thave toolbar - setShown";
    #endif
        // COMPAT: KDE4 does not place the Text Tool Bar in a new row, underneath
        //         the Main Tool Bar, if there isn't enough room.  This makes
        //         accessing the Text Tool Bar's buttons difficult.
        textToolBar ()->setVisible (enable);
    }
}


// private slot
void kpMainWindow::slotTextFontFamilyChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextFontFamilyChanged() alive="
               << d->isFullyConstructed
               << "fontFamily="
               << d->actionTextFontFamily->font ()
               << "action.currentItem="
               << d->actionTextFontFamily->currentItem ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotFontFamilyChanged (d->actionTextFontFamily->font (),
                                           d->textOldFontFamily);
    }

    // Since editable KSelectAction's steal focus from view, switch back to mainView
    // TODO: back to the last view
    if (d->mainView) {
        d->mainView->setFocus ();
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingFontFamily, d->actionTextFontFamily->font ());
    cfg.sync ();

    d->textOldFontFamily = d->actionTextFontFamily->font ();
}

// private slot
void kpMainWindow::slotTextFontSizeChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextFontSizeChanged() alive="
               << d->isFullyConstructed
               << " fontSize="
               << d->actionTextFontSize->fontSize ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotFontSizeChanged (d->actionTextFontSize->fontSize (),
                                         d->textOldFontSize);
    }

    // Since editable KSelectAction's steal focus from view, switch back to mainView
    // TODO: back to the last view
    if (d->mainView) {
        d->mainView->setFocus ();
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingFontSize, d->actionTextFontSize->fontSize ());
    cfg.sync ();

    d->textOldFontSize = d->actionTextFontSize->fontSize ();
}

// private slot
void kpMainWindow::slotTextBoldChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextFontBoldChanged() alive="
               << d->isFullyConstructed
               << " bold="
               << d->actionTextBold->isChecked ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotBoldChanged (d->actionTextBold->isChecked ());
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingBold, d->actionTextBold->isChecked ());
    cfg.sync ();
}

// private slot
void kpMainWindow::slotTextItalicChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextFontItalicChanged() alive="
               << d->isFullyConstructed
               << " bold="
               << d->actionTextItalic->isChecked ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotItalicChanged (d->actionTextItalic->isChecked ());
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingItalic, d->actionTextItalic->isChecked ());
    cfg.sync ();
}

// private slot
void kpMainWindow::slotTextUnderlineChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextFontUnderlineChanged() alive="
               << d->isFullyConstructed
               << " underline="
               << d->actionTextUnderline->isChecked ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotUnderlineChanged (d->actionTextUnderline->isChecked ());
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingUnderline, d->actionTextUnderline->isChecked ());
    cfg.sync ();
}

// private slot
void kpMainWindow::slotTextStrikeThruChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::slotTextStrikeThruChanged() alive="
               << d->isFullyConstructed
               << " strikeThru="
               << d->actionTextStrikeThru->isChecked ();
#endif

    if (!d->isFullyConstructed) {
        return;
    }

    if (d->toolText && d->toolText->hasBegun ())
    {
        toolEndShape ();
        d->toolText->slotStrikeThruChanged (d->actionTextStrikeThru->isChecked ());
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupText);
    cfg.writeEntry (kpSettingStrikeThru, d->actionTextStrikeThru->isChecked ());
    cfg.sync ();
}


// public
KToolBar *kpMainWindow::textToolBar ()
{
    return toolBar (QStringLiteral("textToolBar"));
}

bool kpMainWindow::isTextStyleBackgroundOpaque () const
{
    if (d->toolToolBar)
    {
        kpToolWidgetOpaqueOrTransparent *oot =
            d->toolToolBar->toolWidgetOpaqueOrTransparent ();

        if (oot)
        {
            return oot->isOpaque ();
        }
    }

    return true;
}

// public
kpTextStyle kpMainWindow::textStyle () const
{
    return kpTextStyle (d->actionTextFontFamily->font (),
                        d->actionTextFontSize->fontSize (),
                        d->actionTextBold->isChecked (),
                        d->actionTextItalic->isChecked (),
                        d->actionTextUnderline->isChecked (),
                        d->actionTextStrikeThru->isChecked (),
                        d->colorToolBar ? d->colorToolBar->foregroundColor () : kpColor::Invalid,
                        d->colorToolBar ? d->colorToolBar->backgroundColor () : kpColor::Invalid,
                        isTextStyleBackgroundOpaque ());
}

// public
void kpMainWindow::setTextStyle (const kpTextStyle &textStyle_)
{
#if DEBUG_KP_MAIN_WINDOW
    qCDebug(kpLogMainWindow) << "kpMainWindow::setTextStyle()";
#endif

    d->settingTextStyle++;


    if (textStyle_.fontFamily () != d->actionTextFontFamily->font ())
    {
        d->actionTextFontFamily->setFont (textStyle_.fontFamily ());
        slotTextFontFamilyChanged ();
    }

    if (textStyle_.fontSize () != d->actionTextFontSize->fontSize ())
    {
        d->actionTextFontSize->setFontSize (textStyle_.fontSize ());
        slotTextFontSizeChanged ();
    }

    if (textStyle_.isBold () != d->actionTextBold->isChecked ())
    {
        d->actionTextBold->setChecked (textStyle_.isBold ());
        slotTextBoldChanged ();
    }

    if (textStyle_.isItalic () != d->actionTextItalic->isChecked ())
    {
        d->actionTextItalic->setChecked (textStyle_.isItalic ());
        slotTextItalicChanged ();
    }

    if (textStyle_.isUnderline () != d->actionTextUnderline->isChecked ())
    {
        d->actionTextUnderline->setChecked (textStyle_.isUnderline ());
        slotTextUnderlineChanged ();
    }

    if (textStyle_.isStrikeThru () != d->actionTextStrikeThru->isChecked ())
    {
        d->actionTextStrikeThru->setChecked (textStyle_.isStrikeThru ());
        slotTextStrikeThruChanged ();
    }


    if (textStyle_.foregroundColor () != d->colorToolBar->foregroundColor ())
    {
        d->colorToolBar->setForegroundColor (textStyle_.foregroundColor ());
    }

    if (textStyle_.backgroundColor () != d->colorToolBar->backgroundColor ())
    {
        d->colorToolBar->setBackgroundColor (textStyle_.backgroundColor ());
    }


    if (textStyle_.isBackgroundOpaque () != isTextStyleBackgroundOpaque ())
    {
        if (d->toolToolBar)
        {
            kpToolWidgetOpaqueOrTransparent *oot =
                d->toolToolBar->toolWidgetOpaqueOrTransparent ();

            if (oot)
            {
                oot->setOpaque (textStyle_.isBackgroundOpaque ());
            }
        }
    }


    d->settingTextStyle--;
}

// public
int kpMainWindow::settingTextStyle () const
{
    return d->settingTextStyle;
}

