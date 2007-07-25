
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


#define DEBUG_KP_DOCUMENT 1


#include <kpDocument.h>
#include <kpDocumentPrivate.h>

#include <math.h>

#include <qcolor.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qfile.h>
#include <qimage.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qsize.h>
#include <qmatrix.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>  // LOTODO: isn't this in KIO?
#include <ktemporaryfile.h>

#include <kpColor.h>
#include <kpColorToolBar.h>
#include <kpDefs.h>
#include <kpDocumentEnvironment.h>
#include <kpDocumentSaveOptions.h>
#include <kpDocumentMetaInfo.h>
#include <kpEffectReduceColors.h>
#include <kpPixmapFX.h>
#include <kpAbstractSelection.h>
#include <kpAbstractImageSelection.h>
#include <kpTextSelection.h>
#include <kpTool.h>
#include <kpToolToolBar.h>


// public
kpAbstractSelection *kpDocument::selection () const
{
    return m_selection;
}

// public
kpAbstractImageSelection *kpDocument::imageSelection () const
{
    return dynamic_cast <kpAbstractImageSelection *> (m_selection);
}

// public
kpTextSelection *kpDocument::textSelection () const
{
    return dynamic_cast <kpTextSelection *> (m_selection);
}


// public
void kpDocument::setSelection (const kpAbstractSelection &selection)
{
#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "kpDocument::setSelection() sel boundingRect="
               << selection.boundingRect ()
               << endl;
#endif

    d->environ->setQueueViewUpdates ();


    bool hadSelection = (bool) m_selection;


    bool isTextChanged = false;
    d->environ->switchToCompatibleTool (selection, &isTextChanged);


    if (m_selection)
    {
        if (m_selection->hasContent ())
            slotContentsChanged (m_selection->boundingRect ());
        else
            emit contentsChanged (m_selection->boundingRect ());

        delete m_selection;
    }

    m_selection = selection.clone ();

    d->environ->assertMatchingUIState (selection);

#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "\tcheck sel " << (int *) m_selection
               << " boundingRect=" << m_selection->boundingRect ()
               << endl;
#endif
    if (m_selection->hasContent ())
        slotContentsChanged (m_selection->boundingRect ());
    else
        emit contentsChanged (m_selection->boundingRect ());


    // There's no need to uninitialize the old selection
    // (e.g. call disconnect()) since we:
    //
    // 1. Initialize our _copy_ of the given selection.
    // 2. We delete our copy when setSelection() is called again.
    //
    // See code above for both.

    connect (m_selection, SIGNAL (changed (const QRect &)),
             this, SLOT (slotContentsChanged (const QRect &)));


    if (!hadSelection)
        emit selectionEnabled (true);

    if (isTextChanged)
        emit selectionIsTextChanged (dynamic_cast <kpTextSelection *> (m_selection));

    d->environ->restoreQueueViewUpdates ();
#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "\tkpDocument::setSelection() ended" << endl;
#endif
}

// public
kpImage kpDocument::getSelectedBaseImage () const
{
    kpAbstractImageSelection *imageSel = imageSelection ();
    Q_ASSERT (imageSel);

    // Easy if we already have it :)
    const kpImage image = imageSel->baseImage ();
    if (!image.isNull ())
        return image;


    const QRect boundingRect = imageSel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    // TODO: This is very slow.  Image / More Effects ... calls us twice
    //       unnecessarily.
    return imageSel->givenImageMaskedByShape (getImageAt (boundingRect));
}

// public
void kpDocument::imageSelectionPullFromDocument (const kpColor &backgroundColor)
{
    kpAbstractImageSelection *imageSel = imageSelection ();
    Q_ASSERT (imageSel);

    // Should not already have an image or we would not be pulling.
    Q_ASSERT (!imageSel->hasContent ());

    const QRect boundingRect = imageSel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());


    //
    // Get selection pixmap from document
    //

    kpImage selectedImage = getSelectedBaseImage ();

    d->environ->setQueueViewUpdates ();

    imageSel->setBaseImage (selectedImage);


    //
    // Fill opaque bits of the hole in the document
    //

    // TODO: this assumes backgroundColor == sel->transparency ().transparentColor()
    const kpImage selTransparentImage = imageSel->transparentImage ();

    if (backgroundColor.isOpaque ())
    {
        kpImage eraseImage (boundingRect.width (), boundingRect.height ());
        eraseImage.fill (backgroundColor.toQColor ());

        if (!selTransparentImage.mask ().isNull ())
            eraseImage.setMask (selTransparentImage.mask ());

        paintImageAt (eraseImage, boundingRect.topLeft ());
    }
    else
    {
        kpPixmapFX::paintMaskTransparentWithBrush (m_image,
                                                   boundingRect.topLeft (),
                                                   kpPixmapFX::getNonNullMask (selTransparentImage));
        slotContentsChanged (boundingRect);
    }

    d->environ->restoreQueueViewUpdates ();
}

// public
void kpDocument::selectionDelete ()
{
    Q_ASSERT (m_selection);

    const QRect boundingRect = m_selection->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    const bool selectionHadContent = m_selection->hasContent ();

    delete m_selection;
    m_selection = 0;


    // HACK to prevent document from being modified when
    //      user cancels dragging out a new selection
    // REFACTOR: Extract this out into a method.
    if (selectionHadContent)
        slotContentsChanged (boundingRect);
    else
        emit contentsChanged (boundingRect);

    emit selectionEnabled (false);
}

// public
void kpDocument::selectionCopyOntoDocument (bool applySelTransparency)
{
    Q_ASSERT (m_selection);
    Q_ASSERT (m_selection->hasContent ());

    const QRect boundingRect = m_selection->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    // TODO: We should be setting the modified state.
    //       I think this is a bug in KolourPaint/KDE 3 as well.
    if (imageSelection ())
    {
        if (applySelTransparency)
            imageSelection ()->paint (m_image, rect ());
        else
            imageSelection ()->paintWithBaseImage (m_image, rect ());
    }
    else
    {
        // (for antialiasing with background)
        m_selection->paint (m_image, rect ());
    }
}

// public
void kpDocument::selectionPushOntoDocument (bool applySelTransparency)
{
    selectionCopyOntoDocument (applySelTransparency);
    selectionDelete ();
}

// public
kpImage kpDocument::imageWithSelection () const
{
#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "kpDocument::imageWithSelection()" << endl;
#endif

    // Have floating selection?
    if (m_selection && m_selection->hasContent ())
    {
    #if DEBUG_KP_DOCUMENT && 1
        kDebug () << "\tselection @ " << m_selection->boundingRect () << endl;
    #endif
        kpImage output = *m_image;

        // TODO: In KolourPaint/KDE3, we weren't antialiasing text boxes
        m_selection->paint (&output, rect ());

        return output;
    }
    else
    {
    #if DEBUG_KP_DOCUMENT && 1
        kDebug () << "\tno selection" << endl;
    #endif
        return *m_image;
    }
}