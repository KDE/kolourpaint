
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


#define DEBUG_KP_DOCUMENT 0


#include "kpDocument.h"
#include "kpDocumentPrivate.h"


#include <QImage>
#include <QPainter>
#include <QRect>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "imagelib/kpColor.h"
#include "kpDefs.h"
#include "environments/document/kpDocumentEnvironment.h"
#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/text/kpTextSelection.h"


// public
kpAbstractSelection *kpDocument::selection () const
{
    return m_selection;
}

//---------------------------------------------------------------------

// public
kpAbstractImageSelection *kpDocument::imageSelection () const
{
    return dynamic_cast <kpAbstractImageSelection *> (m_selection);
}

//---------------------------------------------------------------------

// public
kpTextSelection *kpDocument::textSelection () const
{
    return dynamic_cast <kpTextSelection *> (m_selection);
}

//---------------------------------------------------------------------

// public
void kpDocument::setSelection (const kpAbstractSelection &selection)
{
#if DEBUG_KP_DOCUMENT && 1
    qCDebug(kpLogDocument) << "kpDocument::setSelection() sel boundingRect="
               << selection.boundingRect ();
#endif

    d->environ->setQueueViewUpdates ();
    {
        const bool hadSelection = static_cast<bool> (m_selection);
        auto *oldSelection = m_selection;


        // (must be called before giving the document a new selection, to
        //  avoid a potential mess where switchToCompatibleTool() ends
        //  the current selection tool, killing the new selection)
        bool isTextChanged = false;
        d->environ->switchToCompatibleTool (selection, &isTextChanged);
        Q_ASSERT (m_selection == oldSelection);


        m_selection = selection.clone ();

        // There's no need to uninitialize the old selection
        // (e.g. call disconnect()) since we delete it later.
        connect (m_selection, &kpAbstractSelection::changed,
                 this, &kpDocument::slotContentsChanged);


        //
        // Now all kpDocument state has been set.
        // We can _only_ change the environment after that, as the environment
        // may access the document.  Exception is above with
        // switchToCompatibleTool().
        //

        d->environ->assertMatchingUIState (selection);


        //
        // Now all kpDocument and environment state has been set.
        // We can _only_ fire signals after that, as the signal receivers (the
        // "wider environment") may access the document and the environment.
        //

    #if DEBUG_KP_DOCUMENT && 1
        qCDebug(kpLogDocument) << "\tcheck sel " << (int *) m_selection
                   << " boundingRect=" << m_selection->boundingRect ();
    #endif
        if (oldSelection)
        {
            if (oldSelection->hasContent ()) {
                slotContentsChanged (oldSelection->boundingRect ());
            }
            else {
                emit contentsChanged (oldSelection->boundingRect ());
            }

            delete oldSelection;
            oldSelection = nullptr;
        }

        if (m_selection->hasContent ()) {
            slotContentsChanged (m_selection->boundingRect ());
        }
        else {
            emit contentsChanged (m_selection->boundingRect ());
        }


        if (!hadSelection) {
            emit selectionEnabled (true);
        }

        if (isTextChanged) {
            emit selectionIsTextChanged (textSelection ());
        }
    }
    d->environ->restoreQueueViewUpdates ();

#if DEBUG_KP_DOCUMENT && 1
    qCDebug(kpLogDocument) << "\tkpDocument::setSelection() ended";
#endif
}

//---------------------------------------------------------------------

// public
kpImage kpDocument::getSelectedBaseImage () const
{
    auto *imageSel = imageSelection ();
    Q_ASSERT (imageSel);

    // Easy if we already have it :)
    const auto image = imageSel->baseImage ();
    if (!image.isNull ()) {
        return image;
    }


    const auto boundingRect = imageSel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    // OPT: This is very slow.  Image / More Effects ... calls us twice
    //      unnecessarily.
    return imageSel->givenImageMaskedByShape (getImageAt (boundingRect));
}

//---------------------------------------------------------------------

