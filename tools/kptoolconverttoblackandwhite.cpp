
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


#define DEBUG_KP_TOOL_CONVERT_TO_BLACK_AND_WHITE 1

#include <qapplication.h>
#include <qpixmap.h>
#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptoolconverttoblackandwhite.h>


kpToolConvertToBlackAndWhiteCommand::kpToolConvertToBlackAndWhiteCommand (
    bool actOnSelection,
    kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_mainWindow (mainWindow),
      m_oldPixmapPtr (0)
{
}

// public virtual [base KCommand]
QString kpToolConvertToBlackAndWhiteCommand::name () const
{
    QString opName = i18n ("Convert to Black && White");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}

kpToolConvertToBlackAndWhiteCommand::~kpToolConvertToBlackAndWhiteCommand ()
{
    delete m_oldPixmapPtr;
}


// private
kpDocument *kpToolConvertToBlackAndWhiteCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolConvertToBlackAndWhiteCommand::execute ()
{
#if DEBUG_KP_TOOL_CONVERT_TO_BLACK_AND_WHITE && 0
    kdDebug () << "kpToolConvertToBlackAndWhiteCommand::execute()" << endl;
#endif
    kpDocument *doc = document ();
    if (!doc)
        return;


    QApplication::setOverrideCursor (Qt::waitCursor);

    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *doc->pixmap (m_actOnSelection);


    QPixmap newPixmap = kpPixmapFX::convertToBlackAndWhite (*m_oldPixmapPtr);

    doc->setPixmap (m_actOnSelection, newPixmap);

    QApplication::restoreOverrideCursor ();
}

// public virtual [base KCommand]
void kpToolConvertToBlackAndWhiteCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;


    doc->setPixmap (m_actOnSelection, *m_oldPixmapPtr);


    delete m_oldPixmapPtr;
    m_oldPixmapPtr = 0;
}
