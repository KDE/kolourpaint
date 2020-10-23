
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


#define DEBUG_KP_TOOL_WIDGET_BRUSH 0


#include "widgets/toolbars/options/kpToolWidgetBrush.h"

#include <QPainter>

#include <KLocalizedString>

#include "kpLogCategories.h"
#include "kpDefs.h"

//---------------------------------------------------------------------

// LOREFACTOR: more OO, no arrays (use safer structs).
/* sync: <brushes> */
static int BrushSizes [][3] =
{
    {8, 4, 1/*like Pen*/},
    {9, 5, 2},
    {9, 5, 2},
    {9, 5, 2}
};

#define BRUSH_SIZE_NUM_COLS (int (sizeof (::BrushSizes [0]) / sizeof (::BrushSizes [0][0])))
#define BRUSH_SIZE_NUM_ROWS (int (sizeof (::BrushSizes) / sizeof (::BrushSizes [0])))


//---------------------------------------------------------------------

static void Draw (kpImage *destImage, const QPoint &topLeft, void *userData)
{
    auto *pack = static_cast <kpToolWidgetBrush::DrawPackage *> (userData);

#if DEBUG_KP_TOOL_WIDGET_BRUSH
    qCDebug(kpLogWidgets) << "kptoolwidgetbrush.cpp:Draw(destImage,topLeft="
              << topLeft << " pack: row=" << pack->row << " col=" << pack->col
              << " color=" << (int *) pack->color.toQRgb ();
#endif
    const int size = ::BrushSizes [pack->row][pack->col];
#if DEBUG_KP_TOOL_WIDGET_BRUSH
    qCDebug(kpLogWidgets) << "\tsize=" << size;
#endif

    QPainter painter(destImage);

    if ( size == 1 )
    {
      painter.setPen(pack->color.toQColor());
      painter.drawPoint(topLeft);
      return;
    }

    // sync: <brushes>
    switch (pack->row/*shape*/)
    {
      case 0:
      {
        // work around ugly circle when using QPainter on QImage
        if ( size == 4 )
        {
          // do not draw a pixel twice, as with an alpha color it will become darker
          painter.setPen(Qt::NoPen);
          painter.setBrush(pack->color.toQColor());
          painter.drawRect(topLeft.x() + 1, topLeft.y(), 2, size);
          painter.setPen(pack->color.toQColor());
          painter.drawLine(topLeft.x(), topLeft.y() + 1, topLeft.x(), topLeft.y() + 2);
          painter.drawLine(topLeft.x() + 3, topLeft.y() + 1, topLeft.x() + 3, topLeft.y() + 2);
        }
        else if ( size == 8 )  // size defined in BrushSizes above
        {
          // do not draw a pixel twice, as with an alpha color it will become darker
          painter.setPen(Qt::NoPen);
          painter.setBrush(pack->color.toQColor());
          painter.drawRect(topLeft.x() + 2, topLeft.y(), 4, size);
          painter.drawRect(topLeft.x(), topLeft.y() + 2, 2, 4);
          painter.drawRect(topLeft.x() + 6, topLeft.y() + 2, 2, 4);
          painter.setPen(pack->color.toQColor());
          painter.drawPoint(topLeft.x() + 1, topLeft.y() + 1);
          painter.drawPoint(topLeft.x() + 6, topLeft.y() + 1);
          painter.drawPoint(topLeft.x() + 1, topLeft.y() + 6);
          painter.drawPoint(topLeft.x() + 6, topLeft.y() + 6);
        }
        else
        {
          Q_ASSERT(!"illegal size");
        }
        break;
      }

      case 1:
      {
        // only paint filling so that a color with an alpha channel does not
        // create a darker border due to drawing some pixels twice with composition
        painter.setPen(Qt::NoPen);
        painter.setBrush(pack->color.toQColor());
        painter.drawRect(topLeft.x(), topLeft.y(), size, size);
        break;
      }

      case 2:
      {
        painter.setPen(pack->color.toQColor());
        painter.drawLine(topLeft.x() + size - 1, topLeft.y(),
                         topLeft.x(), topLeft.y() + size - 1);
        break;
      }

      case 3:
      {
        painter.setPen(pack->color.toQColor());
        painter.drawLine(topLeft.x(), topLeft.y(),
                         topLeft.x() + size - 1, topLeft.y() + size - 1);
        break;
      }

    default:
        Q_ASSERT (!"Unknown row");
        break;
    }
}

