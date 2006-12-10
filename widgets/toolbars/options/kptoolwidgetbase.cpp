
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


#define DEBUG_KP_TOOL_WIDGET_BASE 0


#include <kptoolwidgetbase.h>

#include <qbitmap.h>
#include <qcolor.h>
#include <qevent.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include <kpdefs.h>
#include <kpEffectInvert.h>


kpToolWidgetBase::kpToolWidgetBase (QWidget *parent, const QString &name)
    : QFrame (parent),
      m_invertSelectedPixmap (true),
      m_selectedRow (-1), m_selectedCol (-1)
{
    setObjectName (name);

    setFrameStyle (QFrame::Panel | QFrame::Sunken);
    setFixedSize (44, 66);
}

kpToolWidgetBase::~kpToolWidgetBase ()
{
}


// public
void kpToolWidgetBase::addOption (const QPixmap &pixmap, const QString &toolTip)
{
    if (m_pixmaps.isEmpty ())
        startNewOptionRow ();

    m_pixmaps.last ().append (pixmap);
    m_pixmapRects.last ().append (QRect ());
    m_toolTips.last ().append (toolTip);
}

// public
void kpToolWidgetBase::startNewOptionRow ()
{
    m_pixmaps.append (QList <QPixmap> ());
    m_pixmapRects.append (QList <QRect> ());
    m_toolTips.append (QList <QString> ());
}

// public
void kpToolWidgetBase::finishConstruction (int fallBackRow, int fallBackCol)
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase(" << objectName ()
               << ")::kpToolWidgetBase(fallBack:row=" << fallBackRow
               << ",col=" << fallBackCol
               << ")"
               << endl;
#endif

    relayoutOptions ();

    const QPair <int, int> rowColPair = defaultSelectedRowAndCol ();
    if (!setSelected (rowColPair.first, rowColPair.second, false/*don't save*/))
    {
        if (!setSelected (fallBackRow, fallBackCol))
        {
            if (!setSelected (0, 0))
            {
                kError () << "kpToolWidgetBase::finishConstruction() "
                              "can't even fall back to setSelected(row=0,col=0)" << endl;
            }
        }
    }
}


// private
QList <int> kpToolWidgetBase::spreadOutElements (const QList <int> &sizes, int max)
{
    if (sizes.count () == 0)
        return QList <int> ();
    else if (sizes.count () == 1)
    {
        QList <int> ret;
        ret.append (sizes.first () > max ? 0 : 1/*margin*/);
        return ret;
    }

    QList <int> retOffsets;
    for (int i = 0; i < sizes.count (); i++)
        retOffsets.append (0);

    int totalSize = 0;
    for (int i = 0; i < (int) sizes.count (); i++)
        totalSize += sizes [i];

    int margin = 1;

    // if don't fit with margin, then just return elements
    // packed right next to each other
    if (totalSize + margin * 2 > max)
    {
        retOffsets [0] = 0;
        for (int i = 1; i < (int) sizes.count (); i++)
            retOffsets [i] = retOffsets [i - 1] + sizes [i - 1];

        return retOffsets;
    }

    int maxLeftOver = max - (totalSize + margin * 2);

    int startCompensating = -1;
    int numCompensate = 0;

    int spacing = 0;

    spacing = maxLeftOver / (sizes.count () - 1);
    if (spacing * int (sizes.count () - 1) < maxLeftOver)
    {
        numCompensate = maxLeftOver - spacing * (sizes.count () - 1);
        startCompensating = ((sizes.count () - 1) - numCompensate) / 2;
    }

    retOffsets [0] = margin;
    for (int i = 1; i < (int) sizes.count (); i++)
    {
        retOffsets [i] += retOffsets [i - 1] +
                          sizes [i - 1] +
                          spacing +
                          ((numCompensate &&
                           i >= startCompensating &&
                           i < startCompensating + numCompensate) ? 1 : 0);
    }

    return retOffsets;
}


// public
QPair <int, int> kpToolWidgetBase::defaultSelectedRowAndCol () const
{
    int row = -1, col = -1;

    if (!objectName ().isEmpty ())
    {
        KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

        row = cfg.readEntry (objectName () + QLatin1String (" Row"), -1);
        col = cfg.readEntry (objectName () + QLatin1String (" Col"), -1);
    }

#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase(" << objectName ()
               << ")::defaultSelectedRowAndCol() returning row=" << row
               << " col=" << col
               << endl;
#endif

    return qMakePair (row, col);
}

// public
int kpToolWidgetBase::defaultSelectedRow () const
{
    return defaultSelectedRowAndCol ().first;
}

// public
int kpToolWidgetBase::defaultSelectedCol () const
{
    return defaultSelectedRowAndCol ().second;
}

