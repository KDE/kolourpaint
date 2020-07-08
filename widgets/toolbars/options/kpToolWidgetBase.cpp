
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


#define DEBUG_KP_TOOL_WIDGET_BASE 0


#include "kpToolWidgetBase.h"

#include "kpDefs.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include "kpLogCategories.h"

#include <QBitmap>
#include <QColor>
#include <QEvent>
#include <QHelpEvent>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QToolTip>


//---------------------------------------------------------------------

kpToolWidgetBase::kpToolWidgetBase (QWidget *parent, const QString &name)
    : QFrame(parent), m_baseWidget(nullptr),
      m_selectedRow(-1), m_selectedCol(-1)
{
    setObjectName (name);

    setFrameStyle (QFrame::Panel | QFrame::Sunken);

    setFixedSize (44, 66);
    setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
}

//---------------------------------------------------------------------

kpToolWidgetBase::~kpToolWidgetBase () = default;

//---------------------------------------------------------------------

// public
void kpToolWidgetBase::addOption (const QPixmap &pixmap, const QString &toolTip)
{
    if (m_pixmaps.isEmpty ()) {
        startNewOptionRow ();
    }

    m_pixmaps.last ().append (pixmap);
    m_pixmapRects.last ().append (QRect ());
    m_toolTips.last ().append (toolTip);
}

//---------------------------------------------------------------------

// public
void kpToolWidgetBase::startNewOptionRow ()
{
    m_pixmaps.append (QList <QPixmap> ());
    m_pixmapRects.append (QList <QRect> ());
    m_toolTips.append (QList <QString> ());
}

//---------------------------------------------------------------------

// public
void kpToolWidgetBase::finishConstruction (int fallBackRow, int fallBackCol)
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase(" << objectName ()
               << ")::kpToolWidgetBase(fallBack:row=" << fallBackRow
               << ",col=" << fallBackCol
               << ")";
#endif

    relayoutOptions ();

    // HACK: Undo the maximum half of setFixedSize() in the ctor to avoid
    //       bizarre redraw errors when tool widgets are hidden and others
    //       are shown.
    //
    //       The reason why we didn't just use setMinimumSize() in the ctor is
    //       because all tool widgets construct pixmaps whose sizes are dependent
    //       on the size() in the ctor, so we needed to get the correct size
    //       in there.  This is bad design because it means that tool widgets
    //       can't really be resized.
    setMaximumSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    const QPair <int, int> rowColPair = defaultSelectedRowAndCol ();
    if (!setSelected (rowColPair.first, rowColPair.second, false/*don't save*/))
    {
        if (!setSelected (fallBackRow, fallBackCol))
        {
            if (!setSelected (0, 0))
            {
                qCCritical(kpLogWidgets) << "kpToolWidgetBase::finishConstruction() "
                              "can't even fall back to setSelected(row=0,col=0)";
            }
        }
    }
}

//---------------------------------------------------------------------

// private
QList <int> kpToolWidgetBase::spreadOutElements (const QList <int> &sizes, int max)
{
    if (sizes.count () == 0) {
        return {};
    }

    if (sizes.count () == 1)
    {
        QList <int> ret;
        ret.append (sizes.first () > max ? 0 : 1/*margin*/);
        return ret;
    }

    QList <int> retOffsets;
    for (int i = 0; i < sizes.count (); i++) {
        retOffsets.append (0);
    }

    int totalSize = 0;
    for (int i = 0; i <  sizes.count (); i++) {
        totalSize += sizes [i];
    }

    int margin = 1;

    // if don't fit with margin, then just return elements
    // packed right next to each other
    if (totalSize + margin * 2 > max)
    {
        retOffsets [0] = 0;
        for (int i = 1; i < sizes.count (); i++) {
            retOffsets [i] = retOffsets [i - 1] + sizes [i - 1];
        }

        return retOffsets;
    }

    int maxLeftOver = max - (totalSize + margin * 2 * sizes.count());

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
    for (int i = 1; i < sizes.count (); i++)
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

//---------------------------------------------------------------------

// public
QPair <int, int> kpToolWidgetBase::defaultSelectedRowAndCol () const
{
    int row = -1, col = -1;

    if (!objectName ().isEmpty ())
    {
        KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupTools);

        row = cfg.readEntry (objectName () + QLatin1String (" Row"), -1);
        col = cfg.readEntry (objectName () + QLatin1String (" Col"), -1);
    }

#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase(" << objectName ()
               << ")::defaultSelectedRowAndCol() returning row=" << row
               << " col=" << col;
#endif

    return qMakePair (row, col);
}

