
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


#ifndef KP_DOCUMENT_SAVE_OPTIONS_H
#define KP_DOCUMENT_SAVE_OPTIONS_H


class QPixmap;
class QString;

class KConfigBase;


class kpDocumentSaveOptions
{
public:
    kpDocumentSaveOptions ();
    kpDocumentSaveOptions (const kpDocumentSaveOptions &rhs);
    kpDocumentSaveOptions (QString mimeType, int colorDepth, bool dither, int quality);
    virtual ~kpDocumentSaveOptions ();

    bool operator== (const kpDocumentSaveOptions &rhs) const;
    bool operator!= (const kpDocumentSaveOptions &rhs) const;

    kpDocumentSaveOptions &operator= (const kpDocumentSaveOptions &rhs);


    void printDebug (const QString &prefix) const;


    QString mimeType () const;
    void setMimeType (const QString &mimeType);

    static QString invalidMimeType ();
    static bool mimeTypeIsInvalid (const QString &mimeType);
    bool mimeTypeIsInvalid () const;


    int colorDepth () const;
    void setColorDepth (int depth);

    static int invalidColorDepth ();
    static bool colorDepthIsInvalid (int colorDepth);
    bool colorDepthIsInvalid () const;


    bool dither () const;
    void setDither (bool dither);

    static int initialDither ();


    int quality () const;
    void setQuality (int quality);

    static int invalidQuality ();
    static bool qualityIsInvalid (int quality);
    bool qualityIsInvalid () const;


    // (All assume that <config>'s group has been set)
    // (None of them call KConfigBase::reparseConfig() nor KConfigBase::sync())

    static QString defaultMimeType (KConfigBase *config);
    static void saveDefaultMimeType (KConfigBase *config, const QString &mimeType);

    static int defaultColorDepth (KConfigBase *config);
    static void saveDefaultColorDepth (KConfigBase *config, int colorDepth);

    static int defaultDither (KConfigBase *config);
    static void saveDefaultDither (KConfigBase *config, bool dither);

    static int defaultQuality (KConfigBase *config);
    static void saveDefaultQuality (KConfigBase *config, int quality);


    static kpDocumentSaveOptions defaultDocumentSaveOptions (KConfigBase *config);
    // (returns true if it encountered a difference (and saved it to <config>))
    static bool saveDefaultDifferences (KConfigBase *config,
                                        const kpDocumentSaveOptions &oldDocInfo,
                                        const kpDocumentSaveOptions &newDocInfo);


public:
    // (purely for informational purposes - not enforced by this class)
    static int mimeTypeMaximumColorDepth (const QString &mimeType);
    int mimeTypeMaximumColorDepth () const;


    static bool mimeTypeHasConfigurableColorDepth (const QString &mimeType);
    bool mimeTypeHasConfigurableColorDepth () const;

    static bool mimeTypeHasConfigurableQuality (const QString &mimeType);
    bool mimeTypeHasConfigurableQuality () const;


    // TODO: checking for mask loss due to format e.g. BMP
    enum LossyType
    {
        LossLess = 0,

        // mimeTypeMaximumColorDepth() < <pixmap>.depth()
        MimeTypeMaximumColorDepthLow = 1,
        // i.e. colorDepth() < <pixmap>.depth() ||
        //      colorDepth() < 32 && <pixmap>.mask()
        ColorDepthLow = 2,
        // i.e. mimeTypeHasConfigurableQuality()
        Quality = 4
    };

    // Returns whether saving <pixmap> with these options will result in
    // loss of information.  Returned value is the bitwise OR of
    // LossType enum possiblities.
    int isLossyForSaving (const QPixmap &pixmap) const;


private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpDocumentSaveOptionsPrivate *d;
};


#endif  // KP_DOCUMENT_SAVE_OPTIONS_H
