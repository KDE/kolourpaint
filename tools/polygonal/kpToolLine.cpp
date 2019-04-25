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


#define DEBUG_KP_TOOL_LINE 0


#include "kpToolLine.h"
#include "kpToolPolyline.h"
#include "kpLogCategories.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpToolLine::kpToolLine (kpToolEnvironment *environ, QObject *parent)
    : kpToolPolygonalBase (
        i18n ("Line"),
        i18n ("Draws lines"),
        &kpToolPolyline::drawShape,
        Qt::Key_L,
        environ, parent,
        QStringLiteral("tool_line"))
{
}

//--------------------------------------------------------------------------------

// private virtual [base kpToolPolygonalBase]
QString kpToolLine::haventBegunShapeUserMessage () const
{
    return i18n ("Drag to draw.");
}

//--------------------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolLine::endDraw (const QPoint &, const QRect &)
{
#if DEBUG_KP_TOOL_LINE
    qCDebug(kpLogTools) << "kpToolLine::endDraw()  points="
        << points ()->toList () << endl;
#endif

    // After the first drag, we should have a line.
    Q_ASSERT (points ()->count () == 2);
    endShape ();
}

//--------------------------------------------------------------------------------
