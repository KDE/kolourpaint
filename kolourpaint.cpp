/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2015,2016 Martin Koller <kollix@aon.at>
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


#include <KAboutData>

#include "kpVersion.h"
#include "mainWindow/kpMainWindow.h"
#include <kolourpaintlicense.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QImageReader>
#include <QDir>
#include <KLocalizedString>

int main(int argc, char *argv [])
{
  QApplication app(argc, argv);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  KLocalizedString::setApplicationDomain("kolourpaint");

  KAboutData aboutData
  (
    QStringLiteral("kolourpaint"),
    i18n("KolourPaint"),
    QStringLiteral(KOLOURPAINT_VERSION_STRING),
    i18n("Paint Program by KDE"),
    KAboutLicense::Custom,
    QString(), // copyright statement - see license instead
    QString(), // other text
    QStringLiteral("http://www.kolourpaint.org/")   // home page
  );

  // (this is _not_ the same as KAboutLicense::BSD)
  aboutData.setLicenseText(i18n(kpLicenseText));
  aboutData.setDesktopFileName(QStringLiteral("org.kde.kolourpaint"));

  // Please add yourself here if you feel you're missing.
  // SYNC: with AUTHORS

  aboutData.addAuthor(i18n("Clarence Dang"), i18n("Project Founder"), QStringLiteral("dang@kde.org"));

  aboutData.addAuthor(i18n("Thurston Dang"), i18n("Chief Investigator"),
                      QStringLiteral("thurston_dang@users.sourceforge.net"));

  aboutData.addAuthor(i18n("Martin Koller"), i18n("Scanning Support, Alpha Support, Current Maintainer"),
                      QStringLiteral("kollix@aon.at"));

  aboutData.addAuthor(i18n("Kristof Borrey"), i18n("Icons"), QStringLiteral("borrey@kde.org"));
  aboutData.addAuthor(i18n("Tasuku Suzuki"), i18n("InputMethod Support"), QStringLiteral("stasuku@gmail.com"));
  aboutData.addAuthor(i18n("Kazuki Ohta"), i18n("InputMethod Support"), QStringLiteral("mover@hct.zaq.ne.jp"));
  aboutData.addAuthor(i18n("Nuno Pinheiro"), i18n("Icons"), QStringLiteral("nf.pinheiro@gmail.com"));
  aboutData.addAuthor(i18n("Danny Allen"), i18n("Icons"), QStringLiteral("dannya40uk@yahoo.co.uk"));
  aboutData.addAuthor(i18n("Mike Gashler"), i18n("Image Effects"), QStringLiteral("gashlerm@yahoo.com"));

  aboutData.addAuthor(i18n("Laurent Montel"), i18n("KDE 4 Porting"), QStringLiteral("montel@kde.org"));
  aboutData.addAuthor(i18n("Christoph Feck"), i18n("KF 5 Porting"), QStringLiteral("cfeck@kde.org"));

  aboutData.addCredit(i18n("Thanks to the many others who have helped to make this program possible."));

  QCommandLineParser cmdLine;
  KAboutData::setApplicationData(aboutData);
  QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kolourpaint"), QApplication::windowIcon()));
  cmdLine.addPositionalArgument(QStringLiteral("files"), i18n("Image files to open, optionally"), QStringLiteral("[files...]"));

  aboutData.setupCommandLine(&cmdLine);
  cmdLine.addOption(QCommandLineOption("mimetypes", i18n("List all readable image MIME types")));
  cmdLine.process(app);
  aboutData.processCommandLine(&cmdLine);

  // produce a list of MimeTypes which kolourpaint can handle (can be used inside the .desktop file)
  if ( cmdLine.isSet("mimetypes") )
  {
    foreach (const QByteArray &type, QImageReader::supportedMimeTypes())
    {
      if ( !type.isEmpty() )
        printf("%s;", type.constData());
    }

    printf("\n");

    return 0;
  }

  if ( app.isSessionRestored() )
  {
    // Creates a kpMainWindow using the default constructor and then
    // calls kpMainWindow::readProperties().
    kRestoreMainWindows<kpMainWindow>();
  }
  else
  {
    kpMainWindow *mainWindow;
    QStringList args = cmdLine.positionalArguments();

    if ( args.count() >= 1 )
    {
      for (int i = 0; i < args.count(); i++)
      {
        mainWindow = new kpMainWindow(QUrl::fromUserInput(args[i], QDir::currentPath(), QUrl::AssumeLocalFile));
        mainWindow->show();
      }
    }
    else
    {
      mainWindow = new kpMainWindow();
      mainWindow->show();
    }
  }

  return QApplication::exec();
}
