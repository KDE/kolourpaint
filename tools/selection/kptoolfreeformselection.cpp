
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_FREE_FROM_SELECTION 0


#include <kptoolfreeformselection.h>

#include <klocale.h>

#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>


kpToolFreeFormSelection::kpToolFreeFormSelection (kpMainWindow *mainWindow)
    : kpToolSelection (kpToolSelection::FreeForm,
                       i18n ("Selection (Free-Form)"),
                       i18n ("Makes a free-form selection"),
                       Qt::Key_M,
                       mainWindow, "tool_free_form_selection")
{
}

kpToolFreeFormSelection::~kpToolFreeFormSelection ()
{
}


// protected virtual [base kpToolSelection]
void kpToolFreeFormSelection::createMoreSelectionAndUpdateStatusBar (QPoint thisPoint,
        QRect /*normalizedRect*/)
{
    QPolygon points;

    if (document ()->selection ())
        points = document ()->selection ()->points ();


    // (not detached so will modify "points" directly but
    //  still need to call kpDocument::setSelection() to
    //  update screen)

    if (!m_dragHasBegun)
    {
        // We thought the drag at startPoint was a NOP
        // but it turns out that it wasn't...
        points.putPoints (points.count (), 1, m_startPoint.x (), m_startPoint.y ());
    }

    // TODO: there should be an upper limit on this before drawing the
    //       polygon becomes too slow
    points.putPoints (points.count (), 1, thisPoint.x (), thisPoint.y ());


    document ()->setSelection (kpSelection (points, mainWindow ()->selectionTransparency ()));
#if DEBUG_KP_TOOL_FREE_FROM_SELECTION && 1
    kDebug () << "\t\tfreeform; #points=" << document ()->selection ()->points ().count () << endl;
#endif

    setUserShapePoints (m_currentPoint);
}
