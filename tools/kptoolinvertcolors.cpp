
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kptoolinvertcolors.h>

#include <qapplication.h>

#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>


kpToolInvertColorsCommand::kpToolInvertColorsCommand (bool actOnSelection,
                                                      kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_mainWindow (mainWindow)
{
}

// public virtual [base KCommand]
QString kpToolInvertColorsCommand::name () const
{
    QString opName = i18n ("Invert Colors");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}

kpToolInvertColorsCommand::~kpToolInvertColorsCommand ()
{
}


// public virtual [base KCommand]
void kpToolInvertColorsCommand::execute ()
{
    invert ();
}

// public virtual [base KCommand]
void kpToolInvertColorsCommand::unexecute ()
{
    // symmetric operation
    invert ();
}


// private
void kpToolInvertColorsCommand::invert ()
{
    kpDocument *doc = m_mainWindow ? m_mainWindow->document () : 0;
    if (!doc)
        return;

    QApplication::setOverrideCursor (Qt::waitCursor);

    QPixmap newPixmap = kpPixmapFX::invertColors (*doc->pixmap (m_actOnSelection));

    doc->setPixmap (m_actOnSelection, newPixmap);

    QApplication::restoreOverrideCursor ();
}
