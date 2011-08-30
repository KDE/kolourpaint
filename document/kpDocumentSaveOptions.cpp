
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

#define DEBUG_KP_DOCUMENT_SAVE_OPTIONS 0


#include <kpDocumentSaveOptions.h>

#include <qbitmap.h>
#include <QImage>
#include <qstring.h>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>

#include <kpDefs.h>
#include <kpPixmapFX.h>

//---------------------------------------------------------------------

class kpDocumentSaveOptionsPrivate
{
public:
    QString m_mimeType;
    int m_colorDepth;
    bool m_dither;
    int m_quality;
};

//---------------------------------------------------------------------

kpDocumentSaveOptions::kpDocumentSaveOptions ()
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = invalidMimeType ();
    d->m_colorDepth = invalidColorDepth ();
    d->m_dither = initialDither ();
    d->m_quality = invalidQuality ();
}

//---------------------------------------------------------------------

kpDocumentSaveOptions::kpDocumentSaveOptions (const kpDocumentSaveOptions &rhs)
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = rhs.mimeType ();
    d->m_colorDepth = rhs.colorDepth ();
    d->m_dither = rhs.dither ();
    d->m_quality = rhs.quality ();
}

//---------------------------------------------------------------------

kpDocumentSaveOptions::kpDocumentSaveOptions (QString mimeType, int colorDepth, bool dither, int quality)
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = mimeType;
    d->m_colorDepth = colorDepth;
    d->m_dither = dither;
    d->m_quality = quality;
}

//---------------------------------------------------------------------

kpDocumentSaveOptions::~kpDocumentSaveOptions ()
{
    delete d;
}

//---------------------------------------------------------------------


// public
bool kpDocumentSaveOptions::operator== (const kpDocumentSaveOptions &rhs) const
{
    return (mimeType () == rhs.mimeType () &&
            colorDepth () == rhs.colorDepth () &&
            dither () == rhs.dither () &&
            quality () == rhs.quality ());
}

//---------------------------------------------------------------------

// public
bool kpDocumentSaveOptions::operator!= (const kpDocumentSaveOptions &rhs) const
{
    return !(*this == rhs);
}

//---------------------------------------------------------------------


// public
kpDocumentSaveOptions &kpDocumentSaveOptions::operator= (const kpDocumentSaveOptions &rhs)
{
    setMimeType (rhs.mimeType ());
    setColorDepth (rhs.colorDepth ());
    setDither (rhs.dither ());
    setQuality (rhs.quality ());

    return *this;
}

//---------------------------------------------------------------------


// public
void kpDocumentSaveOptions::printDebug (const QString &prefix) const
{
    const QString usedPrefix = !prefix.isEmpty () ?
                                   prefix + QLatin1String (": ") :
                                   QString();

    kDebug () << usedPrefix
               << "mimeType=" << mimeType ()
               << " colorDepth=" << colorDepth ()
               << " dither=" << dither ()
               << " quality=" << quality ()
               << endl;
}

//---------------------------------------------------------------------


// public
QString kpDocumentSaveOptions::mimeType () const
{
    return d->m_mimeType;
}

//---------------------------------------------------------------------

// public
void kpDocumentSaveOptions::setMimeType (const QString &mimeType)
{
    d->m_mimeType = mimeType;
}

//---------------------------------------------------------------------


// public static
QString kpDocumentSaveOptions::invalidMimeType ()
{
    return QString();
}

//---------------------------------------------------------------------

// public static
bool kpDocumentSaveOptions::mimeTypeIsInvalid (const QString &mimeType)
{
    return (mimeType == invalidMimeType ());
}

//---------------------------------------------------------------------

// public
bool kpDocumentSaveOptions::mimeTypeIsInvalid () const
{
    return mimeTypeIsInvalid (mimeType ());
}

//---------------------------------------------------------------------


// public
int kpDocumentSaveOptions::colorDepth () const
{
    return d->m_colorDepth;
}

