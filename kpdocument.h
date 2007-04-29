
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


#ifndef KP_DOCUMENT_H
#define KP_DOCUMENT_H

#include <qbitmap.h>
#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

#include <kppixmapfx.h>


class QImage;
class QIODevice;
class QPixmap;
class QPoint;
class QRect;
class QSize;

class kpColor;
class kpDocumentSaveOptions;
class kpDocumentMetaInfo;
class kpMainWindow;
class kpSelection;


class kpDocument : public QObject
{
Q_OBJECT

public:
    kpDocument (int w, int h, kpMainWindow *mainWindow);
    ~kpDocument ();

    kpMainWindow *mainWindow () const;
    void setMainWindow (kpMainWindow *mainWindow);


    /*
     * File I/O
     */

    // Wraps kpPixmapFX::convertToPixmapAsLosslessAsPossible() but also
    // returns document meta information.
    static QPixmap convertToPixmapAsLosslessAsPossible (
        const QImage &image,
        const kpPixmapFX::WarnAboutLossInfo &wali = kpPixmapFX::WarnAboutLossInfo (),
        kpDocumentSaveOptions *saveOptions = 0,
        kpDocumentMetaInfo *metaInfo = 0);

    static QPixmap getPixmapFromFile (const KURL &url, bool suppressDoesntExistDialog,
                                      QWidget *parent,
                                      kpDocumentSaveOptions *saveOptions = 0,
                                      kpDocumentMetaInfo *metaInfo = 0);
    // TODO: fix: open*() should only be called once.
    //            Create a new kpDocument() if you want to open again.
    void openNew (const KURL &url);
    bool open (const KURL &url, bool newDocSameNameIfNotExist = false);

    static bool lossyPromptContinue (const QPixmap &pixmap,
                                     const kpDocumentSaveOptions &saveOptions,
                                     QWidget *parent);
    static bool savePixmapToDevice (const QPixmap &pixmap,
                                    QIODevice *device,
                                    const kpDocumentSaveOptions &saveOptions,
                                    const kpDocumentMetaInfo &metaInfo,
                                    bool lossyPrompt,
                                    QWidget *parent,
                                    bool *userCancelled = 0);
    static bool savePixmapToFile (const QPixmap &pixmap,
                                  const KURL &url,
                                  const kpDocumentSaveOptions &saveOptions,
                                  const kpDocumentMetaInfo &metaInfo,
                                  bool overwritePrompt,
                                  bool lossyPrompt,
                                  QWidget *parent);
    bool save (bool overwritePrompt = false, bool lossyPrompt = false);
    bool saveAs (const KURL &url,
                 const kpDocumentSaveOptions &saveOptions,
                 bool overwritePrompt = true,
                 bool lossyPrompt = true);

    // Returns whether save() or saveAs() have ever been called and returned true
    bool savedAtLeastOnceBefore () const;

    KURL url () const;
    void setURL (const KURL &url, bool isFromURL);

    // Returns whether the document's pixmap was successfully opened from
    // or saved to the URL returned by url().  This is not true for a
    // new kpDocument and in the case of open() being passed
    // "newDocSameNameIfNotExist = true" when the URL doesn't exist.
    //
    // If this returns true and the kpDocument hasn't been modified,
    // this gives a pretty good indication that the pixmap stored at url()
    // is equal to pixmap() (unless the something has happened to that url
    // outside of KolourPaint).
    bool isFromURL (bool checkURLStillExists = true) const;

    // (will convert: empty URL --> "Untitled")
    static QString prettyURLForURL (const KURL &url);
    QString prettyURL () const;

    // (will convert: empty URL --> "Untitled")
    static QString prettyFilenameForURL (const KURL &url);
    QString prettyFilename () const;

    // (guaranteed to return valid pointer)

    const kpDocumentSaveOptions *saveOptions () const;
    void setSaveOptions (const kpDocumentSaveOptions &saveOptions);

    const kpDocumentMetaInfo *metaInfo () const;
    void setMetaInfo (const kpDocumentMetaInfo &metaInfo);


    /*
     * Properties (modified, width, height, color depth...)
     */

    void setModified (bool yes = true);
    bool isModified () const;
    bool isEmpty () const;

    int constructorWidth () const;  // as passed to the constructor
    int width (bool ofSelection = false) const;
    int oldWidth () const;  // only valid in a slot connected to sizeChanged()
    void setWidth (int w, const kpColor &backgroundColor);

    int constructorHeight () const;  // as passed to the constructor
    int height (bool ofSelection = false) const;
    int oldHeight () const;  // only valid in a slot connected to sizeChanged()
    void setHeight (int h, const kpColor &backgroundColor);

    QRect rect (bool ofSelection = false) const;


    /*
     * Pixmap access
     */

    // get a copy of a bit of the doc's pixmap
    // (not including the selection)
    QPixmap getPixmapAt (const QRect &rect) const;

    void setPixmapAt (const QPixmap &pixmap, const QPoint &at);

    void paintPixmapAt (const QPixmap &pixmap, const QPoint &at);

    // (not including the selection)
    QPixmap *pixmap (bool ofSelection = false) const;
    void setPixmap (const QPixmap &pixmap);
    void setPixmap (bool ofSelection, const QPixmap &pixmap);

private:
    void updateToolsSingleKeyTriggersEnabled ();

public:
    kpSelection *selection () const;
    void setSelection (const kpSelection &selection);

    // TODO: this always returns opaque pixmap - need transparent ver
    QPixmap getSelectedPixmap (const QBitmap &maskBitmap = QBitmap ()) const;

    bool selectionPullFromDocument (const kpColor &backgroundColor);
    bool selectionDelete ();
    bool selectionCopyOntoDocument (bool useTransparentPixmap = true);
    bool selectionPushOntoDocument (bool useTransparentPixmap = true);

    // same as pixmap() but returns a _copy_ of the current pixmap
    // + any selection pasted on top
    QPixmap pixmapWithSelection () const;


    /*
     * Transformations
     * (convenience only - you could achieve the same effect (and more) with
     *  kpPixmapFX: these functions do not affect the selection)
     */

    void fill (const kpColor &color);
    void resize (int w, int h, const kpColor &backgroundColor, bool fillNewAreas = true);


public slots:
    // these will emit signals!
    void slotContentsChanged (const QRect &rect);
    void slotSizeChanged (int newWidth, int newHeight);
    void slotSizeChanged (const QSize &newSize);

signals:
    void documentOpened ();
    void documentSaved ();

    // Emitted whenever the isModified() flag changes from false to true.
    // This is the _only_ signal that may be emitted in addition to the others.
    void documentModified ();

    void contentsChanged (const QRect &rect);
    void sizeChanged (int newWidth, int newHeight);  // see oldWidth(), oldHeight()
    void sizeChanged (const QSize &newSize);

    void selectionEnabled (bool on);

    // HACK: until we support Text Selection -> Rectangular Selection for Image ops
    void selectionIsTextChanged (bool isText);

private:
    int m_constructorWidth, m_constructorHeight;
    kpMainWindow *m_mainWindow;
    QPixmap *m_pixmap;

    KURL m_url;
    bool m_isFromURL;
    bool m_savedAtLeastOnceBefore;

    kpDocumentSaveOptions *m_saveOptions;
    kpDocumentMetaInfo *m_metaInfo;

    bool m_modified;

    kpSelection *m_selection;

    int m_oldWidth, m_oldHeight;

    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpDocumentPrivate *d;
};

#endif  // KP_DOCUMENT_H
