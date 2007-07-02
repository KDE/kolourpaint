
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


#include <qfile.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kimageio.h>
#include <klocale.h>

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
        ki18n(0/*copyright statement - see license instead*/),
        ki18n ("Support / Feedback:\n"
                   "kolourpaint-support@lists.sourceforge.net\n"),
        "http://www.kolourpaint.org/"
    );


    // this is _not_ the same as KAboutData::License_BSD
    aboutData.setLicenseText (ki18n(kpLicenseText));


    aboutData.setCustomAuthorText (
        ki18n
        (
            "\n"
            "For support, or to report bugs and feature requests, please email\n"
            "<kolourpaint-support@lists.sourceforge.net>"
            " - the free and friendly\n"
            "KolourPaint support service.\n"
            "\n"
        ),
        ki18n
        (
            "<qt>"
            "For support, or to report bugs and feature requests, please email<br>"
            "<a href=\"mailto:kolourpaint-support@lists.sourceforge.net\">kolourpaint-support@lists.sourceforge.net</a>"
            " - the free and friendly<br>"
            "KolourPaint support service.<br>"
            "<br>"
            "</qt>"
        ));


    // SYNC: with AUTHORS

    aboutData.addAuthor (ki18n("Clarence Dang"), ki18n ("Maintainer"), "dang@kde.org");
    aboutData.addAuthor (ki18n("Thurston Dang"), ki18n ("Chief Investigator"),
                         "thurston_dang@users.sourceforge.net");
    aboutData.addAuthor (ki18n("Kristof Borrey"), ki18n ("Icons"), "borrey@kde.org");
    aboutData.addAuthor (ki18n("Kazuki Ohta"), ki18n ("InputMethod Support"), "mover@hct.zaq.ne.jp");
    aboutData.addAuthor (ki18n("Nuno Pinheiro"), ki18n ("Icons"), "nf.pinheiro@gmail.com");
    aboutData.addAuthor (ki18n("Danny Allen"), ki18n ("Icons"), "dannya40uk@yahoo.co.uk");
    aboutData.addAuthor (ki18n("Laurent Montel"), ki18n ("KDE 4 Porting"), "montel@kde.org");


    aboutData.addCredit (ki18n("Rashid N. Achilov"));
    aboutData.addCredit (ki18n("Toyohiro Asukai"));
    aboutData.addCredit (ki18n("Bela-Andreas Bargel"));
    aboutData.addCredit (ki18n("Waldo Bastian"));
    aboutData.addCredit (ki18n("Ismail Belhachmi"));
    aboutData.addCredit (ki18n("Sashmit Bhaduri"));
    aboutData.addCredit (ki18n("Antonio Bianco"));
    aboutData.addCredit (ki18n("Stephan Binner"));
    aboutData.addCredit (ki18n("Markus Brueffer"));
    aboutData.addCredit (ki18n("Rob Buis"));
    aboutData.addCredit (ki18n("Lucijan Busch"));
    aboutData.addCredit (ki18n("Mikhail Capone"));
    aboutData.addCredit (ki18n("Enrico Ceppi"));
    aboutData.addCredit (ki18n("Tom Chance"));
    aboutData.addCredit (ki18n("Albert Astals Cid"));
    aboutData.addCredit (ki18n("Jennifer Dang"));
    aboutData.addCredit (ki18n("Lawrence Dang"));
    aboutData.addCredit (ki18n("Christoph Eckert"));
    aboutData.addCredit (ki18n("David Faure"));
    aboutData.addCredit (ki18n("P. Fisher"));
    aboutData.addCredit (ki18n("Nicolas Goutte"));
    aboutData.addCredit (ki18n("Herbert Graeber"));
    aboutData.addCredit (ki18n("Brad Grant"));
    aboutData.addCredit (ki18n("David Greenaway"));
    aboutData.addCredit (ki18n("Wilco Greven"));
    aboutData.addCredit (ki18n("Hubert Grininger"));
    aboutData.addCredit (ki18n("Adriaan de Groot"));
    aboutData.addCredit (ki18n("Esben Mose Hansen"));
    aboutData.addCredit (ki18n("Nadeem Hasan"));
    aboutData.addCredit (ki18n("Simon Hausmann"));
    aboutData.addCredit (ki18n("Michael Hoehne"));
    aboutData.addCredit (ki18n("Andrew J"));
    aboutData.addCredit (ki18n("Werner Joss"));
    aboutData.addCredit (ki18n("Derek Kite"));
    aboutData.addCredit (ki18n("Tobias Koenig"));
    aboutData.addCredit (ki18n("Dmitry Kolesnikov"));
    aboutData.addCredit (ki18n("Stephan Kulow"));
    aboutData.addCredit (ki18n("Eric Laffoon"));
    aboutData.addCredit (ki18n("Michael Lake"));
    aboutData.addCredit (ki18n("Sebastien Laout"));
    aboutData.addCredit (ki18n("David Ling"));
    aboutData.addCredit (ki18n("Volker Lochte"));
    aboutData.addCredit (ki18n("Anders Lund"));
    aboutData.addCredit (ki18n("Thiago Macieira"));
    aboutData.addCredit (ki18n("Jacek Masiulaniec"));
    aboutData.addCredit (ki18n("Benjamin Meyer"));
    aboutData.addCredit (ki18n("Amir Michail"));
    aboutData.addCredit (ki18n("Robert Moszczynski"));
    aboutData.addCredit (ki18n("Dirk Mueller"));
    aboutData.addCredit (ki18n("Ruivaldo Neto"));
    aboutData.addCredit (ki18n("Ralf Nolden"));
    aboutData.addCredit (ki18n("Maks Orlovich"));
    aboutData.addCredit (ki18n("Steven Pasternak"));
    aboutData.addCredit (ki18n("Cédric Pasteur"));
    aboutData.addCredit (ki18n("Erik K. Pedersen"));
    aboutData.addCredit (ki18n("Dennis Pennekamp"));
    aboutData.addCredit (ki18n("Jos Poortvliet"));
    aboutData.addCredit (ki18n("Boudewijn Rempt"));
    aboutData.addCredit (ki18n("Marcos Rodriguez"));
    aboutData.addCredit (ki18n("Matt Rogers"));
    aboutData.addCredit (ki18n("Francisco Jose Canizares Santofimia"));
    aboutData.addCredit (ki18n("Bram Schoenmakers"));
    aboutData.addCredit (ki18n("Dirk Schönberger"));
    aboutData.addCredit (ki18n("Lutz Schweizer"));
    aboutData.addCredit (ki18n("Emmeran Seehuber"));
    aboutData.addCredit (ki18n("Peter Simonsson"));
    aboutData.addCredit (ki18n("Andrew Simpson"));
    aboutData.addCredit (ki18n("A T Somers"));
    aboutData.addCredit (ki18n("Igor Stepin"));
    aboutData.addCredit (ki18n("Stephen Sweeney"));
    aboutData.addCredit (ki18n("Bart Symons"));
    aboutData.addCredit (ki18n("Stefan Taferner"));
    aboutData.addCredit (ki18n("Hogne Titlestad"));
    aboutData.addCredit (ki18n("Brandon Mark Turner"));
    aboutData.addCredit (ki18n("Jonathan Turner"));
    aboutData.addCredit (ki18n("Stephan Unknown"));
    aboutData.addCredit (ki18n("Dries Verachtert"));
    aboutData.addCredit (ki18n("Simon Vermeersch"));
    aboutData.addCredit (ki18n("Lauri Watts"));
    aboutData.addCredit (ki18n("Mark Wege"));
    aboutData.addCredit (ki18n("Christoph Wiesen"));
    aboutData.addCredit (ki18n("Andre Wobbeking"));
    aboutData.addCredit (ki18n("Luke-Jr"));
    aboutData.addCredit (ki18n("Maxim_86ualb2"));
    aboutData.addCredit (ki18n("Michele"));


    KCmdLineArgs::init (argc, argv, &aboutData);

    KCmdLineOptions cmdLineOptions;
    cmdLineOptions.add("+[file]", ki18n ("Image file to open"));
    KCmdLineArgs::addCmdLineOptions (cmdLineOptions);

    KApplication app;

    // Qt says this is necessary but I don't think it is...
    QObject::connect (&app, SIGNAL (lastWindowClosed ()),
                      &app, SLOT (quit ()));


    if (app.isSessionRestored ())
        RESTORE (kpMainWindow)
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
