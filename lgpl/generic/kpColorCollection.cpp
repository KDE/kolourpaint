/* This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2007 Clarence Dang (dang@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color collection

#include "kpColorCollection.h"

#include <QtCore/QFile>
#include <QtCore/QTextIStream>

#include <kglobal.h>
#include <kio/netaccess.h>
#include <KLocale>
#include <KMessageBox>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <KUrl>

#include <kpUrlFormatter.h>

struct ColorNode
{
    ColorNode(const QColor &c, const QString &n)
        : color(c), name(n) {}

    QColor color;
    QString name;
};


//BEGIN kpColorCollectionPrivate
class kpColorCollectionPrivate
{
public:
    kpColorCollectionPrivate(const QString&);
    kpColorCollectionPrivate(const kpColorCollectionPrivate&);
    ~kpColorCollectionPrivate() {}
    QList<ColorNode> colorList;

    QString name;
    QString desc;
    kpColorCollection::Editable editable;
};

kpColorCollectionPrivate::kpColorCollectionPrivate(const QString &_name)
    : name(_name)
{
}

kpColorCollectionPrivate::kpColorCollectionPrivate(const kpColorCollectionPrivate& p)
    : colorList(p.colorList), name(p.name), desc(p.desc), editable(p.editable)
{
}
//END kpColorCollectionPrivate

QStringList
kpColorCollection::installedCollections()
{
  QStringList paletteList;
  KGlobal::dirs()->findAllResources("config", "colors/*", KStandardDirs::NoDuplicates, paletteList);

  int strip = strlen("colors/");
  for(QStringList::Iterator it = paletteList.begin();
      it != paletteList.end();
      ++it)
  {
      (*it) = (*it).mid(strip);
  }

  return paletteList;
}

kpColorCollection::kpColorCollection(const QString &name)
{
  d = new kpColorCollectionPrivate(name);

  if (name.isEmpty()) return;

  QString filename = KStandardDirs::locate("config", "colors/"+name);
  if (filename.isEmpty()) return;

  open (KUrl (filename), 0/*HITODO: correct widget*/);
}

kpColorCollection::kpColorCollection(const kpColorCollection &p)
{
    d = new kpColorCollectionPrivate(*p.d);
}

kpColorCollection::~kpColorCollection()
{
  // Need auto-save?
    delete d;
}

bool
kpColorCollection::open(const KUrl &url, QWidget *parent)
{
   // HITODO: This is wrong for remote files.
  QFile paletteFile(url.path ());
  if (!paletteFile.exists() ||
      !paletteFile.open(QIODevice::ReadOnly))
  {
     KMessageBox::sorry (parent,
        i18n ("Could not open color palette \"%1\".",
              kpUrlFormatter::PrettyFilename (url)));
     return false;
  }

  // Read first line
  // Expected "GIMP Palette"
  QString line = QString::fromLocal8Bit(paletteFile.readLine());
  if (line.indexOf(" Palette") == -1)
  {
     KMessageBox::sorry (parent,
        i18n ("Could not open color palette \"%1\" - unsupported format.\n"
              "The file may be corrupt.",
              kpUrlFormatter::PrettyFilename (url)));
     return false;
  }

  QList <ColorNode> newColorList;
  QString newDesc;

  while( !paletteFile.atEnd() )
  {
     line = QString::fromLocal8Bit(paletteFile.readLine());
     if (line[0] == '#')
     {
        // This is a comment line
        line = line.mid(1); // Strip '#'
        line = line.trimmed(); // Strip remaining white space..
        if (!line.isEmpty())
        {
            newDesc += line+'\n'; // Add comment to description
        }
     }
     else
     {
        // This is a color line, hopefully
        line = line.trimmed();
        if (line.isEmpty()) continue;
        int r, g, b;
        int pos = 0;
        if (sscanf(line.toAscii(), "%d %d %d%n", &r, &g, &b, &pos) >= 3)
        {
           r = qBound(0, r, 255);
           g = qBound(0, g, 255);
           b = qBound(0, b, 255);
           QString name = line.mid(pos).trimmed();
           newColorList.append(ColorNode(QColor(r, g, b), name));
        }
     }
  }

  d->colorList = newColorList;
  d->desc = newDesc;

  return true;
}