// public
void kpDocument::imageSelectionPullFromDocument (const kpColor &backgroundColor)
{
    auto *imageSel = imageSelection ();
    Q_ASSERT (imageSel);

    // Should not already have an image or we would not be pulling.
    Q_ASSERT (!imageSel->hasContent ());

    const auto boundingRect = imageSel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    //
    // Get selection image from document
    //

    auto selectedImage = getSelectedBaseImage ();

    d->environ->setQueueViewUpdates ();

    imageSel->setBaseImage (selectedImage);

    //
    // Fill opaque bits of the hole in the document
    //

#if !defined (QT_NO_DEBUG) && !defined (NDEBUG)
    if (imageSel->transparency ().isTransparent ())
    {
        Q_ASSERT (backgroundColor == imageSel->transparency ().transparentColor ());
    }
    else
    {
        // If this method is begin called by a tool, the assert does not
        // have to hold since transparentColor() might not be defined in Opaque
        // Mode.
        //
        // If we were called by a tricky sequence of undo/redo commands, the assert
        // does not have to hold for additional reason, which is that
        // kpMainWindow::setImageSelectionTransparency() does not have to
        // set <backgroundColor> in Opaque Mode.
        //
        // In practice, it probably does hold but I wouldn't bet on it.
    }
#endif

    kpImage eraseImage(boundingRect.size(), QImage::Format_ARGB32_Premultiplied);
    eraseImage.fill(backgroundColor.toQRgb());

    // only paint the region of the shape of the selection
    QPainter painter(m_image);
    painter.setClipRegion(imageSel->shapeRegion());
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(boundingRect.topLeft(), eraseImage);
    slotContentsChanged(boundingRect);

    d->environ->restoreQueueViewUpdates ();
}

//---------------------------------------------------------------------

// public
void kpDocument::selectionDelete ()
{
    if ( !m_selection ) {
        return;
    }

    const auto boundingRect = m_selection->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    const auto selectionHadContent = m_selection->hasContent ();

    delete m_selection;
    m_selection = nullptr;


    // HACK to prevent document from being modified when
    //      user cancels dragging out a new selection
    // REFACTOR: Extract this out into a method.
    if (selectionHadContent) {
        slotContentsChanged (boundingRect);
    }
    else {
        emit contentsChanged (boundingRect);
    }

    emit selectionEnabled (false);
}

//---------------------------------------------------------------------

// public
void kpDocument::selectionCopyOntoDocument (bool applySelTransparency)
{
    // Empty selection, just doing nothing
    if ( !m_selection || !m_selection->hasContent() ) {
        return;
    }

    const QRect boundingRect = m_selection->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    if (imageSelection ())
    {
        if (applySelTransparency) {
            imageSelection ()->paint (m_image, rect ());
        }
        else {
            imageSelection ()->paintWithBaseImage (m_image, rect ());
        }
    }
    else
    {
        // (for antialiasing with background)
        m_selection->paint (m_image, rect ());
    }

    slotContentsChanged (boundingRect);
}

//---------------------------------------------------------------------

// public
void kpDocument::selectionPushOntoDocument (bool applySelTransparency)
{
    selectionCopyOntoDocument (applySelTransparency);
    selectionDelete ();
}

//---------------------------------------------------------------------

// public
kpImage kpDocument::imageWithSelection () const
{
#if DEBUG_KP_DOCUMENT && 1
    qCDebug(kpLogDocument) << "kpDocument::imageWithSelection()";
#endif

    // Have selection?
    //
    // It need not have any content because e.g. a text box with an opaque
    // background, but no content, is still visually there.
    if (m_selection)
    {
    #if DEBUG_KP_DOCUMENT && 1
        qCDebug(kpLogDocument) << "\tselection @ " << m_selection->boundingRect ();
    #endif
        kpImage output = *m_image;

        // (this is a NOP for image selections without content)
        m_selection->paint (&output, rect ());

        return output;
    }
    else
    {
    #if DEBUG_KP_DOCUMENT && 1
        qCDebug(kpLogDocument) << "\tno selection";
    #endif
        return *m_image;
    }
}

//---------------------------------------------------------------------
