
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

#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0


#include <kpcolorsimilaritycube.h>

#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpcolorsimilaritydialog.h>
#include <kpdefs.h>


const double kpColorSimilarityCube::colorCubeDiagonalDistance =
    sqrt (255 * 255 * 3);

kpColorSimilarityCube::kpColorSimilarityCube (int look,
                                              kpMainWindow *mainWindow,
                                              QWidget *parent,
                                              const char *name)
    : QFrame (parent, name, Qt::WNoAutoErase/*no flicker*/),
      m_mainWindow (mainWindow),
      m_colorSimilarity (-1)
{
    if (look & Depressed)
        setFrameStyle (QFrame::Panel | QFrame::Sunken);

    setColorSimilarity (0);


    // Don't cause the translators grief by appending strings
    // - duplicate text with 2 cases

    if (look & DoubleClickInstructions)
    {
        QWhatsThis::add (this,
            i18n ("<qt><p><b>Color Similarity</b> is how close "
                  "colors must be in the RGB Color Cube "
                  "to be considered the same.</p>"

                  "<p>If you set it to something "
                  "other than <b>Exact</b>, "
                  "you can work more effectively with dithered "
                  "images and photos.</p>"

                  "<p>This feature applies to transparent selections, as well as "
                  "the Flood Fill, Color Eraser and Autocrop "
                  "tools.</p>"

                  // sync: different to else case
                  "<p>To configure it, double click on the cube.</p>"

                  "</qt>"));
    }
    else
    {
        QWhatsThis::add (this,
            i18n ("<qt><p><b>Color Similarity</b> is how close "
                  "colors must be in the RGB Color Cube "
                  "to be considered the same.</p>"

                  "<p>If you set it to something "
                  "other than <b>Exact</b>, "
                  "you can work more effectively with dithered "
                  "images and photos.</p>"

                  "<p>This feature applies to transparent selections, as well as "
                  "the Flood Fill, Color Eraser and Autocrop "
                  "tools.</p>"

                  "</qt>"));
    }
}

kpColorSimilarityCube::~kpColorSimilarityCube ()
{
}


// public
double kpColorSimilarityCube::colorSimilarity () const
{
    return m_colorSimilarity;
}

// public
void kpColorSimilarityCube::setColorSimilarity (double similarity)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kdDebug () << "kpColorSimilarityCube::setColorSimilarity(" << similarity << ")" << endl;
#endif

    if (m_colorSimilarity == similarity)
        return;

    if (similarity < 0)
        similarity = 0;
    else if (similarity > kpColorSimilarityDialog::maximumColorSimilarity)
        similarity = kpColorSimilarityDialog::maximumColorSimilarity;

    m_colorSimilarity = similarity;

    repaint (false/*no erase*/);
}


// protected virtual [base QWidget]
QSize kpColorSimilarityCube::sizeHint () const
{
    return QSize (52, 52);
}


// protected
QColor kpColorSimilarityCube::color (int redOrGreenOrBlue,
                                     int baseBrightness,
                                     int similarityDirection) const
{
    int brightness = int (baseBrightness +
                          similarityDirection *
                              .5 * m_colorSimilarity * kpColorSimilarityCube::colorCubeDiagonalDistance);

    if (brightness < 0)
        brightness = 0;
    else if (brightness > 255)
        brightness = 255;

    switch (redOrGreenOrBlue)
    {
    default:
    case 0: return QColor (brightness, 0, 0);
    case 1: return QColor (0, brightness, 0);
    case 2: return QColor (0, 0, brightness);
    }
}

static QPoint pointBetween (const QPoint &p, const QPoint &q)
{
    return QPoint ((p.x () + q.x ()) / 2, (p.y () + q.y ()) / 2);
}

static void drawQuadrant (QPainter *p,
                          const QColor &col,
                          const QPoint &p1, const QPoint &p2, const QPoint &p3,
                          const QPoint pointNotOnOutline)
{
    p->save ();


    QPointArray points (4);
    points [0] = p1;
    points [1] = p2;
    points [2] = p3;
    points [3] = pointNotOnOutline;

    p->setPen (col);
    p->setBrush (col);
    p->drawPolygon (points);


    points.resize (3);

    p->setPen (Qt::black);
    p->setBrush (Qt::NoBrush);
    p->drawPolyline (points);


    p->restore ();
}

