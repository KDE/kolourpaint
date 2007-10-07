
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

#define DEBUG_KP_TOOL_CROP 0


#include <kptoolcrop.h>

#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptoolclear.h>
#include <kptoolresizescale.h>
#include <kptoolselection.h>
#include <kpviewmanager.h>


kpSelection selectionBorderAndMovedTo0_0 (const kpSelection &sel)
{
    kpSelection borderSel = sel;

    borderSel.setPixmap (QPixmap ());  // only interested in border
    borderSel.moveTo (QPoint (0, 0));

    return borderSel;
}


//
// kpToolCropSetImageCommand
//

class kpToolCropSetImageCommand : public kpCommand
{
public:
    kpToolCropSetImageCommand (kpMainWindow *mainWindow);
    virtual ~kpToolCropSetImageCommand ();

    /* (uninteresting child of macro cmd) */
    virtual QString name () const { return QString::null; }

    virtual int size () const
    {
        return kpPixmapFX::pixmapSize (m_oldPixmap) +
               kpPixmapFX::selectionSize (m_fromSelection) +
               kpPixmapFX::pixmapSize (m_pixmapIfFromSelectionDoesntHaveOne);
    }

    virtual void execute ();
    virtual void unexecute ();

protected:
    kpColor m_backgroundColor;
    QPixmap m_oldPixmap;
    kpSelection m_fromSelection;
    QPixmap m_pixmapIfFromSelectionDoesntHaveOne;
};


kpToolCropSetImageCommand::kpToolCropSetImageCommand (kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid),
      m_fromSelection (*mainWindow->document ()->selection ()),
      m_pixmapIfFromSelectionDoesntHaveOne (
        m_fromSelection.pixmap () ?
            QPixmap () :
            mainWindow->document ()->getSelectedPixmap ())
{
}

kpToolCropSetImageCommand::~kpToolCropSetImageCommand ()
{
}


