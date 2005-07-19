
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


#include <kptoolclear.h>

#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>


kpToolClearCommand::kpToolClearCommand (bool actOnSelection,
                                        const kpColor &newColor,
                                        kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_newColor (newColor),
      m_oldPixmapPtr (0)
{
}

kpToolClearCommand::kpToolClearCommand (bool actOnSelection,
                                        kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_actOnSelection (actOnSelection),
      m_newColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid),
      m_oldPixmapPtr (0)
{
}

kpToolClearCommand::~kpToolClearCommand ()
{
    delete m_oldPixmapPtr;
}


// public virtual [base kpCommand]
QString kpToolClearCommand::name () const
{
    QString opName = i18n ("Clear");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}


// public virtual [base kpCommand]
int kpToolClearCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmapPtr);
}


// public virtual [base kpCommand]
void kpToolClearCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolClearCommand::execute() without doc" << endl;
        return;
    }


    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *doc->pixmap (m_actOnSelection);


    if (m_actOnSelection)
    {
        // OPT: could just edit pixmap directly and signal change
        kpSelection *sel = doc->selection ();

        QPixmap newPixmap (sel->width (), sel->height ());
        kpPixmapFX::fill (&newPixmap, m_newColor);
        // TODO: maybe disable Image/Clear if transparent colour
        if (m_newColor.isOpaque ())
            newPixmap.setMask (sel->maskForOwnType ());

        sel->setPixmap (newPixmap);
    }
    else
        doc->fill (m_newColor);
}

// public virtual [base kpCommand]
void kpToolClearCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolClearCommand::execute() without doc" << endl;
        return;
    }


    doc->setPixmap (m_actOnSelection, *m_oldPixmapPtr);


    delete m_oldPixmapPtr;
    m_oldPixmapPtr = 0;
}
