
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
#include <kmimetype.h>  // TODO: isn't this in KIO?
#include <ktemporaryfile.h>

#include <kpColor.h>
#include <kpColorToolBar.h>
#include <kpDefs.h>
#include <kpDocumentSaveOptions.h>
#include <kpDocumentMetaInfo.h>
#include <kpEffectReduceColors.h>
#include <kpMainWindow.h>
#include <kpPixmapFX.h>
#include <kpSelection.h>
#include <kpTool.h>
#include <kpToolToolBar.h>
#include <kpViewManager.h>


// public
kpSelection *kpDocument::selection () const
{
    return m_selection;
}

// public
void kpDocument::setSelection (const kpSelection &selection)
{
#if DEBUG_KP_DOCUMENT && 0
    kDebug () << "kpDocument::setSelection() sel boundingRect="
               << selection.boundingRect ()
               << endl;
#endif

    Q_ASSERT (m_mainWindow);

    kpViewManager *vm = m_mainWindow->viewManager ();
    Q_ASSERT (vm);

    vm->setQueueUpdates ();


    bool hadSelection = (bool) m_selection;


    const bool isTextChanged = (m_mainWindow->toolIsTextTool () !=
                                (selection.type () == kpSelection::Text));

    // (we don't change the Selection Tool if the new selection's
    //  shape is different to the tool's because all the Selection
    //  Tools act the same, except for what would be really irritating
    //  if it kept changing whenever you paste an image - drawing the
    //  selection region)
    if (!m_mainWindow->toolIsASelectionTool () || isTextChanged)
    {
        // Switch to the appropriately shaped selection tool
        // _before_ we change the selection
        // (all selection tool's ::end() functions nuke the current selection)
        switch (selection.type ())
        {
        case kpSelection::Rectangle:
            m_mainWindow->slotToolRectSelection ();
            break;
        case kpSelection::Ellipse:
            m_mainWindow->slotToolEllipticalSelection ();
            break;
        case kpSelection::Points:
            m_mainWindow->slotToolFreeFormSelection ();
            break;
        case kpSelection::Text:
            m_mainWindow->slotToolText ();
            break;
        default:
            break;
        }
    }


    if (m_selection)
    {
        if (m_selection->pixmap ())
            slotContentsChanged (m_selection->boundingRect ());
        else
            vm->updateViews (m_selection->boundingRect ());
        delete m_selection;
    }

    m_selection = new kpSelection (selection);

    // TODO: this coupling to m_mainWindow is bad, careless and lazy
    if (!m_selection->isText ())
    {
        if (m_selection->transparency () != m_mainWindow->selectionTransparency ())
        {
            kDebug () << "kpDocument::setSelection() sel's transparency differs "
                          "from mainWindow's transparency - setting mainWindow's transparency "
                          "to sel"
                       << endl;
            kDebug () << "\tisOpaque: sel=" << m_selection->transparency ().isOpaque ()
                       << " mainWindow=" << m_mainWindow->selectionTransparency ().isOpaque ()
                       << endl;
            m_mainWindow->setSelectionTransparency (m_selection->transparency ());
        }
    }
    else
    {
        if (m_selection->textStyle () != m_mainWindow->textStyle ())
        {
            kDebug () << "kpDocument::setSelection() sel's textStyle differs "
                          "from mainWindow's textStyle - setting mainWindow's textStyle "
                          "to sel"
                       << endl;
            m_mainWindow->setTextStyle (m_selection->textStyle ());
        }
    }

#if DEBUG_KP_DOCUMENT && 0
    kDebug () << "\tcheck sel " << (int *) m_selection
               << " boundingRect=" << m_selection->boundingRect ()
               << endl;
#endif
    if (m_selection->pixmap ())
        slotContentsChanged (m_selection->boundingRect ());
    else
        vm->updateViews (m_selection->boundingRect ());

    connect (m_selection, SIGNAL (changed (const QRect &)),
             this, SLOT (slotContentsChanged (const QRect &)));


    if (!hadSelection)
        emit selectionEnabled (true);

    if (isTextChanged)
        emit selectionIsTextChanged (selection.type () == kpSelection::Text);

    if (vm)
        vm->restoreQueueUpdates ();
}