static void CouldNotSaveDialog (const KUrl &url, QWidget *parent)
{
    // TODO: use file.errorString()
    KMessageBox::error (parent,
                        i18n ("Could not save color palette as \"%1\".",
                              kpUrlFormatter::PrettyFilename (url)));
}

bool
kpColorCollection::saveAs(const KUrl &url, QWidget *parent) const
{
   if (KIO::NetAccess::exists (url, KIO::NetAccess::DestinationSide/*write*/, parent))
   {
       int result = KMessageBox::warningContinueCancel (parent,
          i18n ("A color palette called \"%1\" already exists.\n"
                "Do you want to overwrite it?",
                kpUrlFormatter::PrettyFilename (url)),
          QString (),
          KGuiItem (i18n ("Overwrite")));
       if (result != KMessageBox::Continue)
          return false;
   }

   // HITODO: This is wrong for remote files.
   const QString filename = url.path ();

   KSaveFile sf(filename);
   if (!sf.open())
   {
      ::CouldNotSaveDialog (url, parent);
      return false;
   }


   QTextStream str ( &sf );

   QString description = d->desc.trimmed();
   description = '#'+description.split( "\n", QString::KeepEmptyParts).join("\n#");

   str << "KDE RGB Palette\n";
   str << description << "\n";
   foreach (const ColorNode &node, d->colorList)
   {
       int r,g,b;
       node.color.getRgb(&r, &g, &b);
       str << r << " " << g << " " << b << " " << node.name << "\n";
   }

   sf.flush();
   if (!sf.finalize())
   {
      ::CouldNotSaveDialog (url, parent);
      return false;
   }

   return true;
}

bool
kpColorCollection::save(QWidget *parent) const
{
   QString filename = KStandardDirs::locateLocal("config", "colors/" + d->name);
   return saveAs (KUrl (filename), parent);
}

QString kpColorCollection::description() const
{
    return d->desc;
}

void kpColorCollection::setDescription(const QString &desc)
{
    d->desc = desc;
}

QString kpColorCollection::name() const
{
    return d->name;
}

void kpColorCollection::setName(const QString &name)
{
    d->name = name;
}

kpColorCollection::Editable kpColorCollection::editable() const
{
    return d->editable;
}

void kpColorCollection::setEditable(Editable editable)
{
    d->editable = editable;
}

int kpColorCollection::count() const
{
    return (int) d->colorList.count();
}

kpColorCollection&
kpColorCollection::operator=( const kpColorCollection &p)
{
  if (&p == this) return *this;
    d->colorList = p.d->colorList;
    d->name = p.d->name;
    d->desc = p.d->desc;
    d->editable = p.d->editable;
  return *this;
}

QColor
kpColorCollection::color(int index) const
{
    if ((index < 0) || (index >= count()))
	return QColor();

    return d->colorList[index].color;
}

int
kpColorCollection::findColor(const QColor &color) const
{
    for (int i = 0; i < d->colorList.size(); ++i)
  {
        if (d->colorList[i].color == color)
        return i;
  }
  return -1;
}

QString
kpColorCollection::name(int index) const
{
  if ((index < 0) || (index >= count()))
	return QString();

  return d->colorList[index].name;
}

QString kpColorCollection::name(const QColor &color) const
{
    return name(findColor(color));
}

int
kpColorCollection::addColor(const QColor &newColor, const QString &newColorName)
{
    d->colorList.append(ColorNode(newColor, newColorName));
    return count() - 1;
}

int
kpColorCollection::changeColor(int index,
                      const QColor &newColor,
                      const QString &newColorName)
{
    if ((index < 0) || (index >= count()))
	return -1;

  ColorNode& node = d->colorList[index];
  node.color = newColor;
  node.name  = newColorName;

  return index;
}

int kpColorCollection::changeColor(const QColor &oldColor,
                          const QColor &newColor,
                          const QString &newColorName)
{
    return changeColor( findColor(oldColor), newColor, newColorName);
}