//---------------------------------------------------------------------

// public
int kpToolWidgetBase::defaultSelectedRow () const
{
    return defaultSelectedRowAndCol ().first;
}

//---------------------------------------------------------------------

// public
int kpToolWidgetBase::defaultSelectedCol () const
{
    return defaultSelectedRowAndCol ().second;
}

//---------------------------------------------------------------------

// public
void kpToolWidgetBase::saveSelectedAsDefault () const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase(" << objectName ()
               << ")::saveSelectedAsDefault() row=" << m_selectedRow
               << " col=" << m_selectedCol;
#endif

    if (objectName ().isEmpty ()) {
        return;
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupTools);

    cfg.writeEntry (objectName () + QLatin1String (" Row"), m_selectedRow);
    cfg.writeEntry (objectName () + QLatin1String (" Col"), m_selectedCol);
    cfg.sync ();
}

//---------------------------------------------------------------------

// public
void kpToolWidgetBase::relayoutOptions ()
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase::relayoutOptions() size=" << size ();
#endif

    while (!m_pixmaps.isEmpty () && m_pixmaps.last ().count () == 0)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\tkilling #" << m_pixmaps.count () - 1;
    #endif
        m_pixmaps.removeLast ();
        m_pixmapRects.removeLast ();
        m_toolTips.removeLast ();
    }

    if (m_pixmaps.isEmpty ()) {
        return;
    }

#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "\tsurvived killing of empty rows";
    qCDebug(kpLogWidgets) << "\tfinding heights of rows:";
#endif

    QList <int> maxHeightOfRow;
    for (int r = 0; r < m_pixmaps.count (); r++) {
        maxHeightOfRow.append (0);
    }

    for (int r = 0; r <  m_pixmaps.count (); r++)
    {
        for (int c = 0; c <  m_pixmaps [r].count (); c++)
        {
            if (c == 0 || m_pixmaps [r][c].height () > maxHeightOfRow [r]) {
                maxHeightOfRow [r] = m_pixmaps [r][c].height ();
            }
        }
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\t\t" << r << ": " << maxHeightOfRow [r];
    #endif
    }

    QList <int> rowYOffset = spreadOutElements (maxHeightOfRow, height ());
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "\tspread out offsets of rows:";
    for (int r = 0; r < (int) rowYOffset.count (); r++) {
        qCDebug(kpLogWidgets) << "\t\t" << r << ": " << rowYOffset [r];
    }
#endif

    for (int r = 0; r < m_pixmaps.count (); r++)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\tlaying out row " << r << ":";
    #endif

        QList <int> widths;
        for (int c = 0; c < m_pixmaps [r].count (); c++)
            widths.append (m_pixmaps [r][c].width ());
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\t\twidths of cols:";
        for (int c = 0; c <  m_pixmaps [r].count (); c++) {
            qCDebug(kpLogWidgets) << "\t\t\t" << c << ": " << widths [c];
        }
    #endif

        QList <int> colXOffset = spreadOutElements (widths, width ());
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\t\tspread out offsets of cols:";
        for (int c = 0; c < colXOffset.count (); c++) {
            qCDebug(kpLogWidgets) << "\t\t\t" << c << ": " << colXOffset [c];
        }
    #endif

        for (int c = 0; c < colXOffset.count (); c++)
        {
            int x = colXOffset [c];
            int y = rowYOffset [r];
            int w, h;

            if (c == colXOffset.count () - 1)
            {
                if (x + m_pixmaps [r][c].width () >= width ()) {
                    w = m_pixmaps [r][c].width ();
                }
                else {
                    w = width () - 1 - x;
                }
            }
            else {
                w = colXOffset [c + 1] - x;
            }

            if (r == m_pixmaps.count () - 1)
            {
                if (y + m_pixmaps [r][c].height () >= height ()) {
                    h = m_pixmaps [r][c].height ();
                }
                else {
                    h = height () - 1 - y;
                }
            }
            else {
                h = rowYOffset [r + 1] - y;
            }

            m_pixmapRects [r][c] = QRect (x, y, w, h);
        }
    }

    update ();
}

