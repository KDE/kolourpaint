
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


#define DEBUG_KP_TOOL_POLYLINE 1


#include <kptoolpolyline.h>

#include <kdebug.h>
#include <klocale.h>


kpToolPolyline::kpToolPolyline (kpMainWindow *mainWindow)
    : kpToolPolygonalBase (
        Polyline,
        i18n ("Connected Lines"),
        i18n ("Draws connected lines"),
        Qt::Key_N,
        mainWindow,
        "tool_polyline")
{
}

kpToolPolyline::~kpToolPolyline ()
{
}


// private virtual [base kpToolPolygonalBase]
QString kpToolPolyline::haventBegunShapeUserMessage () const
{
    return i18n ("Drag to draw the first line.");
}


// public virtual [base kpTool]
void kpToolPolyline::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_POLYLINE
    kDebug () << "kpToolPolyline::endDraw()  points="
        << points ()->toList () << endl;
#endif

    // A click of the other mouse button (to finish shape, instead of adding
    // another control point) would have caused endShape() to have been
    // called in kpToolPolygonalBase::beginDraw().  The points list would now
    // be empty.
    if (points ()->count () == 0)
        return;

    if (points ()->count () >= kpToolPolygonalBase::MaxPoints)
    {
    #if DEBUG_KP_TOOL_POLYLINE
        kDebug () << "\tending shape" << endl;
    #endif
        endShape ();
        return;
    }
    
    if (m_mouseButton == 0)
    {
        setUserMessage (i18n ("Left drag another line or right click to finish."));
    }
    else
    {
        setUserMessage (i18n ("Right drag another line or left click to finish."));
    }
}


#include <kptoolpolyline.moc>
