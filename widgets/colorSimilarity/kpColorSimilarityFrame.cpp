
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


#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0


#include <kpColorSimilarityFrame.h>

#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpColor.h>
#include <kpColorSimilarityCubeRenderer.h>
#include <kpDefs.h>


kpColorSimilarityFrame::kpColorSimilarityFrame (int look,
        QWidget *parent)
    : QFrame (parent),
      kpColorSimilarityHolder ()
{
    if (look & Depressed)
        setFrameStyle (QFrame::Panel | QFrame::Sunken);

    setWhatsThis (WhatsThis ());
}

kpColorSimilarityFrame::~kpColorSimilarityFrame ()
{
}


// public virtual [base kpColorSimilarityHolder]
void kpColorSimilarityFrame::setColorSimilarity (double similarity)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kDebug () << "kpColorSimilarityFrame::setColorSimilarity(" << similarity << ")";
#endif

    kpColorSimilarityHolder::setColorSimilarity (similarity);

    repaint ();
}


// protected virtual [base QWidget]
QSize kpColorSimilarityFrame::sizeHint () const
{
    return QSize (52, 52);
}


// protected virtual [base QWidget]
void kpColorSimilarityFrame::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kDebug () << "kpColorSimilarityFrame::paintEvent()"
              << " similarity=" << colorSimilarity () << endl;
#endif

    // Draw frame first.
    QFrame::paintEvent (e);


    int cubeRectSize = qMin (contentsRect ().width () * 6 / 8,
                             contentsRect ().height () * 6 / 8);
    int x = contentsRect ().x () + (contentsRect ().width () - cubeRectSize) / 2,
        y = contentsRect ().y () + (contentsRect ().height () - cubeRectSize) / 2;


    kpColorSimilarityCubeRenderer::WidgetPaint (this,
        x, y, cubeRectSize,
        colorSimilarity ());
}