//---------------------------------------------------------------------


// public
int kpToolWidgetBase::selectedRow () const
{
    return m_selectedRow;
}

//---------------------------------------------------------------------

// public
int kpToolWidgetBase::selectedCol () const
{
    return m_selectedCol;
}

//---------------------------------------------------------------------

// public
int kpToolWidgetBase::selected () const
{
    if (m_selectedRow < 0 ||
        m_selectedRow >= m_pixmaps.count () ||
        m_selectedCol < 0)
    {
        return -1;
    }

    int upto = 0;
    for (int y = 0; y < m_selectedRow; y++) {
        upto += m_pixmaps [y].count ();
    }

    if (m_selectedCol >= m_pixmaps [m_selectedRow].count ()) {
        return -1;
    }

    upto += m_selectedCol;

    return upto;
}

//---------------------------------------------------------------------


// public
bool kpToolWidgetBase::hasPreviousOption (int *row, int *col) const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase(" << objectName ()
               << ")::hasPreviousOption() current row=" << m_selectedRow
               << " col=" << m_selectedCol;
#endif
    if (row) {
        *row = -1;
    }
    if (col) {
        *col = -1;
    }


    if (m_selectedRow < 0 || m_selectedCol < 0) {
        return false;
    }

    int newRow = m_selectedRow,
        newCol = m_selectedCol;

    newCol--;
    if (newCol < 0)
    {
        newRow--;
        if (newRow < 0) {
            return false;
        }

        newCol = m_pixmaps [newRow].count () - 1;
        if (newCol < 0) {
            return false;
        }
    }


    if (row) {
        *row = newRow;
    }
    if (col) {
        *col = newCol;
    }

    return true;
}

//---------------------------------------------------------------------

// public
bool kpToolWidgetBase::hasNextOption (int *row, int *col) const
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase(" << objectName ()
               << ")::hasNextOption() current row=" << m_selectedRow
               << " col=" << m_selectedCol;
#endif

    if (row) {
        *row = -1;
    }
    if (col) {
        *col = -1;
    }


    if (m_selectedRow < 0 || m_selectedCol < 0) {
        return false;
    }

    int newRow = m_selectedRow,
        newCol = m_selectedCol;

    newCol++;
    if (newCol >= m_pixmaps [newRow].count ())
    {
        newRow++;
        if (newRow >= m_pixmaps.count ()) {
            return false;
        }

        newCol = 0;
        if (newCol >= m_pixmaps [newRow].count ()) {
            return false;
        }
    }


    if (row) {
        *row = newRow;
    }
    if (col) {
        *col = newCol;
    }

    return true;
}

//---------------------------------------------------------------------


// public slot virtual
bool kpToolWidgetBase::setSelected (int row, int col, bool saveAsDefault)
{
#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "kpToolWidgetBase::setSelected(row=" << row
               << ",col=" << col
               << ",saveAsDefault=" << saveAsDefault
               << ")";
#endif

    if (row < 0 || col < 0 ||
        row >= m_pixmapRects.count () || col >= m_pixmapRects [row].count ())
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\tout of range";
    #endif
        return false;
    }

    if (row == m_selectedRow && col == m_selectedCol)
    {
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "\tNOP";
    #endif

        if (saveAsDefault) {
            saveSelectedAsDefault ();
        }

        return true;
    }

    const int wasSelectedRow = m_selectedRow;
    const int wasSelectedCol = m_selectedCol;

    m_selectedRow = row;
    m_selectedCol = col;

    if (wasSelectedRow >= 0 && wasSelectedCol >= 0)
    {
        // unhighlight old option
        update (m_pixmapRects [wasSelectedRow][wasSelectedCol]);
    }

    // highlight new option
    update (m_pixmapRects [row][col]);

#if DEBUG_KP_TOOL_WIDGET_BASE
    qCDebug(kpLogWidgets) << "\tOK";
#endif

    if (saveAsDefault) {
        saveSelectedAsDefault ();
    }

    emit optionSelected (row, col);
    return true;
}

