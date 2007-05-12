
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

#define DEBUG_KP_DOCUMENT_SAVE_OPTIONS 0


#include <kpdocumentsaveoptions.h>

#include <qpixmap.h>
#include <qstring.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include <kpdefs.h>


struct kpDocumentSaveOptionsPrivate
{
    QString m_mimeType;
    int m_colorDepth;
    bool m_dither;
    int m_quality;
};


kpDocumentSaveOptions::kpDocumentSaveOptions ()
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = invalidMimeType ();
    d->m_colorDepth = invalidColorDepth ();
    d->m_dither = initialDither ();
    d->m_quality = invalidQuality ();
}

kpDocumentSaveOptions::kpDocumentSaveOptions (const kpDocumentSaveOptions &rhs)
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = rhs.mimeType ();
    d->m_colorDepth = rhs.colorDepth ();
    d->m_dither = rhs.dither ();
    d->m_quality = rhs.quality ();
}

kpDocumentSaveOptions::kpDocumentSaveOptions (QString mimeType, int colorDepth, bool dither, int quality)
    : d (new kpDocumentSaveOptionsPrivate ())
{
    d->m_mimeType = mimeType;
    d->m_colorDepth = colorDepth;
    d->m_dither = dither;
    d->m_quality = quality;
}

kpDocumentSaveOptions::~kpDocumentSaveOptions ()
{
    delete d;
}


// public
bool kpDocumentSaveOptions::operator== (const kpDocumentSaveOptions &rhs) const
{
    return (mimeType () == rhs.mimeType () &&
            colorDepth () == rhs.colorDepth () &&
            dither () == rhs.dither () &&
            quality () == rhs.quality ());
}

// public
bool kpDocumentSaveOptions::operator!= (const kpDocumentSaveOptions &rhs) const
{
    return !(*this == rhs);
}


// public
kpDocumentSaveOptions &kpDocumentSaveOptions::operator= (const kpDocumentSaveOptions &rhs)
{
    setMimeType (rhs.mimeType ());
    setColorDepth (rhs.colorDepth ());
    setDither (rhs.dither ());
    setQuality (rhs.quality ());

    return *this;
}


// public
void kpDocumentSaveOptions::printDebug (const QString &prefix) const
{
    const QString usedPrefix = !prefix.isEmpty () ?
                                   prefix + QString::fromLatin1 (": ") :
                                   QString::null;

    kdDebug () << usedPrefix
               << "mimeType=" << mimeType ()
               << " colorDepth=" << colorDepth ()
               << " dither=" << dither ()
               << " quality=" << quality ()
               << endl;
}


// public
QString kpDocumentSaveOptions::mimeType () const
{
    return d->m_mimeType;
}

// public
void kpDocumentSaveOptions::setMimeType (const QString &mimeType)
{
    d->m_mimeType = mimeType;
}


// public static
QString kpDocumentSaveOptions::invalidMimeType ()
{
    return QString::null;
}

// public static
bool kpDocumentSaveOptions::mimeTypeIsInvalid (const QString &mimeType)
{
    return (mimeType == invalidMimeType ());
}

// public
bool kpDocumentSaveOptions::mimeTypeIsInvalid () const
{
    return mimeTypeIsInvalid (mimeType ());
}


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
QString kpDocumentSaveOptions::defaultMimeType (KConfigBase *config)
{
    return config->readEntry (kpSettingForcedMimeType,
                              QString::fromLatin1 ("image/png"));
}

// public static
void kpDocumentSaveOptions::saveDefaultMimeType (KConfigBase *config,
                                                 const QString &mimeType)
{
    config->writeEntry (kpSettingForcedMimeType, mimeType);
}


// public static
int kpDocumentSaveOptions::defaultColorDepth (KConfigBase *config)
{
    int colorDepth =
        config->readNumEntry (kpSettingForcedColorDepth, -1);

    if (colorDepthIsInvalid (colorDepth))
    {
        // (not screen depth, in case of transparency)
        colorDepth = 32;
    }

    return colorDepth;
}

// public static
void kpDocumentSaveOptions::saveDefaultColorDepth (KConfigBase *config, int colorDepth)
{
    config->writeEntry (kpSettingForcedColorDepth, colorDepth);
}


// public static
int kpDocumentSaveOptions::defaultDither (KConfigBase *config)
{
    return config->readBoolEntry (kpSettingForcedDither, initialDither ());
}

// public static
void kpDocumentSaveOptions::saveDefaultDither (KConfigBase *config, bool dither)
{
    config->writeEntry (kpSettingForcedDither, dither);
}


// public static
int kpDocumentSaveOptions::defaultQuality (KConfigBase *config)
{
    int val = config->readNumEntry (kpSettingForcedQuality, -1);
    if (qualityIsInvalid (val))
        val = -1;

    return val;
}

// public static
void kpDocumentSaveOptions::saveDefaultQuality (KConfigBase *config, int quality)
{
    config->writeEntry (kpSettingForcedQuality, quality);
}


// public static
kpDocumentSaveOptions kpDocumentSaveOptions::defaultDocumentSaveOptions (KConfigBase *config)
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