// protected
void kpColorSimilarityCube::drawFace (QPainter *p,
                                      int redOrGreenOrBlue,
                                      const QPoint &tl, const QPoint &tr,
                                      const QPoint &bl, const QPoint &br)
{
#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kdDebug () << "kpColorSimilarityCube(RorGorB=" << redOrGreenOrBlue
               << ",tl=" << tl
               << ",tr=" << tr
               << ",bl=" << bl
               << ",br=" << br
               << ")"
               << endl;
#endif

    //  tl --- tm --- tr
    //  |      |       |
    //  |      |       |
    //  ml --- mm --- mr
    //  |      |       |
    //  |      |       |
    //  bl --- bm --- br

    const QPoint tm (::pointBetween (tl, tr));
    const QPoint bm (::pointBetween (bl, br));

    const QPoint ml (::pointBetween (tl, bl));
    const QPoint mr (::pointBetween (tr, br));
    const QPoint mm (::pointBetween (ml, mr));


    const int baseBrightness = QMAX (127,
                                     255 - int (kpColorSimilarityDialog::maximumColorSimilarity *
                                                kpColorSimilarityCube::colorCubeDiagonalDistance / 2));
    QColor colors [2] =
    {
        color (redOrGreenOrBlue, baseBrightness, -1),
        color (redOrGreenOrBlue, baseBrightness, +1)
    };

    if (!isEnabled ())
    {
    #if DEBUG_KP_COLOR_SIMILARITY_CUBE
        kdDebug () << "\tnot enabled - making us grey" << endl;
    #endif
        colors [0] = colorGroup ().background ();
        colors [1] = colorGroup ().background ();
    }

#if DEBUG_KP_COLOR_SIMILARITY_CUBE
    kdDebug () << "\tmaxColorSimilarity=" << kpColorSimilarityDialog::maximumColorSimilarity
               << " colorCubeDiagDist=" << kpColorSimilarityCube::colorCubeDiagonalDistance
               << endl
               << "\tbaseBrightness=" << baseBrightness
               << " color[0]=" << ((colors [0].rgb () & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << " color[1]=" << ((colors [1].rgb () & RGB_MASK) >> ((2 - redOrGreenOrBlue) * 8))
               << endl;
#endif


    ::drawQuadrant (p, colors [0], tm, tl, ml, mm);
    ::drawQuadrant (p, colors [1], tm, tr, mr, mm);
    ::drawQuadrant (p, colors [1], ml, bl, bm, mm);
    ::drawQuadrant (p, colors [0], bm, br, mr, mm);
}

// protected virtual [base QFrame]
void kpColorSimilarityCube::drawContents (QPainter *p)
{
    QRect cr (contentsRect ());

    QPixmap backBuffer (cr.width (), cr.height ());
    backBuffer.fill (colorGroup ().background ());

    QPainter backBufferPainter (&backBuffer);

    int cubeRectSize = QMIN (cr.width () * 6 / 8, cr.height () * 6 / 8);
    int dx = (cr.width () - cubeRectSize) / 2,
        dy = (cr.height () - cubeRectSize) / 2;
    backBufferPainter.translate (dx, dy);

    //
    //      P------- Q  ---  ---
    //     /       / |   |    |
    //    /A      /  |  side  |
    //   R-------S   T  --- cubeRectSize
    //   |       |  /   /     |
    // S |       | /  side    |
    //   U-------V   ---     ---
    //   |-------|
    //      side
    //   |-----------|
    //    cubeRectSize
    //
    //

    const double angle = KP_DEGREES_TO_RADIANS (45);
    // S + S sin A = cubeRectSize
    // (1 + sin A) x S = cubeRectSize
    const double side = double (cubeRectSize) / (1 + sin (angle));


    const QPoint pointP ((int) (side * cos (angle)), 0);
    const QPoint pointQ ((int) (side * cos (angle) + side), 0);
    const QPoint pointR (0, (int) (side * sin (angle)));
    const QPoint pointS ((int) (side), (int) (side * sin (angle)));
    const QPoint pointU (0, (int) (side * sin (angle) + side));
    const QPoint pointT ((int) (side + side * cos (angle)), (int) (side));
    const QPoint pointV ((int) (side), (int) (side * sin (angle) + side));


    // Top Face
    drawFace (&backBufferPainter,
              0/*red*/,
              pointP, pointQ,
              pointR, pointS);


    // Bottom Face
    drawFace (&backBufferPainter,
              1/*green*/,
              pointR, pointS,
              pointU, pointV);


    // Right Face
    drawFace (&backBufferPainter,
              2/*blue*/,
              pointS, pointQ,
              pointV, pointT);


#if 0
    backBufferPainter.save ();
    backBufferPainter.setPen (Qt::cyan);
    backBufferPainter.drawRect (0, 0, cubeRectSize, cubeRectSize);
    backBufferPainter.restore ();
#endif


    backBufferPainter.end ();

    p->drawPixmap (cr, backBuffer);
}