//---------------------------------------------------------------------

// public slot
bool kpToolWidgetBase::setSelected (int row, int col)
{
    return setSelected (row, col, true/*set as default*/);
}

//---------------------------------------------------------------------


// public slot
bool kpToolWidgetBase::selectPreviousOption ()
{
    int newRow, newCol;
    if (!hasPreviousOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

//---------------------------------------------------------------------

// public slot
bool kpToolWidgetBase::selectNextOption ()
{
    int newRow, newCol;
    if (!hasNextOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

//---------------------------------------------------------------------


// protected virtual [base QWidget]
bool kpToolWidgetBase::event (QEvent *e)
{
    // TODO: It's unclear when we should call the base, call accept() and
    //       return true or false.  Look at other event() handlers.  The
    //       kpToolText one is wrong since after calling accept(), it calls
    //       its base which calls ignore() :)
    if (e->type () == QEvent::ToolTip)
    {
        auto *he = dynamic_cast<QHelpEvent *> (e);
    #if DEBUG_KP_TOOL_WIDGET_BASE
        qCDebug(kpLogWidgets) << "kpToolWidgetBase::event() QHelpEvent pos=" << he->pos ();
    #endif

        bool showedText = false;
        for (int r = 0; r < m_pixmapRects.count (); r++)
        {
            for (int c = 0; c < m_pixmapRects [r].count (); c++)
            {
                if (m_pixmapRects [r][c].contains (he->pos ()))
                {
                    const QString tip = m_toolTips [r][c];
                #if DEBUG_KP_TOOL_WIDGET_BASE
                    qCDebug(kpLogWidgets) << "\tin option: r=" << r << "c=" << c
                              << "tip='" << tip << "'";
                #endif                
                    if (!tip.isEmpty ())
                    {
                        QToolTip::showText (he->globalPos (), tip, this);
                        showedText = true;
                    }

                    e->accept ();
                    goto exit_loops;
                }
            }
        }

    exit_loops:
        if (!showedText)
        {
        #if DEBUG_KP_TOOL_WIDGET_BASE
            qCDebug(kpLogWidgets) << "\thiding text";
        #endif
            QToolTip::hideText ();
        }

        return true;
    }

    return QWidget::event (e);
}

//---------------------------------------------------------------------


// protected virtual [base QWidget]
void kpToolWidgetBase::mousePressEvent (QMouseEvent *e)
{
    e->ignore ();

    if (e->button () != Qt::LeftButton) {
        return;
    }


    for (int i = 0; i < m_pixmapRects.count (); i++)
    {
        for (int j = 0; j < m_pixmapRects [i].count (); j++)
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

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpToolWidgetBase::paintEvent (QPaintEvent *e)
{
#if DEBUG_KP_TOOL_WIDGET_BASE && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetBase::paintEvent(): rect=" << contentsRect ();
#endif

    // Draw frame first.
    QFrame::paintEvent (e);

    QPainter painter (this);

    for (int i = 0; i < m_pixmaps.count (); i++)
    {
        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            qCDebug(kpLogWidgets) << "\tRow: " << i;
        #endif

        for (int j = 0; j < m_pixmaps [i].count (); j++)
        {
            QRect rect = m_pixmapRects [i][j];
            QPixmap pixmap = m_pixmaps [i][j];

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            qCDebug(kpLogWidgets) << "\t\tCol: " << j << " rect=" << rect;
        #endif

            if (i == m_selectedRow && j == m_selectedCol)
            {
                painter.fillRect(rect, palette().color(QPalette::Highlight).rgb());
            }

        #if DEBUG_KP_TOOL_WIDGET_BASE && 1
            qCDebug(kpLogWidgets) << "\t\t\tdraw pixmap @ x="
                       << rect.x () + (rect.width () - pixmap.width ()) / 2
                       << " y="
                       << rect.y () + (rect.height () - pixmap.height ()) / 2;

        #endif

            painter.drawPixmap(QPoint(rect.x () + (rect.width () - pixmap.width ()) / 2,
                                      rect.y () + (rect.height () - pixmap.height ()) / 2),
                               pixmap);
        }
    }
}

//---------------------------------------------------------------------

