
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

#include <kactionclasses.h>
#include <kapplication.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kptooltext.h>
#include <kpview.h>


#define TOOL_TEXT_READY 0

// private
void kpMainWindow::setupTextToolBarActions ()
{
    KActionCollection *ac = actionCollection ();

    m_actionTextFontFamily = new KFontAction (i18n ("Font Family"), 0/*shortcut*/,
        this, SLOT (slotTextFontFamilyChanged ()), ac, "text_font_family");
    m_actionTextFontSize = new KFontSizeAction (i18n ("Font Size"), 0/*shortcut*/,
        this, SLOT (slotTextFontSizeChanged ()), ac, "text_font_size");

    m_actionTextBold = new KToggleAction (i18n ("Bold"), 0/*shortcut*/,
        this, SLOT (slotTextBoldChanged ()), ac, "text_bold");
    m_actionTextItalic = new KToggleAction (i18n ("Italic"), 0/*shortcut*/,
        this, SLOT (slotTextItalicChanged ()), ac, "text_italic");
    m_actionTextUnderline = new KToggleAction (i18n ("Underline"), 0/*shortcut*/,
        this, SLOT (slotTextUnderlineChanged ()), ac, "text_underline");
    m_actionTextStrikeThru = new KToggleAction (i18n ("Strike Through"), 0/*shortcut*/,
        this, SLOT (slotTextStrikeThruChanged ()), ac, "text_strike_thru");


    readAndApplyTextSettings ();


    enableTextToolBarActions (false);
}

// private
void kpMainWindow::readAndApplyTextSettings ()
{
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();

    m_actionTextFontFamily->setFont (cfg->readEntry (kpSettingFontFamily, QString::fromLatin1 ("Times")));
    m_actionTextFontSize->setFontSize (cfg->readNumEntry (kpSettingFontSize, 14));
    m_actionTextBold->setChecked (cfg->readBoolEntry (kpSettingBold, false));
    m_actionTextItalic->setChecked (cfg->readBoolEntry (kpSettingItalic, false));
    m_actionTextUnderline->setChecked (cfg->readBoolEntry (kpSettingUnderline, false));
    m_actionTextStrikeThru->setChecked (cfg->readBoolEntry (kpSettingStrikeThru, false));
}


// public
void kpMainWindow::enableTextToolBarActions (bool enable)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::enableTextToolBarActions(" << enable << ")" << endl;
#endif

    m_actionTextFontFamily->setEnabled (enable);
    m_actionTextFontSize->setEnabled (enable);
    m_actionTextBold->setEnabled (enable);
    m_actionTextItalic->setEnabled (enable);
    m_actionTextUnderline->setEnabled (enable);
    m_actionTextStrikeThru->setEnabled (enable);

    if (textToolBar ())
    {
    #if DEBUG_KP_MAIN_WINDOW
        kdDebug () << "\thave toolbar - setShown" << endl;
    #endif
        textToolBar ()->setShown (enable);
    }
}


// private slot
void kpMainWindow::slotTextFontFamilyChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextFontFamilyChanged() alive="
               << m_isFullyConstructed
               << " fontFamily="
               << m_actionTextFontFamily->font ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotFontFamilyChanged (m_actionTextFontFamily->font ());
#endif

    // Since editable KSelectAction's steal focus from view, switch back to mainView
    // TODO: back to the last view
    if (m_mainView)
        m_mainView->setFocus ();

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingFontFamily, m_actionTextFontFamily->font ());
    cfg->sync ();
}

// private slot
void kpMainWindow::slotTextFontSizeChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextFontSizeChanged() alive="
               << m_isFullyConstructed
               << " fontSize="
               << m_actionTextFontSize->fontSize ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotFontSizeChanged (m_actionTextFontSize->fontSize ());
#endif

    // Since editable KSelectAction's steal focus from view, switch back to mainView
    // TODO: back to the last view
    if (m_mainView)
        m_mainView->setFocus ();

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingFontSize, m_actionTextFontSize->fontSize ());
    cfg->sync ();
}

// private slot
void kpMainWindow::slotTextBoldChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextFontBoldChanged() alive="
               << m_isFullyConstructed
               << " bold="
               << m_actionTextBold->isChecked ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotBoldChanged (m_actionTextBold->isChecked ());
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingBold, m_actionTextBold->isChecked ());
    cfg->sync ();
}

// private slot
void kpMainWindow::slotTextItalicChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextFontItalicChanged() alive="
               << m_isFullyConstructed
               << " bold="
               << m_actionTextItalic->isChecked ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotItalicChanged (m_actionTextItalic->isChecked ());
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingItalic, m_actionTextItalic->isChecked ());
    cfg->sync ();
}

// private slot
void kpMainWindow::slotTextUnderlineChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextFontUnderlineChanged() alive="
               << m_isFullyConstructed
               << " underline="
               << m_actionTextUnderline->isChecked ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotUnderlineChanged (m_actionTextUnderline->isChecked ());
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingUnderline, m_actionTextUnderline->isChecked ());
    cfg->sync ();
}

// private slot
void kpMainWindow::slotTextStrikeThruChanged ()
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotTextStrikeThruChanged() alive="
               << m_isFullyConstructed
               << " strikeThru="
               << m_actionTextStrikeThru->isChecked ()
               << endl;
#endif

    if (!m_isFullyConstructed)
        return;

#if TOOL_TEXT_READY
    if (m_toolText && m_toolText->hasBegun ())
        m_toolText->slotStrikeThruChanged (m_actionTextStrikeThru->isChecked ());
#endif

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupText);
    KConfigBase *cfg = cfgGroupSaver.config ();
    cfg->writeEntry (kpSettingStrikeThru, m_actionTextStrikeThru->isChecked ());
    cfg->sync ();
}


// public
KToolBar *kpMainWindow::textToolBar ()
{
    return toolBar ("textToolBar");
}

// public
kpTextStyle kpMainWindow::textStyle () const
{
    return kpTextStyle (m_actionTextFontFamily->font (),
                        m_actionTextFontSize->fontSize (),
                        m_actionTextBold->isChecked (),
                        m_actionTextItalic->isChecked (),
                        m_actionTextUnderline->isChecked (),
                        m_actionTextStrikeThru->isChecked ());
}
