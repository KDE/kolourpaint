
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


#ifndef KP_DOCUMENT_H
#define KP_DOCUMENT_H


#include <QBitmap>
#include <QObject>
#include <QString>
#include <QUrl>

#include "imagelib/kpImage.h"
#include "pixmapfx/kpPixmapFX.h"
#undef environ

class QImage;
class QIODevice;
class QPoint;
class QRect;
class QSize;

class kpColor;
class kpDocumentEnvironment;
class kpDocumentSaveOptions;
class kpDocumentMetaInfo;
class kpAbstractImageSelection;
class kpAbstractSelection;
class kpTextSelection;


// REFACTOR: rearrange method order to make sense and reflect kpDocument_*.cpp split.
class kpDocument : public QObject
{
Q_OBJECT

public:
    // REFACTOR: Hide constructor and have 2 factory methods:
    //
    //           Method 1. Creates a blank document with dimensions <w>x<h>.
    //
    //           Method 2. Calls open().  <w> and <h> (aka constructorWidth()
    //              and constructorHeight()) need not be specified.
    //
    //           ?
    kpDocument (int w, int h, kpDocumentEnvironment *environ);
    ~kpDocument () override;

    kpDocumentEnvironment *environ () const;
    void setEnviron (kpDocumentEnvironment *environ);


    //
    // File I/O - Open
    //


    static QImage getPixmapFromFile (const QUrl &url, bool suppressDoesntExistDialog,
                                     QWidget *parent,
                                     kpDocumentSaveOptions *saveOptions = nullptr,
                                     kpDocumentMetaInfo *metaInfo = nullptr);
    // REFACTOR: fix: open*() should only be called once.
    //                Create a new kpDocument() if you want to open again.
    void openNew (const QUrl &url);
    bool open (const QUrl &url, bool newDocSameNameIfNotExist = false);

    static void getDataFromImage(const QImage &image,
                                 kpDocumentSaveOptions &saveOptions,
                                 kpDocumentMetaInfo &metaInfo);

    //
    // File I/O - Save
    //

    static bool lossyPromptContinue (const QImage &pixmap,
                                     const kpDocumentSaveOptions &saveOptions,
                                     QWidget *parent);
    static bool savePixmapToDevice (const QImage &pixmap,
                                    QIODevice *device,
                                    const kpDocumentSaveOptions &saveOptions,
                                    const kpDocumentMetaInfo &metaInfo,
                                    bool lossyPrompt,
                                    QWidget *parent,
                                    bool *userCancelled = nullptr);
    static bool savePixmapToFile (const QImage &pixmap,
                                  const QUrl &url,
                                  const kpDocumentSaveOptions &saveOptions,
                                  const kpDocumentMetaInfo &metaInfo,
                                  bool lossyPrompt,
                                  QWidget *parent);
    bool save (bool lossyPrompt = false);
    bool saveAs (const QUrl &url,
                 const kpDocumentSaveOptions &saveOptions,
                 bool lossyPrompt = true);


    // Returns whether save() or saveAs() have ever been called and returned true
    bool savedAtLeastOnceBefore () const;

    QUrl url () const;
    void setURL (const QUrl &url, bool isFromExistingURL);

    // Returns whether the document's image was successfully opened from
    // or saved to the URL returned by url().  This is not true for a
    // new kpDocument and in the case of open() being passed
    // "newDocSameNameIfNotExist = true" when the URL doesn't exist.
    //
    // If this returns true and the kpDocument hasn't been modified,
    // this gives a pretty good indication that the image stored at url()
    // is equal to image() (unless the something has happened to that url
    // outside of KolourPaint).
    //
    // e.g. If the user types "kolourpaint doesnotexist.png" to start
    //      KolourPaint, this method will return false.
    bool isFromExistingURL () const;

    // Checks whether @p url still exists
    bool urlExists (const QUrl &url) const;

    // (will convert: empty Url --> "Untitled")
    QString prettyUrl () const;

    // (will convert: empty Url --> "Untitled")
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

    // REFACTOR: Rename to originalWidth()?
    int constructorWidth () const;  // as passed to the constructor
    int width (bool ofSelection = false) const;
    int oldWidth () const;  // only valid in a slot connected to sizeChanged()
    void setWidth (int w, const kpColor &backgroundColor);

    // REFACTOR: Rename to originalHeight()?
    int constructorHeight () const;  // as passed to the constructor
    int height (bool ofSelection = false) const;
    int oldHeight () const;  // only valid in a slot connected to sizeChanged()
    void setHeight (int h, const kpColor &backgroundColor);

    QRect rect (bool ofSelection = false) const;


    //
    // Image access
    //

    // Returns a copy of part of the document's image (not including the
    // selection).
    kpImage getImageAt (const QRect &rect) const;

    void setImageAt (const kpImage &image, const QPoint &at);

