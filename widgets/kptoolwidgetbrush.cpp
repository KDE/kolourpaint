
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


#define DEBUG_KP_TOOL_WIDGET_BRUSH 0


#include <kptoolwidgetbrush.h>

#include <qbitmap.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdefs.h>


/* sync: <brushes> */
static int brushSize [][3] =
{
    {8, 4, 1/*like Pen*/},
    {9, 5, 2},
    {9, 5, 2},
    {9, 5, 2}
};

#define BRUSH_SIZE_NUM_COLS (int (sizeof (brushSize [0]) / sizeof (brushSize [0][0])))
#define BRUSH_SIZE_NUM_ROWS (int (sizeof (brushSize) / sizeof (brushSize [0])))

kpToolWidgetBrush::kpToolWidgetBrush (QWidget *parent, const char *name)
    : kpToolWidgetBase (parent, name)
{
    setInvertSelectedPixmap ();

    QPixmap *pm = m_brushBitmaps;
    
    for (int shape = 0; shape < BRUSH_SIZE_NUM_ROWS; shape++)
    {
        for (int i = 0; i < BRUSH_SIZE_NUM_COLS; i++)
        {
            int w = (width () - 2/*margin*/ - 2/*spacing*/) / BRUSH_SIZE_NUM_COLS;
            int h = (height () - 2/*margin*/ - 3/*spacing*/) / BRUSH_SIZE_NUM_ROWS;
            pm->resize ((w <= 0 ? width () : w),
                        (h <= 0 ? height () : h));

            const int s = brushSize [shape][i];
            QRect rect;
            
            if (s >= pm->width () || s >= pm->height ())
                rect = QRect (0, 0, pm->width (), pm->height ());
            else
            {
                rect = QRect ((pm->width () - s) / 2,
                              (pm->height () - s) / 2,
                              s,
                              s);
            }

        #if DEBUG_KP_TOOL_WIDGET_BRUSH
            kdDebug () << "kpToolWidgetBrush::kpToolWidgetBrush() rect=" << rect << endl;
        #endif

            pm->fill (Qt::white);
            
            QPainter painter (pm);
            painter.setPen (Qt::black);
            painter.setBrush (Qt::black);

            // sync: <brushes>
            switch (shape)
            {
            case 0:
                painter.drawEllipse (rect);
                break;
            case 1:
                painter.drawRect (rect);
                break;
            case 2:
                painter.drawLine (rect.topRight (), rect.bottomLeft ());
                break;
            case 3:
                painter.drawLine (rect.topLeft (), rect.bottomRight ());
                break;
            }
            painter.end ();

            pm->setMask (pm->createHeuristicMask ());
            addOption (*pm, brushName (shape, i)/*tooltip*/);

            pm++;
        }
        
        startNewOptionRow ();
    }

    finishConstruction (0, 0);
}

kpToolWidgetBrush::~kpToolWidgetBrush ()
{
}


// private
QString kpToolWidgetBrush::brushName (int shape, int whichSize)
{
    int s = brushSize [shape][whichSize];
    
    if (s == 1)
        return i18n ("1x1");
    
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
    
    if (shapeName.isEmpty ())
        return QString::null;
    
    return i18n ("%1x%2 %3").arg (s).arg (s).arg (shapeName);
}

QPixmap kpToolWidgetBrush::brush () const
{
    return m_brushBitmaps [selectedRow () * BRUSH_SIZE_NUM_COLS + selectedCol ()];
}

bool kpToolWidgetBrush::brushIsDiagonalLine () const
{
    // sync: <brushes>
    return (selectedRow () >= 2);
}

// virtual protected slot [base kpToolWidgetBase]
bool kpToolWidgetBrush::setSelected (int row, int col, bool saveAsDefault)
{
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret)
        emit brushChanged (brush (), brushIsDiagonalLine ());
    return ret;
}

#include <kptoolwidgetbrush.moc>
