
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


#include <kptoolcrop.h>

#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>
#include <kptoolresizescale.h>
#include <kptoolselection.h>


/*
 * kpToolCropSetImageCommand
 */

class kpToolCropSetImageCommand : public kpCommand
{
public:
    kpToolCropSetImageCommand (kpMainWindow *mainWindow);
    virtual ~kpToolCropSetImageCommand ();

    /* (uninteresting child of macro cmd) */
    virtual QString name () const { return QString::null; }

    virtual int size () const { return kpPixmapFX::pixmapSize (m_oldPixmap); }

    virtual void execute ();
    virtual void unexecute ();

protected:
    kpColor m_backgroundColor;
    QPixmap m_oldPixmap;
};


kpToolCropSetImageCommand::kpToolCropSetImageCommand (kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid)
{
}

kpToolCropSetImageCommand::~kpToolCropSetImageCommand ()
{
}


// public virtual [base kpCommand]
void kpToolCropSetImageCommand::execute ()
{
    m_oldPixmap = *document ()->pixmap ();


    QPixmap newDocPixmap (document ()->width (), document ()->height ());
    kpPixmapFX::fill (&newDocPixmap, m_backgroundColor);


    //
    // Original rounded rectangle selection:
    //
    //      T/---\      ...............
    //      | TT |      T = Transparent
    //      T\__/T      ...............
    //
    // After Crop Outside the Selection, the _image_ becomes:
    //
    //      Bbbbbb
    //      bbTTbb      T = Transparent
    //      BbbbbB      B,b = Background Colour
    //
    // The selection pixmap stays the same.
    //

    kpSelection *sel = document ()->selection ();
    // TODO: m_this assumes backgroundColor == sel->transparency ().transparentColor()
    const QPixmap selTransparentPixmap = sel->transparentPixmap ();

    if (selTransparentPixmap.mask ())
    {
        QBitmap docPixmapMask = *selTransparentPixmap.mask ();

        const QBitmap maskForSelType = sel->maskForOwnType (true/*null bitmap for rectangular*/);
        if (!maskForSelType.isNull ())
        {
            //     Dest             Src
            // docPixmapMask  maskForSelType = Result
            // --------------------------------------
            //      0                0           1
            //      0                1           0
            //      1                0           1
            //      1                1           1
            bitBlt (&docPixmapMask,
                    QPoint (0, 0),
                    &maskForSelType,
                    QRect (0, 0, maskForSelType.width (), maskForSelType.height ()),
                    Qt::NotOrROP);
        }

        newDocPixmap.setMask (docPixmapMask);
    }

    document ()->setPixmap (newDocPixmap);
}

// public virtual [base kpCommand]
void kpToolCropSetImageCommand::unexecute ()
{
    document ()->setPixmap (m_oldPixmap);
    m_oldPixmap.resize (0, 0);
}


/*
 * kpToolCropCommand
 */

kpToolCropCommand::kpToolCropCommand (kpMainWindow *mainWindow)
    : kpMacroCommand (i18n ("Set as Image"), mainWindow)
{
    if (!mainWindow ||
        !mainWindow->document () ||
        !mainWindow->document ()->selection ())
    {
        kdError () << "kpToolCropCommand::kpToolCropCommand() without sel" << endl;
        return;
    }

    kpSelection *sel = mainWindow->document ()->selection ();

    addCommand (
        new kpToolResizeScaleCommand (
            false/*act on doc, not sel*/,
            sel->width (), sel->height (),
            kpToolResizeScaleCommand::Resize,
            mainWindow));

    addCommand (new kpToolCropSetImageCommand (mainWindow));

    kpToolSelectionMoveCommand *moveCmd =
        new kpToolSelectionMoveCommand (
            QString::null/*uninteresting child of macro cmd*/,
            mainWindow);
    moveCmd->moveTo (QPoint (0, 0), true/*move on exec, not now*/);
    moveCmd->finalize ();
    addCommand (moveCmd);
}

kpToolCropCommand::~kpToolCropCommand ()
{
}