// public
void kpToolWidgetBase::saveSelectedAsDefault () const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase(" << objectName ()
               << ")::saveSelectedAsDefault() row=" << m_selectedRow
               << " col=" << m_selectedCol << endl;
#endif

    if (objectName ().isEmpty ())
        return;

    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupTools);

    cfg.writeEntry (objectName () + QLatin1String (" Row"), m_selectedRow);
    cfg.writeEntry (objectName () + QLatin1String (" Col"), m_selectedCol);
    cfg.sync ();
}


// public
void kpToolWidgetBase::relayoutOptions ()
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase::relayoutOptions()" << endl;
#endif

    while (!m_pixmaps.isEmpty () && m_pixmaps.last ().count () == 0)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\tkilling #" << m_pixmaps.count () - 1 << endl;
    #endif
        m_pixmaps.removeLast ();
        m_pixmapRects.removeLast ();
        m_toolTips.removeLast ();
    }

    if (m_pixmaps.isEmpty ())
        return;

#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "\tsurvived killing of empty rows" << endl;
    kDebug () << "\tfinding heights of rows:" << endl;
#endif

    QList <int> maxHeightOfRow;
    for (int r = 0; r < m_pixmaps.count (); r++)
        maxHeightOfRow.append (0);

    for (int r = 0; r < (int) m_pixmaps.count (); r++)
    {
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
        {
            if (c == 0 || m_pixmaps [r][c].height () > maxHeightOfRow [r])
                maxHeightOfRow [r] = m_pixmaps [r][c].height ();
        }
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\t\t" << r << ": " << maxHeightOfRow [r] << endl;
    #endif
    }

    QList <int> rowYOffset = spreadOutElements (maxHeightOfRow, height ());
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "\tspread out offsets of rows:" << endl;
    for (int r = 0; r < (int) rowYOffset.count (); r++)
        kDebug () << "\t\t" << r << ": " << rowYOffset [r] << endl;
#endif

    for (int r = 0; r < (int) m_pixmaps.count (); r++)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\tlaying out row " << r << ":" << endl;
    #endif

        QList <int> widths;
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
            widths.append (m_pixmaps [r][c].width ());
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\t\twidths of cols:" << endl;
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
            kDebug () << "\t\t\t" << c << ": " << widths [c] << endl;
    #endif

        QList <int> colXOffset = spreadOutElements (widths, width ());
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\t\tspread out offsets of cols:" << endl;
        for (int c = 0; c < (int) colXOffset.count (); c++)
            kDebug () << "\t\t\t" << c << ": " << colXOffset [c] << endl;
    #endif

        for (int c = 0; c < (int) colXOffset.count (); c++)
        {
            int x = colXOffset [c];
            int y = rowYOffset [r];
            int w, h;

            if (c == (int) colXOffset.count () - 1)
            {
                if (x + m_pixmaps [r][c].width () >= width ())
                    w = m_pixmaps [r][c].width ();
                else
                    w = width () - 1 - x;
            }
            else
                w = colXOffset [c + 1] - x;

            if (r == (int) m_pixmaps.count () - 1)
            {
                if (y + m_pixmaps [r][c].height () >= height ())
                    h = m_pixmaps [r][c].height ();
                else
                    h = height () - 1 - y;
            }
            else
                h = rowYOffset [r + 1] - y;

            m_pixmapRects [r][c] = QRect (x, y, w, h);

            if (!m_toolTips [r][c].isEmpty ())
        		this->setToolTip(m_toolTips [r][c]);
		
		}
    }

    update ();
}


// public
int kpToolWidgetBase::selectedRow () const
{
    return m_selectedRow;
}

// public
int kpToolWidgetBase::selectedCol () const
{
    return m_selectedCol;
}

// public
int kpToolWidgetBase::selected () const
{
    if (m_selectedRow < 0 ||
        m_selectedRow >= (int) m_pixmaps.count () ||
        m_selectedCol < 0)
    {
        return -1;
    }

    int upto = 0;
    for (int y = 0; y < m_selectedRow; y++)
        upto += m_pixmaps [y].count ();

    if (m_selectedCol >= (int) m_pixmaps [m_selectedRow].count ())
        return -1;

    upto += m_selectedCol;

    return upto;
}


// public
bool kpToolWidgetBase::hasPreviousOption (int *row, int *col) const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase(" << objectName ()
               << ")::hasPreviousOption() current row=" << m_selectedRow
               << " col=" << m_selectedCol
               << endl;
#endif
    if (row)
        *row = -1;
    if (col)
        *col = -1;


    if (m_selectedRow < 0 || m_selectedCol < 0)
        return false;

    int newRow = m_selectedRow,
        newCol = m_selectedCol;

    newCol--;
    if (newCol < 0)
    {
        newRow--;
        if (newRow < 0)
            return false;

        newCol = m_pixmaps [newRow].count () - 1;
        if (newCol < 0)
            return false;
    }


    if (row)
        *row = newRow;
    if (col)
        *col = newCol;

    return true;
}

