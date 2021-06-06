
// REFACT0R: Remote open/save file logic is duplicated in kpDocument.
// HITODO: Test when remote file support in KDE 4 stabilizes

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

#define DEBUG_KP_COLOR_COLLECTION 0

#include "kpColorCollection.h"

#include "kpUrlFormatter.h"

#include <KJobWidgets>
#include <KIO/StoredTransferJob>
#include <KIO/FileCopyJob>
#include <KLocalizedString>
#include <KMessageBox>
#include "kpLogCategories.h"
#include <KStringHandler>

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>

struct ColorNode
{
    ColorNode(const QColor &c, const QString &n)
        : color(c), name(n) {}

    QColor color;
    QString name;
};

//---------------------------------------------------------------------

Q_LOGGING_CATEGORY(kpLogColorCollection,           "kp.colorCollection")

//BEGIN kpColorCollectionPrivate
class kpColorCollectionPrivate
{
public:
    kpColorCollectionPrivate();
    kpColorCollectionPrivate(const kpColorCollectionPrivate&);

    QList<ColorNode> colorList;
    QString name;
    QString desc;
    kpColorCollection::Editable editable;
};

kpColorCollectionPrivate::kpColorCollectionPrivate()
  : editable(kpColorCollection::Yes)
{
}

kpColorCollectionPrivate::kpColorCollectionPrivate(const kpColorCollectionPrivate& p)
    : colorList(p.colorList), name(p.name), desc(p.desc), editable(p.editable)
{
}
//END kpColorCollectionPrivate

//---------------------------------------------------------------------

QStringList
kpColorCollection::installedCollections()
{
  QStringList paletteList;

  QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("colors"),
                                                QStandardPaths::LocateDirectory);
  for (const auto &path : paths) {
    paletteList.append(QDir(path).entryList(QStringList(), QDir::Files));
  }

  return paletteList;
}

kpColorCollection::kpColorCollection()
{
  d = new kpColorCollectionPrivate();
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

static void CouldNotOpenDialog (const QUrl &url, QWidget *parent)
{
     KMessageBox::sorry (parent,
        i18n ("Could not open color palette \"%1\".",
              kpUrlFormatter::PrettyFilename (url)));
}

// TODO: Set d->editable?
bool
kpColorCollection::open(const QUrl &url, QWidget *parent)
{
    if (url.isEmpty()) {
        return false;
    }

    KIO::StoredTransferJob *job = KIO::storedGet (url);
    KJobWidgets::setWindow (job, parent);

    if (!job->exec ())
    {
#if DEBUG_KP_COLOR_COLLECTION
        qCDebug(kpLogColorCollection) << "\tcould not download";
#endif
        ::CouldNotOpenDialog (url, parent);
        return false;
    }

  const QByteArray &data = job->data();
  QTextStream stream(data);

  // Read first line
  // Expected "GIMP Palette" or "KDE RGB Palette" or "KDE RGBA Palette"
  QString line = stream.readLine();
  if (line.indexOf(QLatin1String(" Palette")) == -1)
  {
     KMessageBox::sorry (parent,
        i18n ("Could not open color palette \"%1\" - unsupported format.\n"
              "The file may be corrupt.",
              kpUrlFormatter::PrettyFilename (url)));
     return false;
  }

  bool hasAlpha = line == QLatin1String("KDE RGBA Palette");   // new format includes alpha

  QList <ColorNode> newColorList;
  QString newDesc;

  while( !stream.atEnd() )
  {
     line = stream.readLine();
     if ( !line.isEmpty() && (line[0] == '#') )
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
        int r, g, b, a = 255;
        int pos = 0;
        bool ok = false;

        if ( hasAlpha )
          ok = (sscanf(line.toLatin1(), "%d %d %d %d%n", &r, &g, &b, &a, &pos) >= 4);
        else
          ok = (sscanf(line.toLatin1(), "%d %d %d%n", &r, &g, &b, &pos) >= 3);

        if ( ok )
        {
           r = qBound(0, r, 255);
           g = qBound(0, g, 255);
           b = qBound(0, b, 255);
           a = qBound(0, a, 255);
           QString name = line.mid(pos).trimmed();
           newColorList.append(ColorNode(QColor(r, g, b, a), name));
        }
     }
  }

  d->colorList = newColorList;
  d->name.clear ();
  d->desc = newDesc;

  return true;
}

static void CouldNotOpenKDEDialog (const QString &name, QWidget *parent)
{
     KMessageBox::sorry (parent,
        i18n ("Could not open KDE color palette \"%1\".", name));
}

bool
kpColorCollection::openKDE(const QString &name, QWidget *parent)
{
#if DEBUG_KP_COLOR_COLLECTION
  qCDebug(kpLogColorCollection) << "name=" << name;
#endif

  if (name.isEmpty())
  {
  #if DEBUG_KP_COLOR_COLLECTION
    qCDebug(kpLogColorCollection) << "name.isEmpty";
  #endif
    ::CouldNotOpenKDEDialog (name, parent);
    return false;
  }

  QString filename = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                                            "colors/" + name);
  if (filename.isEmpty())
  {
  #if DEBUG_KP_COLOR_COLLECTION
    qCDebug(kpLogColorCollection) << "could not find file";
  #endif
    ::CouldNotOpenKDEDialog (name, parent);
    return false;
  }

  // (this will pop up an error dialog on failure)
  if (!open (QUrl::fromLocalFile (filename), parent))
  {
  #if DEBUG_KP_COLOR_COLLECTION
    qCDebug(kpLogColorCollection) << "could not open";
  #endif
    return false;
  }

  d->name = name;