// public
void kpDocumentSaveOptions::setColorDepth (int depth)
{
    d->m_colorDepth = depth;
}


// public static
int kpDocumentSaveOptions::invalidColorDepth ()
{
    return -1;
}

// public static
bool kpDocumentSaveOptions::colorDepthIsInvalid (int colorDepth)
{
    return (colorDepth != 1 && colorDepth != 8 && colorDepth != 32);
}

// public
bool kpDocumentSaveOptions::colorDepthIsInvalid () const
{
    return colorDepthIsInvalid (colorDepth ());
}


// public
bool kpDocumentSaveOptions::dither () const
{
    return d->m_dither;
}

// public
void kpDocumentSaveOptions::setDither (bool dither)
{
    d->m_dither = dither;
}


// public static
int kpDocumentSaveOptions::initialDither ()
{
    return false;  // to avoid accidental double dithering
}


// public
int kpDocumentSaveOptions::quality () const
{
    return d->m_quality;
}

// public
void kpDocumentSaveOptions::setQuality (int quality)
{
    d->m_quality = quality;
}


// public static
int kpDocumentSaveOptions::invalidQuality ()
{
    return -2;
}

// public static
bool kpDocumentSaveOptions::qualityIsInvalid (int quality)
{
    return (quality < -1 || quality > 100);
}

// public
bool kpDocumentSaveOptions::qualityIsInvalid () const
{
    return qualityIsInvalid (quality ());
}


// public static
QString kpDocumentSaveOptions::defaultMimeType (const KConfigGroup &config)
{
    return config.readEntry (kpSettingForcedMimeType,
                              QString::fromLatin1 ("image/png"));
}

// public static
void kpDocumentSaveOptions::saveDefaultMimeType (KConfigGroup &config,
                                                 const QString &mimeType)
{
    config.writeEntry (kpSettingForcedMimeType, mimeType);
}


// public static
int kpDocumentSaveOptions::defaultColorDepth (const KConfigGroup &config)
{
    int colorDepth =
        config.readEntry (kpSettingForcedColorDepth, -1);

    if (colorDepthIsInvalid (colorDepth))
    {
        // (not screen depth, in case of transparency)
        colorDepth = 32;
    }

    return colorDepth;
}

//---------------------------------------------------------------------

// public static
void kpDocumentSaveOptions::saveDefaultColorDepth (KConfigGroup &config, int colorDepth)
{
    config.writeEntry (kpSettingForcedColorDepth, colorDepth);
}

//---------------------------------------------------------------------


// public static
int kpDocumentSaveOptions::defaultDither (const KConfigGroup &config)
{
    return config.readEntry (kpSettingForcedDither, initialDither ());
}

//---------------------------------------------------------------------

// public static
void kpDocumentSaveOptions::saveDefaultDither (KConfigGroup &config, bool dither)
{
    config.writeEntry (kpSettingForcedDither, dither);
}

//---------------------------------------------------------------------


// public static
int kpDocumentSaveOptions::defaultQuality (const KConfigGroup &config)
{
    int val = config.readEntry (kpSettingForcedQuality, -1);
    if (qualityIsInvalid (val))
        val = -1;

    return val;
}

//---------------------------------------------------------------------

// public static
void kpDocumentSaveOptions::saveDefaultQuality (KConfigGroup &config, int quality)
{
    config.writeEntry (kpSettingForcedQuality, quality);
}

//---------------------------------------------------------------------


// public static
kpDocumentSaveOptions kpDocumentSaveOptions::defaultDocumentSaveOptions (const KConfigGroup &config)
{
    kpDocumentSaveOptions saveOptions;
    saveOptions.setMimeType (defaultMimeType (config));
    saveOptions.setColorDepth (defaultColorDepth (config));
    saveOptions.setDither (defaultDither (config));
    saveOptions.setQuality (defaultQuality (config));

#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS
    saveOptions.printDebug ("kpDocumentSaveOptions::defaultDocumentSaveOptions()");
#endif

    return saveOptions;
}