// public virtual [base kpCommand]
void kpToolCropSetImageCommand::execute ()
{
#if DEBUG_KP_TOOL_CROP
    kdDebug () << "kpToolCropSetImageCommand::execute()" << endl;
#endif

    viewManager ()->setQueueUpdates ();
    {
        m_oldPixmap = kpPixmapFX::getPixmapAt (*document ()->pixmap (),
            QRect (0, 0, m_fromSelection.width (), m_fromSelection.height ()));


        //
        // e.g. original elliptical selection:
        //
        //     t/---\    T = original transparent selection pixel
        //     | TT |    t = outside the selection region
        //     t\__/t    [every other character] = original opaque selection pixel
        //
        // Afterwards, the _document_ image becomes:
        //
        //      b/---\   T = [unchanged]
        //      | TT |   b = background color
        //      b\__/b   [every other character] = [unchanged]
        //
        // The selection is deleted.
        //
        // TODO: Do not introduce a mask if the result will not contain
        //       any transparent pixels.
        //

        QPixmap newDocPixmap (m_fromSelection.width (), m_fromSelection.height ());
        kpPixmapFX::fill (&newDocPixmap, m_backgroundColor);

    #if DEBUG_KP_TOOL_CROP
        kdDebug () << "\tsel: rect=" << m_fromSelection.boundingRect ()
                   << " pm=" << m_fromSelection.pixmap ()
                   << endl;
    #endif
        QPixmap selTransparentPixmap;

        if (m_fromSelection.pixmap ())
        {
            selTransparentPixmap = m_fromSelection.transparentPixmap ();
        #if DEBUG_KP_TOOL_CROP
            kdDebug () << "\thave pixmap; rect="
                       << selTransparentPixmap.rect ()
                       << endl;
        #endif
        }
        else
        {
            selTransparentPixmap = m_pixmapIfFromSelectionDoesntHaveOne;
        #if DEBUG_KP_TOOL_CROP
            kdDebug () << "\tno pixmap in sel - get it; rect="
                       << selTransparentPixmap.rect ()
                       << endl;
        #endif
        }

        kpPixmapFX::paintMaskTransparentWithBrush (&newDocPixmap,
            QPoint (0, 0),
            m_fromSelection.maskForOwnType ());

        kpPixmapFX::paintPixmapAt (&newDocPixmap,
            QPoint (0, 0),
            selTransparentPixmap);


        document ()->setPixmapAt (newDocPixmap, QPoint (0, 0));
        document ()->selectionDelete ();


        if (mainWindow ()->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    viewManager ()->restoreQueueUpdates ();
}

// public virtual [base kpCommand]
void kpToolCropSetImageCommand::unexecute ()
{
#if DEBUG_KP_TOOL_CROP
    kdDebug () << "kpToolCropSetImageCommand::unexecute()" << endl;
#endif

    viewManager ()->setQueueUpdates ();
    {
        document ()->setPixmapAt (m_oldPixmap, QPoint (0, 0));
        m_oldPixmap.resize (0, 0);

    #if DEBUG_KP_TOOL_CROP
        kdDebug () << "\tsel: rect=" << m_fromSelection.boundingRect ()
                   << " pm=" << m_fromSelection.pixmap ()
                   << endl;
    #endif
        document ()->setSelection (m_fromSelection);

        if (mainWindow ()->tool ())
            m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
    }
    viewManager ()->restoreQueueUpdates ();
}


//
// kpToolCropCommand
//


class kpToolCropCommand : public kpMacroCommand
{
public:
    kpToolCropCommand (kpMainWindow *mainWindow);
    virtual ~kpToolCropCommand ();
};


kpToolCropCommand::kpToolCropCommand (kpMainWindow *mainWindow)
    : kpMacroCommand (i18n ("Set as Image"), mainWindow)
{
#if DEBUG_KP_TOOL_CROP
    kdDebug () << "kpToolCropCommand::<ctor>()" << endl;
#endif

    if (!mainWindow ||
        !mainWindow->document () ||
        !mainWindow->document ()->selection ())
    {
        kdError () << "kpToolCropCommand::kpToolCropCommand() without sel" << endl;
        return;
    }

    kpSelection *sel = mainWindow->document ()->selection ();


#if DEBUG_KP_TOOL_CROP
    kdDebug () << "\tsel: w=" << sel->width ()
               << " h=" << sel->height ()
               << " <- resizing doc to these dimen" << endl;
#endif

    // (must resize doc _before_ kpToolCropSetImageCommand in case doc
    //  needs to gets bigger - else pasted down pixmap may not fit)
    addCommand (
        new kpToolResizeScaleCommand (
            false/*act on doc, not sel*/,
            sel->width (), sel->height (),
            kpToolResizeScaleCommand::Resize,
            mainWindow));


    if (sel->isText ())
    {
    #if DEBUG_KP_TOOL_CROP
        kdDebug () << "\tisText" << endl;
        kdDebug () << "\tclearing doc with trans cmd" << endl;
    #endif
        addCommand (
            new kpToolClearCommand (
                false/*act on doc*/,
                kpColor::transparent,
                mainWindow));

    #if DEBUG_KP_TOOL_CROP
        kdDebug () << "\tmoving sel to (0,0) cmd" << endl;
    #endif
        kpToolSelectionMoveCommand *moveCmd =
            new kpToolSelectionMoveCommand (
                QString::null/*uninteresting child of macro cmd*/,
                mainWindow);
        moveCmd->moveTo (QPoint (0, 0), true/*move on exec, not now*/);
        moveCmd->finalize ();
        addCommand (moveCmd);
    }
    else
    {
    #if DEBUG_KP_TOOL_CROP
        kdDebug () << "\tis pixmap sel" << endl;
        kdDebug () << "\tcreating SetImage cmd" << endl;
    #endif
        addCommand (new kpToolCropSetImageCommand (mainWindow));

    #if 0
        addCommand (
            new kpToolSelectionCreateCommand (
                QString::null/*uninteresting child of macro cmd*/,
                selectionBorderAndMovedTo0_0 (*sel),
                mainWindow));
    #endif
    }
}

kpToolCropCommand::~kpToolCropCommand ()
{
}


void kpToolCrop (kpMainWindow *mainWindow)
{
    kpDocument *doc = mainWindow->document ();
    if (!doc)
        return;

    kpSelection *sel = doc ? doc->selection () : 0;
    if (!sel)
        return;


    bool selWasText = sel->isText ();
    kpSelection borderSel = selectionBorderAndMovedTo0_0 (*sel);


    mainWindow->addImageOrSelectionCommand (
        new kpToolCropCommand (mainWindow),
        true/*add create cmd*/,
        false/*don't add pull cmd*/);


    if (!selWasText)
    {
        mainWindow->commandHistory ()->addCommand (
            new kpToolSelectionCreateCommand (
                i18n ("Selection: Create"),
                borderSel,
                mainWindow));
    }
}