#if DEBUG_KP_COLOR_COLLECTION
  qCDebug(kpLogColorCollection) << "opened";
#endif
  return true;
}

static void CouldNotSaveDialog (const QUrl &url, QWidget *parent)
{
    // TODO: use file.errorString()
    KMessageBox::error (parent,
                        i18n ("Could not save color palette as \"%1\".",
                              kpUrlFormatter::PrettyFilename (url)));
}

static void SaveToFile (kpColorCollectionPrivate *d, QIODevice *device)
{
   // HITODO: QTextStream can fail but does not report errors.
   //         Bug in KColorCollection too.
   QTextStream str (device);

   QString description = d->desc.trimmed();
   description = '#' + description.split('\n', Qt::KeepEmptyParts).join(QLatin1String("\n#"));

   str << "KDE RGBA Palette\n";
   str << description << "\n";
   for (const auto &node : d->colorList)
   {
       // Added for KolourPaint.
       if ( !node.color.isValid() )
           continue;

       int r, g, b, a;
       node.color.getRgb(&r, &g, &b, &a);
       str << r << " " << g << " " << b << " " << a << " " << node.name << "\n";
   }

   str.flush();
}

bool
kpColorCollection::saveAs(const QUrl &url, QWidget *parent) const
{
   if (url.isLocalFile ())
   {
       const QString filename = url.toLocalFile ();

        // sync: All failure exit paths _must_ call QSaveFile::cancelWriting() or
        //       else, the QSaveFile destructor will overwrite the file,
        //       <filename>, despite the failure.
        QSaveFile atomicFileWriter (filename);
        {
            if (!atomicFileWriter.open (QIODevice::WriteOnly))
            {
                // We probably don't need this as <filename> has not been
                // opened.
                atomicFileWriter.cancelWriting ();

            #if DEBUG_KP_COLOR_COLLECTION
                qCDebug(kpLogColorCollection) << "\treturning false because could not open QSaveFile"
                          << " error=" << atomicFileWriter.error ();
            #endif
                ::CouldNotSaveDialog (url, parent);
                return false;
            }

            // Write to local temporary file.
            ::SaveToFile (d, &atomicFileWriter);

            // Atomically overwrite local file with the temporary file
            // we saved to.
            if (!atomicFileWriter.commit ())
            {
                atomicFileWriter.cancelWriting ();

            #if DEBUG_KP_COLOR_COLLECTION
                qCDebug(kpLogColorCollection) << "\tcould not close QSaveFile";
            #endif
                ::CouldNotSaveDialog (url, parent);
                return false;
            }
        }  // sync QSaveFile.cancelWriting()
    }
    // Remote file?
    else
    {
        // Create temporary file that is deleted when the variable goes
        // out of scope.
        QTemporaryFile tempFile;
        if (!tempFile.open ())
        {
        #if DEBUG_KP_COLOR_COLLECTION
            qCDebug(kpLogColorCollection) << "\treturning false because could not open tempFile";
        #endif
            ::CouldNotSaveDialog (url, parent);
            return false;
        }

        // Write to local temporary file.
        ::SaveToFile (d, &tempFile);

        // Collect name of temporary file now, as QTemporaryFile::fileName()
        // stops working after close() is called.
        const QString tempFileName = tempFile.fileName ();
    #if DEBUG_KP_COLOR_COLLECTION
            qCDebug(kpLogColorCollection) << "\ttempFileName='" << tempFileName << "'";
    #endif
        Q_ASSERT (!tempFileName.isEmpty ());

        tempFile.close ();
        if (tempFile.error () != QFile::NoError)
        {
        #if DEBUG_KP_COLOR_COLLECTION
            qCDebug(kpLogColorCollection) << "\treturning false because could not close";
        #endif
            ::CouldNotSaveDialog (url, parent);
            return false;
        }

        // Copy local temporary file to overwrite remote.
        KIO::FileCopyJob *job = KIO::file_copy (QUrl::fromLocalFile (tempFileName),
                                                url,
                                                -1,
                                                KIO::Overwrite);
        KJobWidgets::setWindow (job, parent);
        if (!job->exec ())
        {
        #if DEBUG_KP_COLOR_COLLECTION
            qCDebug(kpLogColorCollection) << "\treturning false because could not upload";
        #endif
            ::CouldNotSaveDialog (url, parent);
            return false;
        }
    }

   d->name.clear ();
   return true;
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

void kpColorCollection::resize(int newCount)
{
    if (newCount == count())
        return;
    else if (newCount < count())
    {
        d->colorList.erase(d->colorList.begin() + newCount, d->colorList.end());
    }
    else if (newCount > count())
    {
         while(newCount > count())
         {
             const int ret = addColor(QColor(), QString()/*color name*/);
             Q_ASSERT(ret == count() - 1);
         }
    }
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
    return {};

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
      return {};

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