//---------------------------------------------------------------------

// public static
bool kpDocumentSaveOptions::saveDefaultDifferences (KConfigGroup &config,
                                                    const kpDocumentSaveOptions &oldDocInfo,
                                                    const kpDocumentSaveOptions &newDocInfo)
{
    bool savedSomething = false;

#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS
    kDebug () << "kpDocumentSaveOptions::saveDefaultDifferences()";
    oldDocInfo.printDebug ("\told");
    newDocInfo.printDebug ("\tnew");
#endif

    if (newDocInfo.mimeType () != oldDocInfo.mimeType ())
    {
        saveDefaultMimeType (config, newDocInfo.mimeType ());
        savedSomething = true;
    }

    if (newDocInfo.colorDepth () != oldDocInfo.colorDepth ())
    {
        saveDefaultColorDepth (config, newDocInfo.colorDepth ());
        savedSomething = true;
    }

    if (newDocInfo.dither () != oldDocInfo.dither ())
    {
        saveDefaultDither (config, newDocInfo.dither ());
        savedSomething = true;
    }

    if (newDocInfo.quality () != oldDocInfo.quality ())
    {
        saveDefaultQuality (config, newDocInfo.quality ());
        savedSomething = true;
    }

    return savedSomething;
}

//---------------------------------------------------------------------


static QStringList mimeTypesSupportingProperty (const QString &property,
    const QStringList &defaultMimeTypesWithPropertyList)
{
    QStringList mimeTypeList;

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupMimeTypeProperties);

    if (cfg.hasKey (property))
    {
        mimeTypeList = cfg.readEntry (property, QStringList ());
    }
    else
    {
        mimeTypeList = defaultMimeTypesWithPropertyList;

        cfg.writeEntry (property, mimeTypeList);
        cfg.sync ();
    }

    return mimeTypeList;
}

//---------------------------------------------------------------------

static bool mimeTypeSupportsProperty (const QString &mimeType,
    const QString &property, const QStringList &defaultMimeTypesWithPropertyList)
{
    const QStringList mimeTypeList = mimeTypesSupportingProperty (
        property, defaultMimeTypesWithPropertyList);

    return mimeTypeList.contains (mimeType);
}

//---------------------------------------------------------------------


// SYNC: update mime info
//
// Only care about writable mimetypes.
//
// Run:
//
//     branches/kolourpaint/control/scripts/gen_mimetype_line.sh Write |
//         branches/kolourpaint/control/scripts/split_mimetype_line.pl
//
// in the version of kdelibs/kimgio/ (e.g. KDE 4.0) KolourPaint is shipped with,
// to check for any new mimetypes to add info for.  In the methods below,
// you can specify this info (maximum color depth, whether it's lossy etc.).
//
// Update the below list and if you do change any of that info, bump up
// "kpSettingsGroupMimeTypeProperties" in kpDefs.h.
//
// Currently, Depth and Quality settings are mutually exclusive with
// Depth overriding Quality.  I've currently favoured Quality with the
// below mimetypes (i.e. all lossy mimetypes are only given Quality settings,
// no Depth settings).
//
// Mimetypes done:
//  image/bmp
//  image/jpeg
//  image/jp2
//  image/png
//  image/tiff
//  image/x-eps
//  image/x-pcx
//  image/x-portable-bitmap
//  image/x-portable-graymap
//  image/x-portable-pixmap
//  image/x-rgb
//  image/x-tga
//  image/x-xbitmap
//  image/x-xpixmap
//  video/x-mng [COULD NOT TEST]
//
// To test whether depth is configurable, write an image in the new
// mimetype with all depths and read each one back.  See what
// kpDocument thinks the depth is when it gets QImage to read it.