// public static
bool kpDocumentSaveOptions::saveDefaultDifferences (KConfigBase *config,
                                                    const kpDocumentSaveOptions &oldDocInfo,
                                                    const kpDocumentSaveOptions &newDocInfo)
{
    bool savedSomething = false;

#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS
    kdDebug () << "kpDocumentSaveOptions::saveDefaultDifferences()" << endl;
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


static QStringList mimeTypesSupportingProperty (const QString &property,
    const QStringList &defaultMimeTypesWithPropertyList)
{
    QStringList mimeTypeList;

    KConfigGroupSaver cfgGroupSaver (KGlobal::config (),
                                     kpSettingsGroupMimeTypeProperties);
    KConfigBase *cfg = cfgGroupSaver.config ();

    if (cfg->hasKey (property))
    {
        mimeTypeList = cfg->readListEntry (property);
    }
    else
    {
        mimeTypeList = defaultMimeTypesWithPropertyList;

        cfg->writeEntry (property, mimeTypeList);
        cfg->sync ();
    }

    return mimeTypeList;
}

static bool mimeTypeSupportsProperty (const QString &mimeType,
    const QString &property, const QStringList &defaultMimeTypesWithPropertyList)
{
    const QStringList mimeTypeList = mimeTypesSupportingProperty (
        property, defaultMimeTypesWithPropertyList);

    return mimeTypeList.contains (mimeType);
}


// SYNC: update mime info
//
// Only care about writable mimetypes.
//
// Run "branches/kolourpaint/control/scripts/gen_mimetype_line.sh Write" in
// the version of kdelibs/kimgio/ (e.g. KDE 3.5) KolourPaint is shipped with,
// to check for any new mimetypes to add info for.  In the methods below,
// you can specify this info (maximum color depth, whether it's lossy etc.).
//
// Update the below list also and bump up "kpSettingsGroupMimeTypeProperties"
// in kpdefs.h.
//
// Currently, Depth and Quality settings are mutually exclusive with
// Depth overriding Quality.  I've currently favoured Quality with the
// below mimetypes (i.e. all lossy mimetypes are only given Quality settings,
// no Depth settings).
//
// Mimetypes done:
//  image/jp2 [UNTESTED]
//  image/jpeg
//  image/png
//  image/x-bmp
//  image/x-eps
//  image/x-pcx
//  image/x-portable-bitmap
//  image/x-portable-greymap
//  image/x-portable-pixmap
//  image/x-rgb
//  image/x-targa
//  image/x-xbm
//  image/x-xpm
//
// To test whether depth is configurable, write an image in the new
// mimetype with all depths and read each one back.  See what
// kpDocument thinks the depth is when it gets QImage to read it.


// public static
int kpDocumentSaveOptions::mimeTypeMaximumColorDepth (const QString &mimeType)
{
    QStringList defaultList;

    // SYNC: update mime info here
    
    // Greyscale actually (unenforced since depth not set to configurable)
    defaultList << QString::fromLatin1 ("image/x-eps:32");
    
    defaultList << QString::fromLatin1 ("image/x-portable-bitmap:1");
    
    // Greyscale actually (unenforced since depth not set to configurable)
    defaultList << QString::fromLatin1 ("image/x-portable-greymap:8");
    
    defaultList << QString::fromLatin1 ("image/x-xbm:1");

    const QStringList mimeTypeList = mimeTypesSupportingProperty (
        kpSettingMimeTypeMaximumColorDepth, defaultList);

    const QString mimeTypeColon = mimeType + QString::fromLatin1 (":");
    for (QStringList::const_iterator it = mimeTypeList.begin ();
         it != mimeTypeList.end ();
         it++)
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

// public
int kpDocumentSaveOptions::mimeTypeMaximumColorDepth () const
{
    return mimeTypeMaximumColorDepth (mimeType ());
}


// public static
bool kpDocumentSaveOptions::mimeTypeHasConfigurableColorDepth (const QString &mimeType)
{
    QStringList defaultMimeTypes;

    // SYNC: update mime info here
    defaultMimeTypes << QString::fromLatin1 ("image/png");
    defaultMimeTypes << QString::fromLatin1 ("image/x-bmp");
    defaultMimeTypes << QString::fromLatin1 ("image/x-pcx");
    
    // TODO: Only 1, 24 not 8; Qt only sees 32 but "file" cmd realises
    //       it's either 1 or 24.
    defaultMimeTypes << QString::fromLatin1 ("image/x-rgb");
  
    // TODO: Only 8 and 24 - no 1.
    defaultMimeTypes << QString::fromLatin1 ("image/x-xpm");

    return mimeTypeSupportsProperty (mimeType,
        kpSettingMimeTypeHasConfigurableColorDepth,
        defaultMimeTypes);
}

// public
bool kpDocumentSaveOptions::mimeTypeHasConfigurableColorDepth () const
{
    return mimeTypeHasConfigurableColorDepth (mimeType ());
}


// public static
bool kpDocumentSaveOptions::mimeTypeHasConfigurableQuality (const QString &mimeType)
{
    QStringList defaultMimeTypes;

    // SYNC: update mime info here
    defaultMimeTypes << QString::fromLatin1 ("image/jp2");
    defaultMimeTypes << QString::fromLatin1 ("image/jpeg");

    return mimeTypeSupportsProperty (mimeType,
        kpSettingMimeTypeHasConfigurableQuality,
        defaultMimeTypes);
}

// public
bool kpDocumentSaveOptions::mimeTypeHasConfigurableQuality () const
{
    return mimeTypeHasConfigurableQuality (mimeType ());
}


// public
int kpDocumentSaveOptions::isLossyForSaving (const QPixmap &pixmap) const
{
    int ret = 0;

    if (mimeTypeMaximumColorDepth () < pixmap.depth ())
    {
        ret |= MimeTypeMaximumColorDepthLow;
    }

    if (mimeTypeHasConfigurableColorDepth () &&
        !colorDepthIsInvalid () /*TODO: prevent*/ &&
        (colorDepth () < pixmap.depth () ||
         colorDepth () < 32 && pixmap.mask ()))
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

