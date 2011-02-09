
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


#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kimageio.h>
#include <klocale.h>
#include <KMessageBox>

#include <kpDefs.h>
#include <kpMainWindow.h>

#include <kolourpaintlicense.h>
#include <kolourpaintversion.h>


int main (int argc, char *argv [])
{
    KAboutData aboutData
    (
        "kolourpaint", 0,
        ki18n ("KolourPaint"),
        kpVersionText,
        ki18n ("Paint Program for KDE"),
        KAboutData::License_Custom,
        ki18n (0/*copyright statement - see license instead*/),
        ki18n ("To obtain support, please visit the website."),
        "http://www.kolourpaint.org/"
    );

    // (this is _not_ the same as KAboutData::License_BSD)
    aboutData.setLicenseText (ki18n (kpLicenseText));


    // Please add yourself here if you feel you're missing.
    // SYNC: with AUTHORS

    aboutData.addAuthor (ki18n ("Clarence Dang"), ki18n ("Project Founder"), "dang@kde.org");
    aboutData.addAuthor (ki18n ("Thurston Dang"), ki18n ("Chief Investigator"),
                         "thurston_dang@users.sourceforge.net");
    aboutData.addAuthor (ki18n ("Martin Koller"), ki18n ("Scanning Support, Alpha Support, Current Maintainer"), "kollix@aon.at");
    aboutData.addAuthor (ki18n ("Kristof Borrey"), ki18n ("Icons"), "borrey@kde.org");
    aboutData.addAuthor (ki18n ("Tasuku Suzuki"), ki18n ("InputMethod Support"), "stasuku@gmail.com");
    aboutData.addAuthor (ki18n ("Kazuki Ohta"), ki18n ("InputMethod Support"), "mover@hct.zaq.ne.jp");
    aboutData.addAuthor (ki18n ("Nuno Pinheiro"), ki18n ("Icons"), "nf.pinheiro@gmail.com");
    aboutData.addAuthor (ki18n ("Danny Allen"), ki18n ("Icons"), "dannya40uk@yahoo.co.uk");
    aboutData.addAuthor (ki18n ("Mike Gashler"), ki18n ("Image Effects"), "gashlerm@yahoo.com");

    aboutData.addAuthor (ki18n ("Laurent Montel"), ki18n ("KDE 4 Porting"), "montel@kde.org");

    // TODO: missing a lot of people who helped with the KDE 4 port.

    aboutData.addCredit (ki18n ("Thanks to the many others who have helped to make this program possible."));

    KCmdLineArgs::init (argc, argv, &aboutData);

    KCmdLineOptions cmdLineOptions;
    cmdLineOptions.add ("+[file]", ki18n ("Image file to open"));
    KCmdLineArgs::addCmdLineOptions (cmdLineOptions);

    KApplication app;

    if (app.isSessionRestored ())
    {
        // Creates a kpMainWindow using the default constructor and then
        // calls kpMainWindow::readProperties().
        RESTORE (kpMainWindow)
    }
    else
    {
        kpMainWindow *mainWindow;
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs ();

        if (args->count () >= 1)
        {
            for (int i = 0; i < args->count (); i++)
            {
                mainWindow = new kpMainWindow (args->url (i));
                mainWindow->show ();
            }
        }
        else
        {
            mainWindow = new kpMainWindow ();
            mainWindow->show ();
        }

        args->clear ();
    }

    return app.exec ();
}