// public
bool kpToolWidgetBase::hasNextOption (int *row, int *col) const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase(" << objectName ()
               << ")::hasNextOption() current row=" << m_selectedRow
               << " col=" << m_selectedCol
               << endl;
#endif

    if (row)
        *row = -1;
    if (col)
        *col = -1;


    if (m_selectedRow < 0 || m_selectedCol < 0)
        return false;

    int newRow = m_selectedRow,
        newCol = m_selectedCol;

    newCol++;
    if (newCol >= (int) m_pixmaps [newRow].count ())
    {
        newRow++;
        if (newRow >= (int) m_pixmaps.count ())
            return false;

        newCol = 0;
        if (newCol >= (int) m_pixmaps [newRow].count ())
            return false;
    }


    if (row)
        *row = newRow;
    if (col)
        *col = newCol;

    return true;
}


// public slot virtual
bool kpToolWidgetBase::setSelected (int row, int col, bool saveAsDefault)
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "kpToolWidgetBase::setSelected(row=" << row
               << ",col=" << col
               << ",saveAsDefault=" << saveAsDefault
               << ")"
               << endl;
#endif

    if (row < 0 || col < 0 ||
        row >= (int) m_pixmapRects.count () || col >= (int) m_pixmapRects [row].count ())
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\tout of range" << endl;
    #endif
        return false;
    }

    if (row == m_selectedRow && col == m_selectedCol)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kDebug () << "\tNOP" << endl;
    #endif

        if (saveAsDefault)
            saveSelectedAsDefault ();

        return true;
    }

    const int wasSelectedRow = m_selectedRow;
    const int wasSelectedCol = m_selectedCol;

    m_selectedRow = row, m_selectedCol = col;

    if (wasSelectedRow >= 0 && wasSelectedCol >= 0)
    {
        // unhighlight old option
        update (m_pixmapRects [wasSelectedRow][wasSelectedCol]);
    }

    // highlight new option
    update (m_pixmapRects [row][col]);

#if DEBUG_KP_TOOL_WIDGET_BASE
    kDebug () << "\tOK" << endl;
#endif

    if (saveAsDefault)
        saveSelectedAsDefault ();

    emit optionSelected (row, col);
    return true;
}

// public slot
bool kpToolWidgetBase::setSelected (int row, int col)
{
    return setSelected (row, col, true/*set as default*/);
}


// public slot
bool kpToolWidgetBase::selectPreviousOption ()
{
    int newRow, newCol;
    if (!hasPreviousOption (&newRow, &newCol))
        return false;

    return setSelected (newRow, newCol);
}

// public slot
bool kpToolWidgetBase::selectNextOption ()
{
    int newRow, newCol;
    if (!hasNextOption (&newRow, &newCol))
        return false;

    return setSelected (newRow, newCol);
}


// protected virtual [base QWidget]
void kpToolWidgetBase::mousePressEvent (QMouseEvent *e)
{
    e->ignore ();

    if (e->button () != Qt::LeftButton)
        return;


    for (int i = 0; i < (int) m_pixmapRects.count (); i++)
    {
        for (int j = 0; j < (int) m_pixmapRects [i].count (); j++)
        {
            if (m_pixmapRects [i][j].contains (e->pos ()))
            {
                setSelected (i, j);
                e->accept ();
                return;
            }
        }
    }
}

// protected virtual [base QWidget]
void kpToolWidgetBase::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_TOOL_WIDGET_BASE && 1
    kDebug () << "kpToolWidgetBase::paintEvent(): rect=" << contentsRect () << endl;
#endif

    // Draw frame first.
    QFrame::paintEvent (e);


    QPainter painter (this);

    for (int i = 0; i < (int) m_pixmaps.count (); i++)
    {
        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kDebug () << "\tRow: " << i << endl;
        #endif

        for (int j = 0; j < (int) m_pixmaps [i].count (); j++)
        {
            QRect rect = m_pixmapRects [i][j];
            QPixmap pixmap = m_pixmaps [i][j];

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kDebug () << "\t\tCol: " << j << " rect=" << rect << endl;
        #endif

            if (i == m_selectedRow && j == m_selectedCol)
            {
                painter.fillRect (rect, Qt::blue/*selection color*/);

                if (m_invertSelectedPixmap)
                {
                    // TODO: Should use kpPixmapFX instead i.e. method on view content, not document content
                    kpEffectInvert::applyEffect (&pixmap);
                }
            }

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kDebug () << "\t\t\tdraw pixmap @ x="
                       << rect.x () + (rect.width () - pixmap.width ()) / 2
                       << " y="
                       << rect.y () + (rect.height () - pixmap.height ()) / 2
                       << endl;

        #endif

            painter.drawPixmap (QPoint (rect.x () + (rect.width () - pixmap.width ()) / 2,
                                        rect.y () + (rect.height () - pixmap.height ()) / 2),
                                pixmap);
        }
    }
}


#include <kptoolwidgetbase.moc>