// public static
int kpDocumentSaveOptions::mimeTypeMaximumColorDepth (const QString &mimeType)
{
    QStringList defaultList;

    // SYNC: update mime info here

    // Grayscale actually (unenforced since depth not set to configurable)
    defaultList << QLatin1String ("image/x-eps:32");

    defaultList << QLatin1String ("image/x-portable-bitmap:1");

    // Grayscale actually (unenforced since depth not set to configurable)
    defaultList << QLatin1String ("image/x-portable-graymap:8");

    defaultList << QLatin1String ("image/x-xbitmap:1");

    const QStringList mimeTypeList = mimeTypesSupportingProperty (
        kpSettingMimeTypeMaximumColorDepth, defaultList);

    const QString mimeTypeColon = mimeType + QLatin1String (":");
    for (QStringList::const_iterator it = mimeTypeList.begin ();
         it != mimeTypeList.end ();
         ++it)
    {
        if ((*it).startsWith (mimeTypeColon))
        {
            int number = (*it).mid (mimeTypeColon.length ()).toInt ();
            if (!colorDepthIsInvalid (number))
            {
                return number;
            }
        }
    }

    return 32;
}

//---------------------------------------------------------------------

// public
int kpDocumentSaveOptions::mimeTypeMaximumColorDepth () const
{
    return mimeTypeMaximumColorDepth (mimeType ());
}

//---------------------------------------------------------------------

// public static
bool kpDocumentSaveOptions::mimeTypeHasConfigurableColorDepth (const QString &mimeType)
{
    QStringList defaultMimeTypes;

    // SYNC: update mime info here
    defaultMimeTypes << QLatin1String ("image/png");
    defaultMimeTypes << QLatin1String ("image/bmp");
    defaultMimeTypes << QLatin1String ("image/x-pcx");

    // TODO: Only 1, 24 not 8; Qt only sees 32 but "file" cmd realises
    //       it's either 1 or 24.
    defaultMimeTypes << QLatin1String ("image/x-rgb");

    // TODO: Only 8 and 24 - no 1.
    defaultMimeTypes << QLatin1String ("image/x-xpixmap");

    return mimeTypeSupportsProperty (mimeType,
        kpSettingMimeTypeHasConfigurableColorDepth,
        defaultMimeTypes);
}

//---------------------------------------------------------------------

// public
bool kpDocumentSaveOptions::mimeTypeHasConfigurableColorDepth () const
{
    return mimeTypeHasConfigurableColorDepth (mimeType ());
}

//---------------------------------------------------------------------

// public static
bool kpDocumentSaveOptions::mimeTypeHasConfigurableQuality (const QString &mimeType)
{
    QStringList defaultMimeTypes;

    // SYNC: update mime info here
    defaultMimeTypes << QLatin1String ("image/jp2");
    defaultMimeTypes << QLatin1String ("image/jpeg");

    return mimeTypeSupportsProperty (mimeType,
        kpSettingMimeTypeHasConfigurableQuality,
        defaultMimeTypes);
}

//---------------------------------------------------------------------

// public
bool kpDocumentSaveOptions::mimeTypeHasConfigurableQuality () const
{
    return mimeTypeHasConfigurableQuality (mimeType ());
}

//---------------------------------------------------------------------

// public
int kpDocumentSaveOptions::isLossyForSaving (const QImage &image) const
{
    int ret = 0;

    if (mimeTypeMaximumColorDepth () < image.depth ())
    {
        ret |= MimeTypeMaximumColorDepthLow;
    }

    if (mimeTypeHasConfigurableColorDepth () &&
        !colorDepthIsInvalid () /*REFACTOR: guarantee it is valid*/ &&
        ((colorDepth () < image.depth ()) ||
         (colorDepth () < 32 && image.hasAlphaChannel())))
    {
        ret |= ColorDepthLow;
    }

    if (mimeTypeHasConfigurableQuality () &&
        !qualityIsInvalid ())
    {
        ret |= Quality;
    }

    return ret;
}

//---------------------------------------------------------------------
