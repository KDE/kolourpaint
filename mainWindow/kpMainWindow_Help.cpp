
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


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <QLabel>
#include <QtDBus>

#include <kaction.h>
#include <kactioncollection.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <krun.h>
#include <kshortcut.h>


// private
void kpMainWindow::setupHelpMenuActions ()
{
    KActionCollection *ac = actionCollection ();


    // Explanation for action name:
    // "Taking" is like a digital camera when you record the image and is
    // analogous to pressing PrintScreen.  However, "Acquiring" is when
    // the image is brought into KolourPaint, just as you would acquire
    // from a digital camera in future versions of KolourPaint.  Hence
    // "Acquiring" is more appropriate.
    // -- Thurston
    d->actionHelpTakingScreenshots = ac->addAction ("help_taking_screenshots");
    d->actionHelpTakingScreenshots->setText (i18n ("Acquiring &Screenshots"));
    connect (d->actionHelpTakingScreenshots, SIGNAL (triggered (bool)),
        SLOT (slotHelpTakingScreenshots ()));


    enableHelpMenuDocumentActions (false);
}

// private
void kpMainWindow::enableHelpMenuDocumentActions (bool /*enable*/)
{
    // d->m_actionHelpTakingScreenshots
}


// SYNC: kdebase/kwin/kwinbindings.cpp
static QString printScreenShortcutString ()
{
    KConfigGroup cfg (KGlobal::config (), "Global Shortcuts");

    // TODO: i18n() entry name?  kwinbindings.cpp seems to but it doesn't
    //       make sense.
    const QString cfgEntryString = cfg.readEntry ("Desktop Screenshot");


    // (only use 1st key sequence, if it exists)
    const QString humanReadableShortcut =
        KShortcut (cfgEntryString).primary().toString ();

    if (!humanReadableShortcut.isEmpty ())
    {
        return humanReadableShortcut;
    }
    else
    {
        // (localised)
        return KShortcut (Qt::CTRL + Qt::Key_Print).toString ();
    }
}


// private slot
void kpMainWindow::slotHelpTakingScreenshots ()
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotHelpTakingScreenshots()";
#endif

    toolEndShape ();


    // TODO: Totally bogus logic if kwin not running under same user as KolourPaint.
    // SYNC: KWin contains PrintScreen key logic
#if DEBUG_KP_MAIN_WINDOW
    //kDebug () << "\tdcopApps=" << dcopApps;
#endif
    bool isRunningKDE = true;  // COMPAT QDBus::sessionBus().busService()->nameHasOwner("kwin");

#if 0
{
    int i = 0;
    FILE *fp = fopen ("/home/kdevel/kolourpaint.tmp", "rt");
    if (fp && fscanf (fp, "Hello: %d", &i) == 1)
        isRunningKDE = i, fclose (fp);
}
#endif

    QString message;
    if (isRunningKDE)
    {
        message = i18n
        (
            "<p>"
            "To acquire a screenshot, press <b>%1</b>."
            "  The screenshot will be placed into the clipboard"
            " and you will be able to paste it in KolourPaint."
            "</p>"

            "<p>"
            "You may configure the <b>Desktop Screenshot</b> shortcut"
            " in the KDE Control Center"
            " module <a href=\"configure kde shortcuts\">Keyboard Shortcuts</a>."
            "</p>"

            "<p>Alternatively, you may try the application"
            " <a href=\"run ksnapshot\">KSnapshot</a>."
            "</p>",
            // TODO: Totally bogus logic if kwin not running under same user as KolourPaint.
            ::printScreenShortcutString ()
        );
    }
    else
    {
        message = i18n
        (
            "<p>"
            "You do not appear to be running KDE."
            "</p>"

            // We tell them this much even though they aren't running KDE
            // to entice them to use KDE since it's so easy.
            "<p>"
            "Once you have loaded KDE:<br>"
            "<blockquote>"
            "To acquire a screenshot, press <b>%1</b>."
            "  The screenshot will be placed into the clipboard"
            " and you will be able to paste it in KolourPaint."
            "</blockquote>"
            "</p>"

            "<p>Alternatively, you may try the application"
            " <a href=\"run ksnapshot\">KSnapshot</a>."
            "</p>",
            // TODO: Totally bogus logic if kwin not running under same user as KolourPaint.
            ::printScreenShortcutString ()
        );
    }


    // Add extra vertical space
    message += "<p>&nbsp;</p>";


    KDialog dlg (this);
    dlg.setCaption (i18n ("Acquiring Screenshots"));
    dlg.setButtons (KDialog::Close);
    dlg.setDefaultButton (KDialog::Close);
    dlg.showButtonSeparator (true);

    QLabel *messageLabel = new QLabel (message, &dlg);
    messageLabel->setWordWrap(true);

    connect (messageLabel, SIGNAL (linkActivated (const QString &)),
             this, SLOT (slotHelpTakingScreenshotsFollowLink (const QString &)));

    dlg.setMainWidget (messageLabel);

    dlg.exec ();
}

// private
void kpMainWindow::slotHelpTakingScreenshotsFollowLink (const QString &link)
{
#if DEBUG_KP_MAIN_WINDOW
    kDebug () << "kpMainWindow::slotHelpTakingScreenshotsFollowLink("
               << link << ")" << endl;
#endif

    if (link == "configure kde shortcuts")
    {
        KRun::runCommand ("kcmshell4 keys", this);
    }
    else if (link == "run ksnapshot")
    {
        KRun::runCommand ("ksnapshot", this);
    }
    else
    {
        kError () << "kpMainWindow::slotHelpTakingScreenshotsFollowLink("
                   << link << ")" << endl;
    }
}