//---------------------------------------------------------------------

kpToolWidgetBrush::kpToolWidgetBrush (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
    for (int shape = 0; shape < BRUSH_SIZE_NUM_ROWS; shape++)
    {
        for (int i = 0; i < BRUSH_SIZE_NUM_COLS; i++)
        {
            const int s = ::BrushSizes [shape][i];


            const int w = (width () - 2/*margin*/ - 2/*spacing*/)
                / BRUSH_SIZE_NUM_COLS;
            const int h = (height () - 2/*margin*/ - 3/*spacing*/)
                / BRUSH_SIZE_NUM_ROWS;
            Q_ASSERT (w >= s && h >= s);
            QImage previewPixmap (w, h, QImage::Format_ARGB32_Premultiplied);
            previewPixmap.fill(0);

            DrawPackage pack = drawFunctionDataForRowCol (kpColor::Black, shape, i);
            ::Draw (&previewPixmap,
                QPoint ((previewPixmap.width () - s) / 2,
                        (previewPixmap.height () - s) / 2),
                &pack);


            addOption (QPixmap::fromImage(previewPixmap), brushName (shape, i)/*tooltip*/);
        }

        startNewOptionRow ();
    }

    finishConstruction (0, 0);
}

//---------------------------------------------------------------------

kpToolWidgetBrush::~kpToolWidgetBrush () = default;

//---------------------------------------------------------------------

// private
QString kpToolWidgetBrush::brushName (int shape, int whichSize) const
{
    int s = ::BrushSizes [shape][whichSize];

    if (s == 1) {
        return i18n ("1x1");
    }

    QString shapeName;

    // sync: <brushes>
    switch (shape)
    {
    case 0:
        shapeName = i18n ("Circle");
        break;
    case 1:
        shapeName = i18n ("Square");
        break;
    case 2:
        // TODO: is this really the name of a shape? :)
        shapeName = i18n ("Slash");
        break;
    case 3:
        // TODO: is this really the name of a shape? :)
        shapeName = i18n ("Backslash");
        break;
    }

    if (shapeName.isEmpty ()) {
        return {};
    }

    return i18n ("%1x%2 %3", s, s, shapeName);
}

//---------------------------------------------------------------------

// public
int kpToolWidgetBrush::brushSize () const
{
    return ::BrushSizes [selectedRow ()][selectedCol ()];
}

//---------------------------------------------------------------------

// public
bool kpToolWidgetBrush::brushIsDiagonalLine () const
{
    // sync: <brushes>
    return (selectedRow () >= 2);
}

//---------------------------------------------------------------------


// public
kpTempImage::UserFunctionType kpToolWidgetBrush::drawFunction () const
{
    return &::Draw;
}

//---------------------------------------------------------------------


// public static
kpToolWidgetBrush::DrawPackage kpToolWidgetBrush::drawFunctionDataForRowCol (
        const kpColor &color, int row, int col)
{
    Q_ASSERT (row >= 0 && col >= 0);

    DrawPackage pack;

    pack.row = row;
    pack.col = col;
    pack.color = color;

    return pack;
}

//---------------------------------------------------------------------

// public
kpToolWidgetBrush::DrawPackage kpToolWidgetBrush::drawFunctionData (
        const kpColor &color) const
{
    return drawFunctionDataForRowCol (color, selectedRow (), selectedCol ());
}

//---------------------------------------------------------------------

// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetBrush::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret) {
        emit brushChanged ();
    }
    return ret;
}

//---------------------------------------------------------------------

