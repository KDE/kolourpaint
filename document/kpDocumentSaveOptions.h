
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_DOCUMENT_SAVE_OPTIONS_H
#define KP_DOCUMENT_SAVE_OPTIONS_H

class QImage;
class QString;

class KConfigGroup;

class kpDocumentSaveOptions
{
public:
    kpDocumentSaveOptions();
    kpDocumentSaveOptions(const kpDocumentSaveOptions &rhs);
    kpDocumentSaveOptions(const QString &mimeType, int colorDepth, bool dither, int quality);
    virtual ~kpDocumentSaveOptions();

    bool operator==(const kpDocumentSaveOptions &rhs) const;
    bool operator!=(const kpDocumentSaveOptions &rhs) const;

    kpDocumentSaveOptions &operator=(const kpDocumentSaveOptions &rhs);

    void printDebug(const QString &prefix) const;

    QString mimeType() const;
    void setMimeType(const QString &mimeType);

    static QString invalidMimeType();
    static bool mimeTypeIsInvalid(const QString &mimeType);
    bool mimeTypeIsInvalid() const;

    int colorDepth() const;
    void setColorDepth(int depth);

    static int invalidColorDepth();
    static bool colorDepthIsInvalid(int colorDepth);
    bool colorDepthIsInvalid() const;

    bool dither() const;
    void setDither(bool dither);

    static int initialDither();

    int quality() const;
    void setQuality(int quality);

    static int invalidQuality();
    static bool qualityIsInvalid(int quality);
    bool qualityIsInvalid() const;

    // (All assume that <config>'s group has been set)
    // (None of them call KConfigBase::reparseConfig() nor KConfigBase::sync())

    static QString defaultMimeType(const KConfigGroup &config);
    static void saveDefaultMimeType(KConfigGroup &config, const QString &mimeType);

    static int defaultColorDepth(const KConfigGroup &config);
    static void saveDefaultColorDepth(KConfigGroup &config, int colorDepth);

    static int defaultDither(const KConfigGroup &config);
    static void saveDefaultDither(KConfigGroup &config, bool dither);

    static int defaultQuality(const KConfigGroup &config);
    static void saveDefaultQuality(KConfigGroup &config, int quality);

    static kpDocumentSaveOptions defaultDocumentSaveOptions(const KConfigGroup &config);
    // (returns true if it encountered a difference (and saved it to <config>))
    static bool saveDefaultDifferences(KConfigGroup &config, const kpDocumentSaveOptions &oldDocInfo, const kpDocumentSaveOptions &newDocInfo);

public:
    // (purely for informational purposes - not enforced by this class)
    static int mimeTypeMaximumColorDepth(const QString &mimeType);
    int mimeTypeMaximumColorDepth() const;

    static bool mimeTypeHasConfigurableColorDepth(const QString &mimeType);
    bool mimeTypeHasConfigurableColorDepth() const;

    static bool mimeTypeHasConfigurableQuality(const QString &mimeType);
    bool mimeTypeHasConfigurableQuality() const;

    // TODO: checking for mask loss due to format e.g. BMP
    enum LossyType {
        LossLess = 0,

        // mimeTypeMaximumColorDepth() < <pixmap>.depth()
        MimeTypeMaximumColorDepthLow = 1,
        // i.e. colorDepth() < <pixmap>.depth() ||
        //      colorDepth() < 32 && <pixmap>.mask()
        ColorDepthLow = 2,
        // i.e. mimeTypeHasConfigurableQuality()
        Quality = 4
    };

    // Returns whether saving <image> with these options will result in
    // loss of information.  Returned value is the bitwise OR of
    // LossType enum possiblities.
    int isLossyForSaving(const QImage &image) const;

private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpDocumentSaveOptionsPrivate *d;
};

#endif // KP_DOCUMENT_SAVE_OPTIONS_H