// public
QPixmap kpDocument::getSelectedPixmap () const
{
    kpSelection *sel = selection ();
    Q_ASSERT (sel);

    // easy if we already have it :)
    if (sel->pixmap ())
        return *sel->pixmap ();


    const QRect boundingRect = sel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());

    // TODO: This is very slow.  Image / More Effects ... calls us twice
    //       unnecessarily.
    return sel->givenImageMaskedByShape (getPixmapAt (boundingRect));
}

// public
bool kpDocument::selectionPullFromDocument (const kpColor &backgroundColor)
{
    kpViewManager *vm = m_mainWindow ? m_mainWindow->viewManager () : 0;

    kpSelection *sel = selection ();
    Q_ASSERT (sel);

    // Should not already have a pixmap or we would not be pulling.
    Q_ASSERT (!sel->pixmap ());

    const QRect boundingRect = sel->boundingRect ();
    Q_ASSERT (boundingRect.isValid ());


    //
    // Get selection pixmap from document
    //

    QPixmap selPixmap = getSelectedPixmap ();

    if (vm)
        vm->setQueueUpdates ();

    sel->setPixmap (selPixmap);


    //
    // Fill opaque bits of the hole in the document
    //

    // TODO: this assumes backgroundColor == sel->transparency ().transparentColor()
    const QPixmap selTransparentPixmap = sel->transparentPixmap ();

    if (backgroundColor.isOpaque ())
    {
        QPixmap erasePixmap (boundingRect.width (), boundingRect.height ());
        erasePixmap.fill (backgroundColor.toQColor ());

        if (!selTransparentPixmap.mask ().isNull ())
            erasePixmap.setMask (selTransparentPixmap.mask ());

        paintPixmapAt (erasePixmap, boundingRect.topLeft ());
    }
    else
    {
        kpPixmapFX::paintMaskTransparentWithBrush (m_pixmap,
                                                   boundingRect.topLeft (),
                                                   kpPixmapFX::getNonNullMask (selTransparentPixmap));
        slotContentsChanged (boundingRect);
    }

    if (vm)
        vm->restoreQueueUpdates ();

    return true;
}

// public
bool kpDocument::selectionDelete ()
{
    kpSelection *sel = selection ();

    if (!sel)
        return false;

    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
        return false;

    bool selectionHadPixmap = m_selection ? (bool) m_selection->pixmap () : false;

    delete m_selection;
    m_selection = 0;


    // HACK to prevent document from being modified when
    //      user cancels dragging out a new selection
    if (selectionHadPixmap)
        slotContentsChanged (boundingRect);
    else
        emit contentsChanged (boundingRect);

    emit selectionEnabled (false);


    return true;
}

// public
bool kpDocument::selectionCopyOntoDocument (bool useTransparentPixmap)
{
    kpSelection *sel = selection ();

    // must have a pixmap already
    if (!sel)
        return false;

    // hasn't actually been lifted yet
    if (!sel->pixmap ())
        return true;

    const QRect boundingRect = sel->boundingRect ();
    if (!boundingRect.isValid ())
        return false;

    if (!sel->isText ())
    {
        // TODO: why can't we just use paint()?
        paintPixmapAt (useTransparentPixmap ? sel->transparentPixmap () : sel->opaquePixmap (),
                       boundingRect.topLeft ());
    }
    else
    {
        // (for antialiasing with background)
        sel->paint (m_pixmap, rect ());
    }

    return true;
}

// public
bool kpDocument::selectionPushOntoDocument (bool useTransparentPixmap)
{
    return (selectionCopyOntoDocument (useTransparentPixmap) && selectionDelete ());
}

// public
QPixmap kpDocument::pixmapWithSelection () const
{
#if DEBUG_KP_DOCUMENT && 1
    kDebug () << "kpDocument::pixmapWithSelection()" << endl;
#endif

    // Have floating selection?
    if (m_selection && m_selection->pixmap ())
    {
    #if DEBUG_KP_DOCUMENT && 1
        kDebug () << "\tselection @ " << m_selection->boundingRect () << endl;
    #endif
        QPixmap output = *m_pixmap;

        kpPixmapFX::paintPixmapAt (&output,
                                   m_selection->topLeft (),
                                   m_selection->transparentPixmap ());
        return output;
    }
    else
    {
    #if DEBUG_KP_DOCUMENT && 1
        kDebug () << "\tno selection" << endl;
    #endif
        return *m_pixmap;
    }
}

