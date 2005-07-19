
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


#define DEBUG_KP_TOOL_WIDGET_BASE 0


#include <kptoolwidgetbase.h>

#include <qbitmap.h>
#include <qcolor.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kpdefs.h>
#include <kpeffectinvert.h>


kpToolWidgetBase::kpToolWidgetBase (QWidget *parent, const char *name)
    : QFrame (parent, name),
      m_invertSelectedPixmap (true),
      m_selectedRow (-1), m_selectedCol (-1)
{
    if (!name)
        kdError () << "kpToolWidgetBase::kpToolWidgetBase() without name" << endl;

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
    m_pixmaps.resize (m_pixmaps.count () + 1);
    m_pixmapRects.resize (m_pixmapRects.count () + 1);
    m_toolTips.resize (m_toolTips.count () + 1);
}

// public
void kpToolWidgetBase::finishConstruction (int fallBackRow, int fallBackCol)
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kdDebug () << "kpToolWidgetBase(" << name ()
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
                kdError () << "kpToolWidgetBase::finishConstruction() "
                              "can't even fall back to setSelected(row=0,col=0)" << endl;
            }
        }
    }
}


// private
QValueVector <int> kpToolWidgetBase::spreadOutElements (const QValueVector <int> &sizes, int max)
{
    if (sizes.count () == 0)
        return QValueVector <int> ();
    else if (sizes.count () == 1)
        return QValueVector <int> (1, sizes.first () > max ? 0 : 1/*margin*/);

    QValueVector <int> retOffsets (sizes.count ());

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

    if (name ())
    {
        KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupTools);
        KConfigBase *cfg = cfgGroupSaver.config ();

        QString nameString = QString::fromLatin1 (name ());

        row = cfg->readNumEntry (nameString + QString::fromLatin1 (" Row"), -1);
        col = cfg->readNumEntry (nameString + QString::fromLatin1 (" Col"), -1);
    }

#if DEBUG_KP_TOOL_WIDGET_BASE
    kdDebug () << "kpToolWidgetBase(" << name ()
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
    kdDebug () << "kpToolWidgetBase(" << name ()
               << ")::saveSelectedAsDefault() row=" << m_selectedRow
               << " col=" << m_selectedCol << endl;
#endif

    if (!name ())
        return;

    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupTools);
    KConfigBase *cfg = cfgGroupSaver.config ();

    QString nameString = QString::fromLatin1 (name ());
    cfg->writeEntry (nameString + QString::fromLatin1 (" Row"), m_selectedRow);
    cfg->writeEntry (nameString + QString::fromLatin1 (" Col"), m_selectedCol);
    cfg->sync ();
}


// public
void kpToolWidgetBase::relayoutOptions ()
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    kdDebug () << "kpToolWidgetBase::relayoutOptions()" << endl;
#endif

    while (!m_pixmaps.isEmpty () && m_pixmaps.last ().count () == 0)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\tkilling #" << m_pixmaps.count () - 1 << endl;
    #endif
        m_pixmaps.resize (m_pixmaps.count () - 1);
        m_pixmapRects.resize (m_pixmapRects.count () - 1);
        m_toolTips.resize (m_toolTips.count () - 1);
    }

    if (m_pixmaps.isEmpty ())
        return;

#if DEBUG_KP_TOOL_WIDGET_BASE
    kdDebug () << "\tsurvived killing of empty rows" << endl;
    kdDebug () << "\tfinding heights of rows:" << endl;
#endif

    QValueVector <int> maxHeightOfRow (m_pixmaps.count ());

    for (int r = 0; r < (int) m_pixmaps.count (); r++)
    {
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
        {
            if (c == 0 || m_pixmaps [r][c].height () > maxHeightOfRow [r])
                maxHeightOfRow [r] = m_pixmaps [r][c].height ();
        }
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\t\t" << r << ": " << maxHeightOfRow [r] << endl;
    #endif
    }

    QValueVector <int> rowYOffset = spreadOutElements (maxHeightOfRow, height ());
#if DEBUG_KP_TOOL_WIDGET_BASE
    kdDebug () << "\tspread out offsets of rows:" << endl;
    for (int r = 0; r < (int) rowYOffset.count (); r++)
        kdDebug () << "\t\t" << r << ": " << rowYOffset [r] << endl;
#endif

    for (int r = 0; r < (int) m_pixmaps.count (); r++)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\tlaying out row " << r << ":" << endl;
    #endif

        QValueVector <int> widths (m_pixmaps [r].count ());
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
            widths [c] = m_pixmaps [r][c].width ();
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\t\twidths of cols:" << endl;
        for (int c = 0; c < (int) m_pixmaps [r].count (); c++)
            kdDebug () << "\t\t\t" << c << ": " << widths [c] << endl;
    #endif

        QValueVector <int> colXOffset = spreadOutElements (widths, width ());
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\t\tspread out offsets of cols:" << endl;
        for (int c = 0; c < (int) colXOffset.count (); c++)
            kdDebug () << "\t\t\t" << c << ": " << colXOffset [c] << endl;
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
                QToolTip::add (this, m_pixmapRects [r][c], m_toolTips [r][c]);
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
    kdDebug () << "kpToolWidgetBase" << name ()
               << "::hasPreviousOption() current row=" << m_selectedRow
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
    kdDebug () << "kpToolWidgetBase" << name ()
               << "::hasNextOption() current row=" << m_selectedRow
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
    kdDebug () << "kpToolWidgetBase::setSelected(row=" << row
               << ",col=" << col
               << ",saveAsDefault=" << saveAsDefault
               << ")"
               << endl;
#endif

    if (row < 0 || col < 0 ||
        row >= (int) m_pixmapRects.count () || col >= (int) m_pixmapRects [row].count ())
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\tout of range" << endl;
    #endif
        return false;
    }

    if (row == m_selectedRow && col == m_selectedCol)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        kdDebug () << "\tNOP" << endl;
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
    kdDebug () << "\tOK" << endl;
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

// protected virtual [base QFrame]
void kpToolWidgetBase::drawContents (QPainter *painter)
{
#if DEBUG_KP_TOOL_WIDGET_BASE && 1
    kdDebug () << "kpToolWidgetBase::drawContents(): rect=" << contentsRect () << endl;
#endif

    for (int i = 0; i < (int) m_pixmaps.count (); i++)
    {
        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kdDebug () << "\tRow: " << i << endl;
        #endif

        for (int j = 0; j < (int) m_pixmaps [i].count (); j++)
        {
            QRect rect = m_pixmapRects [i][j];
            QPixmap pixmap = m_pixmaps [i][j];

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kdDebug () << "\t\tCol: " << j << " rect=" << rect << endl;
        #endif

            if (i == m_selectedRow && j == m_selectedCol)
            {
                painter->fillRect (rect, Qt::blue/*selection color*/);

                if (m_invertSelectedPixmap)
                    kpEffectInvertCommand::apply (&pixmap);
            }

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            kdDebug () << "\t\t\tdraw pixmap @ x="
                       << rect.x () + (rect.width () - pixmap.width ()) / 2
                       << " y="
                       << rect.y () + (rect.height () - pixmap.height ()) / 2
                       << endl;

        #endif

            painter->drawPixmap (QPoint (rect.x () + (rect.width () - pixmap.width ()) / 2,
                                         rect.y () + (rect.height () - pixmap.height ()) / 2),
                                 pixmap);
        }
    }
}

#include <kptoolwidgetbase.moc>