    // "image(false)" returns a copy of the document's image, ignoring any
    // floating selection.
    //
    // "image(true)" returns a copy of a floating image selection's base
    // image (i.e. before selection transparency is applied), which may be
    // null if the image selection is a just a border.
    //
    // ASSUMPTION: For <ofSelection> == true only, an image selection exists.
    kpImage image (bool ofSelection = false) const;
    kpImage *imagePointer () const;

    void setImage (const kpImage &image);
    // ASSUMPTION: If setting the selection's image, the selection must be
    //             an image selection.
    void setImage (bool ofSelection, const kpImage &image);


    //
    // Selections
    //

public:
    kpAbstractSelection *selection () const;
    kpAbstractImageSelection *imageSelection () const;
    kpTextSelection *textSelection () const;

    // Sets the document's selection to the given one and changes to the
    // matching selection tool.  Tool changes occur in the following situations:
    //
    // 1. Setting a <selection> when a selection tool is not active.
    //
    // 2. Setting an image <selection> when the text tool is active.
    //    ASSUMPTION: There is no text selection active when calling this
    //                method (push it onto the document before calling this,
    //                to avoid this problem).
    //
    // 3. Setting a text <selection> when an image selection tool is active.
    //    ASSUMPTION: There is no image selection active when calling this
    //                method (push it onto the document before calling this,
    //                to avoid this problem).
    //
    // The justification for the above assumptions are to reduce the complexity
    // of this method's implementation -- changing from an image selection tool
    // to a text selection tool, or vice-versa, calls the end() method of the
    // current tool, which pushes any active selection onto the document.  Since
    // this method sets the selection, losing the old selection in the middle of
    // the method would be tricky to work around.
    //
    // WARNING: Before calling this, you must ensure that the UI (kpMainWindow)
    //          has the <selection>'s selection transparency or
    //          for a text selection, its text style, selected.
    // TODO: Why can't we change it for them, if we change tool automatically for them already?
    void setSelection (const kpAbstractSelection &selection);

    // Returns the base image of the current image selection.  If this is
    // null (because the selection is still a border), it extracts the
    // pixels of the document marked out by the border of the selection.
    //
    // ASSUMPTION: There is an imageSelection().
    //
    // TODO: this always returns base image - need ver that applies selection
    //       transparency.
    kpImage getSelectedBaseImage () const;

    // Sets the base image of the current image selection to the pixels
    // of the document marked out by the border of the selection.
    //
    // ASSUMPTION: There is an imageSelection() that is just a border
    //             (no base image).
    void imageSelectionPullFromDocument (const kpColor &backgroundColor);

    // Deletes the current selection, if there is a selection(), else NOP
    void selectionDelete ();

    // Stamps a copy of the selection onto the document.
    //
    // For image selections, <applySelTransparency> set to true, means that
    // the transparent image of the selection is used.  If set to false,
    // the base image of the selection is used.  This argument is ignored
    // for non-image selections.
    //
    // ASSUMPTION: There is a selection() with content, else NOP
    void selectionCopyOntoDocument (bool applySelTransparency = true);

    // Same as selectionCopyOntoDocument() but deletes the selection
    // afterwards.
    void selectionPushOntoDocument (bool applySelTransparency = true);

    //
    // Same as image() but returns a _copy_ of the document image
    // + any (even non-image) selection pasted on top.
    //
    // Even if the selection has no content, it is still pasted:
    //
    // 1. For an image selection, this makes no difference.
    //
    // 2. For a text selection:
    //
    //    a) with an opaque background: the background rectangle is
    //      included -- this is necessary since the rectangle is visually
    //      there after all, and the intention of this method is to report
    //      everything.
    //
    //    b) with a transparent background: this makes no difference.
    //
    kpImage imageWithSelection () const;


    /*
     * Transformations
     * (convenience only - you could achieve the same effect (and more) with
     *  kpPixmapFX: these functions do not affect the selection)
     */

    void fill (const kpColor &color);
    void resize (int w, int h, const kpColor &backgroundColor);


public slots:
    // these will emit signals!
    void slotContentsChanged (const QRect &rect);
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

    // Emitted when setSelection() is given a selection such that we change
    // from a non-text-selection tool to the text selection tool or vice-versa.
    // <isText> reports whether the new selection is text (and therefore,
    // whether we've switched to the text tool).
    void selectionIsTextChanged (bool isText);

private:
    int m_constructorWidth, m_constructorHeight;
    kpImage *m_image;

    QUrl m_url;
    bool m_isFromExistingURL;
    bool m_savedAtLeastOnceBefore;

    kpDocumentSaveOptions *m_saveOptions;
    kpDocumentMetaInfo *m_metaInfo;

    bool m_modified;

    kpAbstractSelection *m_selection;

    int m_oldWidth, m_oldHeight;

    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    struct kpDocumentPrivate *d;
};


#endif  // KP_DOCUMENT_H
